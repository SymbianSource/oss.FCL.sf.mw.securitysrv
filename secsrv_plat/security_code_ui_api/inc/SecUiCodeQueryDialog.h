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
* Description: 
*		Dialog used for code queries. Inherits AknQueryDialog
*
*
*/

#ifndef __SECUICODEQUERYDIALOG__
#define __SECUICODEQUERYDIALOG__

#include <secui.hrh>

class CCodeQueryDialog : public CBase
	{
	public://construction and destruction
		/**
        * C++ Constructor.
        * @param aDataText TDes& (code which is entered in query)
		* @param aMinLength TInt (code min length)
		* @param aMaxLength TInt (code max length)
		* @param aMode TInt (mode ESecUiCodeEtelReqest\ESecUiNone)
		*/
		IMPORT_C CCodeQueryDialog(TDes& aDataText, TInt aMinLength,TInt aMaxLength,TInt aMode, TBool aIsRemotelockQuery = EFalse);
		/**
        * Destructor.
        */
		~CCodeQueryDialog();

	};
#endif

// End of file
