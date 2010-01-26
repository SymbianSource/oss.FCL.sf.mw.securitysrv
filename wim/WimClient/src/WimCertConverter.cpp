/*
* Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  API which handles certificate related operatons
*
*/



#include "WimCertMgmt.h"
#include "WimCertConverter.h"
#include "WimTrace.h"
#include "WimCertInfo.h"
#include "Wimi.h"   
#include <cctcertinfo.h>



// -----------------------------------------------------------------------------
// CWimCertConverter::CWimCertConverter()
// Default constructor
// -----------------------------------------------------------------------------
CWimCertConverter::CWimCertConverter( MCTToken& aToken ): 
CActive( EPriorityStandard ), iToken( aToken )
    {
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::NewL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
EXPORT_C CWimCertConverter* CWimCertConverter::NewL( MCTToken& aToken )
    {
    CWimCertConverter* self = new( ELeave ) CWimCertConverter( aToken );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self ); //self
    return self;
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::ConstructL()
// Second phase
// -----------------------------------------------------------------------------
void CWimCertConverter::ConstructL()
    {
    CActiveScheduler::Add( this );
    iConnectionHandle = RWimCertMgmt::ClientSessionL();           
    _WIMTRACE ( _L( "CWimCertConverter::ConstructL() completed" ) );
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::Restore()
// Returns CCTCertInfo objects in array
// -----------------------------------------------------------------------------
EXPORT_C void CWimCertConverter::Restore( RPointerArray<CWimCertInfo>& aArray, 
                                          TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertConverter::Restore" ) );
    iClientStatus = &aStatus;
    aStatus = KRequestPending;
    iArray = &aArray;
    iPhase = EListCertsFromWim;
    SignalOwnStatusAndComplete();
    }


// -----------------------------------------------------------------------------
// CWimCertConverter::CancelRestore()
// Cancels outgoing Restore operation. Sets an internal flag to true. 
// After necessary cleanup, caller is signalled with KErrCancel 
// -error code.
// -----------------------------------------------------------------------------
EXPORT_C void CWimCertConverter::CancelRestore()
    {
    _WIMTRACE ( _L( "CWimCertConverter::CancelRestore" ) );
    Cancel(); 
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::RetrieveCertByIndexL()
// Retrieves the actual certificate.
// -----------------------------------------------------------------------------

EXPORT_C void CWimCertConverter::RetrieveCertByIndexL( const TInt aIndex,
                                                    TDes8& aEncodedCert, 
                                                    TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertConverter::RetrieveCertByIndexL" ) );
    //Check that index is valid
    __ASSERT_ALWAYS( aIndex <= iArraySize && iCertRefLst,
        User::Leave( KErrArgument )  );
    iClientStatus = &aStatus;
    iCertRetrieveIndex = aIndex;
    iEncodedCert = &aEncodedCert;
    aStatus = KRequestPending;
    iPhase = ERetrieveCertificate;
    SignalOwnStatusAndComplete();
    }


// -----------------------------------------------------------------------------
// CWimCertConverter::CancelRetrieve()
// Cancels an ongoing Retrieve operation.
// If retrieve is cancelled, then operation is completed with 
// KErrCancel -error code.
// -----------------------------------------------------------------------------
EXPORT_C void CWimCertConverter::CancelRetrieve()
    {
    _WIMTRACE ( _L( "CWimCertConverter::CancelRetrieve" ) );
    Cancel();
    }


// -----------------------------------------------------------------------------
// CWimCertConverter::AddCertificate()
// Adds a certificate to the store.
// -----------------------------------------------------------------------------
//
EXPORT_C void CWimCertConverter::AddCertificate( const TDesC& aLabel, 
                              const TCertificateFormat aFormat, 
                              const TCertificateOwnerType aCertificateOwnerType,
                              const TKeyIdentifier& aSubjectKeyId,
                              const TKeyIdentifier& aIssuerKeyId,
                              const TDesC8& aCert,
                              TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertConverter::AddCertificate()" ) );        
    iClientStatus = &aStatus;
    aStatus = KRequestPending;
    TBool paramErr = EFalse;
    //First convert format 
    switch ( aFormat )
        {
        case EX509Certificate:
            {
            iParam.iFormat = EWimX509Certificate; 
            break;
            }
        case EWTLSCertificate:
            {
            iParam.iFormat = EWimWTLSCertificate;
            break;
            }
        case EX968Certificate:
            {
            paramErr = ETrue; 
            break;
            }
        case EUnknownCertificate:
            {
            paramErr = ETrue;
            break;
            }
        case EX509CertificateUrl:
            {
            iParam.iFormat = EWimCertificateURL;
            break;
            }
        case EWTLSCertificateUrl:
            {
            iParam.iFormat = EWimCertificateURL; 
            break;
            }
        case EX968CertificateUrl:
            {
            paramErr = ETrue; 
            break;
            }
        default:
            {
            paramErr = ETrue;
            break;
            }
            
        }
    
    //Then convert owner type    
    switch ( aCertificateOwnerType )
        {
        case ECACertificate:
            {
            iParam.iUsage = EWimCertTypeCA;
            break;
            }
        case EUserCertificate:
            {
            iParam.iUsage = EWimCertTypePersonal; 
            break;
            }
        case EPeerCertificate:
            {
            paramErr = ETrue;
            break;
            }   
        default:
            {
            paramErr = ETrue;
            break;
            }
            
        }
    if ( paramErr )
        {
        User::RequestComplete( iClientStatus, KErrNotSupported );
        return;
        }

    TRAPD( err, AllocMemoryForAddCertL( aLabel, 
                                        aIssuerKeyId, 
                                        aSubjectKeyId, 
                                        aCert ) );
    
    if ( err != KErrNone ) // is everything ok?
        {
        DeallocCertHBufs();         //No.
        _WIMTRACE ( _L( "CWimCertConverter::AddCertificate error = %d" ) );
        User::RequestComplete( iClientStatus, err );    
        }
    else
        {
        iPhase = EAddCertificate;
        SignalOwnStatusAndComplete(); //Yes.
        }
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::CancelAddCertificate()
// Cancels an ongoing certificate add operation.
// If add operation is cancelled, then operation is completed with 
// KErrCancel -error code.
// -----------------------------------------------------------------------------
EXPORT_C void CWimCertConverter::CancelAddCertificate()
    {
    _WIMTRACE ( _L( "CWimCertConverter::CancelAddCertificate()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::RemoveL()
// Removes a certificate from WIM
// -----------------------------------------------------------------------------
EXPORT_C void CWimCertConverter::RemoveL( const TInt aIndex, 
                                         TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimCertConverter::RemoveL()" ) );
    //Check that index is valid
    __ASSERT_ALWAYS( aIndex <= iArraySize && iCertRefLst, 
        User::Leave( KErrArgument )  );    
    iClientStatus = &aStatus;
    iCertRetrieveIndex = aIndex;
    aStatus = KRequestPending;
    iPhase = ERemove;
    SignalOwnStatusAndComplete();
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::CancelRemove()
// Cancels outgoing certificate remove operation. 
// -----------------------------------------------------------------------------
EXPORT_C void CWimCertConverter::CancelRemove()
    {
    _WIMTRACE ( _L( "CWimCertConverter::CancelRemove()" ) );
    Cancel();
    }


// -----------------------------------------------------------------------------
// CWimCertConverter::AllocMemoryForAddCertL()
// Allocates memory for member variables, which are needed when adding
// a certificate asynchronously to WIM.
// -----------------------------------------------------------------------------
void CWimCertConverter::AllocMemoryForAddCertL( const TDesC& aLabel, 
                                      const TKeyIdentifier& aIssuerKeyId, 
                                      const TKeyIdentifier& aSubjectKeyId, 
                                      const TDesC8& aCert )
    {
    _WIMTRACE ( _L( "CWimCertConverter::AllocMemoryForAddCertL()" ) );
    iCertHBufOne = aCert.AllocL(); //certificate
    iCertHBufOnePtr = new( ELeave ) TPtr8( iCertHBufOne->Des() );

    iCertHBufTwo = aIssuerKeyId.AllocL(); //IssuerKeyId
    iCertHBufTwoPtr = new( ELeave ) TPtr8( iCertHBufTwo->Des() );
    iParam.iCaId.Copy( iCertHBufTwoPtr->Ptr(), iCertHBufTwoPtr->Length() );

    iCertHBufThree = aSubjectKeyId.AllocL(); //SubjectKeyId
    iCertHBufThreePtr = new( ELeave ) TPtr8( iCertHBufThree->Des() );
    iParam.iKeyId.Copy( iCertHBufThreePtr->Ptr(), iCertHBufThreePtr->Length() );

    TBuf8<KLabelLen>buf;
    buf.Copy( aLabel );
    buf.ZeroTerminate();
    iCertHBufFour = buf.AllocL();           //label
    iCertHBufFourPtr = new( ELeave ) TPtr8( iCertHBufFour->Des() );
    iParam.iLabel.Copy( iCertHBufFourPtr->Ptr(), iCertHBufFourPtr->Length() );
    }



// -----------------------------------------------------------------------------
// CWimCertConverter::SignalOwnStatusAndComplete()       
// Sets own iStatus to KRequestPending, and signals it 
// with User::RequestComplete() -request. This gives chance 
// active scheduler to run other active objects. After a quick
// visit in active scheduler, signal returns to RunL() and starts next
// phase of operation. 
// @return void
// -----------------------------------------------------------------------------
void CWimCertConverter::SignalOwnStatusAndComplete()
    {
    _WIMTRACE ( _L( "CWimCertConverter::SignalOwnStatusAndComplete()" ) );
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    }
    
// -----------------------------------------------------------------------------
// CWimCertConverter::~CWimCertConverter()
// Allocated memory is released.
// -----------------------------------------------------------------------------
EXPORT_C CWimCertConverter::~CWimCertConverter()
    {
    _WIMTRACE ( _L( "CWimCertConverter::~CWimCertConverter()" ) );
    Cancel();
    if( iConnectionHandle )
        {
        iConnectionHandle->DeallocWimCertPckgBuf();
        iConnectionHandle->DeallocCertAddParametersPckgBuf();
        iConnectionHandle->DeallocRemoveCertPckgBuf();
        iConnectionHandle->DeallocGetExtrasPckgBuf();
        iConnectionHandle->Close();
        delete iConnectionHandle;
        }
    
    DeallocWimCertInfo();
    DeallocCertHBufs();
    DeallocReferences();
    delete iTrustedUsages;
    delete iTrustedUsagesPtr;
    delete iKeyIdBuf;
    delete iKeyIdPointer;

    }

// -----------------------------------------------------------------------------
// CWimCertConverter::RunL()
// This has eight different phases which are explained here:
// 1. EListCertsFromWim:Allocate member variables for arrays usage. 
//    Array is passed as an argument to server and it is filled with 
//    certificate information found from WIM.
// 2. ECreateNewCertObjects: New certificate objects are created based on 
//    received information on step one. A little bit of conversion is 
//    required. Objects are inserted to an array.
// 3. ERetrieveCertificate:Allocate member variables for struct, 
//    which is used to fetch certificate details
// 4. ERetrievingCompleted: Check cancellation. If not cancelled,
//    copy information to caller's buffer. 
// 5. EAddCertificate: Adds certificate to WIM.
// 6. EAddCertificateCompleted: Certificate is added successfully. Deallocate
//                              variables and complete request.
// 7. ERemove: Before removing, we will check for cancellation. After that
//    cancellation is too late.
// 8. ERemoveCompleted: Removal operation completed ok. Deallocte variables and
//    complete request
// 9. EGetCertExtras: Extra data is fetched and async. waiter needs to be 
//    stopped
// -----------------------------------------------------------------------------
void CWimCertConverter::RunL()
    {
    _WIMTRACE3( _L( "CWimCertConverter::RunL()|iPhase=%d, iStatus = %d" ), iPhase, iStatus.Int() );
    //Check for error
    if( iStatus.Int() != KErrNone )
        {
        //failed to retrieve certificate
        if ( iPhase == ERetrievingCompleted )
            {
             iConnectionHandle->DeallocWimCertPckgBuf();
             DeallocCertHBufs();
             _WIMTRACE2 ( _L( "CWimCertConverter::RunL() failed to retrieve \
                 certificate, ERROR = %d" ),iStatus.Int() );
            }
        //failed to add certificate
        else if ( iPhase == EAddCertificateCompleted )
            {
            //If user has tried to cancel at this stage, ignore flag.
            iConnectionHandle->DeallocCertAddParametersPckgBuf();
            DeallocCertHBufs();
             _WIMTRACE2 ( _L( "CWimCertConverter::RunL() failed to add \
                 certificate, ERROR = %d" ),iStatus.Int() );
            }
        //failed to remove certificate
        else if ( iPhase == ERemoveCompleted )
            {
            //If user has tried to cancel at this stage, ignore flag.
            iConnectionHandle->DeallocRemoveCertPckgBuf();
             _WIMTRACE2 ( _L( "CWimCertConverter::RunL() failed to remove \
                 certificate, ERROR = %d" ), iStatus.Int() );
            }
        User::RequestComplete( iClientStatus, iStatus.Int() );
        }
    else
        {
        switch( iPhase )
            {
            case EListCertsFromWim: //get certificates from WIM.
                {                  
                if ( iArraySize ) //Delete previous preferences. 
                    {           //User might added or removed a certificate. 
                    DeallocWimCertInfo();
                    DeallocReferences();
                    }
            
                //Ok ready to begin. First get Cert count
                iCertCount = iConnectionHandle->CertCount( EWimEntryTypeAll ); 
                _WIMTRACE2 ( _L( "CWimCertConverter::RunL() \
                    certCount =%d" ),iCertCount );
                if ( !iCertCount )  
                    {
                    User::RequestComplete( iClientStatus, KErrNotFound );
                    }
                else
                    {
                    iCertRefLst = new( ELeave ) TCertificateAddress[iCertCount];

                    //Creates new array according to certificate count
                    iCertInfoArr  = new( ELeave ) TWimCertInfo[iCertCount];
            
                    AllocWimCertInfoL( iCertInfoArr, iCertCount );
                    iStatus = KRequestPending;
                    iConnectionHandle->CertRefLst( iCertRefLst, 
                                          iCertInfoArr, 
                                          iCertCount, 
                                          EWimEntryTypeAll, 
                                          iStatus );
                    iPhase = ECreateNewCertObjects;
                    iIndex = 0;
                    SetActive();    
                    _WIMTRACE ( _L( "CWimCertConverter::RunL() \
                        GetCertRefLst" ) );
                    }
                break;
                }
            case ECreateNewCertObjects: //Certificates fetched, 
                { 
                CreateNewCertObjectsL();//create new cert-objects
                break;
                }
            case EGetTrustedUsages:
                {
                GetTrustedUsagesL();
                break;
                }
            case ETrustedUsagesDone:
                {
                TrustedUsagesDoneL();
                break;
                }
            case ECertObjectsDone: 
                {
                User::RequestComplete( iClientStatus, KErrNone );    
                _WIMTRACE ( _L( "CWimCertConverter::RunL() \
                    ECertObjectsDone" ) );
                break;
                }
            case ERetrieveCertificate: //Get Cert details from WIM
                {
                AllocMemoryForCertDetailsL();
                iConnectionHandle->CertDetailsL( 
                            iCertRefLst[iCertRetrieveIndex], 
                            iWimCertDetails, 
                            iStatus );
                iPhase = ERetrievingCompleted;
                SetActive();
                _WIMTRACE ( _L( "CWimCertConverter::RunL() \
                    ERetrieveCertificate" ) );
                break;
                }
            case ERetrievingCompleted: //Cert details fetched, check errors &                         
                {                      //cancellations & complete request                   
                //copy cert details to iEncodedCert -buffer,
                //which points to received aEncoded -cert buffer.
                CopyRetrievedCertData();
                iConnectionHandle->DeallocWimCertPckgBuf();                
                DeallocCertHBufs();
                User::RequestComplete( iClientStatus, KErrNone );
                _WIMTRACE ( _L( "CWimCertConverter::RunL() \
                    ERetrievingCompleted" ) );
                break;
                }
            case EAddCertificate: //Add a certificate
                {
                iConnectionHandle->StoreCertificateL( *iCertHBufOnePtr, 
                                                iParam, 
                                                EWimCertLocationWIMCard, 
                                                iStatus );  
                iPhase = EAddCertificateCompleted;
                SetActive();   
                _WIMTRACE ( _L( "CWimCertConverter::RunL() \
                    EAddCertificate" ) );                    
                break;
                }
            case EAddCertificateCompleted: //certificate added 
                {
                iConnectionHandle->DeallocCertAddParametersPckgBuf();
                DeallocCertHBufs();
                User::RequestComplete( iClientStatus, KErrNone );
                _WIMTRACE ( _L( "CWimCertConverter::RunL() \
                        EAddCertificateCompleted" ) );                    
                break;
                }
            case ERemove:                //remove certificate
                {
                iWimCertRemoveAddr.iCertAddr = 
                                iCertRefLst[iCertRetrieveIndex];
                iWimCertRemoveAddr.iLocation = EWimCertLocationWIMCard;
                iConnectionHandle->RemoveL( iWimCertRemoveAddr, iStatus );
                iPhase = ERemoveCompleted;
                SetActive();
                _WIMTRACE ( _L( "CWimCertConverter::RunL() \
                    ERemove" ) );                    

                break;
                }  
            case ERemoveCompleted:      //certificate removed. 
                {
                iConnectionHandle->DeallocRemoveCertPckgBuf();
                User::RequestComplete( iClientStatus, KErrNone );
                _WIMTRACE ( _L( "CWimCertConverter::RunL() \
                        ERemoveCompleted" ) ); 
                break;
                }
            default:
                {
                User::RequestComplete( iClientStatus, KErrNotSupported );
                break;
                }
                
            }
        }
    }


// -----------------------------------------------------------------------------
// CWimCertConverter::CreateNewCertObjectsL()
// Creates new certificate objects which can be returned to the caller.
// -----------------------------------------------------------------------------
void CWimCertConverter::CreateNewCertObjectsL()
    {
    _WIMTRACE ( _L( "CWimCertConverter::CreateNewCertObjectsL()" ) );

    HBufC16* label16 = NULL;
    HBufC8* hash8 = NULL;

    if ( iIndex < iCertCount )
        {
        TCertificateOwnerType certificateType;
        if ( iCertInfoArr[iIndex].iUsage == 0 )
            {
            certificateType = EUserCertificate; // 0 == User
            }
        else
            {
            certificateType = ECACertificate;   // 1 == CA
            }
                        
        label16 = HBufC16::NewLC( iCertInfoArr[iIndex].iLabel.Length() );
        label16->Des().Copy( iCertInfoArr[iIndex].iLabel );
            
        hash8 = HBufC8::NewLC( iCertInfoArr[iIndex].iIssuerHash.Length() );
        hash8->Des().Copy( iCertInfoArr[iIndex].iIssuerHash );
        // Convert type 
        TCertificateFormat format;
        format = GetCertFormatByIndex( iIndex );
        //Create key indentifiers
        TKeyIdentifier subjectKeyId;
        TKeyIdentifier issuerKeyId;
        //needs these for CCTCertInfo object -creation
        subjectKeyId = iCertInfoArr[iIndex].iKeyId;
        issuerKeyId  = iCertInfoArr[iIndex].iCAId;
        //Create CCTCertInfo object. 
        if ( iCertInfoArr[iIndex].iIssuerHash.Length() > 0 )
            {
            /**
            * Certificate modification is not supported in 3.0, 
            * the iCertInfoArr[iIndex].iModifiable will be replaced 
            * by EFalse. 
            * In the future, when modification is supported, it will be 
            * change back.
            */
            iCertInfoArr[iIndex].iModifiable = EFalse;
            iCert = CCTCertInfo::NewL( label16->Des(), 
                                      format, 
                                      certificateType,
                                      iCertInfoArr[iIndex].iCertlen, 
                                      &subjectKeyId, 
                                      &issuerKeyId, 
                                      iToken, 
                                      iIndex,
                                      iCertInfoArr[iIndex].iModifiable,
                                      hash8 );
            }
        else
            {
            /**
            * Same reason as mentioned above
            */
            iCertInfoArr[iIndex].iModifiable = EFalse;
            iCert = CCTCertInfo::NewL( label16->Des(), 
                                      format, 
                                      certificateType,
                                      iCertInfoArr[iIndex].iCertlen, 
                                      &subjectKeyId, 
                                      &issuerKeyId, 
                                      iToken, 
                                      iIndex,
                                      iCertInfoArr[iIndex].iModifiable );
            }

        CleanupStack::PopAndDestroy( hash8 ); 
        CleanupStack::PopAndDestroy( label16 );

        iOids = new( ELeave ) RArray<HBufC*>();
        //Check whether certificate has extra data. Certificate type must be
        // X509
        if ( iCertInfoArr[iIndex].iCDFRefs &&
             iCertInfoArr[iIndex].iType == WIMI_CT_X509 )
            {  
            iPhase = EGetTrustedUsages;
            }
        else
            {
            CreateNewWimCertObjectL();
            }
        }
    else
        {
        DeallocWimCertInfo(); 
        iPhase = ECertObjectsDone;
        }
    SignalOwnStatusAndComplete();
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::CreateNewWimCertObjectL()
// 
// -----------------------------------------------------------------------------
void CWimCertConverter::CreateNewWimCertObjectL()
    {
    _WIMTRACE ( _L( "CWimCertConverter::CreateNewWimCertObjectL()" ) );

    iCertInfo = CWimCertInfo::NewL( iCert, //CCTCertInfo object
                                    iCert->SubjectKeyId(), //key hash
                                    *iOids, 
                                    iCertInfoArr[iIndex].iCDFRefs );
    
    delete iOids;
    iOids = NULL;

    //Append WimCertInfo -object to array. Ownership to iArray here.
    User::LeaveIfError( iArray->Append( iCertInfo ) );

    iPhase = ECreateNewCertObjects;
    iIndex++;
    }
        
// -----------------------------------------------------------------------------
// CWimCertConverter::GetTrustedUsagesL
// Get trusted usages (OIDs) of a current certificate, which is pointed out 
// by a index
// -----------------------------------------------------------------------------
void CWimCertConverter::GetTrustedUsagesL()
    {
    _WIMTRACE ( _L( "CWimCertConverter::GetTrustedUsagesL() | Begin" ) );

    delete iTrustedUsages;
    iTrustedUsages = NULL;
    delete iTrustedUsagesPtr;
    iTrustedUsagesPtr = NULL;
    //Allocate variables for trusted usage fetching
    iTrustedUsages = HBufC::NewL( iCertInfoArr[iIndex].iTrustedUsageLength );
    iTrustedUsagesPtr = new( ELeave ) TPtr( iTrustedUsages->Des() );

    iCertExtrasInfo.iTrustedUsage = iTrustedUsagesPtr;
    iPhase = ETrustedUsagesDone;

    delete iKeyIdBuf;
    iKeyIdBuf = NULL;
    delete iKeyIdPointer;
    iKeyIdPointer = NULL;
    // Take a copy of key identifier
    iKeyIdBuf = iCertInfoArr[iIndex].iKeyId.AllocL();
    iKeyIdPointer = new( ELeave ) TPtr8( iKeyIdBuf->Des() );

    iConnectionHandle->GetCertExtrasL( iKeyIdPointer,
                                       iCertExtrasInfo,
                                       iCertInfoArr[iIndex].iUsage,
                                       iStatus );
    SetActive();
    _WIMTRACE ( _L( "CWimCertConverter::GetTrustedUsagesL() | End" ) );
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::TrustedUsagesDoneL()
// -----------------------------------------------------------------------------
void CWimCertConverter::TrustedUsagesDoneL()
    {
    _WIMTRACE ( _L( "CWimCertConverter::TrustedUsagesDoneL()" ) );
    if ( iStatus.Int() == KErrNone )
        {
        //Parse oids and put them to an array
        TLex16 lex( *iTrustedUsages );
        TPtrC16 lexToken;
        for ( TBool extrasDone = EFalse; extrasDone == EFalse; )
            {
            lexToken.Set( lex.NextToken() );
            if ( lexToken.Length() )
                {
                HBufC* oid = lexToken.AllocLC();
                User::LeaveIfError( iOids->Append( oid ) );
                CleanupStack::Pop( oid ); //oid
                }
            else
                {
                extrasDone = ETrue;
                }
            }
        CreateNewWimCertObjectL();
        SignalOwnStatusAndComplete();
        }
    else
        {
        User::RequestComplete( iClientStatus, iStatus.Int() );
        }

    iConnectionHandle->DeallocGetExtrasPckgBuf();
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::GetCertFormatByIndex()
// Returns certificate format according to received index
// -----------------------------------------------------------------------------
TCertificateFormat CWimCertConverter::GetCertFormatByIndex( TInt aIndex )
    {
    _WIMTRACE ( _L( "CWimCertConverter::GetCertFormatByIndex()" ) );

    // Convert type 
    TCertificateFormat format;
    switch ( iCertInfoArr[aIndex].iType )
        {
        case WIMI_CT_WTLS: 
            {
            format = EWTLSCertificate;
            break;
            }
        case WIMI_CT_X509: 
            {
            format = EX509Certificate;
            break;
            }
        case WIMI_CT_X968: 
            {
            format = EX968Certificate;
            break;
            }
        case WIMI_CT_URL: 
            {
            format = EX509CertificateUrl;
            break;
            }
        default:
            {
            format = EUnknownCertificate;
            break;
            }         
        }
    return format;
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::DoCancel()
// Deallocates member variables and completes client status with
// KErrCancel error code.
// -----------------------------------------------------------------------------
void CWimCertConverter::DoCancel()
    {
    _WIMTRACE ( _L( "CWimCertConverter::DoCancel()" ) );
    if ( iConnectionHandle && iPhase == ERetrievingCompleted )
        {
        iConnectionHandle->DeallocWimCertPckgBuf();
        }
    else if ( iConnectionHandle && iPhase == EAddCertificateCompleted ) 
        {
        iConnectionHandle->DeallocCertAddParametersPckgBuf();
        }
    else if ( iConnectionHandle && iPhase == ERemoveCompleted ) 
        {
        iConnectionHandle->DeallocRemoveCertPckgBuf();
        }
    else if ( iConnectionHandle && iPhase == ETrustedUsagesDone ) 
        {
        iConnectionHandle->DeallocGetExtrasPckgBuf();
        }

    if ( iPhase == EGetTrustedUsages || iPhase == ETrustedUsagesDone )
        {
        iCert->Release();
        }

    DeallocWimCertInfo();
    DeallocCertHBufs();
    DeallocReferences();
    User::RequestComplete( iClientStatus, KErrCancel );
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::RunError() 
//                                       
// The active scheduler calls this function if this active object's RunL() 
// function leaves. This gives this active object the opportunity to perform 
// any necessary cleanup.
// After array's cleanup, complete request with received error code.
// -----------------------------------------------------------------------------
TInt CWimCertConverter::RunError( TInt aError )
    {
    _WIMTRACE ( _L( "CWimCertConverter::RunError \
            Error = %d" ) );
    DeallocWimCertInfo();
    DeallocCertHBufs();
    delete iTrustedUsages;
    iTrustedUsages = NULL;
    delete iTrustedUsagesPtr;
    iTrustedUsagesPtr = NULL;
    delete iKeyIdBuf;
    iKeyIdBuf = NULL;
    delete iKeyIdPointer;
    iKeyIdPointer = NULL;

    User::RequestComplete( iClientStatus, aError );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::AllocWimCertInfoL()
// Allocates memory for the array which is filled by server.
// -----------------------------------------------------------------------------
void CWimCertConverter::AllocWimCertInfoL( TWimCertInfo* aWimCertInfoArr, 
                                           TInt aCount )
    {
    _WIMTRACE ( _L( "CWimCertConverter::AllocWimCertInfoL()" ) );

    TUint8 index;    
    //These are arrays
    iLabel = new( ELeave ) PHBufC8[aCount];
    iKeyId = new( ELeave ) PHBufC8[aCount];
    iCAId  = new( ELeave ) PHBufC8[aCount];
    iIssuerHash = new( ELeave ) PHBufC8[aCount];

    iLabelPtr = new( ELeave ) PTPtr8[aCount];
    iKeyIdPtr = new( ELeave ) PTPtr8[aCount];
    iCAIdPtr  = new( ELeave ) PTPtr8[aCount];
    iIssuerHashPtr = new( ELeave ) PTPtr8[aCount];

    for ( index = 0; index < aCount ; index++ )
        {
        iLabel[index] = HBufC8::NewL( KLabelLen );
        iKeyId[index] = HBufC8::NewL( KKeyIdLen );
        iCAId[index]  = HBufC8::NewL( KPkcs15IdLen );
        iIssuerHash[index] = HBufC8::NewL( KIssuerHash );
        
        iLabelPtr[index]  = new( ELeave ) TPtr8( iLabel[index]->Des() );
        iKeyIdPtr[index]  = new( ELeave ) TPtr8( iKeyId[index]->Des() );
        iCAIdPtr[index]   = new( ELeave ) TPtr8( iCAId[index]->Des() );
        iIssuerHashPtr[index] = new( ELeave ) 
                                TPtr8( iIssuerHash[index]->Des() );

        aWimCertInfoArr[index].iLabel.Copy( iLabelPtr[index]->Ptr(),
                                            iLabelPtr[index]->Length() );
        aWimCertInfoArr[index].iKeyId.Copy( iKeyIdPtr[index]->Ptr(),
                                            iKeyIdPtr[index]->Length() );
        aWimCertInfoArr[index].iCAId.Copy( iCAIdPtr[index]->Ptr(),
                                           iCAIdPtr[index]->Length() );
        aWimCertInfoArr[index].iIssuerHash.Copy(
            iIssuerHashPtr[index]->Ptr(), iIssuerHashPtr[index]->Length() );
        }
    iArraySize = aCount;   
    }

 

// -----------------------------------------------------------------------------
// CWimCertConverter::AllocMemoryForCertDetailsL()
// Allocates memory for a struct which is filled by server.
// -----------------------------------------------------------------------------     
void CWimCertConverter::AllocMemoryForCertDetailsL()
    {
    _WIMTRACE ( _L( "CWimCertConverter::AllocMemoryForCertDetailsL()" ) );
    iCertHBufOne = HBufC8::NewL( iEncodedCert->MaxLength() ); //whole cert
    iCertHBufOnePtr = new( ELeave ) TPtr8( iCertHBufOne->Des() ); 
    iWimCertDetails.iCert = iCertHBufOnePtr;
    }


// -----------------------------------------------------------------------------
// CWimCertConverter::CopyRetrievedCertData()
// Writes data to caller's buffer by using pointer.
// -----------------------------------------------------------------------------
void CWimCertConverter::CopyRetrievedCertData()
    {
    _WIMTRACE ( _L( "CWimCertConverter::CopyRetrievedCertData()" ) );
    TPtr8 ptr = iCertHBufOne->Des();
    iEncodedCert->Copy( ptr );
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::DeallocCertHBufs()
// DeAllocates memory from member variables, which are used
// when communicating with WIM.
// -----------------------------------------------------------------------------
void CWimCertConverter::DeallocCertHBufs()
    {
    _WIMTRACE ( _L( "CWimCertConverter::DeallocCertHBufs()" ) ); 
    if ( iCertHBufOne )
        {
        delete iCertHBufOne;
        delete iCertHBufOnePtr;
        iCertHBufOne = NULL;
        iCertHBufOnePtr = NULL;
        }
    if ( iCertHBufTwo )
        {
        delete iCertHBufTwo;
        delete iCertHBufTwoPtr;
        iCertHBufTwo = NULL;
        iCertHBufTwoPtr = NULL;
        }
    if ( iCertHBufThree )
        {
        delete iCertHBufThree;
        delete iCertHBufThreePtr;
        iCertHBufThree = NULL;
        iCertHBufThreePtr = NULL;
        }
    if ( iCertHBufFour )
        {
        delete iCertHBufFour;
        delete iCertHBufFourPtr;
        iCertHBufFour = NULL;
        iCertHBufFourPtr = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::DeallocReferences()
// Deallocates memory. If user has cancelled initialization process, we need
// to dealloc our references to loaded certs. 
// -----------------------------------------------------------------------------
void CWimCertConverter::DeallocReferences()
    {
    _WIMTRACE ( _L( "CWimCertConverter::DeallocReferences()" ) ); 
    if ( iCertRefLst )
        {
        delete[] iCertRefLst;  
        iCertRefLst = NULL;
        }
    if ( iCertInfoArr )
        {
        delete[] iCertInfoArr;
        iCertInfoArr = NULL;
        }  
    iArraySize = 0;
    iArray = NULL;
    iCertCount = 0;
    }

// -----------------------------------------------------------------------------
// CWimCertConverter::DeallocWimCertInfo()
// Deallocates memory. If something has leaved during asynchronous request, we 
// will deallocate all member data. 
// -----------------------------------------------------------------------------
void CWimCertConverter::DeallocWimCertInfo()
    {
    _WIMTRACE ( _L( "CWimCertConverter::DeallocWimCertInfo()" ) ); 
    TUint8 index;
    for ( index = 0; index < iArraySize; index ++ )
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
