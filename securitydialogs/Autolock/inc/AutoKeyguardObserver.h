/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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


#ifndef _AUTOKEYGUARDOBSERVER_H__
#define _AUTOKEYGUARDOBSERVER_H__

#include <e32base.h>
#include <aknappui.h>
#include <e32property.h>

class CUserActivityManager;
class CAutoKeyguardCenRepI;

class CAutoKeyguardObserver : public CBase
	{
	public:
		/**
        * Creates instance of the CAutoKeyguardObserver class.
        *
		* @return Returns the instance just created.
        */
		static CAutoKeyguardObserver* NewL();
		/**
        * Symbian OS constructor.
        */
		void ConstructL( CAutoKeyguardObserver* aObserver );
		/**
        * Destructor.
        */
		~CAutoKeyguardObserver();
	public:
		/**
        * Gets new autokeyguard period and starts monitoring user activity 
        */
		void ResetInactivityTimeout();
		/**
        * Activates keyguard 
		*/
		void LockKeysL();
	private:
		/**
        * C++ default constructor.
        */
		CAutoKeyguardObserver(); 
	private:
		/**
        * Return current autokeyguard period
		*
		* @return TInt (autokeyguard period in minutes)
        */	
		TInt AutoKeyguardTimeout();
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
		* @param aPtr (pointer to CAutoKeyguardObserver)
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
        CAutoKeyguardCenRepI*    iCenRepI;
	};
#endif
// END OF FILE
