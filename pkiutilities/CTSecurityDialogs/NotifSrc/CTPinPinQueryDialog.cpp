/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of multiline PIN query dialog
*
*/


// INCLUDE FILES
#include <platform/e32notif.h>
#include "CTPinPinQueryDialog.h"
#include <CTSecDlgs.rsg>
#include <eikenv.h>
#include <barsread.h>
#include <uikon/eiksrvui.h>
#include <secdlg.h>
#include <layoutmetadata.cdl.h>

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCTPinPinQueryDialog::CCTPinPinQueryDialog
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CCTPinPinQueryDialog::CCTPinPinQueryDialog(
    const TDesC& aDlgText1,
    const TDesC& aDlgText2,
    TDes&  aPin1,
    TDes&  aPin2,
    const TInt aMinTextLength, 
    const TInt aMaxTextLength, 
    TBool& aRetVal): 
    CAknMultiLineDataQueryDialog( ENoTone ),
	iDlgText1(aDlgText1),
    iDlgText2(aDlgText2),
	iPinValue1(aPin1),
    iPinValue2(aPin2),
	iRetVal(aRetVal)
    {
    iMaxTextLength = Min(aPin1.MaxLength(), aMaxTextLength);
	iMinTextLength = Max(0, aMinTextLength);
    }

// -----------------------------------------------------------------------------
// CCTPinPinQueryDialog::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCTPinPinQueryDialog::ConstructL()
    {
    }

// -----------------------------------------------------------------------------
// CCTPinPinQueryDialog::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCTPinPinQueryDialog* CCTPinPinQueryDialog::NewL(
    const TDesC& aDlgText1,
    const TDesC& aDlgText2,
    TDes&  aPin1,
    TDes&  aPin2,
    const TInt aMinTextLength, 
    const TInt aMaxTextLength, 
    TBool& aRetVal)
    {
    CCTPinPinQueryDialog* self = new( ELeave ) CCTPinPinQueryDialog(
        aDlgText1, aDlgText2, aPin1, aPin2, aMinTextLength, aMaxTextLength, aRetVal);
    
    CleanupStack::PushL( self );
    self->SetDataL(aPin1, aPin2);
    CleanupStack::Pop();

    return self;
    }
 
// Destructor
CCTPinPinQueryDialog::~CCTPinPinQueryDialog()
    {
    }

// -----------------------------------------------------------------------------
// CCTPinPinQueryDialog::RunDlgLD()
// -----------------------------------------------------------------------------
//
void CCTPinPinQueryDialog::RunDlgLD(TRequestStatus& aClientStatus, TInt aResourceId)
    {
    iClientStatus = &aClientStatus;
	// OkToExitL function handles dialog response
    ExecuteLD(aResourceId);
    }

// -----------------------------------------------------------------------------
// CCTPinPinQueryDialog::PreLayoutDynInitL()
// -----------------------------------------------------------------------------
//
void CCTPinPinQueryDialog::PreLayoutDynInitL()
    {
	SetMaxLengthOfFirstEditor(iMaxTextLength);
    SetMaxLengthOfSecondEditor(iMaxTextLength);
	SetPromptL(iDlgText1, iDlgText2);
	CAknMultiLineDataQueryDialog::PreLayoutDynInitL();
    }

// -----------------------------------------------------------------------------
// CCTPinPinQueryDialog::CheckIfEntryTextOk()
// -----------------------------------------------------------------------------
//

TBool CCTPinPinQueryDialog::CheckIfEntryTextOk() const
    {
    TPINValue pin1;
    TPINValue pin2;
    if (Layout_Meta_Data::IsLandscapeOrientation())
        {
        if ( FirstControl()->IsFocused() )
            {
            TPINValue pin1;
            FirstControl()->GetText(pin1);    
            return ((pin1.Length() <= iMaxTextLength && pin1.Length() >= iMinTextLength));
            }
        else if ( SecondControl()->IsFocused() )
           {
           TPINValue pin2;
           SecondControl()->GetText(pin2);    
           return ((pin2.Length() <= iMaxTextLength && pin2.Length() >= iMinTextLength));
           }
        }

    FirstControl()->GetText(pin1);   
    SecondControl()->GetText(pin2); 
    return ((pin1.Length() <= iMaxTextLength && pin1.Length() >= iMinTextLength)&&(pin2.Length() <= iMaxTextLength && pin2.Length() >= iMinTextLength));       
    }
// -----------------------------------------------------------------------------
// CCTPinPinQueryDialog::UpdateLeftSoftKeyL()
// -----------------------------------------------------------------------------
//
void CCTPinPinQueryDialog::UpdateLeftSoftKeyL()
    {
	CAknMultiLineDataQueryDialog::UpdateLeftSoftKeyL();

	MakeLeftSoftkeyVisible(CheckIfEntryTextOk());
    }
// -----------------------------------------------------------------------------
// CCTPinPinQueryDialog::PostLayoutDynInitL()
// -----------------------------------------------------------------------------
//
void CCTPinPinQueryDialog::PostLayoutDynInitL()
    {
	((CEikServAppUi*)(CEikonEnv::Static())->EikAppUi())->SuppressAppSwitching(ETrue);
    }

// -----------------------------------------------------------------------------
// CCTPinPinQueryDialog::OkToExitL()
// -----------------------------------------------------------------------------
//
TBool CCTPinPinQueryDialog::OkToExitL( TInt aButtonId )
    {
    if (!CAknMultiLineDataQueryDialog::OkToExitL(aButtonId))
        {
        return EFalse;
        }        
    if ( aButtonId == EEikBidCancel || aButtonId == EAknSoftkeyNo )
	    {
	    iRetVal = EFalse;
	    User::RequestComplete(iClientStatus, KErrNone);  	
	    }
    else if ( aButtonId ==EAknSoftkeyYes || aButtonId == EAknSoftkeyOk)
	    {
        FirstControl()->GetText(iPinValue1);
        SecondControl()->GetText(iPinValue2);
	    iRetVal = ETrue;
	    User::RequestComplete(iClientStatus, KErrNone); 
        
	    }
	((CEikServAppUi*)(CEikonEnv::Static())->EikAppUi())->SuppressAppSwitching(EFalse);
	return ETrue; 
    }

// -----------------------------------------------------------------------------
// CCTPinPinQueryDialog::OfferKeyEventL()
// -----------------------------------------------------------------------------
//
TKeyResponse CCTPinPinQueryDialog::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
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
	return CAknMultiLineDataQueryDialog::OfferKeyEventL(aKeyEvent, aType);
    }

//  End of File  
