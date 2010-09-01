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
*		Provides api for changing security settings.  
*
*
*/


#ifndef     __CSECUISECURITYSETTINGS_H
#define     __CSECUISECURITYSETTINGS_H

//  INCLUDES
#include <etelmm.h>
#include <rmmcustomapi.h>
#include <aknnotedialog.h>


class CWait;
class CSecurityHandler;

//  CLASS DEFINITIONS 
class CSecuritySettings : public CBase
	{
    public: 
		/**
        * Creates instance of the CSecuritySettings class.
        *
		* @return Returns the instance just created.
        */
		IMPORT_C static CSecuritySettings* NewL();
		/**
        * Destructor.
        */
		IMPORT_C ~CSecuritySettings();
		/**
        * C++ default constructor.
        */
		IMPORT_C CSecuritySettings();
		/**
		* Symbian OS constructor.
        */
		IMPORT_C void ConstructL();
	public:
		/**
        * Launches dialogs for changing pin code 
        */
		IMPORT_C void  ChangePinL();
        /**
        * Launches dialogs for changing upin code
        */
		IMPORT_C void  ChangeUPinL();
		/**
        * Launches dialogs for changing pin2 code
        */
		IMPORT_C void  ChangePin2L();
		/**
        * Launches dialogs for changing security code
        */
		IMPORT_C void  ChangeSecCodeL();
		/**
        * Launches dialogs for changing autolock period
        *
		* @param aPeriod TInt (old autolock value)
		* @return TInt (new autolock value) 
		*/
		IMPORT_C TInt  ChangeAutoLockPeriodL(TInt aPeriod);	
		/**
        * Launches dialogs for changing sim change security setting
        *
		* @return ETrue: successful
		*         EFalse: user cancelled 
		*/
		IMPORT_C TBool ChangeSimSecurityL();				
		/**
        * Launches dialogs for changing pin request setting
        *
		* @return ETrue: successful
		*         EFalse: user cancelled
		*/
		IMPORT_C TBool ChangePinRequestL();
        /**
        * Launches dialogs for changing upin request setting
        *
		* @return ETrue: successful
		*         EFalse: user cancelled
		*/
		IMPORT_C TBool ChangeUPinRequestL();
        /**
        * Launches dialogs for changing code in use setting
        *
		* @return ETrue: successful
		*         EFalse: user cancelled
		*/
		IMPORT_C TBool SwitchPinCodesL();
		/**
        * Is aLockType lock on or off
        *
		* @param aLockType RAdvGsmPhone::TLockType
		* @return ETrue: lock is on
        *         EFalse: lock is off
		*/
		IMPORT_C TBool IsLockEnabledL(RMobilePhone::TMobilePhoneLock aLockType);
		/**
        * Launches dialogs for fixed dialing settings
        */
		IMPORT_C void SetFdnModeL();		
		/**
		* Gets current fixed dialing mode
		*
		* @param aFdnMode RMobilePhone::TMobilePhoneFdnStatus& (fixed dialing mode)
		* @return TInt: KErrNone (succesful) 
		*/
		IMPORT_C TInt GetFdnMode(RMobilePhone::TMobilePhoneFdnStatus& aFdnMode);		
		/**
		* Asks and verifies the security code.
        *
		* @return ETrue: code was accepted
        *         EFalse: user canceled the code query		  
		*/			
		IMPORT_C TBool AskSecCodeL();			
		/**
		* Asks and verifies the pin2 code.
		*
		* @return ETrue: code was accepted
		*         EFalse: user canceled the code query
		*/
		IMPORT_C TBool AskPin2L();
		/**
		* Checks whether UPIN code is supported.
		*
		* @return ETrue: UPIN code is supported.
		*         EFalse: UPIN code is not supported.
		*/
		IMPORT_C TBool IsUpinSupportedL();
		/**
		* Checks whether a code is blocked.
		*
		* @return ETrue:  code is blocked.
		*         EFalse: code is not blocked.
		*/
		IMPORT_C TBool IsUpinBlocked();
        /**
		* Checks whether a code is blocked.
		*
		* @return ETrue:  Upin is active.
		*         EFalse: Upin is not active.
		*/
		IMPORT_C TBool IsUpinActive();
        /**
        * Shows error note 
        *
		* @return Void
		*/			
		static void ShowErrorNoteL(TInt aError);
		/**
        * Shows error note 
        *
		* @return Void
		*/			
		static void ShowResultNoteL(TInt aResourceID, CAknNoteDialog::TTone aTone);

        IMPORT_C TInt ChangeRemoteLockStatusL( TBool& aRemoteLockStatus, TDes& aRemoteLockCode, TInt aAutoLockPeriod );
        
        TInt RemoteLockCodeQueryL( TDes& aRemoteLockCode );
        
    private:
    
        TInt RemoteLockSetLockSettingL( TBool aLockSetting );    
        
	private: // DATA	
		TBool iPuk1;
		TBool iPuk2;

		/*****************************************************
		*	Series 60 Customer / ETel
		*	Series 60  ETel API
		*****************************************************/
		RTelServer      iServer;
		RMobilePhone    iPhone;
		RMmCustomAPI	iCustomPhone;
		CWait*			iWait;
        CSecurityHandler* iSecurityHandler;
	};


#endif      
            
// End of file
