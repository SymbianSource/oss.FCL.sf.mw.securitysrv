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
* Description:   CT Select Certificate Dialog
*
*/



// INCLUDE FILES
#include "CTSelectCertificateDialog.h"
#include "CTSecurityDialogDefs.h"
#include <CTSecDlgs.rsg>
#include <uikon/eiksrvui.h>


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCTSelectCertificateDialog::CCTSelectCertificateDialog
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//

CCTSelectCertificateDialog::CCTSelectCertificateDialog(
    MDesCArray* /*aCertificates*/, 
    RArray<TCTTokenObjectHandle>& aCertHandleList, 
    TCTTokenObjectHandle& aSelectedCert,
    TRequestStatus& aClientStatus, 
    TBool& aRetVal)
    : CAknListQueryDialog(&iSelectedIndex), 
      iClientStatus(&aClientStatus), 
      iRetVal(aRetVal), 
      iSelectedIndex( 0 ),
      iCertHandleList( aCertHandleList ),
      iSelectedCert( aSelectedCert )
    {
    }

// -----------------------------------------------------------------------------
// CCTSelectCertificateDialog::~CCTSelectCertificateDialog()
// -----------------------------------------------------------------------------
//
CCTSelectCertificateDialog::~CCTSelectCertificateDialog()
    {
    }

// -----------------------------------------------------------------------------
// CCTSelectCertificateDialog::RunDlgLD()
// -----------------------------------------------------------------------------
//
void  CCTSelectCertificateDialog::RunDlgLD( 
    MDesCArray* aCertificates, 
    RArray<TCTTokenObjectHandle>& aCertHandleList,
    TCTTokenObjectHandle& aSelectedCert,
    TRequestStatus& aClientStatus, 
    TBool& aRetVal )
    {
	CCTSelectCertificateDialog* self = 
        new (ELeave) CCTSelectCertificateDialog(
        aCertificates, aCertHandleList, aSelectedCert, aClientStatus, aRetVal );

	self->PrepareLC(R_WIM_SELECT_CERTIFICATE_DIALOG);
	self->SetItemTextArray(aCertificates);
	self->SetOwnershipType(ELbmOwnsItemArray);
	self->RunLD();
    }

// -----------------------------------------------------------------------------
// CCTSelectCertificateDialog::PostLayoutDynInitL()
// -----------------------------------------------------------------------------
//
void CCTSelectCertificateDialog::PostLayoutDynInitL()
    {
	((CEikServAppUi*)(CEikonEnv::Static())->EikAppUi())->SuppressAppSwitching(ETrue);
    }

// -----------------------------------------------------------------------------
// CCTSelectCertificateDialog::OkToExitL()
// -----------------------------------------------------------------------------
//
TInt CCTSelectCertificateDialog::OkToExitL(TInt aButtonId)
    {
    if ( aButtonId == EEikBidCancel )
	{
		iRetVal = EFalse;
		User::RequestComplete(iClientStatus, KErrNone);
	}
	//Left softkey or Selection key was pressed
    else if ( aButtonId ==EAknSoftkeySelect || aButtonId == EAknSoftkeyOk)
	{
		iRetVal = ETrue;
		iSelectedIndex = ListBox()->CurrentItemIndex();
        iSelectedCert = iCertHandleList[iSelectedIndex];
		User::RequestComplete(iClientStatus, KErrNone);  		
	}
	((CEikServAppUi*)(CEikonEnv::Static())->EikAppUi())->SuppressAppSwitching(EFalse);
	return ETrue; 	
    }

// End of file
