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
* Description:   Generic CT Query Dialog
*
*/



// INCLUDE FILES

#include <platform/e32notif.h>
#include "CTQueryDialog.h"
#include "CTSecurityDialogsLogger.h"
#include <CTSecDlgs.rsg>
#include <uikon/eiksrvui.h>


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCTQueryDialog::CCTQueryDialog()
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CCTQueryDialog::CCTQueryDialog( TRequestStatus& aClienStatus, TBool& aRetVal, const TDesC& aQueryText, TInt aDlgButtonResourceId, const TCTQueryDialogType aDlgType):
CAknQueryDialog(
    aDlgType == ECTConfirmationDialog ? EConfirmationTone : 
    aDlgType == ECTErrorDialog ? EErrorTone : 
    aDlgType == ECTWarningDialog ? EWarningTone : ENoTone),
    iDlgType(aDlgType),
    iDlgButtonResourceId(aDlgButtonResourceId),
    iQueryText(aQueryText), 
    iClientStatus(&aClienStatus), 
    iRetVal(aRetVal)
    {
    }
// ---------------------------------------------------------
// CCTQueryDialog::~CCTQueryDialog()
// ---------------------------------------------------------
//

CCTQueryDialog::~CCTQueryDialog()
    {
    }

// ---------------------------------------------------------
// CCTQueryDialog::RunDlgLD
// ---------------------------------------------------------
//
void CCTQueryDialog::RunDlgLD(TRequestStatus& aClienStatus, TBool& aRetVal, const TDesC& aQueryText, TInt aDlgButtonResourceId, const TCTQueryDialogType aDlgType )
    {
	CCTQueryDialog* self = new (ELeave) CCTQueryDialog(aClienStatus, aRetVal, aQueryText, aDlgButtonResourceId, aDlgType);
	self->ExecuteLD(R_WIM_QUERY_DIALOG);
    }

// ---------------------------------------------------------
// CCTQueryDialog::PreLayoutDynInitL
// ---------------------------------------------------------
//
void CCTQueryDialog::PreLayoutDynInitL()
    {
	SetPromptL(iQueryText);

	CAknQueryDialog::PreLayoutDynInitL();
    }

// ---------------------------------------------------------
// CCTQueryDialog::PostLayoutDynInitL
// ---------------------------------------------------------
//
void CCTQueryDialog::PostLayoutDynInitL()
    {
	CAknQueryControl* control = STATIC_CAST(CAknQueryControl*,ControlOrNull(EGeneralQuery));

	TInt animResource = 0;
	TInt softKeyResource = 0;
	switch (iDlgType)
	{
		case ECTConfirmationDialog:
			animResource = R_QGN_NOTE_QUERY_ANIM;
			softKeyResource = R_AVKON_SOFTKEYS_YES_NO;
			break;
		case ECTErrorDialog:
			animResource = R_QGN_NOTE_ERROR_ANIM;
			softKeyResource = R_AVKON_SOFTKEYS_OK_EMPTY;
			break;
		case ECTInfoDialog:
			animResource = R_QGN_NOTE_INFO_ANIM;
			softKeyResource = R_AVKON_SOFTKEYS_OK_EMPTY;
			break;
		case ECTWarningDialog:
			animResource = R_QGN_NOTE_WARNING_ANIM;
			softKeyResource = R_AVKON_SOFTKEYS_OK_EMPTY;
			break;
		case ECTOkDialog:
			animResource = R_QGN_NOTE_OK_ANIM;
			softKeyResource = R_AVKON_SOFTKEYS_OK_EMPTY;
			break;
		default:
			animResource = R_QGN_NOTE_QUERY_ANIM;
			softKeyResource = iDlgButtonResourceId == 0 ? R_AVKON_SOFTKEYS_OK_EMPTY : iDlgButtonResourceId;
			break;
	}

	control->SetAnimationL(animResource);
	ButtonGroupContainer().SetCommandSetL(softKeyResource);

	CAknQueryDialog::PostLayoutDynInitL();
    ((CEikServAppUi*)(CEikonEnv::Static())->EikAppUi())->SuppressAppSwitching(ETrue);
    }

// ---------------------------------------------------------
// CCTQueryDialog::OkToExitL
// ---------------------------------------------------------
//
TBool CCTQueryDialog::OkToExitL( TInt aButtonId )
    {

    if ( aButtonId == EEikBidCancel || aButtonId == EAknSoftkeyNo )
        {

		iRetVal = EFalse;
		User::RequestComplete(iClientStatus, KErrNone); 
		
        }
    else if ( aButtonId ==EAknSoftkeyYes || aButtonId == EAknSoftkeyOk)
        {
		
		iRetVal = ETrue;
		User::RequestComplete(iClientStatus, KErrNone); 		
        
        }
     ((CEikServAppUi*)(CEikonEnv::Static())->EikAppUi())->SuppressAppSwitching(EFalse);

     return ETrue; 
    }

// End of File
