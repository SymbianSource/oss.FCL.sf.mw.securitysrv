/*
* Copyright (c) 2003-2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Crypto Token Security Dialogs PIN Query Dialog
*
*/



// INCLUDE FILES
#include "CTPinQueryDialog.h"
#include "CTSecurityDialogsLogger.h"
#include <CTSecDlgs.rsg>
#include <eikenv.h>
#include <barsread.h>
#include <uikon/eiksrvui.h>


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCTPinQueryDialog::CCTPinQueryDialog
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CCTPinQueryDialog::CCTPinQueryDialog(
    TRequestStatus& aClientStatus,
    const TDesC& aDlgText, TDes& aPinValue, 
    const TInt aMinTextLength, const TInt aMaxTextLength, TBool& aRetVal, TBool& aIsDeleted ): 
    CAknTextQueryDialog(aPinValue),
	iDlgText(aDlgText),
	iPinValue(aPinValue),
	iClientStatus(&aClientStatus),
	iRetVal(aRetVal),
	iIsDeleted(aIsDeleted)
    {
	iMaxTextLength = Min(aPinValue.MaxLength(), aMaxTextLength);
	iMinTextLength = Max(0, aMinTextLength);
    }

// -----------------------------------------------------------------------------
// CCTPinQueryDialog::~CCTPinQueryDialog()
// -----------------------------------------------------------------------------
//
CCTPinQueryDialog::~CCTPinQueryDialog()
    {
    // Allow application switching again
   CEikonEnv* eikonEnv = CEikonEnv::Static();
    if( eikonEnv )
        {
        CEikServAppUi* eikServAppUi = static_cast<CEikServAppUi*>( eikonEnv->EikAppUi() );
        if( eikServAppUi )
            {
            eikServAppUi->SuppressAppSwitching( EFalse );
            }
        }

    // Complete the client request
    if( iClientStatus && *iClientStatus == KRequestPending )
        {
        iRetVal = EFalse;
        User::RequestComplete( iClientStatus, KErrNone );
        }
    
    // Notify that the dialog has been deleted
    iIsDeleted = ETrue;
    }

// -----------------------------------------------------------------------------
// CCTPinQueryDialog::RunDlgLD()
// -----------------------------------------------------------------------------
//
void CCTPinQueryDialog::RunDlgLD(
    TRequestStatus& aClientStatus, 
    const TDesC& aDlgText, 
    TDes& aPinValue, 
    const TInt aMinTextLength, 
    const TInt aMaxTextLength, 
    TBool& aRetVal, 
    TInt aResourceId, CCTPinQueryDialog*& aDialog, TBool& aIsDeleted )
    {
	CCTPinQueryDialog* self = new (ELeave) CCTPinQueryDialog(aClientStatus, aDlgText, aPinValue, aMinTextLength, aMaxTextLength, aRetVal, aIsDeleted );
	aDialog = self;
	// OkToExitL function handles dialog response
	self->ExecuteLD(aResourceId);
    }

// -----------------------------------------------------------------------------
// CCTPinQueryDialog::PreLayoutDynInitL()
// -----------------------------------------------------------------------------
//
void CCTPinQueryDialog::PreLayoutDynInitL()
    {
	SetMaxLength(iMaxTextLength);
	SetPromptL(iDlgText);
	CAknTextQueryDialog::PreLayoutDynInitL();
    }

// -----------------------------------------------------------------------------
// CCTPinQueryDialog::CheckIfEntryTextOk()
// -----------------------------------------------------------------------------
//
TBool CCTPinQueryDialog::CheckIfEntryTextOk() const
    {
    CAknQueryControl* control = QueryControl();
    if (control)
        {
        control->GetText(iDataText);
        }
    const TInt textLength = Text().Length();
    return (textLength <= iMaxTextLength && textLength >= iMinTextLength);
    }

// -----------------------------------------------------------------------------
// CCTPinQueryDialog::UpdateLeftSoftKeyL()
// -----------------------------------------------------------------------------
//
void CCTPinQueryDialog::UpdateLeftSoftKeyL()
    {
    CAknTextQueryDialog::UpdateLeftSoftKeyL();
    MakeLeftSoftkeyVisible(CheckIfEntryTextOk());
    }

// -----------------------------------------------------------------------------
// CCTPinQueryDialog::PostLayoutDynInitL()
// -----------------------------------------------------------------------------
//
void CCTPinQueryDialog::PostLayoutDynInitL()
    {
	((CEikServAppUi*)(CEikonEnv::Static())->EikAppUi())->SuppressAppSwitching(ETrue);
    }
// -----------------------------------------------------------------------------
// CCTPinQueryDialog::OkToExitL()
// -----------------------------------------------------------------------------
//
TBool CCTPinQueryDialog::OkToExitL( TInt aButtonId )
    {

    if ( aButtonId == EEikBidCancel || aButtonId == EAknSoftkeyNo )
        {
        iRetVal = EFalse;
        User::RequestComplete(iClientStatus, KErrNone);   
        }

    else if ( aButtonId ==EAknSoftkeyYes || aButtonId == EAknSoftkeyOk)
        {
        CAknQueryControl* control = QueryControl();
        if (control)
            {
            control->GetText(iPinValue);
            }
        iRetVal = ETrue;
        User::RequestComplete(iClientStatus, KErrNone); 
        }

    return ETrue; 
    }

// -----------------------------------------------------------------------------
// CCTPinQueryDialog::OfferKeyEventL()
// -----------------------------------------------------------------------------
//
TKeyResponse CCTPinQueryDialog::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
	// '#' key is inactive in Pin query dialog
	if( (aKeyEvent.iScanCode == EStdKeyHash)  && (aType == EEventKeyDown || aType == EEventKeyUp) )
	    { 
		return EKeyWasConsumed;
	    }
	// '*' key is inactive in Pin query dialog
	if (aKeyEvent.iCode == '*')
	    {
		return EKeyWasConsumed;
	    }
	return CAknTextQueryDialog::OfferKeyEventL(aKeyEvent, aType);
    }
// End of File
