/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Basic PIN query operation in secui dialog
*
*/

#include "secuidialogoperbasicpinquery.h" // CBasicPinQueryOperation
#include "secuidialogoperationobserver.h" // MSecuiDialogOperationObserver
#include <hb/hbcore/hbtextresolversymbian.h> // HbTextResolverSymbian
#include "secuidialogstrace.h"       // TRACE macro

// TODO: fix this
#include "../../../securitydialogs/SecUi/Inc/SecQueryUi.h"  // CSecQueryUi

#include <secui.h>
#include <secuisecurityhandler.h>
#include <gsmerror.h>
#include <etelmm.h>
#include <rmmcustomapi.h>
#include <startupdomainpskeys.h>
#include <featmgr.h>

const TInt KPhoneIndex( 0 );
const TInt KTriesToConnectServer( 2 );
const TInt KTimeBeforeRetryingServerConnection( 50000 );

_LIT( KMmTsyModuleName, "PhoneTsy"); 

// Descriptors for different password queries
_LIT( KBasicPinQueryKeyStorePassphrase, "Key store passphrase" );
_LIT( KBasicPinQueryNewKeyStorePassphrase, "New key store passphrase" );
_LIT( KBasicPinQueryImportedKeyFilePassphrase, "Passphrase of the imported key file" );
_LIT( KBasicPinQueryExportedKeyFilePassphrase, "Passphrase of the exported key file" );
_LIT( KBasicPinQueryPKCS12, "PKCS12");




// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::NewL()
// ---------------------------------------------------------------------------
//
CBasicPinQueryOperation* CBasicPinQueryOperation::NewL(
        MSecuiDialogOperationObserver& aObserver, const RMessage2& aMessage,
        TInt aReplySlot )
    {
    TRACE( "CBasicPinQueryOperation::NewL, aMessage 0x%08x", aMessage.Handle() );
    TRACE( "CBasicPinQueryOperation::NewL, aMessage.Function = 0x%08x", aMessage.Function() );
    return new( ELeave ) CBasicPinQueryOperation( aObserver, aMessage, aReplySlot );
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::~CBasicPinQueryOperation()
// ---------------------------------------------------------------------------
//
CBasicPinQueryOperation::~CBasicPinQueryOperation()
    {
    TRACE( "CBasicPinQueryOperation::~CBasicPinQueryOperation" );
    Cancel();
    iPinInput = NULL;   // not owned
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::StartL()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::StartL( const TDesC8& aBuffer )
    {
    TRACE( "CBasicPinQueryOperation::StartL" );
    iPinInput = reinterpret_cast< const TPINInput* >( aBuffer.Ptr() );
    ASSERT( iPinInput != NULL );

    iStatus = KRequestPending;
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    SetActive();
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::CancelOperation()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::CancelOperation()
    {
    TRACE( "CBasicPinQueryOperation::CancelOperation" );
    // nothing to do
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::RunL()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::RunL()
    {
    TRACE( "CBasicPinQueryOperation::RunL, iStatus.Int()=%d", iStatus.Int() );
    User::LeaveIfError( iStatus.Int() );

    TBool isRetry = ( iPinInput->iOperation & EPINValueIncorrect );
    if( isRetry )
        {
        // Show "Invalid PIN code" error note, as previous attempt was failed.
        // TODO: localized UI string needed
        _LIT( KInvalidPinCode, "Invalid PIN code" );
        ShowWarningNoteL( KInvalidPinCode );
        }
TRACE( "CBasicPinQueryOperation::RunL, iPinInput->iOperation=%d", iPinInput->iOperation );

    HBufC* caption = NULL;
    if( iPinInput->iPIN.iPINLabel == KBasicPinQueryKeyStorePassphrase )
        {
        	TRACE( "CBasicPinQueryOperation::RunL, KBasicPinQueryKeyStorePassphrase =%d", 1 );

        // TODO: localized UI string needed
        _LIT( KText, "Keystore password:" );
        caption = KText().AllocLC();
        }
    else if( iPinInput->iPIN.iPINLabel == KBasicPinQueryNewKeyStorePassphrase )
        {
        	TRACE( "CBasicPinQueryOperation::RunL, KBasicPinQueryNewKeyStorePassphrase =%d", 1 );

        // "Keystore password must be created for using private keys."
        // SecUi creates two input fields when caption contains two labels
        // separated with a vertical bar. SecUi verifies that user types the
        // same passphrase in both fields.
        // TODO: localized UI strings needed
        _LIT( KText, "Create keystore password:|Verify:" );
        caption = KText().AllocLC();
        }
    else if( iPinInput->iPIN.iPINLabel == KBasicPinQueryImportedKeyFilePassphrase )
        {
        	TRACE( "CBasicPinQueryOperation::RunL, KBasicPinQueryImportedKeyFilePassphrase =%d", 1 );

        // "Enter code for imported key:"
        // TODO: localized UI strings needed
        _LIT( KText, "Enter code for imported key:|Verify:" );
        caption = KText().AllocLC();
        }
    else if( iPinInput->iPIN.iPINLabel == KBasicPinQueryExportedKeyFilePassphrase )
        {
        	TRACE( "CBasicPinQueryOperation::RunL, KBasicPinQueryExportedKeyFilePassphrase =%d", 1 );

        // "Enter new code for exported key:"
        // TODO: localized UI strings needed
        _LIT( KText, "Enter new code for exported key:|Verify:" );
        caption = KText().AllocLC();
        }
    else if( iPinInput->iPIN.iPINLabel == KBasicPinQueryPKCS12 )
        {
        	TRACE( "CBasicPinQueryOperation::RunL, KBasicPinQueryPKCS12 =%d", 1 );

        // "Password for %U:", PKCS#12 password query prompt
        // TODO: localized UI string needed
        _LIT( KText, "Password for PKCS#12 file:" );
        caption = KText().AllocLC();
        }
    else
        {
        	TRACE( "CBasicPinQueryOperation::RunL, else =%d", 1 );

        // "Enter code for %0U in %1U"
        // Data query for PIN request in keystore where %0U is the PIN's name %1U is the keystore's name.
        // TODO: EEnterPinNR
        _LIT( KText, "86" );
        caption = KText().AllocLC();
        }
    iPinValue.Copy(_L("0"));
		TRACE( "CBasicPinQueryOperation::RunL, 1 =%d", 1 );

    TInt resultVerif = KErrNone;
		TRACE( "CBasicPinQueryOperation::RunL, 1 =%d", 1 );

            {
		   					RMobilePhone	iPhone;

								TInt err( KErrGeneral);
								TRACE( "CBasicPinQueryOperation::RunL, ESecurityQueryActive =%d", ESecurityQueryActive );
								err = RProperty::Set(KPSUidStartup, KStartupSecurityCodeQueryStatus, ESecurityQueryActive);
								TRACE( "CBasicPinQueryOperation::RunL, err =%d", err );
								
								TInt thisTry( 0);
								RTelServer iTelServer;
								RMmCustomAPI iCustomPhone;
								while ( ( err = iTelServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
								{
								User::After( KTimeBeforeRetryingServerConnection );
								}
								err = iTelServer.LoadPhoneModule( KMmTsyModuleName );
								RTelServer::TPhoneInfo PhoneInfo;
								err = iTelServer.SetExtendedErrorGranularity( RTelServer::EErrorExtended ) ;
								err = iTelServer.GetPhoneInfo( KPhoneIndex, PhoneInfo ) ;
								err = iPhone.Open( iTelServer, PhoneInfo.iName ) ;
								err = iCustomPhone.Open( iPhone ) ;
								TRACE( "CBasicPinQueryOperation::RunL, err =%d", err );
						    CSecurityHandler* handler = new(ELeave) CSecurityHandler(iPhone);
						    CleanupStack::PushL(handler);
						    // TSecUi::InitializeLibL(); 
								TRACE( "CBasicPinQueryOperation::RunL, TMobilePhoneSecurityEvent 1 =%d", 1 );
								RMobilePhone::TMobilePhoneSecurityEvent iEvent;
						    TInt lEvent = iPinInput->iOperation;
						    TRACE( "CBasicPinQueryOperation::RunL, lEvent =%d", lEvent );
								if( lEvent >= 0x1000 )	// flag for iStartUp
									{
									lEvent-=0x1000;
									}
						    iEvent = static_cast<RMobilePhone::TMobilePhoneSecurityEvent>(lEvent);
						    TRACE( "CBasicPinQueryOperation::RunL, iEvent =%d", iEvent );
						    TRACE( "CBasicPinQueryOperation::RunL, TBool iStartUp =%d", iStartUp );
						    TRAPD( resultHandler, handler->HandleEventL( iEvent, iStartUp, resultVerif ) );
						    TRACE( "CBasicPinQueryOperation::RunL, resultHandler =%d", resultHandler );
						    TRACE( "CBasicPinQueryOperation::RunL, resultVerif =%d", resultVerif );
   
						    // if something went wrong cancel the code request
						    if (resultHandler)
						        {
						        if(resultVerif==KErrNone)	// if the process failed, then the result shoud also indicate the failure (unless it does it already)
						        	resultVerif = resultHandler;
						    TRACE( "CBasicPinQueryOperation::RunL, iEvent =%d", iEvent );
								TBool wcdmaSupported(FeatureManager::FeatureSupported( KFeatureIdProtocolWcdma ));
								TBool upinSupported(FeatureManager::FeatureSupported( KFeatureIdUpin ));
						        switch (iEvent)
						            {
						            case RMobilePhone::EUniversalPinRequired:
						                if(wcdmaSupported || upinSupported)
						                  {
						                   iPhone.AbortSecurityCode(RMobilePhone::ESecurityUniversalPin);
						                  }
						                break;
						            case RMobilePhone::EUniversalPukRequired:
						                if(wcdmaSupported || upinSupported)
						                  {
						                   iPhone.AbortSecurityCode(RMobilePhone::ESecurityUniversalPuk);
						                  }
						                break;
									case RMobilePhone::EPin1Required:
						                iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePin1);
						                break;
									case RMobilePhone::EPuk1Required:
						                iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePuk1);
						                break;
									case RMobilePhone::EPin2Required:
						                iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePin2);
						                break;
									case RMobilePhone::EPuk2Required:
						                iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePuk2);
						                break;
									case RMobilePhone::EPhonePasswordRequired:
						                iPhone.AbortSecurityCode(RMobilePhone::ESecurityCodePhonePassword);
						                break;
						            default:
         								    TRACE( "CBasicPinQueryOperation::RunL, default iEvent =%d", iEvent );
						                break;
						            }
						        }
						
						    // uninitialize security ui
						    TRACE( "CBasicPinQueryOperation::RunL, PopAndDestroy(handler) =%d", 0 );
						    CleanupStack::PopAndDestroy(handler); // handler
								TRACE( "CBasicPinQueryOperation::RunL, UnInitializeLib =%d", 1 );
						    TSecUi::UnInitializeLib();  
								TRACE( "CBasicPinQueryOperation::RunL, ESecurityQueryNotActive =%d", ESecurityQueryNotActive );
								err = RProperty::Set(KPSUidStartup, KStartupSecurityCodeQueryStatus, ESecurityQueryNotActive);
								TRACE( "CBasicPinQueryOperation::RunL, err =%d", err );
						    //close ETel connection
						    if (iTelServer.Handle())
						        {
						        TRACE( "CBasicPinQueryOperation::RunL, iPhone.Close =%d", 0 );
						        iPhone.Close();
						        iTelServer.UnloadPhoneModule(KMmTsyModuleName);
						        iTelServer.Close();
						        TRACE( "CBasicPinQueryOperation::RunL, iTelServer.Close =%d", 1 );
						        }
            }


    ReturnResultL( resultVerif );

    if( caption )
        {
        CleanupStack::PopAndDestroy( caption );
        caption = NULL;
        }
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::DoCancel()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::DoCancel()
    {
    TRACE( "CBasicPinQueryOperation::DoCancel" );
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::CBasicPinQueryOperation()
// ---------------------------------------------------------------------------
//
CBasicPinQueryOperation::CBasicPinQueryOperation(
        MSecuiDialogOperationObserver& aObserver, const RMessage2& aMessage,
        TInt aReplySlot ) : CSecuiDialogOperation( aObserver, aMessage, aReplySlot )
    {
    TRACE( "CBasicPinQueryOperation::CBasicPinQueryOperation, aMessage 0x%08x", aMessage.Handle() );
    TRACE( "CBasicPinQueryOperation::CBasicPinQueryOperation, aMessage.Function = 0x%08x", aMessage.Function() );
    iStartUp=EFalse;
    if(aMessage.Function()>=0x1000)
    	iStartUp=ETrue;
    }

// ---------------------------------------------------------------------------
// CBasicPinQueryOperation::ReturnResultL()
// ---------------------------------------------------------------------------
//
void CBasicPinQueryOperation::ReturnResultL( TInt aErrorCode )
    {
    TRACE( "CBasicPinQueryOperation::ReturnResultL, begin, aErrorCode=%d", aErrorCode );
    if( aErrorCode <= KErrNone )	// TODO should skip WriteL is error?
        {
        TPINValueBuf output( iPinValue );
        iMessage.WriteL( iReplySlot, output );
        }
    TRACE( "CBasicPinQueryOperation::ReturnResultL, completing msg 0x%08x", iMessage.Handle() );
    iMessage.Complete( aErrorCode );
    TRACE( "CBasicPinQueryOperation::ReturnResultL, informing observer" );
    iObserver.OperationComplete();
    TRACE( "CBasicPinQueryOperation::ReturnResultL, end" );
    }

