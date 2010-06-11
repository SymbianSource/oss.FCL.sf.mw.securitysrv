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
* Description:   CT Read and Sign Dialog for Digital Signature
*
*/



// INCLUDE FILES
#include <platform/e32notif.h>
#include "CTSignTextDialog.h"
#include "CTSecurityDialogsLogger.h"
#include <barsread.h>
#include <CTSecDlgs.rsg>
#include <uikon/eiksrvui.h>
#include <StringLoader.h>


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCTSignTextDialog::CCTSignTextDialog
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CCTSignTextDialog::CCTSignTextDialog(const TDesC& aReceiptToSign, TRequestStatus& aStatus, 
                                       TBool& aRetVal, TBool aSignText)
: CAknMessageQueryDialog(EConfirmationTone),
iReceiptToSign(aReceiptToSign), iClientStatus(&aStatus), iRetVal(aRetVal), iSignText(aSignText)
    {
    }

// -----------------------------------------------------------------------------
// CCTSignTextDialog::~CCTSignTextDialog()
// -----------------------------------------------------------------------------
//
CCTSignTextDialog::~CCTSignTextDialog()
    {
    }

// -----------------------------------------------------------------------------
// CCTSignTextDialog::RunDlgLD()
// -----------------------------------------------------------------------------
//
void  CCTSignTextDialog::RunDlgLD(
    TInt aResourceId, const TDesC& aReceiptToSign, 
	TRequestStatus& aStatus, TBool& aRetVal)
    {
    CCTSignTextDialog* self = new (ELeave) CCTSignTextDialog(aReceiptToSign, aStatus, aRetVal, ETrue);  
    self->ExecuteLD(aResourceId);
    
    }

// -----------------------------------------------------------------------------
// CCTSignTextDialog::PreLayoutDynInitL()
// -----------------------------------------------------------------------------
//
void CCTSignTextDialog::PreLayoutDynInitL()
    {
    SetMessageTextL(iReceiptToSign);
	CAknMessageQueryDialog::PreLayoutDynInitL();
    }

// -----------------------------------------------------------------------------
// CCTSignTextDialog::PostLayoutDynInitL()
// -----------------------------------------------------------------------------
//
void CCTSignTextDialog::PostLayoutDynInitL()
    {
	((CEikServAppUi*)(CEikonEnv::Static())->EikAppUi())->SuppressAppSwitching(ETrue);
    }

// -----------------------------------------------------------------------------
// CCTSignTextDialog::OkToExitL()
// -----------------------------------------------------------------------------
//
TBool CCTSignTextDialog::OkToExitL( TInt aButtonId )

    {
    if ( iSignText )
        {
        if ( aButtonId == EAknSoftkeyYes )
            {    
		    iRetVal = ETrue;
		    User::RequestComplete(iClientStatus, KErrNone); 
            }

	    else if ( aButtonId == EAknSoftkeyNo || aButtonId == EEikBidCancel
            || aButtonId == EAknSoftkeyOk )
		    {
		    iRetVal = EFalse;
		    User::RequestComplete(iClientStatus, KErrNone); 
		    }
        }
    else
        {
        if (aButtonId == EAknSoftkeyOk) 
            {
            User::RequestComplete(iClientStatus, KErrNone); 
            }
        else
            {

            }
        }

	((CEikServAppUi*)(CEikonEnv::Static())->EikAppUi())->SuppressAppSwitching(EFalse);
	return ETrue;
    }   

// -----------------------------------------------------------------------------
// CCTSignTextDialog::OfferKeyEventL()
// -----------------------------------------------------------------------------
//
TKeyResponse CCTSignTextDialog::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
	return CAknMessageQueryDialog::OfferKeyEventL(aKeyEvent, aType);
    }

// End of File
