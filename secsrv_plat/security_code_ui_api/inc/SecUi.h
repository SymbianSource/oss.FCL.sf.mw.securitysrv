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
*		Provides initialization and uninitialization for SecUi resource file
*
*
*/


#ifndef     __SECUI_H
#define     __SECUI_H

#if defined(__SECUI_H)
#define RDEBUG( x, y ) RDebug::Printf( "%s %s (%u) %s=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, x, y );
#else
#define RDEBUG( x, y )
#endif

class TSecUi
	{
	public:
		/**
		* Initializes SecUi resource file
		*
		* Should be called before using any other SecUi methods 
		*/
		IMPORT_C static void InitializeLibL();
		/**
		* Uninitializes SecUi resource file
		*
		* Should be called after finished using SecUi methods 
		*/
		IMPORT_C static void UnInitializeLib();
        	/**
        * Check if SecUi can be uninitialized, i.e. if client count is zero (or less).
        *
        * @return ETrue if SecUi can be freed, EFalse otherwise.
        */
		TBool CanBeFreed();

		/**
        * Increse client count for this thread.
        */
		void IncreaseClientCount();

		/**
        * Decrease client count for this thread
        */
		void DecreaseClientCount();

	private:
		/**
		* C++ default constructor.
		*/
		TSecUi();
		/**
		* Destructor.
		*/
		~TSecUi();
		/**
		* Symbian OS constructor.
		*/
		void ConstructL();
	private:
		TInt iResourceFileOffset;
        TInt iClientCount;
        TInt iDialogOpened;

	};
#endif

// End of file

