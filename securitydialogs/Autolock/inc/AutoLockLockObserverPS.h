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
*		Obsererver for Set System Locked event  
*
*
*/


#ifndef     __AUTOLOCKLOCKOBSERVERPS_H
#define     __AUTOLOCKLOCKOBSERVERPS_H

#include <e32svr.h>
#include <e32property.h>
#include "AutolockAppUiPS.h"

class   CLockObserver: public CActive
    {
    public:
		/**
        * Creates instance of the CLockObserver class.
        *
		* @param aAppUi (pointer to autolock appui)
		* @return Returns the instance just created.
        */
        static CLockObserver* NewL(CAutolockAppUi* aAppUi);
		/**
        * Destructor.
        */
		~CLockObserver();    
	private:
        /**
        * Starts asynchronic listening KUidAutolockStatus event
        *
		* @return KErrNone: if no errors
        * @return KErrInUse: if already listening
		*/
		TInt Start();            
    private: // constructors
		/**
        * C++ default constructor.
		*
		* @param aAppUi (pointer to autolock appui)
        */
        CLockObserver(CAutolockAppUi* aAppUi);
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
