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


#ifndef _AUTOLOCKMODELPS_H__
#define _AUTOLOCKMODELPS_H__

#include <e32base.h>
#include <aknappui.h>
#include <e32property.h>

class CUserActivityManager;
class CAutolockCenRepI;
class CAutolockAppUi;
class CLockObserver;

class CAutoLockModel : public CBase
	{
	public:
		/**
        * Creates instance of the CAutoLockModel class.
        *
		* @param aAppUi (pointer to autolock appui)
		* @return Returns the instance just created.
        */
		static CAutoLockModel* NewL(CAutolockAppUi* aAppUi, TBool aLocked );
		/**
        * Symbian OS constructor.
        */
		void ConstructL( TBool aLocked );
		/**
        * Destructor.
        */
		~CAutoLockModel();
	public:
		/**
        * Gets new autolock period and starts monitoring user activity 
        */
		void ResetInactivityTimeout();
		/**
        * Brings autolock app to foreground and locks the system 
		*/
		void LockSystemL(TInt aAutolockState = 0);
		/**
        * Sets device lock value in SharedData and in SystemAgent 
        *
		* @param: aLocked (decice lock on/off) 
		*/
		void SetLockedL(TInt aAutolockState = 0);
	private:
		/**
        * C++ default constructor.
		*
		* @param aAppUi (pointer to autolock appui)
        */
		CAutoLockModel(CAutolockAppUi*	aAppUi); 
	private:
		/**
        * Return current autolock period
		*
		* @return TInt (autolock period in minutes)
        */	
		TInt AutoLockTimeout();
		/**
        * Handles Active event. Called by ActivityManager
		*
		* @param aPtr 
        * @return KErNone
		*/
		static TInt HandleActiveEventL(TAny* aPtr);
		/**
        * Handles Inactive event. Called by ActivityManager
		*
		* @param aPtr (pointer to CAutoLockModel)
        * @return KErNone
		*/
		static TInt HandleInactiveEventL(TAny* aPtr);
		/**
        * Starts monitoring user activity
        */		
		void StartActivityMonitoringL();
		/**
        * Stop monitoring user activity.
        */
		void StopActivityMonitoring();
		/**
        * Initializes activymanager 
        */
		void SetActivityManagerL();
		/**
        * UnInitializes activymanager 
        */
		void CancelActivityManager();
	private: //data
		CUserActivityManager*    iActivityManager;
		CAutolockAppUi*			 iAppUi; //not owned!
        CAutolockCenRepI*        iCenRepI;
		CLockObserver*			 iLockObserver;
		TBool					 iMonitoring;
		RProperty                iProperty;
	};
#endif
// END OF FILE
