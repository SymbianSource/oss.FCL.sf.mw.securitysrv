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
* Description:  Implementation of RemoteLock Engine
*
*/


// INCLUDE FILES 
#include <smscmds.h>
#include <smutset.h>
#include <smuthdr.h>
#include <e32property.h>
#include <PSVariables.h>
#include <mtclreg.h> //CClientMtmRegistry
#include <smsclnt.h> //CSmsClientMtm
#include <txtrich.h>
#include <bautils.h> //file system utilities
#include <remotelock.rsg>
#ifdef RD_MULTIPLE_DRIVE 
#include <driveinfo.h>
#else
#include <pathinfo.h> 
#endif //RD_MULTIPLE_DRIVE 
#include <coreapplicationuisdomainpskeys.h>
#include <charconv.h>
#include <ProfileEngineSDKCRKeys.h>
#include <Profile.hrh>
#include <stringresourcereader.h> //CStringResourceReader
#include "RemoteLockTrace.h"
#include "RemoteLock.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS

_LIT( KRLockResourceFileName ,"z:\\Resource\\apps\\Remotelock.RSC" );

//Interval of reconnecting to the message server 
const TInt KRLockTimerInterval( 10000000 ); 
const TInt KRLockMaxMemoryCardPasswdLength( 8 );
const TInt KBufLengthInUnicodeCheck = 200;

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ---------------------------------------------------------------------------
// E32Main()
// ---------------------------------------------------------------------------
//
GLDEF_C TInt E32Main() 
    {    
    __UHEAP_MARK;
    if ( User::TrapHandler() != NULL )
        {
        User::SetTrapHandler( NULL );
        }
        
    CTrapCleanup* cleanup = CTrapCleanup::New();
    if ( !cleanup )
        {
        return KErrNone;
        }

    TRAPD( err, ThreadStartL() );
    ( void ) err;
    
    delete cleanup;
    __UHEAP_MARKEND;

    return err;    
    }


// ---------------------------------------------------------------------------
// ThreadStartL()
// ---------------------------------------------------------------------------
//
LOCAL_C TInt ThreadStartL()
    {
    RL_TRACE_PRINT(" [ rl.exe ] Inside ThreadStartL() "); 
    CActiveScheduler *threadScheduler = new CActiveScheduler;
    
    CleanupStack::PushL( threadScheduler );
    
    CActiveScheduler::Install( threadScheduler );
    
    RL_TRACE_PRINT(" [ rl.exe ] Create remotelock object "); 

    CRemoteLock* remoteLock = CRemoteLock::NewL();
    
    RL_TRACE_PRINT(" [ rl.exe ] Creation of Remotelock is done" );
    
    CleanupStack::PushL( remoteLock );
        
    RL_TRACE_PRINT(" Doing Rendezvous..."); 
    // Initialisation complete, now signal the client
    RProcess::Rendezvous(KErrNone);            

    CActiveScheduler::Start();
    
    CleanupStack::PopAndDestroy( 2 );
    
    return KErrNone;
    }


// ---------------------------------------------------------------------------
// CRemoteLock::NewL
// Two-Phased constructor.
// ---------------------------------------------------------------------------
//
CRemoteLock* CRemoteLock::NewL()
    {
    RL_TRACE_PRINT(" [ rl.exe ] NewL()");
    
    CRemoteLock* self = new ( ELeave ) CRemoteLock;
    
    CleanupStack::PushL( self );
    
    self->ConstructL();  
    
    CleanupStack::Pop();
    
    RL_TRACE_PRINT(" [ rl.exe ] exit NewL() ");
    
    return self;
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
CRemoteLock::~CRemoteLock()
    {
    Cancel();
    delete iRemoteLockSetting;
    delete iRemoteLockCode;
    delete iMemoryCardPasswd;
    delete iStoredCode;
    delete iStoredTrimmedCode;
    delete iClientMtmForSending;
    delete iMtmReg;
    delete iMsvSession;
    delete iObserver;
    
    if ( iProfileNotifyHandler ) 
        {
      iProfileNotifyHandler->StopListening();
      delete iProfileNotifyHandler;
        }
    delete iProfileSession; 
    }


// ---------------------------------------------------------------------------
// CRemoteLock::HandleSessionEventL
//
// Called by message server when new event occures.
// ---------------------------------------------------------------------------
//
void CRemoteLock::HandleSessionEventL( 
    TMsvSessionEvent aEvent, 
    TAny* aArg1, 
    TAny* aArg2, 
    TAny* /*aArg3*/  )
    {
    RL_TRACE_PRINT(" [ rl.exe ] HandleSessionEventL() ");  
    switch ( aEvent )
        {            
        // A new entry has been created in the message server
        case EMsvEntriesCreated: 
            {
            RL_TRACE_PRINT(" [ rl.exe ] HandleSessionEventL() 1");  
            if ( !iIsEnabled )
                {
                // Do nothing if remote lock is disabled
                return;
                }
            
            if ( *( static_cast<TMsvId*>( aArg2 ) ) != 
                KMsvGlobalInBoxIndexEntryId )
                {
                // Doing nothing if not to inbox
                return;
                }

            CMsvEntrySelection* entries =
                static_cast<CMsvEntrySelection*>( aArg1 );

            for ( TInt i = 0; i < entries->Count(); i++ )
                {
                TMsvEntry entryInfo;
                TMsvId serviceId;
                iMsvSession->GetEntry( entries->At( i ), serviceId, entryInfo );

                if ( entryInfo.iMtm != KUidMsgTypeSMS || 
                    entryInfo.iDescription.Length() > KRLockMaxLockCodeLength || 
                    entryInfo.iDescription.Length() == 0)
                    {
                    // Doing nothing if not a SMS, or length not appropriate.
                    return;
                    }

                // For performance reasons, lets first compare the content of 
                // the SMS (taken from the TMsvEntry, containing no extra 
                // spaces) against the saved digest of a trimmed remote lock 
                // code. This way unnecessary access to CMsvEntry is avoided 
                // if these two do not match.
                
                
                if ( iRemoteLockSetting->CompareLockCodesL( 
                    entryInfo.iDescription, *iStoredTrimmedCode ) )
                    {
                    CMsvEntry* entry = 
                        iMsvSession->GetEntryL( entries->At( i ) );
                
                    // The CMsvEntry object must be deleted by the client
                    // when it is no longer required. 
                    CleanupStack::PushL( entry );

                    TRAPD( error, HandleMessageL( entry ) );

                    CleanupStack::PopAndDestroy( 1 ); // entry
                    
                    delete iRemoteLockCode;
                    iRemoteLockCode = NULL;
                    
                    User::LeaveIfError( error );
                    } // if
                } // for
            break;
            } // case
      
        // When message server is closed and server terminated
        case EMsvCloseSession:
        case EMsvServerFailedToStart:
        case EMsvServerTerminated:
            {
            RL_TRACE_PRINT(" [ rl.exe ] HandleSessionEventL() 2");  
            // When server is terminated remote lock will
            // try to reconnect it in every 10 seconds.
            After( KRLockTimerInterval );
            break;
            }

        default:
            break;
        }
    RL_TRACE_PRINT(" [ rl.exe ] exit HandleSessionEventL() ");
    }


// ---------------------------------------------------------------------------
// CRemoteLock::HandleRemoteLockNotifyL
// Function is called when remote lock setting is changed by client
// ---------------------------------------------------------------------------
//
void CRemoteLock::HandleRemoteLockNotifyL()
    {
    RL_TRACE_PRINT(" [ rl.exe ] HandleNotify() ");
    CheckSettingsL();
    RL_TRACE_PRINT(" [ rl.exe ] exit HandleNotify() ")
    }
    
    
// ---------------------------------------------------------------------------
// CRemoteLock::CRemoteLock
// default constructor
// ---------------------------------------------------------------------------
CRemoteLock::CRemoteLock() : CTimer( CActive::EPriorityStandard )
    {
    iRemoteLockCode = NULL;
    iMemoryCardPasswd = NULL;
    iStoredCode = NULL;
    iStoredTrimmedCode = NULL;
    iClientMtmForSending = NULL;
    iMtmReg = NULL;
    iRemoteLockSetting = NULL;
    iMsvSession = NULL;
    iIsEnabled = EFalse;
    iLockedByRL = EFalse; 
    iMemoryCardLockedByRL = EFalse;
    iSubscribeProfile = EFalse;
    iProfileSession = NULL;
    iObserver = NULL;
    iProfileNotifyHandler = NULL;   
    }


// ---------------------------------------------------------------------------
// CRemoteLock::ConstructL
// Symbian 2nd phase constructor can leave.
// ---------------------------------------------------------------------------
//
void CRemoteLock::ConstructL()
    {    
    RL_TRACE_PRINT(" [ Rl.exe ] ConstructL()"); 
    
    CTimer::ConstructL();
    
    iRemoteLockSetting = CRemoteLockSettings::NewL( this ); 
     
    iRemoteLockSetting->RemoteLockNotifyL( ETrue );
    
    CheckSettingsL(); 
    // Add active object into active scheduler
    CActiveScheduler::Add( this );
        
    
    
    RL_TRACE_PRINT(" [ rl.exe ] exit ConstructL() ");
    }
    
    
// ---------------------------------------------------------------------------
// CRemoteLock::HandleMessageL()
//
// Handles the message: checks if it really is remote lock message, and acts
// accordingly.
// ---------------------------------------------------------------------------
//
void CRemoteLock::HandleMessageL( CMsvEntry* aEntry )
    {
    RL_TRACE_PRINT(" [ rl.exe ] HandleMessageL() ");
    delete iRemoteLockCode;
    iRemoteLockCode = NULL;
    TMsvEntry entryInfo = aEntry->Entry();
    if ( !iMtmReg )
        {
        iMtmReg = CClientMtmRegistry::NewL( *iMsvSession );
        }

    // Loops 100 times to ensure the clientMtm is correctly created
    CBaseMtm* clientMtm = NULL;
    TInt error = 0;
    TInt i;
    for ( i = 0; i < 100; i++ )
        {
        TRAP( error, clientMtm = iMtmReg->NewMtmL( entryInfo.iMtm ));
        if ( error == KErrNone )
            {
            break;
            }
        } 
   
    RL_TRACE_PRINT_NUM("[rl.exe] HandleMessageL() error after NewMTmL = %d", error );

    User::LeaveIfError( error );
    CleanupStack::PushL( clientMtm );        

    clientMtm->SwitchCurrentEntryL( entryInfo.Id() );

    RL_TRACE_PRINT(" [ rl.exe ] HandleMessageL() after switch Current EntryL");
   
    CSmsClientMtm* smsClientMTM = STATIC_CAST( CSmsClientMtm*, clientMtm );
    
    // Change from 100 to 20. When the message storage is memory card. 
    // LoadMessageL() function return -21 when loading the message.
    // It might be caused by slow operation, so we add 0.05 seconds pending 
    // in each loop.
    // @ check why loading operation is slow!
    for ( i = 0; i < 20; i++ )
        {   
        RL_TRACE_PRINT_NUM("[rl.exe] HandleMessageL() error load message1 = %d", error );
        
        // Loops 100 times to ensure the LoadMessageL() succeedes
        TRAP( error, smsClientMTM->LoadMessageL() );
        
        RL_TRACE_PRINT_NUM("[rl.exe] HandleMessageL() error load message2 = %d", error );
        
        if ( error == KErrNone )
            {
            break;
            }
        
        const TInt KDelay = 50000; // 0.05s
        User::After( KDelay );
        }
    
    RL_TRACE_PRINT_NUM("[rl.exe] HandleMessageL() error after LoadMessageL = %d", error );
    
    User::LeaveIfError( error );        

    CRichText& mtmBody = smsClientMTM->Body();                            
    
    // Let's then use the real content of the SMS (taken from 
    // the message body) and compare that against the 
    // saved digest of a complete (not trimmed) remote lock 
    // code. This ensures that remote locking happens only
    // if the SMS is exactly the same as what the user has 
    // specified.
    iRemoteLockCode = HBufC::NewL( mtmBody.DocumentLength() );

    TPtr smsTextPtr = iRemoteLockCode->Des();
    smsTextPtr.Copy( mtmBody.Read( 0, mtmBody.DocumentLength() ) );
    
    
    // The following code is to get the current state
    // of manuallock. If manuallock state is off, that means
    // the owner has unlocked the phone or the phone has
    // never been locked by both remote lock and auto lock.
    RL_TRACE_PRINT(" [ rl.exe ] HandleMessageL() Pub&Sub Get autoLState");    
    TInt autoLState;
    RProperty property;
    
    #ifndef RD_STARTUP_CHANGE    
    User::LeaveIfError( property.Attach( KUidSystemCategory, KPSUidAutolockStatusValue ));
    #else
    User::LeaveIfError( property.Attach( KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus ));
    #endif //RD_STARTUP_CHANGE
    property.Get( autoLState );
    
    #ifndef RD_STARTUP_CHANGE
    if ( autoLState == EPSAutolockOff ) 
    #else
    if ( autoLState == EAutolockOff ) 
    #endif //RD_STARTUP_CHANGE      
      {  
      //Assign iLockedByRL to EFalse to indicate that
      //the phone is not locked by Remote lock, the 
      //reply SMS should be sent if correct remote lock
      //code is received.
      iLockedByRL = EFalse;
      }
    
    property.Close();
    
    // If the phone receives correct lock code, the phone
    // will be locked eventhough it may have been locked by
    // auto lock.
  
    if ( VerifyAndLockL() ) 
        {
        RL_TRACE_PRINT(" [ rl.exe ] HandleMessageL() After VerifyAndLockL() ");
        // Sets remote lock sms as read
        entryInfo.SetVisible( ETrue );
        entryInfo.SetUnread( EFalse );
        entryInfo.SetNew( EFalse );
        entryInfo.SetInPreparation( EFalse );
        entryInfo.SetReadOnly( ETrue );
        aEntry->ChangeL( entryInfo );
        
        
        // Locks MemoryCard drive.
        iStateMemoryCard = SetMemoryCardPasswdL( EFalse );

        if ( iClientMtmForSending )
            {
            // Previous reply sending is still in progress, let's not start 
            // sending new one.
            User::Leave( KErrNone );
            } 
        
        // The value of iLockedByRL is checked before sending SMS
        // to make sure if the phone has been locked by remote 
        // lock. By doing this, it is able to avoid the threat where 
        // malware tries to use remote lock to keep on sending SMS.
        
        
        if ( !iLockedByRL )
            { 
            RL_TRACE_PRINT(" [ rl.exe ] HandleMessageL() Send re-SMS ");
            // Replies sms
            iLockedByRL = ETrue;
            CSmsHeader& header = smsClientMTM->SmsHeader();
            HBufC* recipientAddress = header.FromAddress().AllocLC();

            TBool initialSendMessage = EFalse;
            TRAP( error, initialSendMessage = InitialSendMessageL() );
            if ( error != KErrNone )
                {
                delete iClientMtmForSending;
                iClientMtmForSending = NULL;
                User::Leave( error );
                }
                
            RL_TRACE_PRINT(" [ rl.exe ] HandleMessageL() Check recipientaddresslenght ");
            if ( recipientAddress->Length() > 0 && initialSendMessage )
                {
                RL_TRACE_PRINT(" [ rl.exe ] HandleMessageL() before call sendMEssage ");
                TRAP( error, SendMessageL( *recipientAddress ) );
                if ( error != KErrNone )
                    {
                    RL_TRACE_PRINT(" [ rl.exe ] HandleMessageL() error occurs ");
                    delete iClientMtmForSending;
                    iClientMtmForSending = NULL;
                    User::Leave( error );
                    }
                }
            delete iClientMtmForSending;
            iClientMtmForSending = NULL;

            CleanupStack::PopAndDestroy( 1 ); // recipientAddress
            }
        CleanupStack::PopAndDestroy( 1 ); // clientMtm    
        }
    else
        {
        CleanupStack::PopAndDestroy( 1 ); // clientMtm
        }
    RL_TRACE_PRINT(" [ rl.exe ] exit HandleMessageL() ");
    }


// ---------------------------------------------------------------------------
// CRemoteLock::VerifyAndLockL()
//
// Compares if the remote lock received in SMS is correct.
// If correct, locks the terminal.
// ---------------------------------------------------------------------------
//
TBool CRemoteLock::VerifyAndLockL()
    {
    RL_TRACE_PRINT(" [ rl.exe ] VerifyAndLockL() ");
    
    User::LeaveIfNull( iRemoteLockCode );
    User::LeaveIfNull( iStoredCode );

    if ( iRemoteLockSetting->CompareLockCodesL( 
        *iRemoteLockCode, *iStoredCode ) )
        {        
        return ActivateDeviceLock();
        }
    RL_TRACE_PRINT(" [ rl.exe ] exit VerifyAndLockL() ");
    return EFalse;
    }


// ---------------------------------------------------------------------------
// CRemoteLock::ActivateDeviceLock()
// Activate the device lock to lock the phone
// ---------------------------------------------------------------------------
// 
TBool CRemoteLock::ActivateDeviceLock()
    {
    RL_TRACE_PRINT(" [ rl.exe ] ActivateDeviceLock() ");
       
    RProperty property;
        
    TInt err = 0;
       
    #ifndef RD_STARTUP_CHANGE
    err = property.Attach( KUidSystemCategory, 
                               KPSUidAutolockStatusValue );
    #else
    err = property.Attach( KPSUidCoreApplicationUIs, 
                               KCoreAppUIsAutolockStatus );
    #endif 
    if ( err != KErrNone )
        {
        return EFalse;
        }

    #ifndef RD_STARTUP_CHANGE
    property.Set( EPSRemoteLocked );
    #else
    property.Set( ERemoteLocked );
    #endif //RD_STARTUP_CHANGE 
 
    property.Close();
    RL_TRACE_PRINT(" [ rl.exe ] exit ActivateDeviceLock() ");
    return ETrue;
    }



// ---------------------------------------------------------------------------
// CRemoteLock::CheckSettingsL
// Checks remote lock setting.
// ---------------------------------------------------------------------------
//
void CRemoteLock::CheckSettingsL()
    {
    RL_TRACE_PRINT(" [ rl.exe ] CheckSettingsL () "); 
    iRemoteLockSetting->GetEnabled( iIsEnabled );
    
    delete iStoredCode;
    iStoredCode = NULL;
    iStoredCode = HBufC::NewL( KRLockStoredLockCodeLength );
    TPtr storedCodePtr = iStoredCode->Des();
    
    delete iStoredTrimmedCode;
    iStoredTrimmedCode = NULL;
    iStoredTrimmedCode = HBufC::NewL( KRLockStoredLockCodeLength );
    TPtr storedTrimmedCodePtr = iStoredTrimmedCode->Des();
    
    iRemoteLockSetting->GetLockCode( storedCodePtr, storedTrimmedCodePtr );
        
    if ( iIsEnabled )
        {
        RL_TRACE_PRINT(" [ rl.exe ] CheckSettingsL() enable "); 
       
        if ( !iMsvSession )
            {
            TRAPD( error, iMsvSession = CMsvSession::OpenAsyncL( *this ) );
        
            RL_TRACE_PRINT_NUM(" [ rl.exe ] CheckSettingsL() openasyncL %d ", error );
        
            if ( error != KErrNone )
                {
                After( KRLockTimerInterval );
                }  
            }

       if ( !iProfileSession )
           {
           RL_TRACE_PRINT(" [ rl.exe ] CheckSettingsL() create session "); 
           iProfileSession = CRepository::NewL( KCRUidProfileEngine ); 
           }
       if ( !iObserver )
           {
           iObserver =  CRLLockObserver::NewL( this );      
           } 
       if( !iSubscribeProfile )
           {
           ProfileNotifyL( ETrue );
           iSubscribeProfile = ETrue; 
           }   
        }
    else
        {
        RL_TRACE_PRINT(" [ rl.exe ] CheckSettingsL() disable "); 
        
        delete iClientMtmForSending;
        iClientMtmForSending = NULL;
        delete iMtmReg;
        iMtmReg = NULL;
        delete iMsvSession;
        iMsvSession = NULL;
        
        RL_TRACE_PRINT(" [ rl.exe ] CheckSettingsL() delete iMsvSession ");
        
        if ( iSubscribeProfile )
            {
            if ( !iProfileSession )
                {
                RL_TRACE_PRINT(" [ rl.exe ] CheckSettingsL() create session "); 
                iProfileSession = CRepository::NewL( KCRUidProfileEngine ); 
                }
            ProfileNotifyL( EFalse );
            delete iProfileSession;
            iProfileSession = NULL;
            delete iObserver;
            iObserver = NULL; 
            iSubscribeProfile = EFalse;
            }  
        }   
    
    RL_TRACE_PRINT(" [ rl.exe ] exit CheckSettingsL() ");
    }
    

// ---------------------------------------------------------------------------
// CRemoteLock::IsMemoryCardLocked
// Check MemoryCard whether it is locked or not.
// ---------------------------------------------------------------------------
//
TBool CRemoteLock::IsMemoryCardLocked( const TDriveInfo& aDriveInfo ) const
    {
    return aDriveInfo.iMediaAtt&( KMediaAttLocked );
    }


// ---------------------------------------------------------------------------
// CRemoteLock::HasMemoryCardPassword
// Check MemoryCard whether it has password.
// ---------------------------------------------------------------------------
//
TBool CRemoteLock::HasMemoryCardPassword( const TDriveInfo& aDriveInfo ) const
    {
    return aDriveInfo.iMediaAtt&( KMediaAttHasPassword );
    }


// ---------------------------------------------------------------------------
// CRemoteLock::IsMemoryCardPresent
// Check MemoryCard whether it is mounted into the phone and it supports locking.
// ---------------------------------------------------------------------------
//
TBool CRemoteLock::IsMemoryCardPresent( const TDriveInfo& aDriveInfo ) const
    {
    return aDriveInfo.iDriveAtt&( KDriveAttRemovable ) && 
        aDriveInfo.iMediaAtt&( KMediaAttLockable );
    }


// ---------------------------------------------------------------------------
// CRemoteLock::SetMemoryCardPasswdL
// Sets/remove the password to MemoryCard.
// ---------------------------------------------------------------------------
//
TBool CRemoteLock::SetMemoryCardPasswdL( const TBool aClear )
    {
    RL_TRACE_PRINT(" [ rl.exe ] SetMemoryCardPasswdL() ");
    if ( aClear )
        {
        if ( iMemoryCardLockedByRL && iLockedByRL )
            {
            //remove the password
            if ( DoSetMemoryCardPasswdL( ETrue ) )
                {
                iMemoryCardLockedByRL = EFalse;
                RL_TRACE_PRINT(" [ rl.exe ] exit SetMemoryCardPasswdL() ");    
                return ETrue;   
                }
            else
                {
                //Only try to remove password once no matter success or not 
                iMemoryCardLockedByRL = EFalse; 
              }    
             
            }       
        }
    else
        {
        if ( iMemoryCardLockedByRL )
            {
            //the memory card is locked already by rlock, 
            //just return ETrue;
            return ETrue;
            }
        
        if ( !iRemoteLockCode || iRemoteLockCode->Length() == 0 )
            {
            return EFalse;
            }
        //Change the password
        if ( DoSetMemoryCardPasswdL( EFalse ) )
            {
            iMemoryCardLockedByRL = ETrue;
            RL_TRACE_PRINT(" [ rl.exe ] exit SetMemoryCardPasswdL() ");    
            return ETrue; 
            }
           
        }
    RL_TRACE_PRINT(" [ rl.exe ] exit SetMemoryCardPasswdL() ");    
    return EFalse;
    }   

// ---------------------------------------------------------------------------
// CRemoteLock::DoSetMemoryCardPasswdL
// Do Sets/remove the password to MemoryCard.
// ---------------------------------------------------------------------------
//
TBool CRemoteLock::DoSetMemoryCardPasswdL( TBool aClear )
    {
    RL_TRACE_PRINT(" [ rl.exe ] DoSetMemoryCardPasswdL() ");
    RFs fsMemoryCard;
    TInt err = fsMemoryCard.Connect();
    if ( err != KErrNone )
        {
        RL_TRACE_PRINT(" [ rl.exe ] DoSetMemoryCardPasswdL() error");
        return EFalse;
        }
         
#ifdef RD_MULTIPLE_DRIVE 

   // Get the removeable user visible drives
   TDriveList driveList;
   TInt driveCount;
   
   //Get all removeable drive, both physically and logically
   User::LeaveIfError( DriveInfo::GetUserVisibleDrives(
       fsMemoryCard, driveList, driveCount, KDriveAttRemovable ) );   
   RL_TRACE_PRINT_NUM(" [ rl.exe ] DoSetMemoryCardPasswdL() driveCount = %d ", driveCount );
 
   //boolen to indicate at least one operation(clear/add password) 
   // in the loop is ok.
   TBool OperationSucceed = EFalse;
  
   RArray<TInt> arrayMemoryCardIndex;
  
   TInt max( driveList.Length() );
   
   for ( TInt i = 0; i < max; i++ )
       {
       if ( driveList[ i ] )
          {
          TUint status;	
          DriveInfo::GetDriveStatus( fsMemoryCard, i, status );	
          //To make sure the drive is physically removeable not logically removeable	
          if( status & DriveInfo::EDriveRemovable )	
              {
              //append all physical removable memory card index into this array	
              arrayMemoryCardIndex.Append(i); 
              RL_TRACE_PRINT_NUM(" [ rl.exe ] DoSetMemoryCardPasswdL() physically removable drive %d", i );
		          }
		      else
		          {
		          RL_TRACE_PRINT_NUM(" [ rl.exe ] DoSetMemoryCardPasswdL() logically removable drive %d", i );
		          }    
		      }    
       }
      
   //Lock/Unblock all physical removeable memory card
   for ( TInt i = 0; i < arrayMemoryCardIndex.Count(); i++ )
       {
       TDriveInfo driveInfoT;
       TInt driveIndex = arrayMemoryCardIndex[i];
       RL_TRACE_PRINT_NUM(" [ rl.exe ] DoSetMemoryCardPasswdL() driveIndex %d", driveIndex );	
       fsMemoryCard.Drive( driveInfoT, driveIndex );
    
       if ( IsMemoryCardPresent( driveInfoT ) ) 
          {
          	RL_TRACE_PRINT(" [ rl.exe ] Memory card is present");
          if ( aClear )
              {
              RL_TRACE_PRINT(" [ rl.exe ] DoSetMemoryCardPasswdL() Remove password");	
              if ( HasMemoryCardPassword( driveInfoT ) || IsMemoryCardLocked( driveInfoT ) ) 
                  { 
                  TMediaPassword memoryCardPassword; 
              
                  // Converts MemoryCardpassword
                  ConvertMemoryCardPassword( memoryCardPassword, ETrue  );
              
                  err = fsMemoryCard.ClearPassword( driveIndex, memoryCardPassword );
              
                  if ( err == KErrNone )
                      {
                  	  OperationSucceed = ETrue;
                      }
              
                  RL_TRACE_PRINT_NUM(" [ rl.exe ] exit DoSetMemoryCardPasswdL() password for index %d is cleaned", driveIndex);
                  //Go for next
                  }
              else
                  {
                  //Go for next
                  }      
               }
            else
               {
               RL_TRACE_PRINT(" [ rl.exe ] DoSetMemoryCardPasswdL() Set password");
               if ( !HasMemoryCardPassword( driveInfoT ) && !IsMemoryCardLocked( driveInfoT ) )
                   { 
                   TMediaPassword memoryCardPassword;
                
                   TMediaPassword nullMemoryCardPassword;
                
                   // Converts MemoryCardpassword
                   ConvertMemoryCardPassword( memoryCardPassword, EFalse );
                
                   // Locks MemoryCard drive
                   err = fsMemoryCard.LockDrive( driveIndex, nullMemoryCardPassword, memoryCardPassword, ETrue );        
                
                   if ( err == KErrNone )
                      {
                  	  OperationSucceed = ETrue;
                      }
                
                   RL_TRACE_PRINT_NUM(" [ rl.exe ] DoSetMemoryCardPasswdL() password for index %d is set", driveIndex);
                   
                   //Go for next
                   }
                else
                   {
                   //go for next
                   }   
               }
            } //if ( IsMemoryCardPresent( driveInfoT ) ) 
         } //for
    
     
     if ( aClear )
         {
     	   delete iMemoryCardPasswd; 
         iMemoryCardPasswd = NULL; 
         fsMemoryCard.Close(); 
         
         return OperationSucceed ? ETrue : EFalse;
         }
     else 
        {

        delete iMemoryCardPasswd;
        iMemoryCardPasswd = NULL; 
        iMemoryCardPasswd = iRemoteLockCode->AllocL();
        fsMemoryCard.Close();	
        
        return OperationSucceed ? ETrue : EFalse;
        }       
#else
    
    TInt i = 0;
    TParsePtrC folder( PathInfo::MemoryCardRootPath() );
    fsMemoryCard.CharToDrive( folder.Drive()[ 0 ], i );
      
    TDriveInfo driveInfoT;
    fsMemoryCard.Drive( driveInfoT, i );
    
    if ( IsMemoryCardPresent( driveInfoT ) ) 
      {
      if ( aClear )
          {
          if ( HasMemoryCardPassword( driveInfoT ) || IsMemoryCardLocked( driveInfoT ) ) 
              { 
              TMediaPassword memoryCardPassword; 
              // Converts MemoryCardpassword
              ConvertMemoryCardPassword( memoryCardPassword, ETrue  );
              err = fsMemoryCard.ClearPassword( i, memoryCardPassword );
              
              if( err == KErrNone )
                  {
                  delete iMemoryCardPasswd; 
                  iMemoryCardPasswd = NULL; 
                  }
              fsMemoryCard.Close(); 
              RL_TRACE_PRINT(" [ rl.exe ] exit DoSetMemoryCardPasswdL() clear");
              return ( err == KErrNone );
              }
          else
              {
              fsMemoryCard.Close();  
              return ETrue;
              }      
           }
        else
           {
           if ( !HasMemoryCardPassword( driveInfoT ) && !IsMemoryCardLocked( driveInfoT ) )
                { 
                TMediaPassword memoryCardPassword;
                TMediaPassword nullMemoryCardPassword;
                // Converts MemoryCardpassword
                ConvertMemoryCardPassword( memoryCardPassword, EFalse );
                // Locks MemoryCard drive
                err = fsMemoryCard.LockDrive( i, nullMemoryCardPassword, memoryCardPassword, ETrue );        
                if ( err == KErrNone )
                    {
                    if ( iMemoryCardPasswd )
                          {
                          delete iMemoryCardPasswd;
                          iMemoryCardPasswd = NULL; 
                          }
                     iMemoryCardPasswd = iRemoteLockCode->AllocL();
                     }
          
                fsMemoryCard.Close();
                RL_TRACE_PRINT(" [ rl.exe ] exit DoSetMemoryCardPasswdL() change");
                return ( err == KErrNone );  
                }
            else
                {
                fsMemoryCard.Close();  
                return EFalse;
                }   
            }
        }    
    fsMemoryCard.Close();
    return EFalse;  
    
#endif //RD_MULTIPLE_DRIVE       
    }

// ---------------------------------------------------------------------------
// CRemoteLock::ConvertMemoryCardPasswordL
// Converts MemoryCardPassword to acceptable format.
// ---------------------------------------------------------------------------
//
void CRemoteLock::ConvertMemoryCardPassword( TMediaPassword& aPassword, const TBool aClear ) 
    {
    RL_TRACE_PRINT(" [ rl.exe ] ConvertMemoryCardPassword() ");
    
    TBuf16 < KMaxMediaPassword / 2 > memoryCardPassword; 
    // Takes left most 8 digits of lockcode as
    // the password of MemoryCard.
    RL_TRACE_PRINT(" [ rl.exe ] ConvertMemoryCardPassword() 1");
    
    if ( aClear )
        {
        if ( iMemoryCardPasswd )
           {
           memoryCardPassword.Copy( iMemoryCardPasswd->Left( KRLockMaxMemoryCardPasswdLength ) );
           }
        }
     else
        {
        if ( iRemoteLockCode )
            {
            memoryCardPassword.Copy( iRemoteLockCode->Left( KRLockMaxMemoryCardPasswdLength ) );
            }
        } 
  
    // Fills the descriptor's data area with binary zeroes, i.e. 0x00, 
    // replacing any existing data.
    RL_TRACE_PRINT(" [ rl.exe ] ConvertMemoryCardPassword() 2");
    aPassword.FillZ( KMaxMediaPassword ); 

    aPassword.Zero();
    // Sets the length of the data to zero.
    // Copies data into this descriptor replacing any existing data. 
    // The length of this descriptor is set to reflect the new data
    RL_TRACE_PRINT(" [ rl.exe ] ConvertMemoryCardPassword() 3");
    aPassword.Copy( reinterpret_cast<TUint8 *>
        ( &memoryCardPassword[ 0 ] ), memoryCardPassword.Length() * 2 );
        
    RL_TRACE_PRINT(" [ rl.exe ] ConvertMemoryCardPassword() over ");    
    }
  
  
// ---------------------------------------------------------------------------
// CRemoteLock::InitialSendMessageL
// Reply SMS when the terminal is successfully locked.
// ---------------------------------------------------------------------------
//
TBool CRemoteLock::InitialSendMessageL()
    {
    RL_TRACE_PRINT(" [ rl.exe ] InitialSendMessageL() ");
    
    RProperty property;
    
    TInt err = 0;
     
    #ifndef RD_STARTUP_CHANGE        
    err = property.Attach( KUidSystemCategory, KPSUidAutolockStatusValue );              
    #else
    err = property.Attach( KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus );
    #endif
    
    if ( err != KErrNone )
        {
      return EFalse;
        }
    
    TInt state = 0;
    
    CleanupClosePushL( property );
    
    err = property.Get( state );
    if ( err != KErrNone )
        {
      CleanupStack::PopAndDestroy( 1 );
        return EFalse;
        }
         
    #ifndef RD_STARTUP_CHANGE
    if ( state  == EPSRemoteLocked )
    #else
    if ( state == ERemoteLocked )
    #endif // RD_STARTUP_CHANGE 
      {
      // message server entry id
      TMsvId msvId = NULL;
      // Set up a new message
      msvId = CreateNewMessageL();
      //Set the new message to be the current entry
      SetEntryL( msvId );
      CleanupStack::PopAndDestroy( 1 ); // property
      RL_TRACE_PRINT(" [ rl.exe ] exit InitialSendMessageL() ETrue "); 
      return ETrue; 
      }
    CleanupStack::PopAndDestroy( 1 ); // property   
    RL_TRACE_PRINT(" [ rl.exe ] exit InitialSendMessageL() "); 
    return EFalse;
    }


// ---------------------------------------------------------------------------
// CRemoteLock::CreateNewMessageL()
// Creates a new message server entry and set up default values.
// Return values:      TMsvId (the id of created entry)
// ---------------------------------------------------------------------------
//
TMsvId CRemoteLock::CreateNewMessageL()
    {
    RL_TRACE_PRINT(" [ rl.exe ] CreateNewMessageL() ");
    TMsvEntry newEntry;
    // The type of message is SMS
    newEntry.iMtm = KUidMsgTypeSMS;
    // The type of the entry: message
    newEntry.iType = KUidMsvMessageEntry;                    
    newEntry.iServiceId = KMsvLocalServiceIndexEntryId;
    // Set the date of the entry to home time
    newEntry.iDate.UniversalTime();                           
    newEntry.SetInPreparation( ETrue );                      

    CMsvEntry* entry = CMsvEntry::NewL( *iMsvSession, 
        KMsvDraftEntryIdValue, TMsvSelectionOrdering() );
    CleanupStack::PushL( entry );
    CMsvOperationWait* wait = CMsvOperationWait::NewLC();
    wait->Start();    

    // Asynchronously create a new entry.    
    CMsvOperation* oper = entry->CreateL( newEntry, wait->iStatus );
    CleanupStack::PushL( oper );

    CActiveScheduler::Start();

    // Keep track of the progress of the create operation.
    TMsvLocalOperationProgress progress = 
        McliUtils::GetLocalProgressL( *oper );
    User::LeaveIfError( progress.iError );

    // Set entry context to the created one
    // operation progress contains the ID of the created entry
    entry->SetEntryL( progress.iId ); 

    CleanupStack::PopAndDestroy( 3 );
    RL_TRACE_PRINT(" [ rl.exe ] CreateNewMessageL() end ");
    return progress.iId;
    }


// ---------------------------------------------------------------------------
// CRemoteLock::SetEntryL(TMsvId aEntryId)
// Set up current message entry.
// Note: It can be useful to remember the original entry id for 
//       error handling.
// ---------------------------------------------------------------------------
//
void CRemoteLock::SetEntryL( TMsvId aEntryId )
    {
    RL_TRACE_PRINT(" [ rl.exe ] SetEntryL() ");
    // Get the server entry from our session
    CMsvEntry* entry = iMsvSession->GetEntryL( aEntryId );
    CleanupStack::PushL( entry );
    delete iClientMtmForSending;
    iClientMtmForSending = NULL;
    
    TInt error;
    TInt i;
    for ( i = 0; i < 100; i++ )
        {
        TRAP( error, 
            iClientMtmForSending = iMtmReg->NewMtmL( entry->Entry().iMtm ) );
        if ( error == KErrNone )
            {
             RL_TRACE_PRINT(" [ rl.exe ] SetEntryL() iClientMtmForSending ");
            break;
            }
        } 

    User::LeaveIfError( error );
    iClientMtmForSending->SetCurrentEntryL( entry );

    CleanupStack::Pop( 1 ); // entry
    RL_TRACE_PRINT(" [ rl.exe ] exit SetEntryL() ");
    entry = NULL;
    }


// ---------------------------------------------------------------------------
// CRemoteLock::MoveMessageEntryL(TMsvId aTarget) const
// Moves an entry to another parent.
// Return values:      TMsvId of the moved message
// ---------------------------------------------------------------------------
//
TMsvId CRemoteLock::MoveMessageEntryL( TMsvId aTarget )
    {
    User::LeaveIfNull( iClientMtmForSending );
    TMsvEntry msvEntry( ( iClientMtmForSending->Entry() ).Entry() );
    TMsvId id = msvEntry.Id();

    if ( msvEntry.Parent() != aTarget )
        {
        TMsvSelectionOrdering sort;
        sort.SetShowInvisibleEntries( ETrue );    
        // Take a handle to the parent entry
        CMsvEntry* parentEntry = 
            CMsvEntry::NewL( 
            iClientMtmForSending->Session(), msvEntry.Parent(), sort );

        CleanupStack::PushL( parentEntry );
    
        // Move original from the parent to the new location
        CMsvOperationWait* wait = CMsvOperationWait::NewLC();
        wait->Start();
    
        CMsvOperation* op = 
                parentEntry->MoveL( msvEntry.Id(), aTarget, wait->iStatus );

        CleanupStack::PushL( op );
        CActiveScheduler::Start();     
        TMsvLocalOperationProgress prog=McliUtils::GetLocalProgressL( *op );
        User::LeaveIfError(prog.iError);
    
        id = prog.iId; 
        CleanupStack::PopAndDestroy( 3 ); // op, wait, parentEntry
        }
    return id;
    }


// ---------------------------------------------------------------------------
// CRemoteLock::SendMessageL
// Reply SMS when the terminal is successfully locked.
// Return values:      ETrue or EFalse
// ---------------------------------------------------------------------------
//
TBool CRemoteLock::SendMessageL( const TDesC& aRecipientAddress )
    {
    RL_TRACE_PRINT(" [ rl.exe ] SendMessageL() ");
    
    User::LeaveIfNull( iClientMtmForSending );
    
    RL_TRACE_PRINT(" [ rl.exe ] SendMessageL() 1 ");
    
    TMsvEntry msvEntry = ( iClientMtmForSending->Entry() ).Entry();

    // We get the message body from Mtm and insert a bodytext
    CRichText& mtmBody = iClientMtmForSending->Body();
    mtmBody.Reset();
    
    TFileName fileName; 
    fileName = KRLockResourceFileName;

    // Use CStringResourceReader instead of StringLoader
    // StringLoader is meant for applications in app framework   
    CStringResourceReader* resourceReader = CStringResourceReader::NewLC( fileName );

    RL_TRACE_PRINT(" [ rl.exe ] SendMessageL() 3");
    
    HBufC* retBuf = NULL;

    if ( iStateMemoryCard )
        {
        RL_TRACE_PRINT(" [ rl.exe ] SendMessageL() memory card locked");
        // When memory card is locked.
        TPtrC buf;
        buf.Set( resourceReader->ReadResourceString(R_RLOC_TI_EVERYTHING_LOCKED) );
        retBuf = buf.AllocLC();
        }
    else
        {
        RL_TRACE_PRINT(" [ rl.exe ] SendMessageL() only phone locked ");
        TPtrC buf;
        buf.Set( resourceReader->ReadResourceString(R_RLOC_TI_PHONE_LOCKED) );
        retBuf = buf.AllocLC();
        }
    mtmBody.InsertL( 0, *retBuf );
    msvEntry.iDescription.Set( *retBuf );
     
    // Set aRecipientAddress into the Details of the entry
    msvEntry.iDetails.Set( aRecipientAddress );  
    msvEntry.SetInPreparation( EFalse );         
    
    // Set the sending state (immediately)
    msvEntry.SetSendingState( KMsvSendStateWaiting );                         
    msvEntry.iDate.UniversalTime();   
    // To handle the sms specifics we start using SmsMtm
    CSmsClientMtm* smsMtm = STATIC_CAST( CSmsClientMtm*, 
                                         iClientMtmForSending );
    smsMtm->RestoreServiceAndSettingsL();

    // SMS MTM encapsulation of an SMS message.
    CSmsHeader& header = smsMtm->SmsHeader();
    CSmsSettings* sendOptions = CSmsSettings::NewL();
    CleanupStack::PushL( sendOptions );
    // Reset existing settings
    sendOptions->CopyL( smsMtm->ServiceSettings() ); 
    // Set unicode if needed
    if ( NeedsToBeSentAsUnicodeL( retBuf->Des() ))
        {
        sendOptions->SetCharacterSet( TSmsDataCodingScheme::ESmsAlphabetUCS2 );
        }
    // Set send options to be delivered immediately
    sendOptions->SetDelivery( ESmsDeliveryImmediately );      
    header.SetSmsSettingsL( *sendOptions );
    
    // let's check if there's sc address
    if (header.Message().ServiceCenterAddress().Length() == 0)
        {
        // no, there isn't. Use the default SC number. 
        CSmsSettings* serviceSettings = NULL;
        serviceSettings = &( smsMtm->ServiceSettings() );
        
        RL_TRACE_PRINT(" [ rl.exe ] SendMessageL() Get SC number ");    
        if ( serviceSettings->ServiceCenterCount() )
            {
            //Set sc address to default.
            CSmsServiceCenter& sc = serviceSettings->GetServiceCenter(
                serviceSettings->DefaultServiceCenter() );
            header.Message().SetServiceCenterAddressL( sc.Address() );
            }
        }
    
    // Add recipient to the list.
    smsMtm->AddAddresseeL( aRecipientAddress, msvEntry.iDetails );

    CMsvEntry& entry = iClientMtmForSending->Entry();
    entry.ChangeL( msvEntry );                
    smsMtm->SaveMessageL();                 

    // Move message to outbox
    TMsvId movedId;
    TInt err;
    for ( TInt i = 0; i < 100; i++ )
        {
        TRAP( err, movedId = MoveMessageEntryL( 
                                 KMsvGlobalOutBoxIndexEntryId ));  
        if ( err == KErrNone )
            break;
        }
    RL_TRACE_PRINT(" [ rl.exe ] SendMessageL() put created message in outbox ");
    User::LeaveIfError( err );
    // We must create an entry selection for message copies 
    CMsvEntrySelection* selection = new ( ELeave ) CMsvEntrySelection;
    CleanupStack::PushL( selection );
    selection->AppendL( movedId );        
    // schedule the sending with the active scheduler
    SetScheduledSendingStateL( selection );   
    CleanupStack::PopAndDestroy( 4 ); // resourceReader,retbuf,sendOptions,selection 
    RL_TRACE_PRINT(" [ rl.exe ] Exit SendMessageL() ");
    return ETrue; 
    }


// ---------------------------------------------------------------------------
// CRemoteLock::SetScheduledSendingStateL
// Schedules the message to be sent through the etel server.
// Return values:      none
// ---------------------------------------------------------------------------
//
void CRemoteLock::SetScheduledSendingStateL( CMsvEntrySelection* aSelection )
    {
    User::LeaveIfNull( iClientMtmForSending );
    // Add entry to task scheduler
    TBuf8<1> dummyParams;
    CMsvOperationWait* waiter = CMsvOperationWait::NewLC();
    waiter->Start();

    // invoking async schedule copy command on our mtm
    CMsvOperation* op= iClientMtmForSending->InvokeAsyncFunctionL(
            ESmsMtmCommandScheduleCopy,
            *aSelection,
            dummyParams,
            waiter->iStatus );

    CleanupStack::PushL( op );
    CActiveScheduler::Start();

    CleanupStack::PopAndDestroy( 2 ); // op, wait 
    }


// ---------------------------------------------------------------------------
// CRemoteLock::RunL
// Reconnect msg server after it is shut down
// Return values:      none
// ---------------------------------------------------------------------------
//
void CRemoteLock::RunL()
    {
    RL_TRACE_PRINT(" [ rl.exe ] RunL() ");
    delete iClientMtmForSending;
    iClientMtmForSending = NULL;
    delete iMtmReg;
    iMtmReg = NULL;
    delete iMsvSession;
    iMsvSession = NULL;

    TRAPD( error, iMsvSession = CMsvSession::OpenAsyncL( *this ) );

    if ( error != KErrNone )
        {
        After( KRLockTimerInterval );
        }
    RL_TRACE_PRINT(" [ rl.exe ] Exit RunL() ");    
    }


// ---------------------------------------------------------------------------
// CRemoteLock::NeedsToBeSentAsUnicodeL
// Check if needs to be sent as unicode
// Return value ETrue or EFalse
// ---------------------------------------------------------------------------
//
TBool CRemoteLock::NeedsToBeSentAsUnicodeL( const TDesC& aInputString ) const
    {
    TBool needsToBeSentAsUnicode = EFalse;
    CCnvCharacterSetConverter* const
        characterSetConverter = CCnvCharacterSetConverter::NewLC();
    
    RFs fsSession;
    TInt err = fsSession.Connect();
    User::LeaveIfError( err );
    CleanupClosePushL( fsSession ); 
    const TUint KSmsEdSmsStrictPluginID = 0x101F85CD;
    characterSetConverter->PrepareToConvertToOrFromL(
        KSmsEdSmsStrictPluginID,
        fsSession);

    characterSetConverter->SetDowngradeForExoticLineTerminatingCharacters(
        CCnvCharacterSetConverter::EDowngradeExoticLineTerminatingCharactersToJustLineFeed );
   
    for (TPtrC remainderOfInputString( aInputString ); remainderOfInputString.Length()>0 ; )
        {
        TBuf8<KBufLengthInUnicodeCheck> notUsed;
        TInt numberOfUnconvertibleCharacters = 0;
        const TInt returnValue = 
            characterSetConverter->ConvertFromUnicode( 
                notUsed,
                remainderOfInputString, 
                numberOfUnconvertibleCharacters );
        if (( returnValue < 0 ) || ( numberOfUnconvertibleCharacters > 0 )) 
            // if there was an error in trying to do the conversion, or if there was an
            // unconvertible character (e.g. a Chinese character)
            {
            needsToBeSentAsUnicode = ETrue;
            break;
            }
        
        remainderOfInputString.Set( remainderOfInputString.Right( returnValue ));
        }
    CleanupStack::PopAndDestroy( 2 );
    return needsToBeSentAsUnicode;
    }


// ---------------------------------------------------------------------------
// CRemoteLock::HandleUnlockEvent()
// Callback function for unlocking event observer
// ---------------------------------------------------------------------------
void CRemoteLock::HandleUnlockEvent()
    { 
    TRAPD( err, SetMemoryCardPasswdL( ETrue ) );
    err = err;
    iLockedByRL = EFalse;
    }   

// -----------------------------------------------------------------------------
// CRemoteLock::ProfileNotifyL
// Setup Profile notifier
// Returns:   ETrue if everything is OK
//            EFalse otherwise
// -----------------------------------------------------------------------------
//
TBool CRemoteLock::ProfileNotifyL( 
    const TBool aNotifyEnable ) 
    {
    RL_TRACE_PRINT(" [ rl.exe ] ProfileNotifyL() ");
    
    if ( !iProfileNotifyHandler )
        {
        RL_TRACE_PRINT(" [ rl.exe ] ProfileNotifyL() create notify handler");
        iProfileNotifyHandler = CCenRepNotifyHandler::NewL( *this, *iProfileSession ); 
        }
    
    if ( aNotifyEnable )
        {
        
        TInt err = iProfileSession->Get( KProEngActiveProfile, iCurrentProfile );
        User::LeaveIfError( err );
        RL_TRACE_PRINT(" [ rl.exe ] ProfileNotifyL() startlisten");
        iProfileNotifyHandler->StartListeningL();  
        }
    else
        {
        RL_TRACE_PRINT(" [ rl.exe ] ProfileNotifyL() stop listen ");
        iProfileNotifyHandler->StopListening();
        
        delete iProfileNotifyHandler;
        iProfileNotifyHandler = NULL;
        }
    RL_TRACE_PRINT(" [ rl.exe ] exit ProfileNotifyL() ");
    return ETrue;
    }
    
// -----------------------------------------------------------------------------
// CRemoteLock::GetProfile
// Get the current Profile
// Returns:   ETrue if everything is OK
//            EFalse otherwise
// -----------------------------------------------------------------------------
//
TBool CRemoteLock::GetProfile( TInt& aProfile )
    {
    RL_TRACE_PRINT(" [ rl.exe ] GetProfile() ");
    TInt err;
    err = iProfileSession->Get( KProEngActiveProfile, aProfile );
    RL_TRACE_PRINT(" [ rl.exe ] exit GetProfile() ");
    return ( err == KErrNone );
    }    


// -----------------------------------------------------------------------------
// CRemoteLock::HandleNotifyGeneric
// Remote lock Notify handler 
//
// -----------------------------------------------------------------------------
//
void CRemoteLock::HandleNotifyGeneric(
    TUint32 /*aId*/ ) 
    {
    RL_TRACE_PRINT(" [ rl.exe ] HandleNotifyGeneric() "); 
    
    TInt profile = 0; 
    GetProfile( profile ); 
    
    if ( ( profile == EProfileOffLineId ) && ( iIsEnabled  ) && ( iCurrentProfile != EProfileOffLineId )) 
        {
        ActivateDeviceLock();  
        }
        
    iCurrentProfile = profile;
    RL_TRACE_PRINT(" [ rl.exe ] exit HandleNotifyGeneric() ");
    }
    
//EOF  

