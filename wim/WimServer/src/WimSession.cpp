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
* Description:  Session class for WimServer
*
*/



// INCLUDE FILES
#include    "WimSession.h"
#include    "WimMemMgmt.h"
#include    "WimCertHandler.h"
#include    "WimAuthObjHandler.h"
#include    "WimTokenHandler.h"
#include    "WimKeyMgmtHandler.h"
#include    "WimSignTextHandler.h"
#include    "WimServer.h"
#include    "WimResponse.h"
#include    "WimCallbackImpl.h"
#include    "WimUtilityFuncs.h"
#include    "WimSessionRegistry.h"
#include    "WimOmaProvisioning.h"
#include    "WimJavaProvisioning.h"
#include    "WimTrustSettingsHandler.h"
#include    "WimTrace.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimSession::CWimSession
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimSession::CWimSession() : CSession2()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSession::CWimSession | Begin"));
    __DECLARE_NAME( _S( "CWimSession" ) );
    }

// -----------------------------------------------------------------------------
// CWimSession::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimSession::ConstructL( CWimServer* aWimServer )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSession::ConstructL | Begin"));
    iWimSvr = aWimServer;
#ifdef WIMSERVER_SHUTDOWN    
    iWimSvr->AddSession();
#endif    
    iWimMgmt = CWimMemMgmt::NewL();
    iWimCertHandler = CWimCertHandler::NewL();
    iWimAuthObjHandler = CWimAuthObjHandler::NewL();
    iWimTokenHandler = CWimTokenHandler::NewL();
    iWimKeyMgmtHandler = CWimKeyMgmtHandler::NewL();
    iWimSignTextHandler = CWimSignTextHandler::NewL();
    //iTimer = CWimTimer::NewL( this );
    iWimUtilFuncs = CWimUtilityFuncs::NewL();
    iWimOmaProvisioning = CWimOmaProvisioning::NewL();
    iWimJavaProvisioning = CWimJavaProvisioning::NewL();
    iWimTrustSettingsHandler = CWimTrustSettingsHandler::NewL( iWimSvr->WimTrustSettingsStore() );
    }

// -----------------------------------------------------------------------------
// CWimSession::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimSession* CWimSession::NewL( CWimServer* aWimServer )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSession::NewL | Begin"));
    CWimSession* self = new( ELeave ) CWimSession();
    CleanupStack::PushL( self );
    self->ConstructL( aWimServer );
    CleanupStack::Pop( self );
    return self;
    }

// Destructor
CWimSession::~CWimSession()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSession::~CWimSession | Begin"));
    
#ifdef WIMSERVER_SHUTDOWN   
    if ( iWimSvr )
        {
        iWimSvr->DropSession();	
    	  } 
#endif    
    //Check if message is not completed before session is desctructed
    if ( iMessage )
        {
        iMessage->Complete( KErrCancel );
        delete iMessage;
        }

    if ( iNotifyMessage )   // NotifyOnRemoval message pending
        {
        iNotifyMessage->Complete( KErrCancel );
        delete iNotifyMessage;
        }

    // Delete sessions from session registry
    if ( iWimSvr )
        {
        iWimSvr->WimSessionRegistry()->RemoveSession( this );
    	}

    delete iWimMgmt;
    //delete iTimer;
    delete iWimCertHandler;
    delete iWimAuthObjHandler;
    delete iWimTokenHandler;
    delete iWimKeyMgmtHandler;
    delete iWimSignTextHandler;
    delete iWimUtilFuncs;
    delete iWimOmaProvisioning;
    delete iWimJavaProvisioning;
    delete iWimTrustSettingsHandler;
    }

// -----------------------------------------------------------------------------
// CWimSession::ServiceL
// Handles the servicing of client requests to the server.
// -----------------------------------------------------------------------------
//
void CWimSession::ServiceL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSession::ServiceL | Begin"));

    iMessage = new( ELeave ) RMessage2( aMessage );

    TRAPD( appErr, DispatchMessageL( aMessage ) );
    // The error is an application error 
    // If the appErr is '0' or if the appErr is WIM_Error 
    // then it is completed by the called functions or by
    // the callback functions
    if ( appErr )
        {
        _WIMTRACE2(_L("WIM | WIMServer | CWimSession::ServiceL | Leaved with %d"), appErr);
        aMessage.Complete( appErr );
        }

    // Message is completed so delete copy of message
    delete iMessage;
    iMessage = NULL;
    }

// -----------------------------------------------------------------------------
// CWimSession::DispatchMessageL
// Handles client requests and forwards them to appropriate handler class.
// -----------------------------------------------------------------------------
//
void CWimSession::DispatchMessageL( const RMessage2& aMessage )
    {
    _WIMTRACE2(_L("WIM|WIMServer|CWimSession::DispatchMessageL|Begin, Function=%d"), aMessage.Function());
    // Check if the WIM has been initialized properly. If not just complete
    // message with WimServer status and return
    if ( CWimServer::iWimStatus != KErrNone
        && aMessage.Function() != EWimInitialize
        && aMessage.Function() != ECancelNotifyOnRemoval )
        {
        aMessage.Complete( CWimServer::iWimStatus );
        return;
        }

	// Are we going to card? (blocks sim refresh until done)
	if ( RequestAccessesHW( aMessage.Function() ))
	    {
	    iWimSvr->SetIsAccessingToken( ETrue );
	    }

    _WIMTRACE2( _L( "WIM|WIMServer|CWimSession::DispatchMessageL|Begin, heap=%d" ),
                User::CountAllocCells() ); 
#ifdef __WIM_ENABLE_TRACES
	TTime funcStart;
	TTime funcEnd;
	funcStart.UniversalTime();
#endif // __WIM_ENABLE_TRACES

    switch ( aMessage.Function() )
        {
//==================== WIM operations ==========================================
        case EWimInitialize:
            _WIMTRACE2(_L("WIM|WIMServer|CWimSession::DispatchMessageL|EWimInitialize, iWimInitialized=%d"), CWimServer::iWimInitialized);
            iWimSvr->WimInitialize( aMessage );
            break;
        case ECancelWimInitialize:
            iWimSvr->CancelWimInitialize( aMessage );  
            break; 
        case EGetWIMCount:
            GetWimCountL();
            break;
        case EGetWIMRefs:
            GetWimRefListL();
            break;

//==================== WIM card management =====================================
        case EGetWIMInfo:
            iWimSvr->WimTimer()->ResetTimer();
            iWimTokenHandler->GetWIMInfoL( aMessage, iWimMgmt );
            break;
        case EIsWIMOpen:
            iWimTokenHandler->IsWIMOpenL( aMessage, iWimSvr->WimTimer(), iWimMgmt );
            break;
        case ECloseWIMAfter:
            iWimSvr->WimTimer()->SetCloseAfter( aMessage );
            break;
        case EGetCloseWIMAfter:
            iWimSvr->WimTimer()->GetCloseAfterL( aMessage );
            break;
        case EWimTimeRemaining:
            iWimSvr->WimTimer()->TimeRemainingL( aMessage );
            break;
        case EWIMClose:
            iWimTokenHandler->CloseWIM( aMessage );
            break;

//==================== Notify on removal =======================================
        case ENotifyOnRemoval:
            NotifyOnRemovalL();
            break;
        case ECancelNotifyOnRemoval:
            CancelNotifyOnRemoval();
            break;

//==================== WIM Reference handling ==================================
        case EFreeMemory:
            iWimMgmt->FreeRef( ( WIMI_Ref_pt* )aMessage.Ptr0() );  
            aMessage.Complete( KErrNone );
            break;
        case EFreeWIMMemoryLst:
            { 
            TUint wimCount = aMessage.Int1(); 
    
            WIMI_Ref_t* temp = NULL;
            TUint8 wimIndex = 0 ;
            for ( ; wimIndex < KWimMaxCount && ( temp = WIMI_GetWIMRef( wimIndex ) ) != NULL; wimIndex++ )
                {
                free_WIMI_Ref_t( temp );
                }

            if ( wimCount != wimIndex )
               {
               User::LeaveIfError( KErrArgument );
               }
    
            iWimMgmt->FreeWIMRefs( aMessage );  
            break;
            }
        case EFreeMemoryLst:
            iWimMgmt->FreeRefLst( aMessage );
            break;

//==================== PIN operations ==========================================
        case EGetPINCount:
            iWimSvr->WimTimer()->ResetTimer();
            iWimAuthObjHandler->GetPINCountL( aMessage );
            break;
        case EGetPINRefs:
            iWimSvr->WimTimer()->ResetTimer();
            iWimAuthObjHandler->GetPINRefListL( aMessage, iWimMgmt );
            break;
        case EGetPINInfo:
            iWimSvr->WimTimer()->ResetTimer();
            iWimAuthObjHandler->GetPINInfoL( aMessage );
            break;
        case EGetPINsInfo:
            iWimSvr->WimTimer()->ResetTimer();
            iWimAuthObjHandler->GetPINsInfoL( aMessage ); 
            break;
        case EIsPinBlocked:
            iWimAuthObjHandler->VerifyPINRequestL( aMessage, EFalse );
            break;
        case EIsDisabledPinBlocked:
            iWimAuthObjHandler->VerifyDisabledPINRequestL( aMessage );
            break;
        case EChangePINReq:
            iWimSvr->WimTimer()->ResetTimer();
            iWimAuthObjHandler->ChangePINRequestL( aMessage );
            break;
        case ECancelChangePin:
            iWimAuthObjHandler->CancelChangePin( aMessage );
            break;
        case EEnablePINReq:
            iWimSvr->WimTimer()->ResetTimer();
            iWimAuthObjHandler->EnablePINReqL( aMessage );
            break;
        case ECancelEnablePin:
            iWimAuthObjHandler->CancelEnablePin( aMessage );
            break;
        case EUnblockPinReq:
            iWimSvr->WimTimer()->ResetTimer();
            iWimAuthObjHandler->UnblockPinReqL( aMessage );
            break;
        case ECancelUnblockPin:
            iWimAuthObjHandler->CancelUnblockPin( aMessage );
            break;
        case EVerifyPinReq:
            iWimSvr->WimTimer()->ResetTimer();
            iWimAuthObjHandler->VerifyPINRequestL( aMessage, ETrue );
            break;
        case ECancelVerifyPin:
            iWimAuthObjHandler->CancelVerifyPin( aMessage );
            break;
        case ERetrieveAuthObjsInfo:
            iWimAuthObjHandler->RetrieveAuthObjectsInfo( aMessage ); 
            break;

//==================== Key operations ==========================================
        case EGetKeyDetails:
            iWimKeyMgmtHandler->GetKeyDetailsL( aMessage );
            break;
        case EGetKeyList:
            iWimKeyMgmtHandler->GetKeyListL( aMessage, iWimMgmt );
            break;
        case EDoesPvKeyExist:
            iWimKeyMgmtHandler->DoesKeyExistL( aMessage );
            break; 

//==================== Certificate Management ==================================
        case EGetWIMCertLst:
            iWimCertHandler->GetCertificatesFromWimL( aMessage, iWimMgmt );
            break;
        case EGetCertCount:
            iWimCertHandler->GetCerticateCountL( aMessage, iWimMgmt );
            break;
        case EGetWIMCertDetails:
            iWimCertHandler->GetCertificateDetailsL( EGetWIMCertDetails,
                                                     aMessage );
            break;
        case EGetCertExtras:    // Get cert extra data (e.g. trusted usage)
            iWimCertHandler->GetExtrasFromWimL( aMessage, iWimMgmt );
            break;
        case EStoreCertificate:
            iWimCertHandler->StoreCertificateL( EStoreCertificate, aMessage );
            break;
        case ERemoveCertificate:
            iWimCertHandler->RemoveCertificateL( aMessage, iWimMgmt );
            break;
        case EExportPublicKey:
            iWimCertHandler->ExportPublicKeyL( aMessage );
            break;

//==================== Digital signature =======================================
        case ESignTextReq:
            iWimSignTextHandler->SignTextL( aMessage );
            break;

//==================== Java Provisioning ========================================
        case EGetACIFFileSize:
            iWimJavaProvisioning->ACIFFileSizeL( aMessage );
            break;
        case EGetACIFFile:
            iWimJavaProvisioning->ACIFFileContentL( aMessage );
            break;
        case EGetACFFileSize:
            iWimJavaProvisioning->ACFFileSizeL( aMessage );
            break;
        case EGetACFFile:
            iWimJavaProvisioning->ACFFileContentL( aMessage );
            break;    
        case EGetLabelAndPath:
            iWimJavaProvisioning->LabelAndPath( aMessage );
            break;
            
//==================== OMA Provisioning ========================================
        case EGetOMAFileSize:
            iWimOmaProvisioning->OmaFileSizeL( aMessage );
            break;
        case EGetOMAFile:
            iWimOmaProvisioning->OmaFileContentL( aMessage );
            break;            

//==================== Trust Settings  =========================================
        case EGetTrustSettings:
            iWimTrustSettingsHandler->GetTrustSettingsL( aMessage );
            break;
        case ESetApplicability: 
            iWimTrustSettingsHandler->SetApplicabilityL( aMessage );
            break;
        case ESetTrust:
            iWimTrustSettingsHandler->SetTrustL( aMessage );
            break;
        case ESetDefaultTrustSettings:
            iWimTrustSettingsHandler->SetDefaultTrustSettingsL( aMessage );
            break;
        case ERemoveTrustSettings:
            iWimTrustSettingsHandler->RemoveTrustSettingsL( aMessage );
            break;
        case ECancelTrustSettings:
            iWimTrustSettingsHandler->CancelDoing();
            break;

//==================== Opcode not supported ====================================
        default:
            aMessage.Complete( KErrArgument );
            break;
        }
#ifdef __WIM_ENABLE_TRACES
    funcEnd.UniversalTime();
    TTimeIntervalMicroSeconds funcTime = funcEnd.MicroSecondsFrom( funcStart );
    _WIMTRACE3( _L("WIMServer function %d, time %Ld us"), aMessage.Function(), funcTime.Int64() );
	
    TInt allocSize = 0;
    TInt allocCount = User::AllocSize( allocSize );
    _WIMTRACE3( _L( "WIM|WIMServer|CWimSession::DispatchMessageL|End, heap=%d, heapsize=%d" ), allocCount, allocSize );
#endif // __WIM_ENABLE_TRACES

    // Assuming that WIMI calls are asynchronous
    iWimSvr->SetIsAccessingToken( EFalse ); // HW access complete

    // Has the card data been modified during service? -> re-init
    if ( iWimSvr->RefreshNotificationReceived() )
        {
        iWimSvr->SetRefreshNotificationReceived( EFalse );
        RefreshWimi();
        }
    }

// -----------------------------------------------------------------------------
// CWimSession::GetWimRefListL
// Fetches the list of the references of WIMs currently associated with readers.
// -----------------------------------------------------------------------------
//
void CWimSession::GetWimRefListL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSession::GetWIMRefListL | Begin"));
    WIMI_Ref_pt pWimRefTemp = NULL;

    HBufC8* buf = iWimUtilFuncs->DesLC( 0, *iMessage );
    TUint32* wimRef =  ( TUint32* )( buf->Des().Ptr() );
    TUint8 wimIndex = 0;
    for ( ; wimIndex < KWimMaxCount && 
            ( pWimRefTemp = WIMI_GetWIMRef( wimIndex ) ) != NULL; wimIndex++ )
        {
        wimRef[wimIndex] = ( TUint32 )pWimRefTemp;
        iWimMgmt->AppendWIMRefL( pWimRefTemp );     // takes ownership
        }
    iMessage->WriteL( 0, buf->Des() );
    CleanupStack::PopAndDestroy( buf );

    iMessage->Complete( KErrNone );
    }

// -----------------------------------------------------------------------------
// CWimSession::GetWimCountL
// Fetches the count of WIM cards in use.
// -----------------------------------------------------------------------------
//
void CWimSession::GetWimCountL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSession::GetWimCountL | Begin"));
    WIMI_Ref_t* temp = NULL;
    TUint8 wimIndex = 0 ;
    for ( ; wimIndex < KWimMaxCount &&
        ( temp = WIMI_GetWIMRef( wimIndex ) ) != NULL; wimIndex++ )
        {
        free_WIMI_Ref_t( temp );
        }

    TPckgBuf<TUint> pckg( wimIndex );
    iMessage->WriteL( 0, pckg );
    iMessage->Complete( KErrNone );
    }

// -----------------------------------------------------------------------------
// CWimSession::TimerExpired
// Timer for closing WIM card. WIM is opened again with EVerifyPinReq after
// new initialization.
// -----------------------------------------------------------------------------
//
void CWimSession::TimerExpired()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSession::TimerExpired | Begin"));
    WIMI_Ref_pt pWimRefTemp = NULL;

    pWimRefTemp = WIMI_GetWIMRef( 0 );
 
    if ( pWimRefTemp )  // Close the WIM
        {
        WIMI_CloseWIM( pWimRefTemp );
        free_WIMI_Ref_t( pWimRefTemp );
        }
    CWimServer::SetWimInitialized( EFalse, KErrNone );
    }

// -----------------------------------------------------------------------------
// CWimSession::NotifyOnRemoval
// Notify on card removal. Store message for completing.
// -----------------------------------------------------------------------------
//
void CWimSession::NotifyOnRemovalL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSession::NotifyOnRemoval | Begin"));
    if ( iNotifyMessage ) // Message already pending
        {
        _WIMTRACE(_L("WIM|WIMServer|CWimSession::NotifyOnRemoval|KErrAlreadyExists"));
        iMessage->Complete( KErrAlreadyExists );
        return;
        }
    iNotifyMessage = new( ELeave ) RMessage2( *iMessage ); // Store message
    _WIMTRACE(_L("WIM|WIMServer|CWimSession::NotifyOnRemoval|iNotifyMessage created"));
    }

// -----------------------------------------------------------------------------
// CWimSession::CancelNotifyOnRemoval
// Cancels NotifyOnRemoval request if it is pending.
// -----------------------------------------------------------------------------
//
void CWimSession::CancelNotifyOnRemoval()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSession::CancelNotifyOnRemoval | Begin"));
    if ( iNotifyMessage )
        {
        _WIMTRACE(_L("WIM | WIMServer | CWimSession::CancelNotifyOnRemoval | iNotifyMessage completed with KErrCancel"));
        iNotifyMessage->Complete( KErrCancel ); // Complete pending message
        delete iNotifyMessage;
        iNotifyMessage = NULL;
        }
    _WIMTRACE(_L("WIM | WIMServer | CWimSession::CancelNotifyOnRemoval | Message completed with KErrNone"));
    iMessage->Complete( KErrNone ); // Complete Cancel message
    }

// -----------------------------------------------------------------------------
// CWimSession::NotifyComplete
// Complete NotifyOnRemoval message
// -----------------------------------------------------------------------------
//
void CWimSession::NotifyComplete()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimSession::NotifyComplete | Begin"));
    if ( iNotifyMessage )
        {
        iNotifyMessage->Complete( KErrNone );
        delete iNotifyMessage;
        iNotifyMessage = NULL;
        }
    }


// -----------------------------------------------------------------------------
// CWimSession::RequestAccessesHW
// Check if current request accesses HW
// -----------------------------------------------------------------------------
//
TBool CWimSession::RequestAccessesHW( TInt aFunction )
	{
	TBool rc( EFalse );

	switch ( aFunction )
		{
        // Requests that access HW
        case EWimInitialize:        //lint -fallthrough
//==================== WIM card management =====================================
        case EWIMClose:             //lint -fallthrough
//==================== PIN operations ==========================================
        case EIsPinBlocked:         //lint -fallthrough
        case EChangePINReq:         //lint -fallthrough
        case EEnablePINReq:         //lint -fallthrough
        case EUnblockPinReq:        //lint -fallthrough
        case EVerifyPinReq:         //lint -fallthrough
//==================== Certificate Management ==================================
        case EGetWIMCertDetails:    //lint -fallthrough
        case EStoreCertificate:     //lint -fallthrough
        case ERemoveCertificate:    //lint -fallthrough
        case EExportPublicKey:      //lint -fallthrough
//==================== Digital signature =======================================
        case ESignTextReq:          //lint -fallthrough
//==================== OMA Provisioning ========================================
        case EGetOMAFileSize:       //lint -fallthrough
        case EGetOMAFile:
//==================== JAVA Proisioning ========================================
		case EGetACIFFileSize:
		case EGetACIFFile:
		case EGetACFFileSize:
		case EGetACFFile:
	    case EGetLabelAndPath:        
			rc = ETrue;
            break;

        // Request does not access HW
        default:
            break;
		}
	return rc;
	}


// -----------------------------------------------------------------------------
// CWimSession::RefreshWimi
// Data on card has changed, re-init Wimi.
// -----------------------------------------------------------------------------
//
void CWimSession::RefreshWimi()
    {
    RArray<CWimSession*> sessions;
    TRAPD( err, iWimSvr->GetSessionsL( sessions ));

    if ( !err )  // Got sessions correctly
        {
        TInt count = sessions.Count();
        // Loop through all sessions and notify all clients that are
        // requesting the notification
        for ( TInt i( 0 ); i < count; i++ )
            {
            sessions[i]->NotifyComplete();
            }
        sessions.Reset();
        // Close WIMI
        WIMI_CloseDownReq();
        _WIMTRACE(_L("WIM | CWimSession::RefreshWimi | WIMI closed."));
        }
    else
        {
        _WIMTRACE(_L("WIM | CWimSession::RefreshWimi: FAILED to get sessions."));
        }

    sessions.Close();
    }

//  End of File
