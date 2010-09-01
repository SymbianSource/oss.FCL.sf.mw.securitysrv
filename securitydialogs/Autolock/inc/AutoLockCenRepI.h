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


#ifndef _AUTOLOCKCENREPI_H__
#define _AUTOLOCKCENREPI_H__

#include <e32std.h>
#include <cenrepnotifyhandler.h> // link against CenRepNotifHandler.lib
#include "AutolockAppUiPS.h"


class CAutolockCenRepI : public CBase, public MCenRepNotifyHandlerCallback
	{
	public:
		/**
        * Creates instance of the CAutolockShareDataI class.
        *
		* @param aAppUi (pointer to autolock appui)
		* @return Returns the instance just created.
        */
		static CAutolockCenRepI* NewLC(CAutolockAppUi* aAppUi);
		/**
        * Creates instance of the CAutolockShareDataI class.
        *
		* @param aAppUi (pointer to autolock appui)
		* @return Returns the instance just created.
        */
		static CAutolockCenRepI* NewL(CAutolockAppUi* aAppUi);
		/**
        * Destructor.
        */      
		~CAutolockCenRepI();
	public:
		/**
        * Currenst autolock period
        *
		* @return TInt (curenst autolock period in minutes) 
		*/      
		TInt Timeout();	
		/**
        * Sets device lock value in SharedData
        *
		* @param: aLocked (decice lock on/off) 
		*/
		void SetLockedL(TBool aLockValue);
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
		* @param aAppUi (pointer to autolock appui)
        */
		CAutolockCenRepI(CAutolockAppUi* aAppUi);
		/**
        * Symbian OS constructor.
        */
		void ConstructL();
	private:
		/**
        * Handles period changed event. Called by CSharedDataInteger
		*
		* @param aPtr (pointer to CAutolockShareDataI)
        * @return KErNone
		*/
		TInt HandleTimeoutChangedL();
		/**
        * Tells model that autolock period has been changed
		*/
		void ResetInactivityTimeout();	
	private: // data
		CAutolockAppUi*          iAppUi; //not owned!
        CCenRepNotifyHandler* iNotifyHandler;
        CRepository* iSession;

	};

#endif
// END OF FILE
