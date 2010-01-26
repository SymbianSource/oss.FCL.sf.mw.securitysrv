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
* Description:  API for applications to access remote lock settings.
*
*/


#ifndef REMOTELOCKSETTINGS_H
#define REMOTELOCKSETTINGS_H


// INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <e32cons.h>
#include <cenrepnotifyhandler.h>      

// CONSTANT

// MACROS 
/** Maximum length of user-given remote lock code */
const TInt KRLockMaxLockCodeLength = 20;
/** Minimum length of user-given remote lock code */
const TInt KRLockMinLockCodeLength = 5;
/** Length of the stored remote lock code */
const TInt KRLockStoredLockCodeLength = 32;

// DATA TYPES 

// FUNCTION PROTOTYPES 

// FORWARD DECLARATIONS 
class MRemoteLockSettingsHandler;


// CLASS DECLARATION
/**
* Interface for handling remote lock settings
*
* @lib rlocksettings.lib
* @since S60 3.2
*
*/
class CRemoteLockSettings: public CBase, public MCenRepNotifyHandlerCallback 
    {
    public:                
        /**
        * Two-Phased constructor
        *
        * @since S60 3.2
        * @param aHandler Client implemented notification handler that will be
        *                 called when remote lock settings are changed. 
        *                 If the client doesn't need notification, then no 
        *                 parameter needed.
        * @return
        */
        IMPORT_C static CRemoteLockSettings* NewL( 
            MRemoteLockSettingsHandler* aHandler = NULL );        
         
        /**
        * Two-Phased constructor
        *
        * @since S60 3.2
        * @param aHandler Client implemented notification handler that will be
        *                 called when remote lock settings are changed. 
        *                 If the client doesn't need notification, then no 
        *                 parameter needed.
        * @return
        */              
        IMPORT_C static CRemoteLockSettings* NewLC( 
            MRemoteLockSettingsHandler* aHandler  = NULL);
        
        IMPORT_C virtual ~CRemoteLockSettings();

        /**
        * Gets the remote lock code. Note that the retrieved remote lock code 
        * is not in plain format.
        *
        * @since S60 3.2
        * @param aLockCode For retrieving the remote lock code. The length 
        *                  must be at least KRLStoredLockCodeLength.
        * @return ETrue if succeeded,
        *         EFalse otherwise.
        */
        IMPORT_C TBool GetLockCode( TDes& aLockCode ) const;
        
        /**
        * Gets the remote lock code. Note that the retrieved remote lock code 
        * is not in plain format.
        *
        * @since S60 3.2
        * @param aLockCode For retrieving the remote lock code. The length 
        *                  must be at least KRLStoredLockCodeLength.
        * @param aLockCodeTrimmed For retrieving the remote lock code. 
        *                         The length must be at least 
        *                         KRLStoredLockCodeLength. Same as aLockCode, 
        *                         but formatted differently before storing: 
        *                         leading and trailing space characters 
        *                         removed, multiple spaces replaced with 
        *                         single space.
        * @return ETrue if succeeded,
        *         EFalse otherwise.
        */
        IMPORT_C TBool GetLockCode( 
        	TDes& aLockCode, 
        	TDes& aLockCodeTrimmed ) const;
        
        /**
        * Enables remote lock and stores the correct remote lock code.
        *
        * @since S60 3.2
        * @param aLockCode New remote lock code.
        * @return ETrue if succeeded,
        *         EFalse otherwise. 
        */
        IMPORT_C TBool SetEnabledL( const TDesC& aLockCode );
        
        /**
        * Disables remote lock.
        *
        * @since S60 3.2
        * @param
        * @return ETrue if succeeded,
        *         EFalse otherwise. 
        */
        IMPORT_C TBool SetDisabled();

        /**
        * Gets the current remote lock status.
        *
        * @since S60 3.2
        * @param aEnabled For retrieving the current remote lock status.
        * @return ETrue if succeeded,
        *         EFalse otherwise.
        */
        IMPORT_C TBool GetEnabled( TBool& aEnabled ) const;      
        
        /**
        * Compares two remote lock codes.
        *
        * @since S60 3.2
        * @param aLockCode The lock code to compare. This should be in plain 
       	*        format.
        * @param aCorrectLockCode The correct lock code to compare against. 
        *        This should be the one retrieved using GetLockCode(), and 
        *        thus is not in plain format.
        * @return ETrue if the lock codes are the same,
        *         EFalse otherwise.
        */
        IMPORT_C TBool CompareLockCodesL( 
            const TDesC& aLockCode, 
            const TDesC& aCorrectLockCode ) const;      
        
        /**
        * Enables or disables remote lock settings notification. If the 
        * settings are changed, the notification handler given in NewL()
        * or NewLC() will be called.
        *
        * @since S60 3.2
        * @param aNotifyEnable For enabling or disabling the remote lock 
        *                      settings notification.
        * @return ETrue if succeeded,
        *         EFalse otherwise.
        */
        IMPORT_C TBool RemoteLockNotifyL( const TBool aNotifyEnable ); 
   
    private:
        
        /**
        * Constructor.
        *
        * @since S60 3.2
        * @param aHandler Client implemented notification handler that will be
        *                 called when remote lock settings are changed. 
        */   
        CRemoteLockSettings( MRemoteLockSettingsHandler* aHandler );
        
        /**
        * 2nd phase constructor.
        */
        void ConstructL();
        
        // from base class MCenRepNotifyHandler
        
        /**
        * From MCenRepNotifyHandler
        * Notify handler.
        */
        void HandleNotifyGeneric( TUint32 aId );

    public: // data
        
    private: // data
    
        CRepository* iSession;
        CCenRepNotifyHandler* iNotifyHandler;
        MRemoteLockSettingsHandler* iHandler;                  
    };

// CLASS DECLARATION

/**
* Abstract interface for handling the notification when remote lock settings 
* are changed. User derives his class from this and implements 
* HandleRemoteLockNotifyL().
*
* @lib rlocksettings.lib
* @since S60 3.2
*/
class MRemoteLockSettingsHandler
    {    
    public:
        /**
        * This function will be called when the remote lock settings are 
        * changed.
        *
        * @since S60 3.2
        * @param
        * @return 
        */
        virtual void HandleRemoteLockNotifyL() = 0; 
    };

#endif
//End of File. 