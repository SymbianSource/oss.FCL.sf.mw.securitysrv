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
* Description:  Implementation of RemoteLock Settings API
*
*/



// INCLUDE FILES 
#include <RemoteLockSettings.h>
#include <hash.h>
#include <centralrepository.h>
#include "RemoteLockPrivateCRKeys.h"
#include "RemoteLockTrace.h"

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS
_LIT( KRLockLeadingHexZero, "0" );

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS


// ============================ MEMBER FUNCTIONS ===============================
// -----------------------------------------------------------------------------
// CRemoteLockSettings::NewL
// Two-Phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CRemoteLockSettings* CRemoteLockSettings::NewL( 
    MRemoteLockSettingsHandler* aHandler )
    {
    RL_TRACE_PRINT(" [ rlsetting.dll ] NewL() ");
    
    CRemoteLockSettings* self = CRemoteLockSettings::NewLC( aHandler );
         
    CleanupStack::Pop( self );
    RL_TRACE_PRINT(" [ rlsetting.dll ] exit NewL() ");
    return self;
    }


// -----------------------------------------------------------------------------
// CRemoteLockSettings::NewLC
// Two-Phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CRemoteLockSettings* CRemoteLockSettings::NewLC( 
    MRemoteLockSettingsHandler* aHandler )
    {
    
    CRemoteLockSettings* self = new ( ELeave ) CRemoteLockSettings( aHandler );
    
    CleanupStack::PushL( self );
    
    self->ConstructL();
    return self;
    }


// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------
EXPORT_C CRemoteLockSettings::~CRemoteLockSettings()
    {
    if ( iNotifyHandler )
        {
        iNotifyHandler->StopListening();
        delete iNotifyHandler;
        }

    delete iSession;
    }


// ----------------------------------------------------------------------
// CRemoteLockSettings::GetLockCode
// Gets remote lock code
// Returns: ETrue
//          EFalse  
// ----------------------------------------------------------------------
//
EXPORT_C TBool CRemoteLockSettings::GetLockCode( TDes& aLockCode ) const                                               
    {
    RL_TRACE_PRINT(" [ rlsetting.dll ] Getlockcode() ");
    TInt err = iSession->Get( KRLLockCode, aLockCode ); 
    RL_TRACE_PRINT(" [ rlsetting.dll ] exit Getlockcode() ");   
    return ( err == KErrNone );
    }


// ----------------------------------------------------------------------
// CRemoteLockSettings::GetLockCode
// Gets remote lock code and trimmed remote lock code
// Returns: ETrue
//          EFalse  
// ----------------------------------------------------------------------
//
EXPORT_C TBool CRemoteLockSettings::GetLockCode( 
    TDes& aLockCode, 
    TDes& aLockCodeTrimmed ) const 
    {
    RL_TRACE_PRINT(" [ rlsetting.dll ] Getlockcode() ");
    TInt err = iSession->Get( KRLLockCode, aLockCode );    
    
    if ( err != KErrNone )
        {
        return EFalse;
        }
    
    err = iSession->Get( KRLLockCode2, aLockCodeTrimmed );
    
    if ( err == KErrNone )
        {
        RL_TRACE_PRINT(" [ rlsetting.dll ] exit Getlockcode() noError");
        return ETrue;
        }
    else
        {
        RL_TRACE_PRINT(" [ rlsetting.dll ] exit Getlockcode() Error");
        return EFalse;
        }
    }


// -----------------------------------------------------------------------------
// CRemoteLockSettings::SetEnabled
// Enables and sets remote lock code
// Returns   ETrue
//           EFalse  
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CRemoteLockSettings::SetEnabledL( const TDesC& aLockCode )
    {
    RL_TRACE_PRINT(" [ rlsetting.dll ] SetEnabledL() ");
    if ( aLockCode.Length() > KRLockMaxLockCodeLength )
       {
       return EFalse;
       }  
       
    // Creates MD5 object.
    CMD5* hash = CMD5::NewL();
    CleanupStack::PushL( hash );

    TBuf<KRLockMaxLockCodeLength> tempBuf;
    tempBuf = aLockCode;
       
    HBufC8* hashBuf = HBufC8::NewLC( KRLockMaxLockCodeLength * 2 );        
    TPtr8 tempPtr = hashBuf->Des();
    tempPtr.Copy( tempBuf );
       
    // Calculate the digest.
    TPtrC8 hashPtr = hash->Hash( *hashBuf );
    HBufC* storeBuf = HBufC::NewLC( hash->HashSize());     
    storeBuf->Des().Copy( hashPtr );

       // Convert digest into hexadecimal representation
    HBufC* hexStoreBuf = HBufC::NewLC( storeBuf->Length() * 2);
    TBuf<2> singleHex;
    TInt i = 0;
    for ( i = 0; i < storeBuf->Length(); i++ )
        {
        singleHex.Num( storeBuf->Des()[i], EHex );
        if ( singleHex.Length() == 1 )
            hexStoreBuf->Des().Append( KRLockLeadingHexZero );
        hexStoreBuf->Des().Append( singleHex );
        }
       
    // Saves the digest into repository file.
    TInt err = 0;
    err = iSession->Set( KRLLockCode, *hexStoreBuf );
       
    if ( err != KErrNone )
        {
        CleanupStack::PopAndDestroy( 4 ); 
        return EFalse;
        }
        
     // Then process the trimmed password the same way.
    tempBuf.TrimAll();
    tempPtr.Copy( tempBuf );
    // Calculate the digest.
    hash->Reset();
    TPtrC8 hashPtr1 = hash->Hash( *hashBuf );
    storeBuf->Des().Copy( hashPtr1 );
    // Convert digest into hexadecimal representation
    hexStoreBuf->Des().Zero();
    for ( i = 0; i < storeBuf->Length(); i++ )
        {
        singleHex.Num( storeBuf->Des()[i], EHex );
        if ( singleHex.Length() == 1 )
            hexStoreBuf->Des().Append( KRLockLeadingHexZero );
        hexStoreBuf->Des().Append( singleHex );
        }
        
    // Saves the digest into ini file.
    err = iSession->Set( KRLLockCode2, *hexStoreBuf );
        
    if ( err != KErrNone )
        {
        RL_TRACE_PRINT(" [ rlsetting.dll ] SetenabledL() exit Error ");
        CleanupStack::PopAndDestroy( 4 );
        return EFalse;
        }
    
    err = iSession->Set( KRLEnable, 1 );
        
    if ( err != KErrNone )
        {
        RL_TRACE_PRINT(" [ rlsetting.dll ] SetenabledL() exit Error ");
        CleanupStack::PopAndDestroy( 4 );
        return EFalse;
        }
    RL_TRACE_PRINT(" [ rlsetting.dll ] SetenabledL() exit no Error ");
    CleanupStack::PopAndDestroy( 4 );//MD5, 3 Heap buffer
    return ETrue;
    }


// -----------------------------------------------------------------------------
// CRemoteLockSettings::SetDisabled
// Disables remote lock
// Returns   ETrue
//           EFalse  
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CRemoteLockSettings::SetDisabled()
    {
    RL_TRACE_PRINT(" [ rlsetting.dll ] SetDisabled() ");
    TInt err;
    err = iSession->Set( KRLEnable, 0 );
    RL_TRACE_PRINT(" [ rlsetting.dll ] exit SetDisabled() ");
    return ( err == KErrNone );
    }

// -----------------------------------------------------------------------------
// CRemoteLockSettings::GetEnabled
// Returns whether remote lock is enabled or disabled
// Returns    ETrue
//            EFalse  
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CRemoteLockSettings::GetEnabled( TBool& aEnable ) const
    {
    RL_TRACE_PRINT(" [ rlsetting.dll ] GetEnabled() ");  
    TInt err;
    err = iSession->Get( KRLEnable, aEnable );
    RL_TRACE_PRINT(" [ rlsetting.dll ] Exit GetEnabled() ");
    return ( err == KErrNone );
    }
    
   
// -----------------------------------------------------------------------------
// CRemoteLockSettings::CompareLockCodes
// Compares the given remote lock code to the stored one.
// Returns   ETrue
//           EFalse  
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CRemoteLockSettings::CompareLockCodesL( 
    const TDesC& aLockCode, 
    const TDesC& aCorrectLockCode ) const
    {
    RL_TRACE_PRINT(" [ rlsetting.dll ] CompareLockCodesL() ");
    if ( aLockCode.Length() > KRLockMaxLockCodeLength || 
        aCorrectLockCode.Length() > KRLockStoredLockCodeLength )
        {
        RL_TRACE_PRINT(" [ rlsetting.dll ] exit CompareLockCodeL() lockcode length is illegal ");
        return EFalse;
        }

    // Creates MD5 object.
    CMD5* hash = CMD5::NewL();
    CleanupStack::PushL( hash );

    HBufC8* hashBuf = HBufC8::NewLC( KRLockMaxLockCodeLength * 2 );
    TPtr8 tempPtr = hashBuf->Des();
    tempPtr.Copy( aLockCode );

    // Hashes data.
    TPtrC8 hashPtr = hash->Hash( *hashBuf );

    HBufC* lockCode = HBufC::NewLC( hash->HashSize() );

    // Copies hashPtr to lockCode.
    lockCode->Des().Copy( hashPtr );

    // Convert digest into hexadecimal representation
    HBufC* hexLockCode = HBufC::NewLC( lockCode->Length() * 2);
    TBuf<2> singleHex;
    for ( TInt i = 0; i < lockCode->Length(); i++ )
        {
        singleHex.Num( lockCode->Des()[i], EHex );
        if ( singleHex.Length() == 1 )
            hexLockCode->Des().Append( KRLockLeadingHexZero );
        hexLockCode->Des().Append( singleHex );
        }

    RL_TRACE_PRINT(" [ rlsetting.dll ] CompareLockCodeL() compare ");
    if ( hexLockCode->Compare( aCorrectLockCode ) == 0 )
        {
        // When two hash results matches each other.
        RL_TRACE_PRINT(" [ rlsetting.dll ] ComparelockcodeL() match ");
        CleanupStack::PopAndDestroy( 4 );
        return ETrue;
        }
    else
        {
        RL_TRACE_PRINT(" [ rlsetting.dll ] CompareLockCodeL() dismatch ");
        CleanupStack::PopAndDestroy( 4 );
        return EFalse;
        }
    }


// -----------------------------------------------------------------------------
// CRemoteLockSettings::RemoteLockNotifyL
// Setup RemoteLock notifier
// Returns:   ETrue
//            EFalse
// -----------------------------------------------------------------------------
//
EXPORT_C TBool CRemoteLockSettings::RemoteLockNotifyL( 
    const TBool aNotifyEnable ) 
    {
    RL_TRACE_PRINT(" [ rlsetting.dll ] RemotelockNotifyL() ");
    if ( !iNotifyHandler )
        {
        iNotifyHandler = CCenRepNotifyHandler::NewL( *this, *iSession ); 
        }

    if ( aNotifyEnable )
        {
        iNotifyHandler->StartListeningL();  
        }
    else
        {
        iNotifyHandler->StopListening();
        }
    RL_TRACE_PRINT(" [ rlsetting.dll ] exit RemotelockNotifyL() ");
    return ETrue;
    }

// ----------------------------------------------------------------------------
// CRemoteLockSettings::CRemoteLockSettings
// default constructor
// ----------------------------------------------------------------------------
CRemoteLockSettings::CRemoteLockSettings( MRemoteLockSettingsHandler* aHandler ) 
    {    
    iHandler = aHandler;
    } 
    
      
// -----------------------------------------------------------------------------
// CRemoteLockSettings::ConstructL
// Symbian 2nd phase constructor can leave
//------------------------------------------------------------------------------
//
void CRemoteLockSettings::ConstructL()
    {
    RL_TRACE_PRINT(" [ rlsetting.dll ] ConstructL() ");  
    iSession = CRepository::NewL( KCRUidRemoteLock ); 
    RL_TRACE_PRINT(" [ rlsetting.dll ] exit ConstructL() ");
    }
    

// -----------------------------------------------------------------------------
// CRemoteLockSettings::HandleNotifyGeneric
// Notify handler
//
// -----------------------------------------------------------------------------
//
void CRemoteLockSettings::HandleNotifyGeneric( 
    TUint32 /*aId*/ ) 
    {
    RL_TRACE_PRINT(" [ rlsetting.dll ] HandleNotifyGeneric() ");
    TRAPD( error, iHandler->HandleRemoteLockNotifyL() );
    error = error;
    RL_TRACE_PRINT(" [ rlsetting.dll ] exit HandleNotifyGeneric() ");
    }


