/*
* Copyright (c) 2002-2010 Nokia Corporation and/or its subsidiary(-ies). 
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  API which handles certificate extra data related operations.
*
*/


#include "WimCertMgmt.h"
#include "WimMgmt.h"
#include "WimCert.h"
#include "WimTrace.h"
#include "Wimi.h"


// -----------------------------------------------------------------------------
// CWimCert::CWimCert()
// Default constructor
// -----------------------------------------------------------------------------
CWimCert::CWimCert() : CActive( EPriorityStandard )
    {
    }

// -----------------------------------------------------------------------------
// CWimCert::NewL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
EXPORT_C CWimCert* CWimCert::NewL()
    {
    _WIMTRACE ( _L( "CWimCert::NewL()" ) );
    CWimCert* self = new( ELeave ) CWimCert();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimCert::ConstructL()
// Second phase
// -----------------------------------------------------------------------------
//
void CWimCert::ConstructL()
    {
    CActiveScheduler::Add( this );
    _WIMTRACE ( _L( "CWimCert::ConstructL() completed" ) );
    }

// -----------------------------------------------------------------------------
// CWimCert::Initialize()
// Initializes the WIM server cache and restores certificates to
// WimClient memory area. This is the most time consuming call. 
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimCert::Initialize( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCert::Initialize()" ) );
    aStatus = KRequestPending;
    iClientStatus = &aStatus;
    if ( iClientSession )
        {
        iClientSession->Close();
        delete iClientSession;
        iClientSession = NULL;
        }
    if ( iConnectionHandle )
        {
        iConnectionHandle->Close();
        delete iConnectionHandle;
        iConnectionHandle = NULL;
        }
    iPhase = ECreateNewSession;
    SignalOwnStatusAndComplete();
    }

// -----------------------------------------------------------------------------
// CWimCert::CancelInitialize()
// Cancels the initialization process.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimCert::CancelInitialize()
    {
    _WIMTRACE ( _L( "CWimCert::CancelInitialize" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimCert::SignalOwnStatusAndComplete()       
// Sets own iStatus to KRequestPending, and signals it 
// with User::RequestComplete() -request. This gives chance 
// active scheduler to run other active objects. After a quick
// visit in active scheduler, signal returns to RunL() and starts next
// phase of operation. 
// -----------------------------------------------------------------------------
//
void CWimCert::SignalOwnStatusAndComplete()
    {
    _WIMTRACE ( _L( "CWimCert::SignalOwnStatusAndComplete()" ) );
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    }

// -----------------------------------------------------------------------------
// CWimCert::GetExtras()
// Returns the extra data from certificate that is 
// not possible yet to fetch from crypto token framework
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimCert::GetExtras( const TDesC8& aKeyId, 
                                   RCPointerArray<HBufC>& aTrustedUsages,
                                   TCertLocation& aLocation,
                                   TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCert::GetExtras()" ) );
    aStatus = KRequestPending;
    iClientStatus = &aStatus;

    if ( !iClientSession || !iConnectionHandle )
        {
        User::RequestComplete( iClientStatus, KErrNotReady );
        return;
        }
    if ( !iCertCount || aKeyId.Length() != KKeyIdLen )
        {
        User::RequestComplete( iClientStatus, KErrNotFound );
        }
    else
        {                
        if ( iKeyIdToBeMatched )
            {
            delete iKeyIdToBeMatched;
            iKeyIdToBeMatched = NULL;
            }

        TRAPD ( err, iKeyIdToBeMatched = aKeyId.AllocL() );
        if ( err )
            {
            User::RequestComplete( iClientStatus, KErrNoMemory );
            }
        else
            {
            iTrustedUsages = &aTrustedUsages;
            iLocation = &aLocation;
            iPhase = ELocateCertificate;
            SignalOwnStatusAndComplete();
            }
        }
    }

// -----------------------------------------------------------------------------
// CWimCert::CancelGetExtras()
// Cancels GetExtras -request.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimCert::CancelGetExtras()
    {
    _WIMTRACE ( _L( "CWimCert::CancelGetExtras" ) );
    Cancel();
    }
    
// -----------------------------------------------------------------------------
// CWimCert::~CWimCert()
// Allocated memory is released.
// -----------------------------------------------------------------------------
EXPORT_C CWimCert::~CWimCert()
    {
    _WIMTRACE ( _L( "CWimCert::~CWimCert()" ) );
    Cancel();
    if ( iConnectionHandle )
        {
        iConnectionHandle->Close();
        delete iConnectionHandle;
        }
    if ( iClientSession )
        {
        iClientSession->Close();
        delete iClientSession;
        }
    DeallocWimCertInfo();
    DeAllocMemoryForExtraDataRetrieve();
    DeallocRArrays();
    
    delete iKeyIdToBeMatched;
    delete iKeyIdPointer;

    iCertTypes.Close();
    iTrustedUsageLengths.Close();
    iKeyIds.Close();
    iUsages.Close();
    }

// -----------------------------------------------------------------------------
// CWimCert::RunL()
// Different phases are handled here. This might be considered as state machine. 
// iPhase = ECreateNewSession: Gets ClientSession handle. Tries to connect
//          to existing session. If there is no existing session, a new session 
//          is created.
// iPhase = EConnectClientSession: If new session was needed to be created, 
//          new connecting request is needed.
// iPhase = EInitializeWim: Sends initialize request to server and waits
//          asyncronously.
// iPhase = EGetCertClientSession:Create certificate clientsession and connect
//          it to WIM server.
// iPhase = EListCertsFromWim: List certificates from WimServer.
// iPhase = ELocateCertificate: Try to find matching certificate hash from array
//          (Application has sent a Key ID, which is used to indentify a 
//          certificate. If matching certificate has found, retrieve possible 
//          extra data from WimServer.
// iPhase = EGiveExtraData: Extra data is retrieved from WimServer. Chop it to 
//          correct pieces.
// -----------------------------------------------------------------------------
//
void CWimCert::RunL()
    {
    //Check for error
    if ( iStatus.Int() != KErrNone )
        {
        if ( iPhase == EGiveExtraData )
            {
            iConnectionHandle->DeallocGetExtrasPckgBuf();
            DeAllocMemoryForExtraDataRetrieve();
            }
        User::RequestComplete( iClientStatus, iStatus.Int() );
        _WIMTRACE2 ( _L( "CWimCert::RunL() error = %d" ), iStatus.Int() );
        _WIMTRACE2 ( _L( "CWimCert::RunL() phase = %d" ), iPhase );
        return;
        }

    switch ( iPhase )
        {
        case ECreateNewSession:
            { 
            //get ClientSession handle
            iClientSession = RWimMgmt::ClientSessionL();
            
            iPhase = EConnectClientSession;
            TInt retVal = 0;
            iWimStartErr = iClientSession->Connect();
            if ( iWimStartErr == KErrNotFound ) 
                {
                retVal = iClientSession->StartWim();
                if( retVal != KErrNone )
                    {
                    User::RequestComplete( iClientStatus, retVal ); 
                    }
                else
                    {
                    SignalOwnStatusAndComplete();
                    }
                }
            else
                {
                 SignalOwnStatusAndComplete(); 
                }
            break;
            }
        case EConnectClientSession:
            {
            //Raise iPhase
            iPhase = EInitializeWim; 
            if ( iWimStartErr )
                {
                //New session was needed to be created. Try to reconnect.
                iWimStartErr = iClientSession->Connect();
                if ( iWimStartErr )
                    {
                    //Failed to connect. No reason to continue
                    User::RequestComplete( iClientStatus, iWimStartErr );
                    }
                else
                    {
                    SignalOwnStatusAndComplete();
                    }
                }
            else
                {
                SignalOwnStatusAndComplete();
                }
            break;
            }
        case EInitializeWim:
            {
            //Initialize WIM
            SetActive();
            iClientSession->Initialize( iStatus );
            iPhase = EGetCertClientSession;
            break;   
            }
        case EGetCertClientSession:
            {
                // Create certificate clientsession and connect
                // it to WIM server.
            _WIMTRACE ( _L( "CWimCert::RunL(), \
                             case:EGetCertClientSession") );
            iConnectionHandle = RWimCertMgmt::ClientSessionL();
            iPhase = EListCertsFromWim;
            SignalOwnStatusAndComplete();
            break;   
            }
        case EListCertsFromWim: //get certificates from WIM.
            {
            _WIMTRACE ( _L( "CWimCert::RunL(), \
                             case:EListCertsFromWim") );
         
            if ( iCertCount ) //Delete previous preferences. 
                {
                DeallocWimCertInfo();
                DeallocRArrays();
                }
            //Ok ready to begin. First get Cert count
            iCertCount = iConnectionHandle->CertCount( EWimEntryTypeAll );
            _WIMTRACE2 ( _L( "CWimCert::RunL() certCount =%d" ), 
                             iCertCount );
            if ( !iCertCount )  
                {
                User::RequestComplete( iClientStatus, KErrNotFound );
                }
            else
                {
                TCertificateAddressList  certRefLst = 
                     new( ELeave ) TCertificateAddress[ iCertCount ];
                CleanupStack::PushL( TCleanupItem( CleanupRefLst, certRefLst ) );

                TWimCertInfo* certInfoArr = new( ELeave ) 
                                        TWimCertInfo[iCertCount];

                CleanupStack::PushL( TCleanupItem( Cleanup, certInfoArr ) );

                _WIMTRACE ( _L( "CWimCert::RunL(), Allocated certRefLst") );
               
                //Create arrays
                TUint8 index = 0;
                iLabel = new( ELeave ) PHBufC8[iCertCount];
                iKeyId = new( ELeave ) PHBufC8[iCertCount];
                iCAId  = new( ELeave ) PHBufC8[iCertCount];
                iIssuerHash = new( ELeave ) PHBufC8[iCertCount];

                iLabelPtr = new( ELeave ) PTPtr8[iCertCount];
                iKeyIdPtr = new( ELeave ) PTPtr8[iCertCount];
                iCAIdPtr  = new( ELeave ) PTPtr8[iCertCount];
                iIssuerHashPtr = new( ELeave ) PTPtr8[iCertCount];

                 _WIMTRACE ( _L( "CWimCert::RunL(), \
                 Allocated Member variable arrays") );


                for ( index = 0; index < iCertCount; index++ )
                    {
                    iLabel[index] = HBufC8::NewL( KLabelLen );
                    iKeyId[index] = HBufC8::NewL( KKeyIdLen );
                    iCAId[index]  = HBufC8::NewL( KPkcs15IdLen );
                    iIssuerHash[index] = HBufC8::NewL( KIssuerHash );
        
                    iLabelPtr[index] = new( ELeave ) 
                                            TPtr8( iLabel[index]->Des() );
                    iKeyIdPtr[index] = new( ELeave ) 
                                            TPtr8( iKeyId[index]->Des() );
                    iCAIdPtr[index]  = new( ELeave ) 
                                            TPtr8( iCAId[index]->Des() );
                    iIssuerHashPtr[index] = new( ELeave ) 
                                            TPtr8( iIssuerHash[index]->Des() );

                    certInfoArr[index].iLabel.Copy( iLabelPtr[index]->Ptr(),
                                                   iLabelPtr[index]->Length() );
                    certInfoArr[index].iKeyId.Copy( iKeyIdPtr[index]->Ptr(),
                                                   iKeyIdPtr[index]->Length() );
                    certInfoArr[index].iCAId.Copy( iCAIdPtr[index]->Ptr(),
                                                   iCAIdPtr[index]->Length() );
                    certInfoArr[index].iIssuerHash.Copy(
                        iIssuerHashPtr[index]->Ptr(),
                        iIssuerHashPtr[index]->Length() );
                    
                    _WIMTRACE2 ( _L( "CWimCert::RunL(), \
                    Assigned data to array, loopCount =%d"),index );

                    }
                _WIMTRACE ( _L( "CWimCert::RunL(), \
                Array ready for retrieving") );

                //Quick synchronous visit in server.
                TInt ret = iConnectionHandle->CertRefLst( certRefLst, 
                                               certInfoArr, 
                                               iCertCount, 
                                               EWimEntryTypeAll ); 
                 _WIMTRACE ( _L( "CWimCert::RunL(), \
                 Retrieve call issued") );

                if ( ret == KErrNone )
                    {
                    //Add necessary certificate information to API arrays
                    for ( TInt i = 0; i < iCertCount; i++ )
                        {
                        iCertTypes.Append( certInfoArr[i].iType );
                        iTrustedUsageLengths.Append( 
                                          certInfoArr[i].iTrustedUsageLength );
                        _WIMTRACE ( _L( "CWimCert::RunL(), \
                        Trusted usage appended") );

                        iKeyIds.Append( certInfoArr[i].iKeyId.AllocL() );

                        _WIMTRACE ( _L( "CWimCert::RunL(), KeyId appended") );

                        iUsages.Append( certInfoArr[i].iUsage );
                    
                        _WIMTRACE2 ( _L( "CWimCert::RunL(), \
                        Allocating data to RArrays, loopCount =%d"),i );
                        }

                    }
                //Clean up unwanted member variable arrays
                DeallocWimCertInfo(); 

                //Delete local arrays.
                CleanupStack::PopAndDestroy( 2 ); //certRefLst, certInfoArr

                 _WIMTRACE ( _L( "CWimCert::RunL(), All done") );
                User::RequestComplete( iClientStatus, iStatus.Int() );
                }
            break;
            }
        case ELocateCertificate:
            {
            _WIMTRACE ( _L( "CWimCert::RunL(), \
                        case:ELocateCertificate") );
             TInt retVal = CertificateLocated();
           
             delete iKeyIdToBeMatched;
             iKeyIdToBeMatched = NULL;
             
             if ( retVal == KErrNone )
                {
                //Allocate memory for Trusted usage string
                AllocMemoryForExtraDataRetrieveL();
                TCertExtrasInfo certExtrasInfo;
                certExtrasInfo.iTrustedUsage = 
                            iCertHBufTrustedUsageFromServerPtr;
                
                //Get reference to right keyId from Array, create pointer and 
                //get extra information
                HBufC8* keyId = iKeyIds.operator[]( iCertIndex );
                iKeyIdPointer = new( ELeave ) TPtr8( keyId->Des() );
                iConnectionHandle->GetCertExtrasL( 
                                            iKeyIdPointer,
                                            certExtrasInfo,
                                            iUsages.operator[]( iCertIndex ),
                                            iStatus );
                iPhase = EGiveExtraData;
                SetActive();
                }
             else
                {
                User::RequestComplete( iClientStatus, retVal );
                }
            break;
            }
        case EGiveExtraData:
            {
            _WIMTRACE ( _L( "CWimCert::RunL(), case:EGiveExtraData") );
            ExtraDataFoundL();
            delete iKeyIdPointer;
            iKeyIdPointer = NULL;
            User::RequestComplete( iClientStatus, iStatus.Int() );
            break;
            }
        default:
            {
            _WIMTRACE2 ( _L( "CWimCert::RunL(), \
                            case:default, iPhase=%d" ), iPhase );
            User::RequestComplete( iClientStatus, KErrCorrupt );
            break;
            }
        }
    }
    
// -----------------------------------------------------------------------------
// CWimCert::CertificateLocated()
// Compares received KeyId to our KeyIds and if match is found,
// check does that certificate have any extra data. If certificate is not found,
// return KErrNotFound. If TrustedUsage(Location) is not valid, return 
// KErrPathNotFound.
// -----------------------------------------------------------------------------
//
TInt CWimCert::CertificateLocated()
    {
    _WIMTRACE ( _L( "CWimCert::CertificateLocated" ) );
    TInt found = 0;
    TBool certificateMatch = EFalse;
    //Check, do we have that kind of certificate what user asks.
    TUint8 i;
    for ( i = 0; i < iCertCount && !certificateMatch; i++ )
        {
        //KeyId must match and certificate type must be X509
        if ( !iKeyIds.operator []( i )->Compare( *iKeyIdToBeMatched ) &&
             iCertTypes.operator[]( i ) == WIMI_CT_X509 )
            {
            iCertIndex = i;
            certificateMatch = ETrue;
            }
        }

    if ( certificateMatch ) //Certificate was found, check location validity
        {
        //Ok, we have found certificate, check does it have any 
        //extra data.
        if ( !iTrustedUsageLengths.operator []( iCertIndex ) )
            {
            found = KErrPathNotFound;
            }
        }
    else //Certificate was not found, return KErrNotFound
        {
        found = KErrNotFound;
        }
    return found;
    }

// -----------------------------------------------------------------------------
// CWimCert::AllocMemoryForExtraDataRetrieveL()
// Allocates memory for TrustedUsage String, which is fetched from server.
// -----------------------------------------------------------------------------
//
void CWimCert::AllocMemoryForExtraDataRetrieveL()
    {
    _WIMTRACE ( _L( "CWimCert::AllocMemoryForExtraDataRetrieveL" ) );
    //Trusted usage
    iCertHBufTrustedUsageFromServer = 
        HBufC::NewL( iTrustedUsageLengths.operator[]( iCertIndex ) );
    iCertHBufTrustedUsageFromServerPtr = new( ELeave ) TPtr( 
                                       iCertHBufTrustedUsageFromServer->Des() );
    }

// -----------------------------------------------------------------------------
// CWimCert::DeAllocMemoryForExtraDataRetrieve()
// DeAllocates memory from TrustedUsage String, which was fetched from server.
// -----------------------------------------------------------------------------
//
void CWimCert::DeAllocMemoryForExtraDataRetrieve()
    {
    _WIMTRACE ( _L( "CWimCert::DeAllocMemoryForExtraDataRetrieve" ) );
    delete iCertHBufTrustedUsageFromServer;
    iCertHBufTrustedUsageFromServer = NULL;

    delete iCertHBufTrustedUsageFromServerPtr;
    iCertHBufTrustedUsageFromServerPtr = NULL;
    }

// -----------------------------------------------------------------------------
// CWimCert::ExtraDataFoundL()
// Now we have got whole Trusted usage information in one buffer. We need 
// to chop it to corrects pieces and append it to iTrustedUsage array.
// Location is added to a member variable .
// -----------------------------------------------------------------------------
//
void CWimCert::ExtraDataFoundL()
    {
    _WIMTRACE ( _L( "CWimCert::ExtraDataFoundL" ) );
    TLex16 lex( *iCertHBufTrustedUsageFromServer );
    TPtrC16 lexToken;
    for ( TBool extrasDone = EFalse; extrasDone == EFalse; )
        {
        lexToken.Set( lex.NextToken() );
        if ( lexToken.Length() )
            {
            HBufC* oid = lexToken.AllocLC();
            User::LeaveIfError( iTrustedUsages->Append( oid ) );
            CleanupStack::Pop( oid ); //oid
            }
        else
            {
            extrasDone = ETrue;
            }
        }

    TPckgBuf<TCertExtrasInfo> certInfoPckg;
    
    certInfoPckg.operator = 
        ( *iConnectionHandle->CertExtrasPckgBuf()->PckgBuf() );

    switch ( certInfoPckg().iCDFRefs )
        {
        case EWimTrustedCertsCDF:
            {
            *iLocation = ETrustedCerts_CDF;
            break;
            }
        case EWimCertificatesCDF:
            {
            *iLocation = ECertificates_CDF;
            break;
            }
        case EWimUsefulCertsCDF:
            {
            *iLocation = EUsefulCerts_CDF;
            break;
            }
        default:
            {
            *iLocation = EUnknown_CDF;
            break;
            }
        }

    iConnectionHandle->DeallocGetExtrasPckgBuf();
    DeAllocMemoryForExtraDataRetrieve();
    }

// -----------------------------------------------------------------------------
// CWimCert::DoCancel()
// Deallocates member variables and completes client status with
// KErrCancel error code.
// -----------------------------------------------------------------------------
//
void CWimCert::DoCancel()
    {
    if ( iConnectionHandle && 
        ( iPhase == EGiveExtraData || iPhase == ELocateCertificate ) )
        {
        iConnectionHandle->DeallocGetExtrasPckgBuf();
        DeAllocMemoryForExtraDataRetrieve();
        delete iKeyIdPointer;
        iKeyIdPointer = NULL;
        delete iKeyIdToBeMatched;
        iKeyIdToBeMatched = NULL;
        }
    else 
        {//Cancelled initialization
        DeallocWimCertInfo();
        DeAllocMemoryForExtraDataRetrieve();
        DeallocRArrays();
        }
    User::RequestComplete( iClientStatus, KErrCancel );
    }

// -----------------------------------------------------------------------------
// CWimCert::RunError() 
// The active scheduler calls this function if this active object's RunL() 
// function leaves. This gives this active object the opportunity to perform 
// any necessary cleanup.
// After array's cleanup, complete request with received error code.
// -----------------------------------------------------------------------------
TInt CWimCert::RunError( TInt aError )
    {
    _WIMTRACE2 ( _L( "CWimCert::RunError, aError = %d" ), aError );
    _WIMTRACE2 ( _L( "CWimCert::RunError, iPhase = %d" ), iPhase );
    DeallocWimCertInfo();
    DeAllocMemoryForExtraDataRetrieve();
    DeallocRArrays();

    delete iKeyIdPointer;
    iKeyIdPointer = NULL;

    User::RequestComplete( iClientStatus, aError );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CWimCert::Cleanup()
// Handles cleanup for an object which is not derived from CBase
// -----------------------------------------------------------------------------
//
void CWimCert::Cleanup( TAny* aObject )
    {
    _WIMTRACE ( _L( "CWimCert::Cleanup()" ) );
    delete[] static_cast<TWimCertInfo*>(aObject);
    aObject = NULL;
    }

// -----------------------------------------------------------------------------
// CWimCert::CleanupRefLst()
// Handles cleanup of an TCertificateAddressList array
// -----------------------------------------------------------------------------
//
void CWimCert::CleanupRefLst( TAny* aObject )
    {
    _WIMTRACE ( _L( "CWimCert::CleanupRefLst()" ) );
    TCertificateAddressList* certRefLst = static_cast< TCertificateAddressList* >( aObject );
    delete[] certRefLst;
    aObject = NULL;
    }

// -----------------------------------------------------------------------------
// CWimCert::DeallocRArrays()
// Deallocates member variable RArrays
// -----------------------------------------------------------------------------
//
void CWimCert::DeallocRArrays()
    {
    for ( TInt i = 0; i< iKeyIds.Count(); i++ )
        {
        delete iKeyIds[i];
        iKeyIds[i] = NULL;
        }
    iKeyIds.Close();
    iCertTypes.Close();
    iTrustedUsageLengths.Close();
    iKeyIds.Close();
    }

// -----------------------------------------------------------------------------
// CWimCert::DeallocWimCertInfo()
// Deallocates memory. If something has leaved during asynchronous request, we 
// will deallocate all member data. 
// -----------------------------------------------------------------------------
void CWimCert::DeallocWimCertInfo()
    {
    _WIMTRACE ( _L( "CWimCert::DeallocWimCertInfo()" ) ); 
    TUint8 index;
    for ( index = 0; index < iCertCount; index++ )
        {
        if ( iLabel )
            {
            delete iLabel[index];
            }
        if ( iKeyId )
            {
            delete iKeyId[index];
            }
        if ( iCAId )
            {
            delete iCAId[index];
            }
        if ( iIssuerHash )
            {
            delete iIssuerHash[index];
            }
        if ( iLabelPtr )
            {
            delete iLabelPtr[index];
            }
        if ( iKeyIdPtr )
            {
            delete iKeyIdPtr[index];
            }
        if ( iCAIdPtr )
            {
            delete iCAIdPtr[index];
            }
        if ( iIssuerHashPtr )
            {
            delete iIssuerHashPtr[index];
            }
        }
    
    delete[] iLabel;   
    delete[] iKeyId;                
    delete[] iCAId;
    delete[] iIssuerHash; 
    delete[] iLabelPtr;        
    delete[] iKeyIdPtr;        
    delete[] iCAIdPtr;        
    delete[] iIssuerHashPtr;  
    iLabel = NULL;
    iKeyId  = NULL;
    iCAId = NULL;
    iIssuerHash = NULL;
    iLabelPtr = NULL;
    iKeyIdPtr = NULL;
    iCAIdPtr = NULL;
    iIssuerHashPtr = NULL;
    }

// End of File
