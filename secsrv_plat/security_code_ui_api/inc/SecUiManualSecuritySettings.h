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
*    Provides api for changing security settings via PhoneApp
*
*
*/


#ifndef     __SECUIMANUALSECURITYSETTINGS_H
#define     __SECUIMANUALSECURITYSETTINGS_H

//  INCLUDES
// #include <etelagsm.h>
#include <etelmm.h>
#include <aknnotedialog.h>
#include <rmmcustomapi.h>

class CWait;

class CManualSecuritySettings : public CBase
	{
    public:
		/**
        * Creates instance of the CManualSecuritySettings class.
        *
		* @return Returns the instance just created.
        */
		IMPORT_C static CManualSecuritySettings* NewL();
		/**
        * Destructor.
        */
		IMPORT_C ~CManualSecuritySettings();		
	public:
		/**
		* Enumerates pins.
		*/
		enum TPin
			{
			EPin1,
			EPin2
			};
		
		/**
		* Changes pin.
		*
		* Note that contents of aOld, aNew & aVerifyNew haven't been checked. If they could not been parsed,
		* they are empty strings (KNullDesC).
		*
		* @param aPin pin in question.
		* @param aOld old pin.
		* @param aNew new pin.
		* @param aVerifyNew new pin, verification.
		* @return ETrue iff pin was changed successfully.
		*/
		IMPORT_C TBool ChangePinL(TPin aPin,const TDesC& aOld,const TDesC& aNew,const TDesC& aVerifyNew );	

		/**
		* Cancels change pin operation.
		*
		* If there is no ongoing change pin operation, then
		* this method does nothing.
		*/
		IMPORT_C void CancelChangePin();

		/**
		* Unblocks pin.
		*
		* Note that contents of aPuk, aNew & aVerifyNew haven't been checked. If they could not been parsed,
		* they are empty strings (KNullDesC).
		*
		* @param aPin pin in question.
		* @param aPuk puk code for the pin.
		* @param aNew new pin.
		* @param aVerifyNew new pin, verificatio.
		* @return ETrue iff pin was unblocked successfully.
		*/
		IMPORT_C TBool UnblockPinL(TPin aPin,const TDesC& aPuk,const TDesC& aNew,const TDesC& aVerifyNew );

		/**
		* Cancels unblock pin operation.
		*
		* If there is no ongoing unblock pin operation, then
		* this method does nothing.
		*/
		IMPORT_C void CancelUnblockPin();
			
		/**
		* Locks sim.
		*
		* Note that contents of aLockCode & aType haven't been checked. If they could not been parsed, 
		* they are empty strings (KNullDesC).
		*
		* @param aLockCode lock closing password.
		* @param aType type of lock.
		* @return ETrue iff SIM was locked successfully.
		*/
		IMPORT_C TBool LockSimL(const TDesC& aLockCode,const TDesC& aType );

		/**
		* Cancels lock sim operation.
		*
		* If there is no ongoing lock sim operation, then
		* this method does nothing.
		*/
		IMPORT_C void CancelLockSim();

		/**
		* Unlocks sim.
		*
		* Note that contents of aUnlockCode & aType haven't been checked. If they could not been parsed, 
		* they are empty strings (KNullDesC).
		*
		* @param aUnlockCode lock opening password.
		* @param aType type of lock.
		* @return ETrue if SIM was unlocked successfully.
		*/
		IMPORT_C TBool UnlockSimL(const TDesC& aUnlockCode,const TDesC& aType );

		/**
		* Cancels unlock sim operation.
		*
		* If there is no ongoing unlock sim operation, then
		* this method does nothing.
		*/
		IMPORT_C void CancelUnlockSim();
	public:
		/**
		* Shows operation results in note
		*
		* @param aTone CAknNoteDialog::TTone (tone which is played)
		* @param aResourceID TInt (notes resource id)
		*/
		void ShowResultNoteL(CAknNoteDialog::TTone aTone, TInt aResourceID);
	private: // constructors
		/**
        * C++ default constructor.
        */
        CManualSecuritySettings();
		 /**
		* Symbian OS constructor.
        */
        void ConstructL();	
	private:  // data
		/*****************************************************
		*	Series 60 Customer / ETel
		*	Series 60  ETel API
		*****************************************************/
		RMmCustomAPI	iCustomPhone;
		RTelServer      iServer;
		// RAdvGsmPhone    iPhone;
		RMobilePhone	iPhone;
		CAknNoteDialog* iNote;
		TRequestStatus	iDummyStatus;
		CWait*			iWait;

	};
#endif      
            
// End of file
