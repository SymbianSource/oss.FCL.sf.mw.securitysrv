/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of DevTokenDialog
*
*/



#include <e32base.h>
#include "DevTokenDialog.h"
#include <secdlg.h>

MSecurityDialog* DevTokenDialog::iDialog;

void DevTokenDialog::InitialiseL()
    {
	  iDialog = SecurityDialogFactory::CreateL();
	  }


void DevTokenDialog::Cleanup()
    {
	  if ( iDialog )
	      {
	      iDialog->Release();	
	  	  }
	  }

//EOF
