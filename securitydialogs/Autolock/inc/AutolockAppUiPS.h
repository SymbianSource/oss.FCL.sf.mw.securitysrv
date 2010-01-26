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
*     Declares UI class for application.
*
*/


#ifndef AUTOLOCKAPPUIPS_H
#define AUTOLOCKAPPUIPS_H

// INCLUDES
#include <e32std.h>
#include <aknappui.h>
#include <etelmm.h>
#include <aknViewAppUi.h>
#include <AknEcs.h>
#include <aknnotedialog.h>
#include <AknIncallBubbleNotify.h>
#include <e32property.h>
#include <rmmcustomapi.h>
#include "AutolockWait.h"
#include "AutoKeyguardObserver.h"

#include "AutolockGripStatusObserver.h"
#include "AutolockFpsStatusObserver.h"

// FORWARD DECLARATIONS
class CAutolockContainer;
class CAutoLockModel;
class CValueObserver;

// CONSTANTS
const TUid KAutoLockViewId = {101}; // CDMA StartUp needs this UID.

// CLASS DECLARATIONS

/**
* part of emergency call handling when telephony+devicelock is active
* this solution is meant only for 3.1 and 3.2
*CEcsNote
*It defines security note used in security view.
*/
class CEcsNote : public CAknNoteDialog
    {
    public: // Constructors and destructors
        /**
        * C++ constructor.
        */
        CEcsNote();
    public: // new
        /**
        * Constructs sleeping note
        */
        void ConstructSleepingNoteL(TInt aResourceId);
        /**
        * Shows slpeeping note
        */
        TInt ShowNote();
        /**
        * Hides sleeping note
        */
        void SleepNote();

        TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,
                                         TEventCode aType);
                                         
        /**
        * API to set the emergency number to be displayed
        *
        * aMatchedNumber    text to display (e.g. "112" )
        */
        void SetEmergencyNumber( const TDesC& aMatchedNumber );
                
    public:
        TBool iNoteOnScreen;        
    };
  

/**
* Application UI class.
* Provides support for the following features:
* - UIKON control architecture
* 
*/
class CAutolockAppUi : public CAknViewAppUi, public MAknEcsObserver,
                       public MAutolockGripStatusObserver,
                       public MAutolockFpsStatusObserver
    {
    public: // // Constructors and destructor

        /**
        * Symbian OS default constructor.
        */      
        void ConstructL();

        /**
        * Destructor.
        */      
        ~CAutolockAppUi();
        
        /**
         * From MAutolockGripStatusObserver
         */
        TBool DeviceLockQueryStatus();
        
        TBool DeviceLockStatus();

        /**
         * From MAutolockFpsStatusObserver
         */
        TBool DeviceFpsLock(TInt iStatus);

   public: // New functions
		/**
        * Activates previous app
        */     
		void SwitchToPreviousAppL();
		/**
        * Activates device lock view
        */     
		void BringAppToForegroundL();
		/**
        * Pointer to model
        *
		* @return CAutoLockModel*
		*/     
		inline CAutoLockModel* Model() const;
		/**
        * Locks voice-key and app-key
        */     
		void LockKeysL();
		/**
        * UnLocks voice-key and app-key
        */     
		void UnLockKeys();
		/**
        * Locks voice-key
        */     
		void LockSideKeyL();
		/**
        * UnLocks voice-key
        */     
		void UnLockSideKey();
        /**
        * Returns ETrue if system is locked. 
        */
        inline TBool Locked() { return (iSideKey2 != 0); }
        /**
        * Checks whether PUK1 dialog is active. 
        */
		TBool IsPinBlocked();
		/**
        * Checks whether the phone has done hidden reset. 
        */
		TBool HiddenReset();
		void EnableWGListChangeEventListening();
		void DisableWGListChangeEventListening();

    protected:    
        /**
        * From CEikAppUi, handles the TARM unlock message, other messages
        * are propagated to the superclass handler.
        * @param aClientHandleOfTargetWindowGroup The window group that the message was sent to.
        * @param aMessageUid The message UID. 
        * @param aMessageParameters The message parameters
        * @return TMessageResponse EMessageHandled if the message was the TARM unlock message, 
        * otherwise the return value from the superclass handler. 
        */        
        MCoeMessageObserver::TMessageResponse HandleMessageL(
            TUint32 aClientHandleOfTargetWindowGroup,
            TUid aMessageUid,
            const TDesC8& aMessageParameters );
            

    private:
        // From MEikMenuObserver
        void DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane);
		
		// from CCoeAppUi
		void HandleForegroundEventL(TBool aForeground);
        /**
        * From CAknAppUi, called when screen layout changes 
        */
        void HandleScreenDeviceChangedL();
    private:
        /**
        * From CEikAppUi, takes care of command handling.
        * @param aCommand command to be handled
        */
        void HandleCommandL(TInt aCommand);

        /**
        * From CEikAppUi, handles key events.
        * @param aKeyEvent Event to handled.
        * @param aType Type of the key event. 
        * @return Reponse code (EKeyWasConsumed, EKeyWasNotConsumed). 
        */
        virtual TKeyResponse HandleKeyEventL(
            const TKeyEvent& aKeyEvent,TEventCode aType);
    virtual void HandlePointerEventL(const TPointerEvent& aPointerEvent);
     private:
    	/**
    	* From AknViewAppUi. Handles pointer-initiated view switch.
    	* @param aEvent Window server event.
    	* @param aDestination Pointer to the control which the event is targeted to.
    	*/
    	void HandleWsEventL( const TWsEvent& aEvent, CCoeControl* aDestination );

        /**
        * Used for communication with SysAp
        * @param aMessage message enumeration defined in eikon.hrh
        */        
        void SendMessageToSysAp(TInt aMessage);
    	
    	/**
        * From MAknEcsObserver
        * part of emergency call handling when telephony+devicelock is active
        * this solution is meant only for 3.1 and 3.2
        * handles emergency call
        */
        virtual void HandleEcsEvent( CAknEcsDetector* aEcsDetector, CAknEcsDetector::TState aState );
            
        TBool TarmState();
        void HandleWindowGroupListChange();
    private: //Data
   		CAutoLockModel*	 iModel;		 
		RTelServer       iServer;
	    RMobilePhone     iPhone;  
	    TInt			 iAppKey;
		TInt			 iSideKey1;
		TInt			 iSideKey2;
		CValueObserver*  iPhoneObserver;
		TBool			 iLocked;
		TBool            iDeviceLockQueryStatus;
        // Idle Incall Bubbles.
        CAknIncallBubble* iIncallBubble;
        CAutoKeyguardObserver* iKeyguardObserver;
        CAutolockGripStatusObserver* iGripStatusObserver;
        CAutolockFpsStatusObserver* iFpsStatusObserver;
        RMmCustomAPI	 iCustomPhone;
        
        // part of emergency call handling when t//iEmergencySupportReadyelephony+devicelock is active
        // this solution is meant only for 3.1 and 3.2
        CAknEcsDetector* iEcsDetector;
        CEcsNote*		 iEcsNote;	
        TBool            iEmergencySupportReady;
        CWait*			 iWait;	
		TRect aCallButtonRect;
	};

	inline CAutoLockModel* CAutolockAppUi::Model() const
		{return iModel;}

#endif

// End of File
