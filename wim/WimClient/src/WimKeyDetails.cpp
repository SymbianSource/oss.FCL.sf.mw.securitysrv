/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Gets keydetails from WIM -card
*
*/



#include "WimCertMgmt.h"
#include "WimKeyDetails.h"
#include "WimTrace.h"


// -----------------------------------------------------------------------------
// CWimKeyDetails::CWimKeyDetails()
// Default constructor
// -----------------------------------------------------------------------------
CWimKeyDetails::CWimKeyDetails( MCTToken& aToken ):
CActive( EPriorityStandard ), iToken( aToken ), iPckg( iKeyNumber )
    {
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::NewL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
EXPORT_C CWimKeyDetails* CWimKeyDetails::NewL( MCTToken& aToken )
    {
    _WIMTRACE ( _L( "CWimKeyDetails::NewL()" ) );
    CWimKeyDetails* self = new( ELeave ) CWimKeyDetails( aToken );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self ); //self
    return self;
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::ConstructL()
// Second phase
// -----------------------------------------------------------------------------
void CWimKeyDetails::ConstructL()
    {
    _WIMTRACE ( _L( "CWimKeyDetails::ConstructL()" ) );
    CActiveScheduler::Add( this );
    iConnectionHandle = RWimCertMgmt::ClientSessionL();
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::~CWimKeyDetails()
// Allocated memory is released.
// -----------------------------------------------------------------------------
EXPORT_C CWimKeyDetails::~CWimKeyDetails()
    {
    _WIMTRACE ( _L( "CWimKeyDetails::~CWimKeyDetails()" ) );
    Cancel();
    iConnectionHandle->Close();
    delete iConnectionHandle;
    iKeyReferences.Close();
    DeallocMemoryFromSign();
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::GetKeyList()
// Starts to list all keys found from WIM -card
// -----------------------------------------------------------------------------
EXPORT_C void CWimKeyDetails::GetKeyList( RPointerArray<CCTKeyInfo>& aKeys,
                                          CArrayFixFlat<TUint8>& aKeyNumbers,
                                          TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimKeyDetails::GetKeyList()" ) );
    iClientStatus = &aStatus;
    aStatus = KRequestPending;
    iKeys = &aKeys;
    iKeyNumberArray = &aKeyNumbers;
    iPhase = EGetKeyList;
    SignalOwnStatusAndComplete();
    iFetchedKeyInfos = 0;
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::CancelList()
// Cancels listing operation.
// -----------------------------------------------------------------------------
EXPORT_C void CWimKeyDetails::CancelList()
    {
    _WIMTRACE ( _L( "CWimKeyDetails::CancelList()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::Sign()
// Sign some data. Authentication is handled by server.
// -----------------------------------------------------------------------------

EXPORT_C void CWimKeyDetails::Sign( const TDesC8& aData, 
                                   TDesC8& aKeyId, 
                                   HBufC8*& aSignature, 
                                   TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimKeyDetails::Sign()" ) );

    aStatus = KRequestPending;
    iClientStatus = &aStatus;
    
    if ( aKeyId.Length() )
        {
        //Check that the data is not too long
        if ( aData.Length() > KMaxRSADigestSize )
            {
            User::RequestComplete( iClientStatus, KErrOverflow );
            }
        else
            {     
            iSignature = aSignature; 
            TRAPD( err, AllocMemoryForSignL( aData, aKeyId ) );
            if ( err )
                {
                User::RequestComplete( iClientStatus, err );
                }
            else
                {
                iPhase = ESign;
                SignalOwnStatusAndComplete();
                }
            }
        }
    else
        {
        User::RequestComplete( iClientStatus, KErrArgument );
        }
    }
  
// -----------------------------------------------------------------------------
// CWimKeyDetails::CancelSign()
// Cancels outgoing signing operation 
// -----------------------------------------------------------------------------
EXPORT_C void CWimKeyDetails::CancelSign()
    {
    _WIMTRACE ( _L( "CWimKeyDetails::CancelSign()" ) );
    Cancel();
    }


// -----------------------------------------------------------------------------
// CWimKeyDetails::ExportPublicKeyL()
// Export public key
// -----------------------------------------------------------------------------
EXPORT_C void CWimKeyDetails::ExportPublicKeyL( TDesC8& aKeyId, 
                                                HBufC8*& aPublicKey, 
                                                TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimKeyDetails::ExportPublicKeyL()" ) );

    iClientStatus = &aStatus;
    aStatus = KRequestPending;
    
    if ( aKeyId.Length() )
        {   
        //KeyId from Plugin 
        iKeyId = aKeyId.AllocL();
        iKeyIdPtr = new( ELeave ) TPtr8( iKeyId->Des() );
        iExportPublicKey.iKeyId.Copy( iKeyIdPtr->Ptr(), iKeyIdPtr->Length() );
    
        //Exported key from Server
        iPublicKey = aPublicKey;
        iPublicKeyPtr = new( ELeave ) TPtr8( iPublicKey->Des() );
        iExportPublicKey.iPublicKey = iPublicKeyPtr;
    
        //Request to Server
        iConnectionHandle->ExportPublicKeyL( iExportPublicKey, 
                                             iStatus );
        iPhase = EExportPublicKey;
        SetActive();
        }
    else 
        {
        User::RequestComplete( iClientStatus, KErrArgument );
        }
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::CancelExport()
// Cancel public key export
// -----------------------------------------------------------------------------
EXPORT_C void CWimKeyDetails::CancelExport()
    {
    _WIMTRACE ( _L( "CWimKeyDetails::CancelExport()" ) );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::AllocMemoryForSignL()
// Allocates memory for signing operation
// -----------------------------------------------------------------------------
void CWimKeyDetails::AllocMemoryForSignL( const TDesC8& aData,
                                          const TDesC8& aKeyId )
    {
    _WIMTRACE ( _L( "CWimKeyDetails::AllocMemoryForSignL()" ) );

    iSigningData = aData.AllocL(); //Data to be signed
    iSigningDataPtr = new( ELeave ) TPtr8( iSigningData->Des() );

    iKeyId = aKeyId.AllocL(); //KeyId used to match correct signing key
    iKeyIdPtr = new( ELeave ) TPtr8( iKeyId->Des() );

    //Alloc pointer for signature buffer, which is owned by keystore
    iSignaturePtr = new( ELeave ) TPtr8( iSignature->Des() );
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::DeallocMemoryFromSign()
// Deallocates memory after signing  operation
// -----------------------------------------------------------------------------
void CWimKeyDetails::DeallocMemoryFromSign()
    {
    _WIMTRACE ( _L( "CWimKeyDetails::DeallocMemoryFromSign()" ) );

    delete iSigningData;
    delete iSigningDataPtr;
    delete iSignaturePtr;
    delete iKeyId;
    delete iKeyIdPtr;
    iSigningData = NULL;
    iSigningDataPtr = NULL;
    iSignaturePtr = NULL;
    iKeyId = NULL;
    iKeyIdPtr = NULL;
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::SignalOwnStatusAndComplete()       
// Sets own iStatus to KRequestPending, and signals it 
// with User::RequestComplete() -request. This gives chance 
// active scheduler to run other active objects. After a quick
// visit in active scheduler, signal returns to RunL() and starts next
// phase of operation. 
// @return void
// -----------------------------------------------------------------------------
void CWimKeyDetails::SignalOwnStatusAndComplete()
    {
    _WIMTRACE ( _L( "CWimKeyDetails::SignalOwnStatusAndComplete()" ) );
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::RunL()
// EGetKeyList: Get the number of keys key and references to each key.
// EConvertParams: Gets key references from string and puts them to RArray
// EGetKeyInfo: Fetches a single keyinfo. According to received values, a new
//              CCTKeyInfo -object is created and inserted to an Array. Received
//              parameters requires a little bit conversion. 
// ESign:       Sends signing request with data to be signed to server.
// ESignCompleted: Sign is done 
// EExportPublicKey: Sends Export public key request to server. Receives 
//              exported public key.
// -----------------------------------------------------------------------------
void CWimKeyDetails::RunL()
    {
	//Check for error
    if ( iStatus.Int() != KErrNone )
        {
        if ( iPhase == EConvertParams )
            {
            DeallocMemoryFromKeyList();
            }
        else if ( iPhase == ESignCompleted )
            {
            iConnectionHandle->DeallocKeySignPckgBuf();
            DeallocMemoryFromSign();
            }
        else if ( iPhase == EExportPublicKey )
            {
            iConnectionHandle->DeallocExportPublicKeyPckgBuf();
            DeallocMemoryFromKeyInfo();
            delete iPublicKeyPtr;
            iPublicKey = NULL;      // no delete, ownership moved to caller
            iPublicKeyPtr = NULL;
            }
        User::RequestComplete( iClientStatus, iStatus.Int() );
        }
    else
        {
        switch ( iPhase )
            {
            case EGetKeyList:
                {
                AllocMemoryForKeyListL();
                iConnectionHandle->KeyList( *iKeyListPtr, 
                                            iPckg,
                                            iStatus );  
                iPhase = EConvertParams;
                SetActive();
                _WIMTRACE ( _L( "CWimKeyDetails::RunL(),\
                        case:EGetKeyList" ) );
                break;
                }
            case EConvertParams: //Convert previously fetched parameters
                {                //and put them to an RArray.
                ConvertKeyListL();
                DeallocMemoryFromKeyList(); //We don't need keylist anymore
                iPhase = EGetKeyInfo;
                SignalOwnStatusAndComplete();
                _WIMTRACE ( _L( "CWimKeyDetails::RunL(),EConvertParams" ) );
                break;
                }
            case EGetKeyInfo: //Now we can fetch details for every key.
                {
                //Are all keyinfo -objects fetched?
                if ( iFetchedKeyInfos < iKeyReferences.Count() )
                    { //No, get next keyinfo..
                    TKeyInfo keyInfo;
                    AllocMemoryForKeyInfoL( keyInfo );
                    TInt ret = iConnectionHandle->GetKeyInfo(
                              iKeyReferences.operator[]( iFetchedKeyInfos ),
                              keyInfo );

                    if ( ret != KErrNone )//Something went wrong
                        {
                        DeallocMemoryFromKeyInfo();
                        User::RequestComplete( iClientStatus, ret );
                        }
                    else
                        {
                        TKeyUsagePKCS15 keyUsage;
                        keyUsage = ConvertKeyUsage( keyInfo.iUsage );

                        //Convert key type to support only RSA signing or
                        //invalidAlgorithm
                        if ( keyInfo.iType != CKeyInfoBase::ERSA )
                            {
                            keyInfo.iType = 
                                  ( TUint8 )CKeyInfoBase::EInvalidAlgorithm;
                            }
                        //Create new KeyInfo -object and put it into array.
                        CCTKeyInfo* cctKeyInfo = NULL;

                        // Create a 16 bit heap-based buffer descriptor and a
                        // pointer to it
                        HBufC16* label16 = HBufC16::NewLC( KLabelLen );
                        // 16 bit modifiable pointer descriptor to object 
                        // above in order to manipulate data
                        TPtr16 labelPtr16 = label16->Des();
                        
                        labelPtr16.Copy( keyInfo.iLabel.Left(
                                                    keyInfo.iLabel.Length() ) );
                       
						TTime nullTime( _L( "00000000:" ) );
#ifdef __SECURITY_PLATSEC_ARCH__
						TSecurityPolicy usePolicy; 
                        TSecurityPolicy managementPolicy;

                        cctKeyInfo = CCTKeyInfo::NewL( 
                                   keyInfo.iKeyId,      //Key ID
                                   keyUsage,            //Key usage
                                   keyInfo.iLength,     //Key length
                                   NULL,                //protector
                                   label16,             //Key label
                                   iToken,              //token
                                   keyInfo.iKeyNumber,  //Key number
                                   usePolicy,			//
								   managementPolicy,	//
                                   ( CKeyInfoBase::EKeyAlgorithm )keyInfo.iType,
                                   CKeyInfoBase::EInvalidAccess, //bitf. access
                                   ETrue,               //Always Native
                                   nullTime,            //NULL time
                                   nullTime );          //NULL time
#else
                        RArray<TUid> array;

                        cctKeyInfo = CCTKeyInfo::NewL( 
                                   keyInfo.iKeyId,      //Key ID
                                   keyUsage,            //Key usage
                                   keyInfo.iLength,     //Key length
                                   NULL,                //protector
                                   label16,             //Key label
                                   iToken,              //token
                                   keyInfo.iKeyNumber,  //Key number
								   KWimServerUid, //WimServer owns all keys
                                   array,               //Null array
                                   ( CKeyInfoBase::EKeyAlgorithm )keyInfo.iType,
                                   CKeyInfoBase::EInvalidAccess, //bitf. access
                                   ETrue,               //Always Native
                                   nullTime,            //NULL time
                                   nullTime );          //NULL time
#endif

                         _WIMTRACE ( _L( "CWimKeyDetails::RunL(),EGetKeyInfo | CCTKeyInfo is initialized " ) );

                        ret =  iKeys->Append( cctKeyInfo );

                        if ( ret != KErrNone )
                            {
                            CleanupStack::Pop( label16 );
                            cctKeyInfo->Release();
                            User::Leave( ret );
                            }
                       
                         //Append corresponding pinnumber to array.
                        iKeyNumberArray->AppendL( keyInfo.iPinNumber );  
                         
                        //We must not destroy label16, because CCTKeyInfo 
                        //-object takes ownership.
                        CleanupStack::Pop( label16 );
                        //Increase counter, we have successfully fetched 
                        //infos for single key and corresponding CCTKeyInfo 
                        //is created
                        iFetchedKeyInfos++;
                        //Cleanup label and keyid modifiers.
                        DeallocMemoryFromKeyInfo();
                        iPhase = EGetKeyInfo;
                        SignalOwnStatusAndComplete();
                        _WIMTRACE ( _L( "CWimKeyDetails::RunL(),\
                            case:EGetKeyInfo" ) );
                            }
                        }
                    else//Yep, All done.
                        {
                        iFetchedKeyInfos = 0;
                        //Don't need key references anymore
                        iKeyReferences.Close(); 
                        User::RequestComplete( iClientStatus, iStatus.Int() );
                        }
                break;
                }
            case ESign:
                {
                iPhase = ESignCompleted;
                iKeySignParameters.iSigningData = iSigningDataPtr;
                iKeySignParameters.iSignature = iSignaturePtr;
                iKeySignParameters.iKeyId.Copy( iKeyIdPtr->Ptr(), iKeyIdPtr->Length() );
                iConnectionHandle->SignL( iKeySignParameters, iStatus );
                SetActive();
                break;
                }
            case ESignCompleted:
                {
                //signature data is copied by now to plugins memory.   
                iConnectionHandle->DeallocKeySignPckgBuf();
                DeallocMemoryFromSign();
                User::RequestComplete( iClientStatus, iStatus.Int() );
                break;
                }
            case EExportPublicKey:
                {
                iConnectionHandle->DeallocExportPublicKeyPckgBuf();
                DeallocMemoryFromKeyInfo();
                delete iPublicKeyPtr;
                iPublicKeyPtr = NULL;
                User::RequestComplete( iClientStatus, iStatus.Int() );
                break;
                }
            default:
                {
                break;
                }    
            }
        }
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::ConvertKeyUsage()
// Converts keyUsage to new form. All keyUsages loaded from WIM are treated
// as private keys.
// -----------------------------------------------------------------------------
TKeyUsagePKCS15 CWimKeyDetails::ConvertKeyUsage( TUint16 aKeyUsage )
    {
    _WIMTRACE ( _L( "CWimKeyDetails::ConvertKeyUsage()" ) );

    TKeyUsagePKCS15 usage = EPKCS15UsageNone;
    switch ( aKeyUsage ) 
        {
        case KPkcs15KeyUsageFlagsDecrypt:
            {
            usage = EPKCS15UsageDecrypt;
            break;
            }
        case KPkcs15KeyUsageFlagsSign:
            {
            usage = EPKCS15UsageSign;
            break;
            }
        case KPkcs15KeyUsageFlagsSignRecover:
            {
            usage = EPKCS15UsageSignRecover;
            break;
            }
        case KPkcs15KeyUsageFlagsUnwrap:
            {
            usage = EPKCS15UsageUnwrap;
            break;
            }
        case KPkcs15KeyUsageFlagsDerive:
            {
            usage = EPKCS15UsageDerive;
            break;
            }
        case KPkcs15KeyUsageFlagsNonRepudiation:
            {
            usage = EPKCS15UsageNonRepudiation;
            break;
            }
        default:
            {
            break;
            }
        }
    return usage;
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::AllocMemoryForKeyListL()
// Allocates memory for Array which is filled by server.
// -----------------------------------------------------------------------------
void CWimKeyDetails::AllocMemoryForKeyListL()
    {
    _WIMTRACE ( _L( "CWimKeyDetails::AllocMemoryForKeyListL()" ) );

    iKeyList = HBufC8::NewL( KLabelLen );
    iKeyListPtr = new( ELeave ) TPtr8( iKeyList->Des() );
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::DeallocMemoryFromKeyList()
// Deallocates memory from Array which was filled by server.
// -----------------------------------------------------------------------------
void CWimKeyDetails::DeallocMemoryFromKeyList()
    {
    _WIMTRACE ( _L( "CWimKeyDetails::DeallocMemoryFromKeyList()" ) );

    delete iKeyList;
    delete iKeyListPtr;
    iKeyList = NULL;
    iKeyListPtr = NULL;
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::ConvertKeyListL()
// Converts keylist parameters. Extracts data out from keylist & keynumber.
// Extracted data is inserted to RArray.
// -----------------------------------------------------------------------------
void CWimKeyDetails::ConvertKeyListL()
    {
    _WIMTRACE ( _L( "CWimKeyDetails::ConvertKeyListL()" ) );

    TInt32 output = 0;
    
    TLex8 lex8( iKeyListPtr->Ptr() );
    for ( TInt i = 0; i < iKeyNumber; i++ ) //Put keyreferences to array.
        {
        lex8.SkipSpaceAndMark();
        lex8.Val( output );
        User::LeaveIfError( iKeyReferences.Append( output ) );
        }
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::AllocMemoryForKeyInfoL()
// Allocates memory for label and keyid.
// -----------------------------------------------------------------------------
void CWimKeyDetails::AllocMemoryForKeyInfoL( TKeyInfo& aKeyInfo )
    {
    _WIMTRACE ( _L( "CWimKeyDetails::AllocMemoryForKeyInfoL()" ) );

    iLabel = HBufC8::NewL( KLabelLen ); //Label
    iLabelPtr = new( ELeave ) TPtr8( iLabel->Des() );
    aKeyInfo.iLabel.Copy( iLabelPtr->Ptr(), iLabelPtr->Length() );
   
    iKeyId = HBufC8::NewL( KKeyIdLen ); //KeyId
    iKeyIdPtr = new( ELeave ) TPtr8( iKeyId->Des() );
    aKeyInfo.iKeyId.Copy( iKeyIdPtr->Ptr(), iKeyIdPtr->Length() ); 
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::DeallocMemoryFromKeyInfo()
// Deallocates memory from label and keyid.
// -----------------------------------------------------------------------------
void CWimKeyDetails::DeallocMemoryFromKeyInfo()
    {
    _WIMTRACE ( _L( "CWimKeyDetails::DeallocMemoryFromKeyInfo()" ) );

    delete iLabel;
    delete iLabelPtr;
    delete iKeyId;
    delete iKeyIdPtr;
    iLabel = NULL;
    iLabelPtr = NULL;
    iKeyId = NULL;
    iKeyIdPtr = NULL;
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::RunError()                                        
// The active scheduler calls this function if this active object's RunL() 
// function leaves. This gives this active object the opportunity to perform 
// any necessary cleanup.
// After array's cleanup, complete request with received error code.
// -----------------------------------------------------------------------------
TInt CWimKeyDetails::RunError( TInt aError )
    {  
    _WIMTRACE ( _L( "CWimKeyDetails::RunError()" ) );
    DeallocMemoryFromKeyInfo();
    DeallocMemoryFromKeyList();
    User::RequestComplete( iClientStatus, aError );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CWimKeyDetails::DoCancel()
// Deallocates member variables and completes client status with
// KErrCancel error code.
// -----------------------------------------------------------------------------
void CWimKeyDetails::DoCancel()
    {    
    _WIMTRACE ( _L( "CWimKeyDetails::DoCancel()" ) );

    if ( iConnectionHandle && iPhase == ESignCompleted )
        {
        iConnectionHandle->DeallocKeySignPckgBuf();
        }
    else if ( iConnectionHandle && iPhase == EExportPublicKey )
        {
        delete iPublicKeyPtr;
        iPublicKey = NULL;
        iPublicKeyPtr = NULL;
        iConnectionHandle->DeallocExportPublicKeyPckgBuf();
        }
    DeallocMemoryFromKeyList();
    DeallocMemoryFromKeyInfo();
    DeallocMemoryFromSign();
    iKeyReferences.Close();
    User::RequestComplete( iClientStatus, KErrCancel );
    }

// End of File
