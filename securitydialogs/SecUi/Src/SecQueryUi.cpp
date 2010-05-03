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
* Description:  Implementation of RSecQueryUiCli class.
*
*/

#include "SecQueryUi.h"                              // CSecQueryUi
// #include <SecQueryUidefs.h>                          // SIF UI device dialog parameters
#include <hb/hbcore/hbsymbiandevicedialog.h>    // CHbDeviceDialog
#include <hb/hbcore/hbsymbianvariant.h>         // CHbSymbianVariantMap
#include <apgicnfl.h>                           // CApaMaskedBitmap

#include <CPhCltEmergencyCall.h>
#include <SCPClient.h>
#include "SecUiWait.h"

// Variant map keys for notification device dialog
_LIT( KNotifDeviceDialogLiteral, "com.nokia.hb.devicenotificationdialog/1.0" );
_LIT( KNotifDeviceDialogKeyTimeOut, "timeout" );
_LIT( KNotifDeviceDialogKeyIconName, "iconName" );
_LIT( KNotifDeviceDialogKeyText, "text" );
_LIT( KNotifDeviceDialogKeyTitle, "title" );
_LIT( KNotifDeviceDialogKeyTouchActivation, "touchActivation" );
_LIT( KNotifDeviceDialogKeyActivated, "result" );
_LIT( KNotifDeviceDialogKeyActivatedValue, "activated" );
_LIT( KNotifDeviceDialogKeyTitleTextWrapping, "titleTextWrapping" );

const TInt KMaxNumberOfPINAttempts(3);
const TInt KLastRemainingInputAttempt(1);

const TUid KSWInstHelpUid = { 0x101F8512 };	// TODO


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSecQueryUi::NewLC()
// ---------------------------------------------------------------------------
//
EXPORT_C CSecQueryUi* CSecQueryUi::NewLC()
    {
    CSecQueryUi* self = new( ELeave ) CSecQueryUi();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::NewL()
// ---------------------------------------------------------------------------
//
EXPORT_C CSecQueryUi* CSecQueryUi::NewL()
    {
    CSecQueryUi* self = CSecQueryUi::NewLC();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// CSecQueryUi::~CSecQueryUi()
// ---------------------------------------------------------------------------
//
CSecQueryUi::~CSecQueryUi()
    {
    Cancel();
    delete iWait;
    delete iDeviceDialog;
    delete iVariantMap;
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::InstallConfirmationQueryL()
// ---------------------------------------------------------------------------
//
EXPORT_C TBool CSecQueryUi::InstallConfirmationQueryL( TInt aType, RMobilePhone::TMobilePassword& password )
/*
        const TDesC& aAppName,
        const TDesC& aIconFile, const TDesC& aAppVersion, TInt aAppSize,
        const TDesC& aAppDetails ) */
    {

    RDEBUG( "This should never be called. Obsolete", 0 );
    return KErrAbort ;
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::SecQueryDialog()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CSecQueryUi::SecQueryDialog(const TDesC& aCaption, TDes& aDataText, TInt aMinLength,TInt aMaxLength,TInt aMode)
    {
    	RDEBUG( "aCaption", 0 );
    RDebug::Print( aCaption );
    	RDEBUG( "aMode", aMode );

    ClearParamsAndSetNoteTypeL( aMode );
    AddParamL( _L("KSecQueryUiApplicationName"), aCaption );

		_LIT(KTitle, "title");
    // _LIT(KTitleValue1, "Enter PIN");
   	AddParamL( KTitle, aCaption );
		AddParamL( _L("MinLength"), aMinLength );
		AddParamL( _L("MaxLength"), aMaxLength );
    
		_LIT( KCodeTop, "codeTop" );						_LIT( KCodeTopValue, "codeTop" );
    AddParamL( KCodeTop, KCodeTopValue );

		if( aCaption.Find(_L("|"))>0 )
			{
				RDEBUG( "codeBottom aMode", aMode );
			_LIT( KCodeBottom, "codeBottom" );						_LIT( KCodeBottomValue, "codeBottom" );
  	  AddParamL( KCodeBottom, KCodeBottomValue );
  		}

			RDEBUG( "0", 0 );
	  DisplayDeviceDialogL();
	  TInt error = WaitUntilDeviceDialogClosed();
	  	RDEBUG( "error", error );
    User::LeaveIfError( error );

    aDataText.Copy(iPassword);
    	RDEBUG( "iReturnValue", iReturnValue );
    return iReturnValue;
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DisplayInformationNoteL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::DisplayInformationNoteL( const TDesC& aText )
    {
    /*
    ClearParamsAndSetNoteTypeL( SecQueryUiInformationNote );
    AddParamL( KNotifDeviceDialogKeyText, aText );
    AddParamL( KNotifDeviceDialogKeyTimeOut, 0 );
    iDeviceDialog->Show( KNotifDeviceDialogLiteral, *iVariantMap, this );
    User::LeaveIfError( WaitUntilDeviceDialogClosed() );
    */
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DisplayWarningNoteL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::DisplayWarningNoteL( const TDesC& aText )
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DisplayErrorNoteL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::DisplayErrorNoteL( const TDesC& aText )
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DisplayPermanentNoteL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::DisplayPermanentNoteL( const TDesC& aText )
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::ClosePermanentNote()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::ClosePermanentNote()
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DisplayProgressNoteL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::DisplayProgressNoteL( const TDesC& aText, TInt aFinalValue )
    {
/*
    ClearParamsAndSetNoteTypeL( ESecQueryUiProgressNoteType );
    AddParamL( KSecQueryUiProgressNoteText, aText );
    AddParamL( KSecQueryUiProgressNoteFinalValue, aFinalValue );
    DisplayDeviceDialogL();
*/    }

// ---------------------------------------------------------------------------
// CSecQueryUi::UpdateProgressNoteValueL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::UpdateProgressNoteValueL( TInt aNewValue )
    {
/*
    ClearParamsAndSetNoteTypeL( ESecQueryUiProgressNoteType );
    AddParamL( KSecQueryUiProgressNoteValue, aNewValue );
    DisplayDeviceDialogL();
*/    }

// ---------------------------------------------------------------------------
// CSecQueryUi::CloseProgressNoteL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::CloseProgressNoteL()
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DisplayWaitNoteL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::DisplayWaitNoteL( const TDesC& aText,
        TRequestStatus& aStatus )
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::CloseWaitNote()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::CloseWaitNote()
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::LaunchHelpL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::LaunchHelpL( const TDesC& aContext, const TUid& aUid )
    {
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::LaunchHelpL()
// ---------------------------------------------------------------------------
//
EXPORT_C void CSecQueryUi::LaunchHelpL( const TDesC& aContext )
    {
    LaunchHelpL( aContext, KSWInstHelpUid );
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DoCancel()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::DoCancel()
    {
    	RDEBUG( "0", 0 );
    if( iWait && iWait->IsStarted() && iWait->CanStopNow() )
        {
        iCompletionCode = KErrCancel;
        iWait->AsyncStop();
        }
    	RDEBUG( "0", 0 );
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::RunL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::RunL()
    {
    	RDEBUG( "0", 0 );
    if( iWait )
        {
        iWait->AsyncStop();
        }
    	RDEBUG( "0", 0 );
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DataReceived()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::DataReceived( CHbSymbianVariantMap& aData )
    {
    	RDEBUG( "0", 0 );
    const CHbSymbianVariant* acceptedVariant = aData.Get( _L("accepted") );	// KSecQueryUiQueryAccepted
    	RDEBUG( "0", 0 );
    if( acceptedVariant )
        {
    			RDEBUG( "0", 0 );
        TInt* acceptedValue = acceptedVariant->Value<TInt>();
    			RDEBUG( "acceptedValue", acceptedValue );
    			RDEBUG( "*acceptedValue", *acceptedValue );
        if( acceptedValue )
            {
            iReturnValue = *acceptedValue;
            }
        }
    const CHbSymbianVariant* acceptedVariantTop = aData.Get( _L("codeTop") );	// KSecQueryUiQueryAccepted
    	RDEBUG( "0", 0 );
    if( acceptedVariantTop )
        {
        TPtrC acceptedValueTop = *acceptedVariantTop->Value<TDesC>();
        	RDEBUG( "acceptedValueTop", 0 );
   	    RDebug::Print( acceptedValueTop );
   	    iPassword.Copy(acceptedValueTop);
   	    
   	    if(iReturnValue == KErrCompletion )	// the user didn't OK. It was send automatically because validating new lock code
   	    	{
					_LIT( KInvalidNewLockCode, "invalidNewLockCode" );
					_LIT( KInvalidNewLockCode0, "invalidNewLockCode#0" );
					_LIT( KInvalidNewLockCode1, "invalidNewLockCode#1" );
					_LIT( KInvalidNewLockCode2, "invalidNewLockCode#2" );
   				AddParamL( KInvalidNewLockCode, KInvalidNewLockCode0 );	// for starter
            RSCPClient scpClient;
            TSCPSecCode newCode;
            newCode.Copy( acceptedValueTop );
            	RDEBUG( "scpClient.Connect", 0 );
            if ( scpClient.Connect() == KErrNone )
                {
                /*
                RArray<TDevicelockPolicies> aFailedPolicies;
                TDevicelockPolicies failedPolicy;
                TInt retLockcode = KErrNone;
									RDEBUG( "scpClient.VerifyNewLockcodeAgainstPolicies", 0 );
                retLockcode = scpClient.VerifyNewLockcodeAgainstPolicies( newCode, aFailedPolicies );
                	RDEBUG( "retLockcode", retLockcode );
                	RDEBUG( "aFailedPolicies.Count()", aFailedPolicies.Count() );
                for(TInt i=0; i<aFailedPolicies.Count(); i++)
                	{
               		failedPolicy = aFailedPolicies[i];
                		RDEBUG( "failedPolicy", failedPolicy );
									TBuf<0x100> KInvalidNewLockCodeX;	KInvalidNewLockCodeX.Zero();	KInvalidNewLockCodeX.Append(_L("invalidNewLockCode"));	KInvalidNewLockCodeX.Append(_L("#"));
									KInvalidNewLockCodeX.AppendNum(failedPolicy);
                	AddParamL( KInvalidNewLockCode, KInvalidNewLockCodeX );
                	}
                */
                scpClient.Close();
                }                                               
						RDEBUG( "iDeviceDialog->Update", 0 );
					iDeviceDialog->Update( *iVariantMap );
					}	// KErrCompletion

				if(acceptedValueTop.Length()<=4)	// TODO store aMinLenght and check it here, instead of "4"
					{
	   	    	RDEBUG( "CPhCltEmergencyCall", 0 );
					CPhCltEmergencyCall* emergencyCall = CPhCltEmergencyCall::NewL( NULL );
						RDEBUG( "PushL", 0 );
					CleanupStack::PushL( emergencyCall );
					TPhCltEmergencyNumber emNumber;
					
					// this relies on the fact that emergency has 3 digits, and password needs at least 4
					TBool isEmergency( EFalse );
						RDEBUG( "calling IsEmergencyPhoneNumber", 0 );
					TInt error = emergencyCall->IsEmergencyPhoneNumber( acceptedValueTop, isEmergency );
						RDEBUG( "error", error );
						RDEBUG( "emNumber", 0 );
					
						RDEBUG( "isEmergency", isEmergency );
					#ifdef __WINS__
						RDEBUG( "__WINS__ checking", 0 );
					if(!acceptedValueTop.CompareF(_L("112")) || !acceptedValueTop.CompareF(_L("911")) || !acceptedValueTop.CompareF(_L("555")) )
						{
							isEmergency = ETrue;
							error = KErrNone;
								RDEBUG( "__WINS__ isEmergency", isEmergency );
						}
					#endif
	
					if ( !error )	// oddly enough, missing capabilities also gives KErrNone
						{
		   	    if(iReturnValue == KErrAbort )	// the user didn't OK. It was send automatically because short code
		   	    	{
							_LIT( KEmergency, "emergency" );						_LIT( KEmergencyValueYes, "emergencyYes" );	_LIT( KEmergencyValueNo, "emergencyNo" );
							if(isEmergency)
								{
									RDEBUG( "KEmergencyValueYes", 1 );
		    				AddParamL( KEmergency, KEmergencyValueYes );
								}
							else
								{
									RDEBUG( "KEmergencyValueNo", 0 );
		    				AddParamL( KEmergency, KEmergencyValueNo );
								}
							iDeviceDialog->Update( *iVariantMap );
							}
						else if(iReturnValue == KErrNone )
							{	// user pressed Call and number is valid
							if(isEmergency)
								{
									 RDEBUG( "DialEmergencyCallL", isEmergency );
								emergencyCall->DialEmergencyCallL( emNumber );
								iReturnValue = KErrAbort;	// this means emergency call
								}
							}
						}	// if !error
						RDEBUG( "0", 0 );
					CleanupStack::PopAndDestroy( emergencyCall );
					}	// lenght<3
				}	// acceptedVariantTop
    	RDEBUG( "iReturnValue", iReturnValue );
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DeviceDialogClosed()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::DeviceDialogClosed( TInt aCompletionCode )
    {
    	RDEBUG( "aCompletionCode", aCompletionCode );
    iCompletionCode = aCompletionCode;
    iIsDisplayingDialog = EFalse;

    TRequestStatus* status( &iStatus );
    	RDEBUG( "0", 0 );
    User::RequestComplete( status, KErrNone );
    	RDEBUG( "0", 0 );
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::CSecQueryUi()
// ---------------------------------------------------------------------------
//
CSecQueryUi::CSecQueryUi() : CActive( CActive::EPriorityStandard )
    {
    	RDEBUG( "0", 0 );
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::ConstructL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::ConstructL()
    {
    	RDEBUG( "0", 0 );
    iWait = new( ELeave ) CActiveSchedulerWait;
    // iDeviceDialog is allocated later, first call of DisplayDeviceDialogL()
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::ClearParamsL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::ClearParamsL()
    {
    	RDEBUG( "0", 0 );
    if( iVariantMap )
        {
        delete iVariantMap;
        iVariantMap = NULL;
        }
    iVariantMap = CHbSymbianVariantMap::NewL();
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::ClearParamsAndSetNoteTypeL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::ClearParamsAndSetNoteTypeL( TInt aType )
    {
    	RDEBUG( "aType", aType );
    ClearParamsL();
    AddParamL( _L("type"), aType );
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::AddParamL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::AddParamL( const TDesC& aKey, TInt aValue )
    {
    	RDEBUG( "aValue", aValue );
    CHbSymbianVariant* variant = NULL;
    variant = CHbSymbianVariant::NewL( &aValue, CHbSymbianVariant::EInt );
    iVariantMap->Add( aKey, variant );
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::AddParamL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::AddParamL( const TDesC& aKey, const TDesC& aValue )
    {
    	RDEBUG( "0", 0 );
    CHbSymbianVariant* variant = NULL;
    variant = CHbSymbianVariant::NewL( &aValue, CHbSymbianVariant::EDes );
    iVariantMap->Add( aKey, variant );
    }

TInt strlen( const char* aStr )
    {
    TInt len = 0;
    while( *aStr++ != 0 )
        ++len;
    return len;
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DisplayDeviceDialogL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::DisplayDeviceDialogL()
    {
    	RDEBUG( "0", 0 );
    if( iDeviceDialog && iIsDisplayingDialog )
        {
        iDeviceDialog->Update( *iVariantMap );
        }
    else
        {
        if( !iDeviceDialog )
            {
            iDeviceDialog = CHbDeviceDialog::NewL();
            }
        _LIT( KSecQueryUiDeviceDialog, "com.nokia.secuinotificationdialog/1.0" );
        	RDEBUG( "Show", 0 );
        iDeviceDialog->Show( KSecQueryUiDeviceDialog, *iVariantMap, this );
        	RDEBUG( "iIsDisplayingDialog", iIsDisplayingDialog );
        iIsDisplayingDialog = ETrue;
        	RDEBUG( "iIsDisplayingDialog", iIsDisplayingDialog );
        }
    	RDEBUG( "0", 0 );
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::WaitUntilDeviceDialogClosed()
// ---------------------------------------------------------------------------
//
TInt CSecQueryUi::WaitUntilDeviceDialogClosed()
    {
    	RDEBUG( "0", 0 );
    iCompletionCode = KErrInUse;
    iReturnValue = KErrUnknown;
    if( !IsActive() && iWait && !iWait->IsStarted() )
        {
        iStatus = KRequestPending;
        SetActive();
        	RDEBUG( "Start", 0 );
        iWait->Start();
    			RDEBUG( "Started", 1 );
        }
    	RDEBUG( "iCompletionCode", iCompletionCode );
    return iCompletionCode;
    }

