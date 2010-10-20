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
* Description:  Dialog used for code queries.
*
*
*/

// #include <AknQueryDialog.h>
#include "secuicodequerydialog.h"

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CCodeQueryDialog::CCodeQueryDialog()
// C++ constructor
// ----------------------------------------------------------
//
EXPORT_C CCodeQueryDialog::CCodeQueryDialog(TDes& aDataText,TInt aMinLength,TInt aMaxLength,TInt aMode, TBool aIsRemoteLockQuery) : CBase()
	{		
		aDataText = aDataText;
		aMinLength = aMinLength;
		aMaxLength = aMaxLength;
		aMode = aMode;
		aIsRemoteLockQuery = aIsRemoteLockQuery;
	}
//
// ----------------------------------------------------------
// CCodeQueryDialog::~CodeQueryDialog()
// Destructor
// ----------------------------------------------------------
//
CCodeQueryDialog::~CCodeQueryDialog()
	{
	}

// End of file

