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
* Description:  Provides editor clearing method for CodeQueryDialog
*               and CodeRequestQueryDialog.
*
*
*/




#ifndef __SECUICODEQUERYCONTROL__
#define __SECUICODEQUERYCONTROL__

#include <aknQueryControl.h>

class CCodeQueryControl : public CAknQueryControl
	{
	public://new functions
		/**
		* Clears the secret editor
		* 
		*/
		void ResetEditorL();
		/**
		* Plays sounds
		* 
		* @param aSid TInt 
		*/
		void PlaySound(TInt aSid);
		/**
		* Checks if there is an emergency number in the input field.
		*  
		*/
		TBool IsEmergencyNumber();
		/**
		* Checks if there is an emergency call support flag is on.
		*  
		*/
		TBool IsEmergencyCallSupportOn();
	};

#endif

// End of file
