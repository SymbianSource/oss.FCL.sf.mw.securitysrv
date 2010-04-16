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
    RDebug::Printf( "%s %s (%u) aType=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aType );
    RDebug::Printf( "%s %s (%u) password=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    TInt	ESecQueryUiInstallConfirmationQueryType=0x101;
    ClearParamsAndSetNoteTypeL( ESecQueryUiInstallConfirmationQueryType );

    AddParamL( _L("KSecQueryUiApplicationName"), _L("SecUi") );

		_LIT(KTitle, "title");
    _LIT(KTitleValue1, "Enter PIN");
    _LIT(KTitleValue2, "Enter PIN with care");
    _LIT(KTitleValue3, "Enter PIN last");
    if(aType==KMaxNumberOfPINAttempts)
    	AddParamL( KTitle, KTitleValue1 );
    else if(aType> KLastRemainingInputAttempt)
    	AddParamL( KTitle, KTitleValue2 );
    else
    	AddParamL( KTitle, KTitleValue3 );
    
		_LIT( KCodeTop, "codeTop" );						_LIT( KCodeTopValue, "codeTop" );
    AddParamL( KCodeTop, KCodeTopValue );

		RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
	  DisplayDeviceDialogL();
    User::LeaveIfError( WaitUntilDeviceDialogClosed() );
    password.Copy(iPassword);
    return( iReturnValue == KErrNone );
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::SecQueryDialog()
// ---------------------------------------------------------------------------
//
EXPORT_C TInt CSecQueryUi::SecQueryDialog(const TDesC& aCaption, TDes& aDataText, TInt aMinLength,TInt aMaxLength,TInt aMode)
    {
    RDebug::Printf( "%s %s (%u) aCaption=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 1 );
    RDebug::Print( aCaption );
    RDebug::Printf( "%s %s (%u) aMode=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aMode );

    ClearParamsAndSetNoteTypeL( aMode );
    AddParamL( _L("KSecQueryUiApplicationName"), aCaption );

		_LIT(KTitle, "title");
    // _LIT(KTitleValue1, "Enter PIN");
   	AddParamL( KTitle, aCaption );
    
		_LIT( KCodeTop, "codeTop" );						_LIT( KCodeTopValue, "codeTop" );
    AddParamL( KCodeTop, KCodeTopValue );

		RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
	  DisplayDeviceDialogL();
    User::LeaveIfError( WaitUntilDeviceDialogClosed() );
    aDataText.Copy(iPassword);
    return( iReturnValue == KErrNone );
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
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    if( iWait && iWait->IsStarted() && iWait->CanStopNow() )
        {
        iCompletionCode = KErrCancel;
        iWait->AsyncStop();
        }
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::RunL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::RunL()
    {
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    if( iWait )
        {
        iWait->AsyncStop();
        }
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DataReceived()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::DataReceived( CHbSymbianVariantMap& aData )
    {
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    const CHbSymbianVariant* acceptedVariant = aData.Get( _L("accepted") );	// KSecQueryUiQueryAccepted
    if( acceptedVariant )
        {
        TBool* acceptedValue = acceptedVariant->Value<TBool>();
        if( acceptedValue && *acceptedValue )
            {
            iReturnValue = KErrNone;
            }
        else
            {
            iReturnValue = KErrCancel;
            }
        }
    const CHbSymbianVariant* acceptedVariantTop = aData.Get( _L("codeTop") );	// KSecQueryUiQueryAccepted
    if( acceptedVariantTop )
        {
        TPtrC acceptedValueTop = *acceptedVariantTop->Value<TDesC>();
        RDebug::Printf( "%s %s (%u) acceptedValueTop=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
   	    RDebug::Print( acceptedValueTop );
   	    iPassword.Copy(acceptedValueTop);
        }
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::DeviceDialogClosed()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::DeviceDialogClosed( TInt aCompletionCode )
    {
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    iCompletionCode = aCompletionCode;
    iIsDisplayingDialog = EFalse;

    TRequestStatus* status( &iStatus );
    User::RequestComplete( status, KErrNone );
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::CSecQueryUi()
// ---------------------------------------------------------------------------
//
CSecQueryUi::CSecQueryUi() : CActive( CActive::EPriorityStandard )
    {
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::ConstructL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::ConstructL()
    {
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    iWait = new( ELeave ) CActiveSchedulerWait;
    // iDeviceDialog is allocated later, first call of DisplayDeviceDialogL()
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::ClearParamsL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::ClearParamsL()
    {
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
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    ClearParamsL();
    AddParamL( _L("type"), aType );
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::AddParamL()
// ---------------------------------------------------------------------------
//
void CSecQueryUi::AddParamL( const TDesC& aKey, TInt aValue )
    {
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
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
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
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
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
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
        RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
        iDeviceDialog->Show( KSecQueryUiDeviceDialog, *iVariantMap, this );
        RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
        iIsDisplayingDialog = ETrue;
        }
    }

// ---------------------------------------------------------------------------
// CSecQueryUi::WaitUntilDeviceDialogClosed()
// ---------------------------------------------------------------------------
//
TInt CSecQueryUi::WaitUntilDeviceDialogClosed()
    {
    RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    iCompletionCode = KErrInUse;
    iReturnValue = KErrUnknown;
    if( !IsActive() && iWait && !iWait->IsStarted() )
        {
        iStatus = KRequestPending;
        SetActive();
        iWait->Start();
        }
    return iCompletionCode;
    }

