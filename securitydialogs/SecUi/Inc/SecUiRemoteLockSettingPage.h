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
* Description:  RemoteLock pass phrase setting page
*
*
*/


#ifndef __SECUIREMOTELOCKSETTINGPAGE_H__
#define __SECUIREMOTELOCKSETTINGPAGE_H__

#include <aknradiobuttonsettingpage.h>
#include <eikmobs.h>



// LOCAL CONSTANTS
const TInt KRemoteLockSettingItemOn  = 0;
const TInt KRemoteLockSettingItemOff = 1;



// CLASS DECLARATIONS

/** 
* CRemoteLockSettingPage
*/
NONSHARABLE_CLASS(CRemoteLockSettingPage) : public CAknRadioButtonSettingPage
	{
	public:
		/**
		* C++ default constructor.
		*/
		CRemoteLockSettingPage( TInt aResourceId, TInt& aCurrentSelectionItem, CDesCArrayFlat* aItemArray );

		/**
		* Symbian OS constructor.
		*/
		void ConstructL();

	protected:
		/**
		* Destructor.
		*/
		~CRemoteLockSettingPage();

		/**
		* Process ui commands
		* from CAknRadioButtonSettingPage
		* @param aCommandId TInt 
		*/
		void ProcessCommandL( TInt aCommandId );
			/**
    	* From CAknRadioButtonSettingPage 
    	* Handles AutoLockSettingPage's PointerEvent 
    	*    
    	* @param aPointerEvent PointerEvent to be handled
    	*/     
    	void HandlePointerEventL(const TPointerEvent& aPointerEvent);

	private: // data

        /** Remote lock status */
		TInt& iRemoteLockStatus;
		TInt iPrevSelectionItem;
	};



#endif 



// End of file


