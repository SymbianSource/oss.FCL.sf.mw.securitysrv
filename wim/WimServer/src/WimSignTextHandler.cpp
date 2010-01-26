/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Services for Sign Text operation
*
*/



// INCLUDE FILES
#include    "WimServer.h"
#include    "Wimi.h"            // WIMI definitions
#include    "WimClsv.h"
#include    "WimSignTextHandler.h"
#include    "WimSecurityDlgHandler.h"
#include    "WimCallbackImpl.h"
#include    "WimTrace.h"
#include    "WimCleanup.h"
#include    <secdlg.h>

#ifdef _DEBUG
_LIT( KWimSignTextPanic, "WimSignText" );
#endif

//In secdlg.h Maxlength is defined as 32, which disobeies WIM standard.
//According to WIM standard Pin length should be 4~8
const TInt KWIMMaxPINLength = 8;
// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimSignTextHandler::CWimSignTextHandler
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimSignTextHandler::CWimSignTextHandler() : CActive( EPriorityStandard )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSignTextHandler::CWimSignTextHandler | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimSignTextHandler::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimSignTextHandler::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSignTextHandler::ConstructL | Begin"));
    CActiveScheduler::Add( this );
    iWimUtilFuncs = CWimUtilityFuncs::NewL();
    iWimSecDlg = CWimSecurityDlgHandler::NewL();
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimSignTextHandler* CWimSignTextHandler::NewL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSignTextHandler::NewL | Begin"));
    CWimSignTextHandler* self = new( ELeave ) CWimSignTextHandler;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// Destructor
CWimSignTextHandler::~CWimSignTextHandler()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSignTextHandler::~CWimAuthObjHandler | Begin"));
    Cancel();
    CleanUp();      // frees iSigningDataBuf, iSigningDataPtr, and iKeyReference
    delete iWimUtilFuncs;
    delete iWimSecDlg;
    // iResponseID and iTrId are deleted in callback functions
    }

// -----------------------------------------------------------------------------
// CWimSignTextHandler::SignTextL
// Sign given text. Response is handled in CWimCallback if operation is given
// to card. Otherwise message is completed here with error value.
// -----------------------------------------------------------------------------
//
void CWimSignTextHandler::SignTextL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSignTextHandler::SignTextL | Begin"));

    __ASSERT_DEBUG( iResponseID == NULL, User::Panic( KWimSignTextPanic, KErrGeneral ) );
    iResponseID = new( ELeave ) CWimResponse( aMessage );
    iResponseID->iOpcode = ESignTextReq;

    __ASSERT_DEBUG( iTrId == NULL, User::Panic( KWimSignTextPanic, KErrGeneral ) );
    iTrId = iWimUtilFuncs->TrIdLC( iResponseID, EWimMgmtReq );
    CleanupStack::Pop( iTrId );

    TPckgBuf<TKeySignParameters> signPckg;
    aMessage.ReadL( 1, signPckg );

    // Data to be signed
    iSigningDataBuf = iWimUtilFuncs->DesLC( 2, aMessage );
    CleanupStack::Pop( iSigningDataBuf );
    
    iSigningDataPtr = new( ELeave ) TPtr8( iSigningDataBuf->Des() );
    
    // Key ID
    TBuf8<KKeyIdLen> keyIdBuf = signPckg().iKeyId;
    TPtr8 keyIdHash( const_cast<TUint8*>( keyIdBuf.Ptr() ), keyIdBuf.Length() );
    WIMI_Ref_t* keyRef;
    WIMI_GetKeyByHash( ( TUint8* )keyIdHash.Ptr(), &keyRef );
    iKeyReference = ( TAny* )keyRef; // Key reference

    iRetry = EFalse;
    GetPinParamsL( iPinParams ); // Get PIN parameters

    AskPin(); // Ask PIN
    }

// -----------------------------------------------------------------------------
// CWimSignTextHandler::AskPin
// Ask PIN from user
// -----------------------------------------------------------------------------
//
void CWimSignTextHandler::AskPin()
    {
    TBool IsWIMOpen = EFalse;
    iSigningState = EAskPin;
    iStatus = KRequestPending;
    SetActive();
    
   	_WIMTRACE(_L("WIM | WIMServer | CWimSignTextHandler::AskPin "));
    WIMI_Ref_pt pWimRefTemp = NULL;
    pWimRefTemp = WIMI_GetWIMRef( 0 );
 
    if ( pWimRefTemp )  // Close the WIM
       {
       if ( WIMI_IsWIMOpened( pWimRefTemp ) )
           {
       	   IsWIMOpen = ETrue;
           }
       free_WIMI_Ref_t( pWimRefTemp );
       }
    
    if( IsWIMOpen )
    	{
    	TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
    	}
    else
    	{
    	iWimSecDlg->EnterPIN( iRetry,
        	                  iPinParams,
        	                  iSigningPin, 
        	                  iStatus );	
    	}
    }

// -----------------------------------------------------------------------------
// CWimSignTextHandler::GetPinParamsL
// Get parameters for PIN. iKeyReferense has to be set before this function
// is used.
// -----------------------------------------------------------------------------
//
void CWimSignTextHandler::GetPinParamsL( TPINParams& aPinParams ) const
    {
    TInt pushed = 0;
    TPINLabel pinLabel;    // The label that identifies the PIN
	TPINLabel tokenLabel;  // The label of the token
	TUint8 minLength = 0;  // The minimum length of the PIN

    WIMI_STAT status = WIMI_Ok;

    // WIMI_GetKeyInfo()
    WIMI_Ref_t* pWimRef;    
    TUint8 keyType;
    TUint8 keyNumber;
    TUint8 pinNumber;
    TUint16 usage;
    TUint16 keyLength;
    WIMI_BinData_t ptKeyLabel;
    WIMI_BinData_t ptKeyId;
    // WIMI_GetPINList()
    WIMI_Ref_pt pPinRef = NULL;
    TUint16 pinNum;
    WIMI_RefList_t pinRefLst;
    // WIMI_GetPINStatus()
    TUint8 flags;
    WIMI_BinData_t ptPinLabel;
    WIMI_Ref_t* cmeWimRef = NULL;
    // WIMI_GetWIMInfo()
    TUint16 wimFlags;
    TUint8 seSet;
    TUint8 version;
    WIMI_BinData_t ptWimID;
    WIMI_BinData_t ptManufacturerID;
    WIMI_BinData_t ptWimLabel;
    TUint8 reader = 0; 
    TBool sim; 

    // Get PIN number by Key reference
    status = WIMI_GetKeyInfo( iKeyReference,
                              &pWimRef,
                              NULL,
                              &keyType,
                              &keyNumber,
                              &pinNumber,
                              &usage,
                              &ptKeyId,
                              &ptKeyLabel,
                              &keyLength );
    if ( status != WIMI_Ok )
        {
        User::Leave( CWimUtilityFuncs::MapWIMError( status ) );
        }

    // TODO: leaving PushL functions should not be used

    CleanupPushWimRefL( pWimRef );
    pushed++;
    CleanupPushWimBufL( ptKeyId );
    pushed++;
    CleanupPushWimBufL( ptKeyLabel );
    pushed++;
    
    // Get PIN references
    status = WIMI_GetPINList( pWimRef, &pinNum, &pinRefLst );
    if ( status == WIMI_Ok )
        {
        // Select right reference by PIN number
        pPinRef = pinRefLst[pinNumber];
        }
    CleanupPushWimRefListL( pinRefLst );
    pushed++;

    if ( pPinRef )
        {
        // Get PIN info (label, min length etc.)
        status = WIMI_GetPINStatus( pPinRef,
                                    &cmeWimRef,
                                    &flags,
                                    &minLength,
                                    &pinNumber,
                                    &ptPinLabel );
        if ( status != WIMI_Ok )
            {
            User::Leave( CWimUtilityFuncs::MapWIMError( status ) );
            }
        CleanupPushWimRefL( cmeWimRef );
        pushed++;
        CleanupPushWimBufL( ptPinLabel );
        pushed++;

        HBufC8* pinLabelBuf = HBufC8::NewLC( ptPinLabel.ui_buf_length );
        pushed++;

        TPtr8 pinPtr = pinLabelBuf->Des();
        pinPtr.Copy( ptPinLabel.pb_buf, ptPinLabel.ui_buf_length );
        pinLabel.Copy( pinPtr );
        }
    else
        {
        User::Leave( KErrArgument );
        }

    // Get token label
    status = WIMI_GetWIMInfo( cmeWimRef,
                              &wimFlags,
                              &seSet,
                              &ptWimID,
                              &ptManufacturerID,
                              &ptWimLabel,
                              &reader, 
                              &pPinRef, 
                              &sim, 
                              &version );
    if ( status != WIMI_Ok )
        {
        User::Leave( CWimUtilityFuncs::MapWIMError( status ) );
        }
    CleanupPushWimBufL( ptWimLabel );
    pushed++;
    CleanupPushWimBufL( ptWimID );
    pushed++;
    CleanupPushWimBufL( ptManufacturerID );
    pushed++;

    HBufC8* wimLabelBuf = HBufC8::NewLC( ptWimID.ui_buf_length );
    pushed++;

    TPtr8 wimLabelPtr( wimLabelBuf->Des() );
    wimLabelPtr.Copy( ptWimLabel.pb_buf, ptWimLabel.ui_buf_length );

    tokenLabel.Copy( wimLabelPtr ); // Copy token label
    
    aPinParams.iPINLabel = pinLabel;
    aPinParams.iTokenLabel = tokenLabel;
    aPinParams.iMinLength = minLength;
    aPinParams.iMaxLength = KWIMMaxPINLength; // Use max value

    CleanupStack::PopAndDestroy( pushed, pWimRef );
    }

// -----------------------------------------------------------------------------
// CWimSignTextHandler::ContinueSigningL
// Continues signing operation after signing PIN has asked from user.
// Call WIMI and return error code in iStatus in case digital signature
// can not be done e.g. wrong PIN has entered.
// -----------------------------------------------------------------------------
//
void CWimSignTextHandler::ContinueSigningL()
    {
    _WIMTRACE(_L("WIM|WIMServer|CWimSignTextHandler::ContinueSigningL|Begin"));

    WIMI_BinData_t pPin;

    // Copy signing PIN to 8 bit buffer
    HBufC8* signingPinBuf = HBufC8::NewLC( iSigningPin.Length() );
    TPtr8 signingPin = signingPinBuf->Des();
    signingPin.Copy( iSigningPin );
    pPin.pb_buf = ( TUint8* )signingPin.Ptr();
    pPin.ui_buf_length = ( TUint16 )signingPin.Length();

    iStatus = KRequestPending;
    TRequestStatus* status = &iStatus;
    iSigningState = ECallbackResponse;
    SetActive();

    iResponseID->iStatus = WIMI_SignReq( iTrId,
                                         ( TUint8* )iSigningDataPtr->Ptr(),
                                         ( TUint8 )iSigningDataPtr->Length(),
                                         iKeyReference,
                                         &pPin );

    CleanupStack::PopAndDestroy( signingPinBuf );
    
    // Some error in WIMI (no connection to card etc.) 
    // Request not completed by callback function
    if ( iResponseID->iStatus != WIMI_Ok )
        {
        iSigningState = EWimiError;
        User::RequestComplete( status, iResponseID->iStatus );
        }
    }

// -----------------------------------------------------------------------------
// CWimSignTextHandler::CleanUp
// Clean up member data
// -----------------------------------------------------------------------------
//
void CWimSignTextHandler::CleanUp()
    {
    if( iSigningDataBuf )
        {
        delete iSigningDataBuf;
        iSigningDataBuf = NULL;	
        }
    
    if( iSigningDataPtr )
        {
        delete iSigningDataPtr;
        iSigningDataPtr = NULL;	
        }
    
    WIMI_Ref_t* keyRef = iKeyReference;
    free_WIMI_Ref_t( keyRef );
    }

// -----------------------------------------------------------------------------
// CWimSignTextHandler::RunL
// Handle digital signature operation as state machine. Handle different
// states here.
// -----------------------------------------------------------------------------
//
void CWimSignTextHandler::RunL()
    {
    _WIMTRACE2(_L("WIM|WIMServer|CWimSignTextHandler::RunL|status=%d"), iStatus.Int());

    switch ( iSigningState )
        {
        // PIN asked
        case EAskPin:
            {       
            iRetry = ETrue; // Next is retry

            if ( iStatus.Int() != KErrNone ) // Cancelled or other error
                {
                if( iResponseID )
                    {
                    if( iStatus.Int() == KErrCancel )
	                	{
	                	iResponseID->iStatus = WIMI_ERR_UserCancelled;
	                	}
                    else
	                	{
	                	iResponseID->iStatus = ( TUint16 )iStatus.Int();	
	                	}
                    iResponseID->CompleteMsgAndDelete();
                    iResponseID = NULL;	
                    }
                
                if( iTrId )
                    {
                    delete iTrId;
                    iTrId = NULL;	
                    }
                
                CleanUp();
                }
            else  // User entered PIN without errors
                {
                CWimCallBack::SetSignTextRequestStatus( &iStatus );
                ContinueSigningL(); // Continue digital signature operation
                }
            break;
            }
        
        // We come here from CWimCallBack::SignResp()
        case ECallbackResponse:
            {
            CWimCallBack::SetSignTextRequestStatus( NULL );

            if ( iStatus.Int() == WIMI_ERR_BadPIN ) // Wrong PIN, continue
                {
                AskPin(); // Ask PIN again and continue
                }
            // If PIN is blocked show note
            else if ( iStatus.Int() == WIMI_ERR_PINBlocked )
                {
                iSigningState = EShowPinBlocked;
                iWimSecDlg->ShowPINBlocked( iPinParams, iStatus );
                SetActive();
                }
             else if ( iStatus.Int() == WIMI_ERR_CardIOError )
                    {
                    iSigningState = EShowCardIsRemoved;
	                  iStatus = KRequestPending;
                    TRequestStatus* status = &iStatus;
                    SetActive();
                    User::RequestComplete( status, KErrNone );
                	  }
            else // Response completed in callbacks, just finalize operation 
                {
                iStatus = KRequestPending;
                iSigningState = ESigningDone;
                TRequestStatus* status = &iStatus;
                SetActive();
                User::RequestComplete( status, KErrNone );
                }
            break;
            }

        // PIN Blocked note showed to user
        case EShowPinBlocked:
            {
            // Complete message here with KErrLocked
            if( iResponseID )
                {
                iResponseID->iStatus = WIMI_ERR_PINBlocked;
                iResponseID->CompleteMsgAndDelete();
                iResponseID = NULL;	
                }
            
            if( iTrId )
                {
                delete iTrId;
                iTrId = NULL;	
                }
            
            iStatus = KRequestPending;
            iSigningState = ESigningDone;
            TRequestStatus* status = &iStatus;
            SetActive();
            User::RequestComplete( status, KErrNone );
            break;
            }
        case EShowCardIsRemoved:
               {
               if( iResponseID )
                        {
                        iResponseID->iStatus = WIMI_ERR_CardIOError;
                    	iResponseID->CompleteMsgAndDelete();
                    	iResponseID = NULL;	
                        }
                	if( iTrId )
                		{
                		delete iTrId;
                    	iTrId = NULL;	
                		}
               
               iStatus = KRequestPending;
               iSigningState = ESigningDone;
               TRequestStatus* status = &iStatus;
               SetActive();
               User::RequestComplete( status, KErrNone );
               break;
               }            
        // WIMI error. Message is not completed in callback function.
        // So, complete it here
        case EWimiError:
            {
            CWimCallBack::SetSignTextRequestStatus( NULL );
            
            if( iResponseID )
                {
                iResponseID->iStatus = ( TUint16 )iStatus.Int();
                iResponseID->CompleteMsgAndDelete();
                iResponseID = NULL;	
                }
            if ( iTrId )
                {
                delete iTrId;
                iTrId = NULL;	
                }
           
            CleanUp();
            break;
            }
        
        // Text is signed, handle response in callback functions
        case ESigningDone:
            {
            CleanUp();
            iResponseID = NULL;
            iTrId = NULL;
            break;
            }    

        default:
            {
            CleanUp();
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CWimSignTextHandler::DoCancel
// Asyncronous call cancelled.
// -----------------------------------------------------------------------------
//
void CWimSignTextHandler::DoCancel()
    {
    _WIMTRACE(_L("WIM|WIMServer|CWimSignTextHandler::DoCancel|Begin"));
    }

// -----------------------------------------------------------------------------
// CWimSignTextHandler::RunError
// RunL leaved, handle error here.
// -----------------------------------------------------------------------------
//
TInt CWimSignTextHandler::RunError( TInt aError )
    {
    CWimCallBack::SetSignTextRequestStatus( NULL );
    if( iResponseID )
        {
        iResponseID->iStatus = ( TUint16 )aError;
        iResponseID->CompleteMsgAndDelete();
        iResponseID = NULL;	
        }
    if( iTrId )
        {
   	    delete iTrId;
        iTrId = NULL;
        }
    
    CleanUp();
    return KErrNone;
    }

//  End of File  
