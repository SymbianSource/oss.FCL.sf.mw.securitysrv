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
* Description:  The main header file of RemoteLock Engine
*
*/



#ifndef REMOTELOCK_H
#define REMOTELOCK_H

// INCLUDE FILES 
#include <msvapi.h>
#include <RemoteLockSettings.h>
#include <cenrepnotifyhandler.h>   
#include <centralrepository.h>
#include "RLLockObserver.h"

// CONSTANTS 

// MACROS 

// FORWARD DECLARATIONS 
class CClientMtmRegistry;

// LOCAL FUNCTION PROTOTYPES 
LOCAL_C TInt ThreadStartL();

// MEMBER FUNCTIONS 

// CLASS DECLARATION
/**
* Main class for remote lock program
* 
* @lib 
* @since S60 3.2
*/
class CRemoteLock : public CTimer,
                    public MMsvSessionObserver,
                    public MRemoteLockSettingsHandler,
                    public MRLLockObserverHandler,
                    public MCenRepNotifyHandlerCallback
    {
    public:
        /**
        * Two-Phased constructor
        */
        static CRemoteLock* NewL();  
        
        /**
        * Destructor.
        */ 
        virtual ~CRemoteLock();   
         
        // from base class MMsvSessionObserver 

        /**
        * From MMsvSessionObserver 
        * Session observer 
        *
        * @param aEvent EMsvEntriesCreated when new sms arrives, 
        *  EMsvServerReady while MSV server is ready,
        *  EMsvServerFailedToStart while server is failed to start
        *  EMsvCloseSession while session is closed
        *  EMsvServerTerminated while server is terminated.
        * @param aArg1 A CMsvEntrySelection of the new entries. 
        * @param aArg2 The TMsvId of the parent entry 
        * @param aArg3 Event type-specific argument value 
        * @return
        */
        void HandleSessionEventL( 
            TMsvSessionEvent aEvent, 
            TAny* aArg1, 
            TAny* aArg2, 
            TAny* aArg3  );
       
        /**
        * Remote Lock Settings notify handler called
        * when remote lock settings are changed by client.
        *
        * @param 
        * @return
        */
        void HandleRemoteLockNotifyL();
        
        
    public: // from MRLLockObserverHandler
        
        void HandleUnlockEvent(); 

    private:
    
        CRemoteLock();

        void ConstructL();  
        
        /**
        * Handles remote lock message.
        *
        * @param aEntry Message entry to be handled. 
        * @return       
        */
        void HandleMessageL( CMsvEntry* aEntry );
        
        /**
        * Compares lock code from SMS against correct lock code. 
        * If the lock code is correct, lock the phone.
        *
        * @param
        * @return ETrue when remotelock is correct.
        *         EFalse remotelock is not correct.
        */
        TBool VerifyAndLockL();

        /**
        * Check the remote lock settings.
        *
        * @param
        * @return
        */
        void CheckSettingsL();

        /**
        * Check whether memory card is locked or not.
        *
        * @param  aDriveInfo Drive information
        * @return ETrue if memory card is locked, 
        *         EFalse if memory card isn't locked.
        */
        TBool IsMemoryCardLocked( const TDriveInfo& aDriveInfo ) const;
        
        /**
        * Check whether memory card has password or not.
        *
        * @param  aDriveInfo Drive information
        * @return ETrue if memory card has password, 
        *         EFalse if memory card doesn't have password.
        */
        TBool HasMemoryCardPassword( const TDriveInfo& aDriveInfo ) const;
        
        /**
        * Check whether memory card is present or not.
        *
        * @param  aDriveInfo Drive information
        * @return ETrue memory card is in the phone, 
        *         EFalse memory card is not in the phone.
        */
        TBool IsMemoryCardPresent( const TDriveInfo& aDriveInfo ) const;
        
        /**
        * Sets the password to memory card, if the memory card is mounted into the phone
        * and memory card doesn't have password or not locked
        *
        * @param  aClear if ETrue, then remove the password
        * @return ETrue if memory card is locked or password removed,
        *         EFalse memory card is not locked.  
        */
        TBool SetMemoryCardPasswdL( const TBool aClear ); 

        /**
        * Converts memory card password format
        *
        * @param aPassword Password that needs to be converted
        *        aClear if ETrue, then remove the password
        * @return
        */
        void ConvertMemoryCardPassword( TMediaPassword& aPassword, const TBool aClear );
         
        /**
        * Initialize a new reply message when phone is on lock status.
        *
        * @param
        * @return ETrue if initialize is succeed,
        *         EFalse inistialize is failed.
        */
        TBool InitialSendMessageL();

        /**
        * Creates a new message server entry and set up default values.
        *
        * @param
        * @return TMsvId a Id of operational progress entry.
        */
        TMsvId CreateNewMessageL();
                
        /**
        * Set up current message entry.
        *
        * @param aEntryId Entry Id of message.
        * @return
        */
        void SetEntryL( TMsvId aEntryId );
                
        /**
        * Moves an entry to another parent entry.
        *
        * @param aEntryId Entry Id of target.
        * @return TMsvId a message entry
        */
        TMsvId MoveMessageEntryL( TMsvId aTarget );
        
        /**
        * Reply SMS when the terminal or memory card is successfully locked.
        *
        * @param aRecipientAddress the phone number which is used to send Lock
        *                          sms
        * @return ETrue if message is successfully sent,
        *         EFalse message is not sent.  
        */
        TBool SendMessageL( const TDesC& aRecipientAddress );
        
        /**
        * Schedules the message to be immediately sent through the message server.
        *
        * @param aSelection A point contains TMsvId
        * @return       
        */
        void SetScheduledSendingStateL( CMsvEntrySelection* aSelection );
        
        /**
        * Try to re-connect message server every 10 seconds after 
        * it is shut down or terminated.
        *
        * @param 
        * @return       
        */
        void RunL();
        
        /**
        * Checks if there are any unicode characters in the message
        *
        * @param TDesC& a reference to a string to be checked
        * @return TBool indicates whether we have unicode chars or not
        */
        TBool NeedsToBeSentAsUnicodeL( const TDesC& aInputString ) const;
        
        /**
        * Active the device lock to lock the phone
        */
        TBool ActivateDeviceLock();

        
        /**
        * Get the current profile 
        *
        * @param    aProfile profile id
        * @return   ETrue if succeeded
        *           EFalse otherwise
        */
        TBool GetProfile( TInt& aProfile );
        
        /**
        * Setup Profile notifier
        *
        * @param aNotifyEnable enable or disable notifier
        * @return    ETrue
        *            EFalse
        */
        TBool ProfileNotifyL( const TBool aNotifyEnable ); 
        
        
        /**
        * Call back function of Central Repository
        * from MCenRepNotifyHandlerCallback
        */
        void HandleNotifyGeneric( TUint32 aId );

        
        /**
        * Do Sets the password to memory card, if the memory card is mounted into the phone
        * and memory card doesn't have password or not locked
        *
        * @param  aClear if ETrue, then remove the password
        * @return ETrue if memory card is locked or password removed,
        *         EFalse memory card is not locked.  
        */
        TBool DoSetMemoryCardPasswdL( TBool aClear );
        
    private: // Data       
        // Remote lock code from SMS.
        HBufC* iRemoteLockCode;
        
        // memory card password buffer having the same
        // content with iRemoteLockCode buffer 
        HBufC* iMemoryCardPasswd;
        
        // Correct remote lock code digest.
        HBufC* iStoredCode;
        
        // Correct remote lock code digest (spaces trimmed).
        HBufC* iStoredTrimmedCode;

        // If remote lock is enabled or not.
        TBool iIsEnabled; 
        
        // The lock state of memory card
        TBool iStateMemoryCard;
        
        CRemoteLockSettings* iRemoteLockSetting;       

        CMsvSession* iMsvSession;        

        CBaseMtm* iClientMtmForSending;       

        CClientMtmRegistry* iMtmReg;  
        
        //Indicate if the phone is locked by remote lock
        TBool iLockedByRL;   
        
        //Indicate if the memory card is locked by remote lock
        TBool iMemoryCardLockedByRL;
        
        //Device lock observer 
        CRLLockObserver* iObserver;
        
        CRepository*  iProfileSession;
        
        CCenRepNotifyHandler*  iProfileNotifyHandler;  
        
        //Boolean used to indicate whether we have subscribed profile 
        //changes in CenRep or not, default value is EFalse; 
        TBool iSubscribeProfile;
        
        //Current profile
        TInt iCurrentProfile;
             
    };        

#endif 
