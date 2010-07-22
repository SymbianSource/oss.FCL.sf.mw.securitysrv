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
* Description:  Autolock period settingpage     
*
*
*/


#ifndef     __CAUTOLOCKSETTINGPAHGE_H
#define     __CAUTOLOCKSETTINGPAHGE_H

#include    <aknradiobuttonsettingpage.h>
#include    <eikmobs.h>

class CAutoLockSettingPage : public CBase
	{
	public:
		/**
		* C++ default constructor.
		*/
		CAutoLockSettingPage(TInt aResourceId, TInt& aCurrentSelectionItem, CDesCArrayFlat* aItemArray, TInt& aAutoLockValue);
		/**
		* Symbian OS constructor.
		*/
		void ConstructL();
		/**
		* Setmaximum value for autolock period. Used in TARM.
		*/
		void SetPeriodMaximumValue(TInt aMaximumValue);
		
	protected:
		/**
		* Destructor.
		*/
		~CAutoLockSettingPage();
		/**
		* Process ui commands
		* from CAknRadioButtonSettingPage
		* @param aCommandId TInt 
		*/
		void ProcessCommandL(TInt aCommandId);
		/**
    	* From CAknRadioButtonSettingPage 
    	* Handles AutoLockSettingPage's PointerEvent 
    	*    
    	* @param aPointerEvent PointerEvent to be handled
    	*/     
    	void HandlePointerEventL(const TPointerEvent& aPointerEvent);
    	
	private: // data
		// CEikButtonGroupContainer* iPopoutCba;
		TInt& iAutoLockValue;
		TInt iMaximum;
		TInt iOriginalIndex;

	};

#endif 

// End of file
