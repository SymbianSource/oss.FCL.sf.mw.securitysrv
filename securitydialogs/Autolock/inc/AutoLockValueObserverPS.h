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
*		Observer for phone events. Used to deactive/active the side-key
*
*
*/


#ifndef     __AUTOLOCKVALUEOBSERVERPS_H
#define     __AUTOLOCKVALUEOBSERVERPS_H

#include <e32svr.h>
#include <e32property.h>

class CAutolockAppUi;

class   CValueObserver: public CActive
    {
    public:
		/**
        * Creates instance of the CValueObserver class.
        *
		* @param aAppUi (pointer to autolock appui)
		* @return Returns the instance just created.
        */
        static CValueObserver* NewL(CAutolockAppUi* aAppUi);
		/**
        * Destructor.
        */
		~CValueObserver();    
	public:
		/**
        * Starts asynchronic listening KUidCurrentCall event
        *
		* @return KErrNone: if no errors
        * @return KErrInUse: if already listening
		*/
        TInt Start();
		/**
        * Stops asynchronic listening KUidAutolockStatus event
    	*/
		void Stop();
    private: // constructors
        /**
        * C++ default constructor.
		*
		* @param aAppUi (pointer to autolock appui)
        */
		CValueObserver(CAutolockAppUi* aAppUi);
        /**
        * Symbian OS constructor.
        */
		void ConstructL();
    private: // from CActive    
		/** @see CActive::RunL() */
		void RunL();
		/** @see CActive::DoCancel() */
        void DoCancel();
    private: // data
    	CAutolockAppUi*      iAppUi; //not owned!
        RProperty            iProperty;
    };

#endif 
// End of file
