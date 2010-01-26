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
* Description:  Utility functions for certificate related functionality
*
*/



#include    "WimCertUtil.h"
#include    "WimTrace.h"
#include    "WimCertInfo.h"
#include    "WimUtilityFuncs.h"
#include    "WimCleanup.h"
#include    <cctcertinfo.h>
#include    <asn1dec.h>         // ASN.1 decoding


// -----------------------------------------------------------------------------
// CWimCertUtil::CWimCertUtil()
// Default constructor
// -----------------------------------------------------------------------------
CWimCertUtil::CWimCertUtil( MCTToken& aToken )
    : CActive( EPriorityStandard ),
      iToken( aToken )
    {
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::NewL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
CWimCertUtil* CWimCertUtil::NewL( MCTToken& aToken )
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::NewL" ) );
    CWimCertUtil* self = new( ELeave ) CWimCertUtil( aToken );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self ); //self
    return self;
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::ConstructL()
// Second phase
// -----------------------------------------------------------------------------
void CWimCertUtil::ConstructL()
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::ConstructL" ) );
    CActiveScheduler::Add( this );
    iWimUtilFuncs = CWimUtilityFuncs::NewL();
    _WIMTRACE ( _L( "CWimCertUtil::ConstructL() completed" ) );
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::Restore()
// Returns CCTCertInfo objects in array
// -----------------------------------------------------------------------------
void CWimCertUtil::Restore( RPointerArray<CWimCertInfo>& aArray, 
                            TRequestStatus& aStatus )
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::Restore" ) );
    iClientStatus = &aStatus;
    aStatus = KRequestPending;
    iArray = &aArray;
    iPhase = EListCertsFromWim;
    SignalOwnStatusAndComplete();
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::CancelRestore()
// Cancels outgoing Restore operation. Sets an internal flag to true. 
// After necessary cleanup, caller is signalled with KErrCancel 
// -error code.
// -----------------------------------------------------------------------------
void CWimCertUtil::CancelRestore()
    {
    _WIMTRACE ( _L( "CWimCertUtil::CancelRestore" ) );
    Cancel(); 
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::RetrieveCertByIndexL()
// Retrieves the actual certificate.
// -----------------------------------------------------------------------------

void CWimCertUtil::RetrieveCertByIndexL( const TInt aIndex,
                                         TDes8& aEncodedCert, 
                                         TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "WimServer|CWimCertUtil::RetrieveCertByIndexL" ) );
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
// CWimCertUtil::SignalOwnStatusAndComplete()       
// Sets own iStatus to KRequestPending, and signals it 
// with User::RequestComplete() -request. This gives chance 
// active scheduler to run other active objects. After a quick
// visit in active scheduler, signal returns to RunL() and starts next
// phase of operation. 
// -----------------------------------------------------------------------------
void CWimCertUtil::SignalOwnStatusAndComplete()
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::SignalOwnStatusAndComplete()" ) );
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    }
    
// -----------------------------------------------------------------------------
// CWimCertUtil::~CWimCertUtil()
// Allocated memory is released.
// -----------------------------------------------------------------------------
CWimCertUtil::~CWimCertUtil()
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::~CWimCertUtil()" ) );
    Cancel();
    DeallocWimCertInfo();
    DeallocCertHBufs();
    DeallocReferences();
    delete iTrustedUsages;
    delete iTrustedUsagesPtr;
    delete iKeyIdBuf;
    delete iKeyIdPointer;
    delete iWimUtilFuncs;
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::RunL()
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
void CWimCertUtil::RunL()
    {
    _WIMTRACE3(_L("WimServer|CWimCertUtil::RunL(), iStatus=%d, iPhase=%d"), iStatus.Int(), iPhase);
    //Check for error
    if ( iStatus.Int() != KErrNone )
        {
        //failed to retrieve certificate
        if ( iPhase == ERetrievingCompleted )
            {
             DeallocCertHBufs();
             _WIMTRACE2 ( _L( "CWimCertUtil::RunL() failed to retrieve \
                 certificate, ERROR = %d" ),iStatus.Int() );
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
                iCertCount = CertCount( EWimEntryTypeAll ); 
                _WIMTRACE2 ( _L( "CWimCertUtil::RunL() \
                    certCount =%d" ),iCertCount );
                if ( !iCertCount )  
                    {
                    User::RequestComplete( iClientStatus, KErrNotFound );
                    }
                else
                    {
                    __ASSERT_DEBUG( iCertRefLst == NULL, User::Invariant() );
                    iCertRefLst = new( ELeave ) TCertificateAddress[iCertCount];

                    //Creates new array according to certificate count
                    __ASSERT_DEBUG( iCertInfoArr == NULL, User::Invariant() );
                    iCertInfoArr  = new( ELeave ) TWimCertInfo[iCertCount];
            
                    AllocWimCertInfoL( iCertInfoArr, iCertCount );
                    iStatus = KRequestPending;

                    CertRefLstL( iCertRefLst, 
                                 iCertInfoArr, 
                                 EWimEntryTypeAll );

                    iPhase = ECreateNewCertObjects;
                    iIndex = 0;
                    SetActive();

                    TRequestStatus* status = &iStatus;
                    User::RequestComplete( status, KErrNone );

                    _WIMTRACE( _L( "CWimCertUtil::RunL() \
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
                _WIMTRACE ( _L( "CWimCertUtil::RunL() \
                    ECertObjectsDone" ) );
                break;
                }
            case ERetrieveCertificate: //Get Cert details from WIM
                {
                AllocMemoryForCertDetailsL();
                CertDetailsL( iCertRefLst[iCertRetrieveIndex], 
                              iWimCertDetails );
                iPhase = ERetrievingCompleted;
                SetActive();

                TRequestStatus* status = &iStatus;
                    User::RequestComplete( status, KErrNone );

                _WIMTRACE ( _L( "CWimCertUtil::RunL() \
                    ERetrieveCertificate" ) );
                break;
                }
            case ERetrievingCompleted: //Cert details fetched, check errors &                         
                {                      //cancellations & complete request                   
                //copy cert details to iEncodedCert -buffer,
                //which points to received aEncoded -cert buffer.
                CopyRetrievedCertData();
                DeallocCertHBufs();
                User::RequestComplete( iClientStatus, KErrNone );
                _WIMTRACE ( _L( "CWimCertUtil::RunL() \
                    ERetrievingCompleted" ) );
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
// CWimCertUtil::CreateNewCertObjectsL()
// Creates new certificate objects which can be returned to the caller.
// -----------------------------------------------------------------------------
void CWimCertUtil::CreateNewCertObjectsL()
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::CreateNewCertObjectsL() | Begin" ) );

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
// CWimCertUtil::CreateNewWimCertObjectL()
// 
// -----------------------------------------------------------------------------
void CWimCertUtil::CreateNewWimCertObjectL()
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::CreateNewWimCertObjectL()" ) );

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
// CWimCertUtil::GetTrustedUsagesL
// Get trusted usages (OIDs) of a current certificate, which is pointed out 
// by a index
// -----------------------------------------------------------------------------
void CWimCertUtil::GetTrustedUsagesL()
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::GetTrustedUsagesL() | Begin" ) );

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

    GetCertExtrasL( iKeyIdPointer,
                    iCertExtrasInfo,
                    iCertInfoArr[iIndex].iUsage,
                    iStatus );
    SetActive();
    _WIMTRACE ( _L( "CWimCertUtil::GetTrustedUsagesL() | End" ) );
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::TrustedUsagesDoneL()
// -----------------------------------------------------------------------------
void CWimCertUtil::TrustedUsagesDoneL()
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::TrustedUsagesDoneL()" ) );
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
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::GetCertFormatByIndex()
// Returns certificate format according to received index
// -----------------------------------------------------------------------------
TCertificateFormat CWimCertUtil::GetCertFormatByIndex( TInt aIndex )
    {
    _WIMTRACE ( _L( "WimServer|CWimCertUtil::GetCertFormatByIndex()" ) );

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
// CWimCertUtil::DoCancel()
// Deallocates member variables and completes client status with
// KErrCancel error code.
// -----------------------------------------------------------------------------
void CWimCertUtil::DoCancel()
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::DoCancel()" ) );

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
// CWimCertUtil::RunError() 
//                                       
// The active scheduler calls this function if this active object's RunL() 
// function leaves. This gives this active object the opportunity to perform 
// any necessary cleanup.
// After array's cleanup, complete request with received error code.
// -----------------------------------------------------------------------------
TInt CWimCertUtil::RunError( TInt aError )
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::RunError Error = %d" ) );
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
// CWimCertUtil::AllocWimCertInfoL()
// Allocates memory for the array which is filled by server.
// -----------------------------------------------------------------------------
void CWimCertUtil::AllocWimCertInfoL( 
    TWimCertInfo* aWimCertInfoArr, 
    TInt aCount )
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::AllocWimCertInfoL()" ) );

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
// CWimCertUtil::AllocMemoryForCertDetailsL()
// Allocates memory for a struct which is filled by server.
// -----------------------------------------------------------------------------     
void CWimCertUtil::AllocMemoryForCertDetailsL()
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::AllocMemoryForCertDetailsL()" ) );
    iCertHBufOne = HBufC8::NewL( iEncodedCert->MaxLength() ); //whole cert
    iCertHBufOnePtr = new( ELeave ) TPtr8( iCertHBufOne->Des() ); 
    iWimCertDetails.iCert = iCertHBufOnePtr;
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::CopyRetrievedCertData()
// Writes data to caller's buffer by using pointer.
// -----------------------------------------------------------------------------
void CWimCertUtil::CopyRetrievedCertData()
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::CopyRetrievedCertData()" ) );
    TPtr8 ptr = iCertHBufOne->Des();
    iEncodedCert->Copy( ptr );
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::DeallocCertHBufs()
// DeAllocates memory from member variables, which are used
// when communicating with WIM.
// -----------------------------------------------------------------------------
void CWimCertUtil::DeallocCertHBufs()
    {
    _WIMTRACE ( _L( "WimServer|CWimCertUtil::DeallocCertHBufs()" ) ); 
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
// CWimCertUtil::DeallocReferences()
// Deallocates memory. If user has cancelled initialization process, we need
// to dealloc our references to loaded certs. 
// -----------------------------------------------------------------------------
void CWimCertUtil::DeallocReferences()
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::DeallocReferences()" ) ); 
    if ( iCertRefLst )
        {
        for( TInt index = 0; index < iCertCount; ++index )
            {
            WIMI_Ref_t* ref = reinterpret_cast< WIMI_Ref_t* >( iCertRefLst[ index ] );
            _WIMTRACE2( _L( "WimServer|CWimCertUtil::DeallocReferences(), -ref 0x%08x" ), ref );
            free_WIMI_Ref_t( ref );
            iCertRefLst[ index ] = 0;
            }
        delete[] iCertRefLst;
        iCertRefLst = NULL;
        }
    if ( iCertInfoArr )
        {
        delete iCertInfoArr;
        iCertInfoArr = NULL;
        }  
    iArraySize = 0;
    iArray = NULL;
    iCertCount = 0;
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::DeallocWimCertInfo()
// Deallocates memory. If something has leaved during asynchronous request, we 
// will deallocate all member data. 
// -----------------------------------------------------------------------------
void CWimCertUtil::DeallocWimCertInfo()
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::DeallocWimCertInfo()" ) ); 
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
    
    delete iLabel;   
    delete iKeyId;                
    delete iCAId;
    delete iIssuerHash; 
    delete iLabelPtr;        
    delete iKeyIdPtr;        
    delete iCAIdPtr;        
    delete iIssuerHashPtr;  
    iLabel = NULL;
    iKeyId  = NULL;
    iCAId = NULL;
    iIssuerHash = NULL;
    iLabelPtr = NULL;
    iKeyIdPtr = NULL;
    iCAIdPtr = NULL;
    iIssuerHashPtr = NULL;
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::CertCount
// Get count of certificate in WIM
// -----------------------------------------------------------------------------
//
TUint8 CWimCertUtil::CertCount( TWimEntryType aType )
    {
    _WIMTRACE( _L( "WimServer|CWimCertUtil::DeallocWimCertInfo()" ) ); 
    WIMI_STAT callStatus = WIMI_Ok;
    
    WIMI_Ref_t* wimRef = NULL;
    TUint8 certCount = 0;

    wimRef = WIMI_GetWIMRef( 0 );

    if ( wimRef )
        {
        if ( aType == EWimEntryTypeAll || aType == EWimEntryTypeCA )
            {
            callStatus = GetCertificateCountByWIM( wimRef, 
                                                   certCount, 
                                                   WIMI_CU_CA );            
            }

        if ( callStatus == WIMI_Ok && ( aType == EWimEntryTypeAll || 
                                        aType == EWimEntryTypePersonal ) )
            {
            callStatus = GetCertificateCountByWIM( wimRef, 
                                                   certCount, 
                                                   WIMI_CU_Client );
            }

        free_WIMI_Ref_t( wimRef );
        }
    else
        {
        certCount = 0;
        }

    return certCount;
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::CertRefLstL
// 
// -----------------------------------------------------------------------------
//
void CWimCertUtil::CertRefLstL(
    TCertificateAddressList aCertAddrLst, 
    TWimCertInfo* aCertInfoArr, 
    TWimEntryType aCertEntryType )
    {
    
    WIMI_STAT callStatus = WIMI_Ok;
    TUint8 certNum = 0;
    WIMI_Ref_t* wimRef = NULL;

    wimRef = WIMI_GetWIMRef( 0 );

    if ( wimRef )
        {
        CleanupPushWimRefL( wimRef );

        if ( aCertEntryType == EWimEntryTypeAll || 
             aCertEntryType == EWimEntryTypeCA )
            {
            callStatus = GetCertificateFromWimRefL( wimRef, 
                                                    WIMI_CU_CA,
                                                    certNum,
                                                    aCertAddrLst,
                                                    aCertInfoArr );
            }
        if ( callStatus == WIMI_Ok && 
            ( aCertEntryType == EWimEntryTypeAll || 
              aCertEntryType == EWimEntryTypePersonal ) )
            {
            callStatus = GetCertificateFromWimRefL( wimRef,
                                                    WIMI_CU_Client,
                                                    certNum,
                                                    aCertAddrLst,
                                                    aCertInfoArr );
            }

        CleanupStack::PopAndDestroy( wimRef );
        }
    else
        {
        callStatus = WIMI_ERR_OutOfMemory;
        }
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::GetCertExtrasL
// 
// -----------------------------------------------------------------------------
//
void CWimCertUtil::GetCertExtrasL(
    const TPtr8* aKeyId, 
    TCertExtrasInfo& aCertExtrasInfo,
    TUint aUsage,
    TRequestStatus& aStatus )
    {
    WIMI_STAT callStatus = WIMI_Ok;
    TInt8 certUsage = 0;

    TPtrC8 keyID( aKeyId->Ptr(), aKeyId->Length() );

    WIMI_Ref_t* wimTempRef = WIMI_GetWIMRef( 0 );

    if ( wimTempRef )
        {
        CleanupPushWimRefL( wimTempRef );

        switch ( aUsage )
            {
            case EWimEntryTypeCA:
                {
                certUsage = WIMI_CU_CA;
                break;
                }
            case EWimEntryTypeAll: //Flow through
            case EWimEntryTypePersonal:
                {
                certUsage = WIMI_CU_Client;
                break;
                }
            default:
                {
                callStatus = WIMI_ERR_BadParameters;
                break;
                }
            }

        if ( callStatus == WIMI_Ok )
            {
            callStatus = GetExtrasFromWimRefL( wimTempRef,
                                               certUsage,
                                               keyID,
                                               aCertExtrasInfo );
            }

        CleanupStack::PopAndDestroy( wimTempRef );
        }
    else
        {
        callStatus = WIMI_ERR_BadReference;
        }

    TRequestStatus* status = &aStatus;
    User::RequestComplete( status, iWimUtilFuncs->MapWIMError( callStatus ) );
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::CertDetailsL
// 
// -----------------------------------------------------------------------------
//
void CWimCertUtil::CertDetailsL(
    const TCertificateAddress aCertAddr, 
    TWimCertDetails& aWimCertDetails )
    {
    WIMI_STAT callStatus = WIMI_Ok;
    WIMI_TransactId_t trId = ( void* )EInitializationCertListHashing;

    WIMI_BinData_t ptCertData;
    ptCertData.pb_buf = NULL;
    ptCertData.ui_buf_length = 0;

    callStatus = WIMI_CertificateReqT( trId, ( void* )aCertAddr, &ptCertData );

    if ( callStatus == WIMI_Ok )
        {
        // Now we have certificate data, copy it to allocated buffer
        if( aWimCertDetails.iCert->MaxLength() < ptCertData.ui_buf_length )
            {
            WSL_OS_Free( ptCertData.pb_buf );
            User::Leave( KErrOverflow );
        	}
        aWimCertDetails.iCert->Copy( ptCertData.pb_buf,
                                     ptCertData.ui_buf_length );
        WSL_OS_Free( ptCertData.pb_buf );
        }
    }

// -----------------------------------------------------------------------------
// CWimCertUtil::GetCertificateCountByWIM
// Fetches count of certicates in certain WIM card.
// -----------------------------------------------------------------------------
//
WIMI_STAT CWimCertUtil::GetCertificateCountByWIM(
    WIMI_Ref_t* aRef, 
    TUint8& aCertCount, 
    TUint8 aUsage ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertUtil::GetCertificateCountByWIM | Begin"));
    TUint8 certNum = 0;
    WIMI_RefList_t refList ;
    WIMI_STAT callStatus = WIMI_GetCertificateListByWIM( aRef, 
                                                         aUsage,
                                                         &certNum, 
                                                         &refList );
    if ( callStatus == WIMI_Ok )
        {
        aCertCount = ( TUint8 )( aCertCount + certNum );
        }
    free_WIMI_RefList_t( refList );
    return callStatus;
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::GetExtrasFromWimRefL
// Fetches extra information (e.g. certs trusted usage) from the WIM card.
// -----------------------------------------------------------------------------
//
WIMI_STAT CWimCertUtil::GetExtrasFromWimRefL(
    WIMI_Ref_t* aTmpWimRef,
    TInt8 aUsage,
    TDesC8& aKeyHash,
    TCertExtrasInfo& aCertExtrasInfo ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::GetExtrasFromWimRefL | Begin"));
  
    TUint8 tempCertCount = 0;
    WIMI_RefList_t certRefList = NULL;
    WIMI_STAT callStatus = WIMI_Ok;
    TInt certIndex = 0;
    TPtrC8 keyHash;

    if ( aTmpWimRef )
        {
        // List all certificates (by WIM and usage)
        callStatus = WIMI_GetCertificateListByWIM( aTmpWimRef, 
                                                   aUsage, 
                                                   &tempCertCount, 
                                                   &certRefList );
        }
    else
        {
        callStatus = WIMI_ERR_BadReference;
        }
    
    if ( callStatus == WIMI_Ok )
        {
        CleanupPushWimRefListL( certRefList );

        WIMI_Ref_t* tempRef = NULL;
        WIMI_BinData_t ptLabel;
        WIMI_BinData_t ptKeyID;
        WIMI_BinData_t ptCAID;
        WIMI_BinData_t ptIssuerHash;
        WIMI_BinData_t ptTrustedUsage;
        TUint8 uiCDFRefs;
        TUint8 usage;
        TUint8 certType;
        TUint16 certLen;
        TUint8 modifiable = 0;

        for ( TInt i = 0; i < tempCertCount; i++ )
            {
            // Get info for each certificate until we find valid cert
            callStatus = WIMI_GetCertificateInfo( certRefList[i],
                                                  &tempRef,
                                                  &ptLabel,
                                                  &ptKeyID,
                                                  &ptCAID,
                                                  &ptIssuerHash,
                                                  &ptTrustedUsage,
                                                  &uiCDFRefs,
                                                  &usage,
                                                  &certType,   
                                                  &certLen,
                                                  &modifiable );
            if ( callStatus == WIMI_Ok )
                {
                WSL_OS_Free( ptLabel.pb_buf );
                WSL_OS_Free( ptCAID.pb_buf );
                WSL_OS_Free( ptIssuerHash.pb_buf );
                WSL_OS_Free( ptTrustedUsage.pb_buf );
                free_WIMI_Ref_t( tempRef );

                keyHash.Set( ptKeyID.pb_buf, ptKeyID.ui_buf_length );

                // Compare given and fetched key hash
                if ( keyHash.Compare( aKeyHash ) == 0 &&
                     certType == WIMI_CT_X509 ) //Match
                    {
                    certIndex = i; // Found one
                    i = tempCertCount; // Stop looping
                    callStatus = WIMI_Ok;
                    }
                else // Cert not supported
                    {
                    callStatus = WIMI_ERR_UnsupportedCertificate;
                    }

                WSL_OS_Free( ptKeyID.pb_buf );
                }
            }

        if ( callStatus == WIMI_Ok )
            {
            CopyCertExtrasInfoL( certRefList[certIndex], aCertExtrasInfo );
            }
        CleanupStack::PopAndDestroy( certRefList );
        }

    return callStatus;
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::CopyCertExtrasInfoL
// Copies certs extra information to client's allocated structure.
// -----------------------------------------------------------------------------
//
void CWimCertUtil::CopyCertExtrasInfoL(
    WIMI_Ref_t* aCert,
    TCertExtrasInfo& aCertExtrasInfo ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::CopyCertExtrasInfoL | Begin"));
    WIMI_Ref_t* tempref = NULL;
    WIMI_BinData_t ptLabel;
    WIMI_BinData_t ptKeyID;
    WIMI_BinData_t ptCAID;
    WIMI_BinData_t ptIssuerHash;
    WIMI_BinData_t ptTrustedUsage;
    TUint8 uiCDFRefs;
    TUint8 usage;
    TUint8 type;
    TUint16 certlen;
    TUint8 modifiable = 0;
    TBool found = ETrue;

    WIMI_STAT callStatus = WIMI_GetCertificateInfo( 
                                aCert,
                                &tempref,
                                &ptLabel,
                                &ptKeyID, /* Key Id (hash)*/
                                &ptCAID,
                                &ptIssuerHash,
                                &ptTrustedUsage,
                                &uiCDFRefs,
                                &usage,  /* 0 = client, 1 = CA */
                                &type,   
                                &certlen,   /* cert. content or URL length */
                                &modifiable); 
    if ( callStatus == WIMI_Ok )
        {
        free_WIMI_Ref_t( tempref );
        WSL_OS_Free( ptLabel.pb_buf );
        WSL_OS_Free( ptKeyID.pb_buf );
        WSL_OS_Free( ptCAID.pb_buf );
        WSL_OS_Free( ptIssuerHash.pb_buf );

        CleanupPushWimBufL( ptTrustedUsage );

        TPtrC8 undecodedUsage;
        undecodedUsage.Set( ptTrustedUsage.pb_buf ); 

        if ( ptTrustedUsage.ui_buf_length == 0 ) // No OIDs
            {
            found = EFalse;
            }

        // DECODE OIDs
        TASN1DecObjectIdentifier decoder;
        RPointerArray<HBufC> decodedOIDArray;
        HBufC* decodedOIDs = NULL;
        TInt oidsLength = 0;    // length of total OID buffer
        TInt err;

        for ( TInt position = 0; found; )   //Loop until no OIDs found anymore
            {
            if ( undecodedUsage.Length() > position ) //Don't go over buffer
                {
                TRAP( err, decodedOIDs = decoder.DecodeDERL( undecodedUsage, position ) );
                if ( err == KErrNone ) //Found OID
                    {
                    CleanupStack::PushL( decodedOIDs );
                    if ( decodedOIDs->Length() )
                        {
                        found = ETrue;
                        decodedOIDArray.AppendL( decodedOIDs );
                        oidsLength += decodedOIDs->Length();
                        CleanupStack::Pop( decodedOIDs );
                        }
                    else    // Not found OID from buffer
                        {
                        found = EFalse;
                        CleanupStack::PopAndDestroy( decodedOIDs );
                        }
                    decodedOIDs = NULL;
                    }
                else    // Error in OID parsing -> Not found OID
                    {
                    found = EFalse;
                    }
                }
            else    // undecoded OID buffer seeked through
                {
                found = EFalse;
                }
            }

        _LIT( KDelimeter16, " " ); //Delimeter between OIDs

        if ( oidsLength > 0 ) // OID's found
            {
            // Add OID's to one buffer from separate buffers
            for ( TInt i = 0; i < decodedOIDArray.Count(); i++ )
                {
                if ( i == 0 ) //First one
                    {
                    aCertExtrasInfo.iTrustedUsage->Copy( decodedOIDArray[i]->Des() );
                    }
                else // Append other OIDs, with delimeter
                    {
                    aCertExtrasInfo.iTrustedUsage->Append( KDelimeter16 );
                    aCertExtrasInfo.iTrustedUsage->Append( decodedOIDArray[i]->Des() );
                    }
                }
            }

        aCertExtrasInfo.iCDFRefs = iWimUtilFuncs->MapCertLocation( uiCDFRefs );
        
        // Free the memory
        decodedOIDArray.ResetAndDestroy();
        CleanupStack::PopAndDestroy( ptTrustedUsage.pb_buf );
        }
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::GetCertificateFromWimRefL
// Fetches certificate from the WIM card.
// -----------------------------------------------------------------------------
//
WIMI_STAT CWimCertUtil::GetCertificateFromWimRefL(
    WIMI_Ref_t* aTmpWimRef,
    TInt8 aUsage,
    TUint8& aCertNum,
    TUint32* aCertRefLst,
    TWimCertInfo* aCertInfoLst )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::GetCertificateFromWimRefL | Begin"));
    TUint8 tempCrtCount;
    WIMI_RefList_t refList ;
    WIMI_STAT callStatus = WIMI_Ok;

    if ( aTmpWimRef )
        {
        callStatus = WIMI_GetCertificateListByWIM( aTmpWimRef, 
                                                   aUsage, 
                                                   &tempCrtCount, 
                                                   &refList );
        if ( callStatus == WIMI_Ok )
            {
            for ( TUint8 certIndex = 0; certIndex < tempCrtCount; certIndex++ )
                {
                TInt current = aCertNum + certIndex;
                _WIMTRACE2( _L( "CWimCertHandler::GetCertificateFromWimRefL, +ref 0x%08x" ),
                        refList[certIndex] );
                // transfers ownership of the refList item to aCertRefLst
                aCertRefLst[current] = reinterpret_cast< TUint32 >( refList[certIndex] );
                CopyCertificateInfo( aCertInfoLst[current], refList[certIndex] );
                }
            aCertNum = static_cast< TUint8 >( aCertNum + tempCrtCount );

            // Because list items are moved to aCertRefLst, only refList array
            // needs to be freed. Cannot use free_WIMI_RefList_t() as it would
            // delete also items contained in refList.
            WSL_OS_Free( refList );
            }
        }
    return callStatus;
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::CopyCertificateInfo
// Copies certificate information to client's allocated memory area.
// -----------------------------------------------------------------------------
//
void CWimCertUtil::CopyCertificateInfo(
    TWimCertInfo& aCertInfo,
    WIMI_Ref_t* aCert )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::CopyCertificateInfoL | Begin"));

    WIMI_Ref_t* tempRef = NULL;
    WIMI_BinData_t ptLabel;
    WIMI_BinData_t ptKeyID;
    WIMI_BinData_t ptCAID;
    WIMI_BinData_t ptIssuerHash;
    WIMI_BinData_t ptTrustedUsage;
    TUint8 uiCDFRefs;
    TUint8 usage;
    TUint8 type;
    TUint16 certLen;
    TUint8 modifiable = 0;
    WIMI_STAT callStatus = WIMI_GetCertificateInfo( aCert,
                                                    &tempRef,
                                                    &ptLabel,
                                                    &ptKeyID,
                                                    &ptCAID,
                                                    &ptIssuerHash,
                                                    &ptTrustedUsage,
                                                    &uiCDFRefs,
                                                    &usage,
                                                    &type,
                                                    &certLen, 
                                                    &modifiable );

    if ( callStatus == WIMI_Ok )
        {
        free_WIMI_Ref_t( tempRef );

        aCertInfo.iLabel.Copy( TPtr8( ptLabel.pb_buf,
                ptLabel.ui_buf_length,
                ptLabel.ui_buf_length ) );

        aCertInfo.iKeyId.Copy( TPtr8( ptKeyID.pb_buf,
                ptKeyID.ui_buf_length,
                ptKeyID.ui_buf_length ) );
        
        aCertInfo.iCAId.Copy( TPtr8( ptCAID.pb_buf,
                ptCAID.ui_buf_length,
                ptCAID.ui_buf_length ) );
        
        aCertInfo.iIssuerHash.Copy( TPtr8( ptIssuerHash.pb_buf,
                ptIssuerHash.ui_buf_length,
                ptIssuerHash.ui_buf_length ) );

        aCertInfo.iUsage = usage;
        aCertInfo.iType = type;
        aCertInfo.iCertlen = certLen;
        aCertInfo.iModifiable = modifiable;
        
        // Certificate location
        aCertInfo.iCDFRefs = iWimUtilFuncs->MapCertLocation( uiCDFRefs );

        //Allocate enough memory for OID
        aCertInfo.iTrustedUsageLength = ptTrustedUsage.ui_buf_length * 6;

        WSL_OS_Free( ptLabel.pb_buf );
        WSL_OS_Free( ptKeyID.pb_buf );
        WSL_OS_Free( ptCAID.pb_buf );
        WSL_OS_Free( ptIssuerHash.pb_buf );
        WSL_OS_Free( ptTrustedUsage.pb_buf );
        }
    }

// End of File
