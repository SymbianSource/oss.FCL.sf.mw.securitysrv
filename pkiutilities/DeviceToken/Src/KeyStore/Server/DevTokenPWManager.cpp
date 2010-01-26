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
* Description:   Implementation of DevToken Password manager
*
*/



#include "DevTokenPWManager.h"
#include "DevTokenDialog.h"
#include <pbedata.h>
#include <mctauthobject.h>
#include <securityerr.h>

//CONST
_LIT( KKeyStoreImportKeyLabel, "Passphrase of the imported key file" );

void TDevTokenPWManager::GetPassword( TPINValue& aValue, 
									  TRequestStatus& aStatus,
									  TBool aRetry)
    {
	  TPINParams params;
    //CTSecDlg will check the key label to decide the show-up dialog
	  params.iPINLabel.Copy(KKeyStoreImportKeyLabel);
	  params.iMinLength = 0;
	  params.iMaxLength = KMaxPINLength;
	  
	  DevTokenDialog::Dialog()->EnterPIN(params, aRetry, aValue, aStatus);
	  }

	
void TDevTokenPWManager::ImportPassword(TPINValue& aValue, 
												 TRequestStatus& aStatus)
	  {
	  GetPassword( aValue, aStatus );
	  }
	  

void TDevTokenPWManager::Cancel()
      {
	  DevTokenDialog::Dialog()->Cancel();
	  }

//EOF


