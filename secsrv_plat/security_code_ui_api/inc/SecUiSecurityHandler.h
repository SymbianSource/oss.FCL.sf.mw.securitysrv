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
*		Provides api for handling security events.
*
*
*/


#ifndef     __CSECURIHANDLER_H__
#define     __CSECURIHANDLER_H__

//  INCLUDES

#include <etelmm.h>
#include <rmmcustomapi.h>
#include <aknquerydialog.h>
#include <aknnotedialog.h>


// FORWARD DECLARATIONS
class RTASecuritySession;
class CCodeQueryDialog;

//  CLASS DEFINITIONS 
class CSecurityHandler : public CBase
	{
    public:  
		/**
        * C++ default constructor.
        */
		IMPORT_C CSecurityHandler(RMobilePhone& aPhone);	
		/**
        * Destructor.
        */
		IMPORT_C ~CSecurityHandler();
	public:
		/**
		* Handles different security events. Called from security notifier
		*
		* @param aEvent MAdvGsmPhoneSecurity::TSecurityEvent
		*/	
		IMPORT_C void HandleEventL(RMobilePhone::TMobilePhoneSecurityEvent aEvent);
        IMPORT_C void HandleEventL( RMobilePhone::TMobilePhoneSecurityEvent aEvent, TInt& aResult );
        IMPORT_C void HandleEventL( RMobilePhone::TMobilePhoneSecurityEvent aEvent, TBool aStartup, TInt& aResult );
		/**
		* Asks and verifies the security code.
        *
		* @return ETrue: code was accepted
        *         EFalse: user canceled the code query		  
		*/			
		IMPORT_C TBool AskSecCodeL();			
		/**
        * Clears activated security query. Called from security notifier when
		* there is incoming call
     	*/	
		IMPORT_C void CancelSecCodeQuery();	
		/**
		* Asks and verifies the security code when device is locked.
        * Unlocks the device if code was accepted
		*
		* @return ETrue: code was accepted and device is unlocked
        *         EFalse: user canceled the code query		  
		*/			
		IMPORT_C TBool AskSecCodeInAutoLockL(); 		
	private:    
		/**
        * Handles EPin1Required security event
     	*/	
        TInt Pin1RequiredL();
		/**
        * Handles EPuk1Required security event
     	*/	
        TInt Puk1RequiredL();
		/**
        * Handles EPin2Required security event
     	*/	
		void Pin2RequiredL();
		/**
        * Handles EPuk2Required security event
     	*/	
		void Puk2RequiredL();
        /**
        * Handles EUniversalPinRequired security event
     	*/	
        TInt UPinRequiredL();
        /**
        * Handles EUniversalPukRequired security event
     	*/	
        TInt UPukRequiredL();
		/**
        * Handles EPassPhraseRequired security event
     	*/	
        TInt PassPhraseRequiredL();
		/**
        * Handles sim lock pending event
     	*/	
		void SimLockEventL();
        /**
        *  Removes the splashscreen
        */
        void RemoveSplashScreenL() const;
        
        void ShowGenericErrorNoteL(TInt aStatus);
	private: // DATA
		/*****************************************************
		*	Series 60 Customer / ETel
		*	Series 60  ETel API
		*****************************************************/
		RMobilePhone& iPhone;
		TBool iQueryCanceled;
		CCodeQueryDialog* iSecurityDlg;
		CAknNoteDialog* iNoteDlg;
		TBool* iDestroyedPtr;
        RMmCustomAPI iCustomPhone;
        RTASecuritySession* iSecuritySession;
        TBool iStartup; // System state: true means we are in the middle of a boot.
	};
#endif                  
// End of file
