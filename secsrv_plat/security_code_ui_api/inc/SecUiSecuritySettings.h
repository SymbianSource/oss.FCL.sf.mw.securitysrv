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

#define KSecUiAskNever 1
#define KSecUiAskOnlyIfInvalid 2
#define KSecUiAskAlways 3

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
		static void ShowResultNoteL(TInt aResourceID, TInt aTone);

        IMPORT_C TInt ChangeRemoteLockStatusL( TBool& aRemoteLockStatus, TDes& aRemoteLockCode, TInt aAutoLockPeriod );
        
        TInt RemoteLockCodeQueryL( TDes& aRemoteLockCode );
        
		/** 
		* Same functions, taking parameters. This is used in QT because the Settings handles it.
		*
		* @param aOldPassword RMobilePhone::TMobilePassword  : current password, probably asked to the user by other means
		* @param aNewPassword RMobilePhone::TMobilePassword  : current password, probably asked to the user by other means
		* @param aFlags	 			TInt  : when to ask for the password
		* @param aCaption			TDes& : caption to show in the dialog. If NULL, then the function will calculate it. If empty, it will be empty
		* @param aShowError		TInt  : in case of error, it will show it. If this is not set, then no errors will be displayed.
		* @return TInt: KErrNone (succesful) , KErrGsm0707IncorrectPassword, KErrAccessDenied, KErrGsmSSPasswordAttemptsViolation, KErrLocked, KErrGsm0707OperationNotAllowed, KErrAbort, KErrNotSupported, ...
		*/
		IMPORT_C TInt  ChangePinParamsL(RMobilePhone::TMobilePassword aOldPassword, RMobilePhone::TMobilePassword aNewPassword, TInt aFlags, TDes& aCaption, TInt aShowError);
		IMPORT_C TInt  ChangeUPinParamsL(RMobilePhone::TMobilePassword aOldPassword, RMobilePhone::TMobilePassword aNewPassword, TInt aFlags, TDes& aCaption, TInt aShowError);
		IMPORT_C TInt  ChangePin2ParamsL(RMobilePhone::TMobilePassword aOldPassword, RMobilePhone::TMobilePassword aNewPassword, TInt aFlags, TDes& aCaption, TInt aShowError);
		IMPORT_C TInt  ChangeSecCodeParamsL(RMobilePhone::TMobilePassword aOldPassword, RMobilePhone::TMobilePassword aNewPassword, TInt aFlags, TDes& aCaption, TInt aShowError);
		IMPORT_C TInt  ChangeAutoLockPeriodParamsL(TInt aPeriod, RMobilePhone::TMobilePassword aOldPassword, TInt aFlags, TDes& aCaption, TInt aShowError);
		/* if aOldPassword is used, then try to verify.
		KSecUiAskNever, KSecUiAskOnlyIfInvalid, KSecUiAskAlways
		*/
		IMPORT_C TInt  AskSecCodeParamsL(RMobilePhone::TMobilePassword &aOldPassword, TInt aFlags, TDes& aCaption, TInt aShowError);
		IMPORT_C TInt  ChangePinRequestParamsL(TInt aEnable, RMobilePhone::TMobilePassword aOldPassword, TInt aFlags, TDes& aCaption, TInt aShowError);
		IMPORT_C static HBufC* TranslateLC(const TDesC& aMessageId, TInt aFlags);

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
