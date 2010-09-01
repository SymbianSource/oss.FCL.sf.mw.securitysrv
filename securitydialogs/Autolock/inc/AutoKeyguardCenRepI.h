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
*
*/


#ifndef _AUTOKEYGUARDCENREPI_H__
#define _AUTOKEYGUARDCENREPI_H__

#include <e32std.h>
#include <cenrepnotifyhandler.h> // link against CenRepNotifHandler.lib

class CAutoKeyguardObserver;

class CAutoKeyguardCenRepI : public CBase, public MCenRepNotifyHandlerCallback
	{
	public:
		/**
        * Creates instance of the CAutoKeyguardCenRepI class.
        *
		* @param aObserver (pointer to keyguard timeout observer)
		* @return Returns the instance just created.
        */
		static CAutoKeyguardCenRepI* NewLC(CAutoKeyguardObserver* aObserver);
		/**
        * Creates instance of the CAutoKeyguardCenRepI class.
        *
		* @param aObserver (pointer to keyguard timeout observer)
		* @return Returns the instance just created.
        */
		static CAutoKeyguardCenRepI* NewL(CAutoKeyguardObserver* aObserver);
		/**
        * Destructor.
        */      
		~CAutoKeyguardCenRepI();
	public:
		/**
        * Current autokeyguard period
        *
		* @return TInt (curenst autokeyguard period in minutes) 
		*/      
		TInt Timeout();	
        /**
        * From MCenRepNotifyHandlerCallback. Handles period changed event. Called by CenRep.
		*
		* @param aId (The id of the changed setting)
        * @param aNewValue (The new value of the changed setting)
        * @return KErNone
		*/
        void HandleNotifyInt(TUint32 aId, TInt aNewValue);

        void HandleNotifyError(TUint32 aId, TInt error, CCenRepNotifyHandler* aHandler);

        void HandleNotifyGeneric(TUint32 aId);

	private:
		/**
        * C++ default constructor.
		*
		* @param aObserver (pointer to keyguard timeout observer)
        */
		CAutoKeyguardCenRepI(CAutoKeyguardObserver* aObserver);
		/**
        * Symbian OS constructor.
        */
		void ConstructL();
	private:
		/**
        * Tells observer that keyguard period has been changed
		*/
		void ResetInactivityTimeout();	
	private: // data
		CAutoKeyguardObserver* iObserver; //not owned!
        CCenRepNotifyHandler* iNotifyHandler;
        CRepository* iSession;

	};

#endif
// END OF FILE
