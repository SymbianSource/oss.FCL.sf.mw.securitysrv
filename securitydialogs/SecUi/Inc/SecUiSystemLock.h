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
* Description:  System Lock interface
*
*/


#ifndef     __SECUISYSTEMLOCK_H
#define     __SECUISYSTEMLOCK_H

//  INCLUDES

#include    <e32base.h>
#include <e32property.h>
#include <etelmm.h>
#include	<rmmcustomapi.h>

//  CLASS DEFINITIONS 
class  CSystemLock : public CActive
    {
    public:
       /**
        * Creates instance of the CSystemLock class.
        *
		* @return Returns the instance just created.
        */
		IMPORT_C static CSystemLock* NewL();
		/**
        * Destructor.
        */
        IMPORT_C ~CSystemLock();
    public:
         /**
		* Sets the device as locked
		*/
		IMPORT_C void SetLockedL();
	private:
		/**
		* C++ default constructor.
		*/
		CSystemLock();
		/**
		* Symbian OS constructor.
		*/
		void ConstructL();
	private: // from CActive
        /** @see CActive::RunL() */
		void RunL();
		/** @see CActive::DoCancel() */
        void DoCancel();
	private:  // data
		/*****************************************************
		*	Series 60 Customer / ETel
		*	Series 60  ETel API
		*****************************************************/
		RTelServer			iServer;
		RMobilePhone		iPhone;
        RProperty           iProperty;
		RMmCustomAPI		iCustomPhone;
	};
#endif

// End of file
