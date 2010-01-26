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
* Description:  Obsererver for Set System Locked event  
*
*
*/


#ifndef     __SECUILOCKOBSERVER_H
#define     __SECUILOCKOBSERVER_H

#include <e32svr.h>
#include <e32property.h>
#include "secui.hrh"

class CCodeQueryDialog;


NONSHARABLE_CLASS(CSecUiLockObserver): public CActive
    {
    public:
		/**
        * Creates instance of the CLockObserver class.
        *
		* @param aDialog (pointer to code dialog)
		* @return Returns the instance just created.
        */
        static CSecUiLockObserver* NewL(CCodeQueryDialog* aDialog, TInt aType  = ESecUiDeviceLockObserver);
		/**
        * Destructor.
        */
		~CSecUiLockObserver();
		/**
        * Stops listenig PubSub events.
        */
		void StopObserver(); 
		/**
        * Starts listenig PubSub events.
        */
		void StartObserver();
		/**
        * Sets the dialog's address.
        */
		void SetAddress(CCodeQueryDialog* aDialog);    
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
		* @param aDialog (pointer to code dialog)
        */
        CSecUiLockObserver(CCodeQueryDialog* aDialog, TInt aType);
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
    	CCodeQueryDialog*      iDialog; //not owned!
        RProperty            iProperty;
        TBool				 iSubscribedToEvent;
        TInt                 iType; //Type of the observer
    };

#endif 
// End of file
