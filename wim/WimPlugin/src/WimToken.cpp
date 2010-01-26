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
* Description:  A token represents one instance of a particular kind of 
*               cryptographic module
*
*/


// INCLUDE FILES

#include "WimToken.h"
#include "WimCertStore.h"
#include "WimAuthenticationObjectList.h"
#include "WimAuthenticationObject.h"
#include "WimKeyStore.h"
#include "WimSecModuleMgr.h"
#include "WimImplementationUID.hrh"
#include "WimTokenListener.h"
#include "WimTrace.h"
#include <mctkeystoreuids.h>            // KInterfaceKeyStore


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimToken::CWimToken()
// Default constructor.
// -----------------------------------------------------------------------------
//
CWimToken::CWimToken( CWimSecModule* aWimSecModule, MCTTokenType& aTokenType )
          :iWimSecModule( aWimSecModule ),
           iTokenType( aTokenType )
    {
    }

// -----------------------------------------------------------------------------
// CWimToken::ConstructL()
// Second phase constructor
// -----------------------------------------------------------------------------
//
void CWimToken::ConstructL()
    {
    _WIMTRACE ( _L( "CWimToken::ConstructL()" ) );
    iLabel = WimSecModule()->Label().AllocL();
    iWimTokenListener = CWimTokenListener::NewL( iWimSecModule );
    iWimTokenListener->StartListening(); //Start listening
    iCount = 0;
    }

// -----------------------------------------------------------------------------
// CWimToken::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimToken* CWimToken::NewL( CWimSecModule* aWimSecModule, 
                            MCTTokenType& aTokenType )
    {
    _WIMTRACE ( _L( "CWimToken::NewL()" ) );
    // First parameter should not be NULL
    __ASSERT_ALWAYS( aWimSecModule, User::Leave( KErrCorrupt ) );
    CWimToken* self = new( ELeave ) CWimToken( aWimSecModule, aTokenType );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimToken::~CWimToken()
// Destructor
// -----------------------------------------------------------------------------
//
CWimToken::~CWimToken()
    {
    _WIMTRACE ( _L( "CWimToken::~CWimToken()" ) );        
    delete iLabel;
    iWimSecModule = NULL;
    delete iWimTokenListener;
    }

// -----------------------------------------------------------------------------
// CWimToken::Label()
// Get the label from current token
// -----------------------------------------------------------------------------
//
const TDesC& CWimToken::Label()
    {
    _WIMTRACE ( _L( "CWimToken::Label()" ) );
    return reinterpret_cast<TDesC&>( *iLabel );
    }

// -----------------------------------------------------------------------------
// CWimToken::Information()
// Returns the specified information string about the token
// -----------------------------------------------------------------------------
//
const TDesC& CWimToken::Information( TTokenInformation aRequiredInformation )
    {
    _WIMTRACE ( _L( "CWimToken::Information()" ) );
    if ( TokenRemoved() )
        {
        return KNullDesC;
        }

    if ( iWimSecModule )
        {
        switch( aRequiredInformation ) 
           {
            case EVersion: // Returns the version from current token
                {   
                return WimSecModule()->Version();
                }
            case ESerialNo: // Returns the Serialnumber from current token
                {
                return WimSecModule()->SerialNumber();
                }
            case EManufacturer: // Returns the Manufacturer from current token
                {
                return WimSecModule()->Manufacturer();
                }
            default:    // Returns empty string 
                break;
             }
        }
    return KNullDesC;
    }

// -----------------------------------------------------------------------------
// MCTTokenType& CWimToken::TokenType()
// Returns the associated token type.
// -----------------------------------------------------------------------------
//
MCTTokenType& CWimToken::TokenType()
    {
    _WIMTRACE ( _L( "CWimToken::TokenType()" ) );
    return iTokenType;
    }

// -----------------------------------------------------------------------------
// TCTTokenHandle CWimToken::Handle()
// Returns the token's handle. TCTTokenHandle defines a handle to a subclass 
// of the MCTToken class
// WimSecModule()->TokenNumber retrieves the token actually exists. 
// Values 0..7 are for hardware tokens. Value 255 is for SoftId-token.
// -----------------------------------------------------------------------------
//
TCTTokenHandle CWimToken::Handle()
    {
    _WIMTRACE ( _L( "CWimToken::Handle()" ) );
    const TInt tokenNumber = WimSecModule()->TokenNumber();
    return TCTTokenHandle( TokenType().Type(), tokenNumber );
    }

// -----------------------------------------------------------------------------
// CWimToken::DoGetInterface()
// Returns a valid interface and KErrNone, or interface = NULL and 
// KErrNotSupported if it isn't supported by this token
// -----------------------------------------------------------------------------
// 
void CWimToken::DoGetInterface( TUid aRequiredInterface,
                                MCTTokenInterface*& aReturnedInterface,
                                TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimToken::DoGetInterface()" ) );

    if ( TokenRemoved() )
        {
        Release();
        TRequestStatus* status = &aStatus;
        User::RequestComplete( status, KErrHardwareNotAvailable );             
        return;
        }
    
	    TInt error = KErrNone;

	    if ( TokenType().Type().iUid == WIM_IMPLEMENTATION_UID )
	        {
	        TRAP( error, MakeInterfaceL( aRequiredInterface, aReturnedInterface ) );
	        }
	    else
	        {
	        error = KErrArgument;
	        }

	    TRequestStatus* status = &aStatus;
	    User::RequestComplete( status, error );

	    // If something went wrong, this interface must be released, 
	    // because framework has just before incremented references
	    if ( error != KErrNone )
	        {
	        Release();
	        }	
    }

// -----------------------------------------------------------------------------
// CWimToken::MakeInterfaceL()
// At first checks if interface is supported. Note: support is depending on
// Uids that are indicated in resource file.
// Calls actual interface maker. All leavable functions are gathered into this.
// -----------------------------------------------------------------------------
//
void CWimToken::MakeInterfaceL( TUid aRequiredInterface,
                                MCTTokenInterface*& aReturnedInterface )
    {
    _WIMTRACE ( _L( "CWimToken::MakeInterfaceL()" ) );

    RCPointerArray <CCTTokenTypeInfo> tokenTypes;

    TCurrentTokenType filter;

    CCTTokenTypeInfo::ListL( tokenTypes, filter );

    if ( tokenTypes.Count() != 1 ) // There shoud be only one Wim implementation
        {
        tokenTypes.Close();
        User::Leave( KErrCorrupt );
        }

    // Check that given interface uid is found from resource file
    TInt count = tokenTypes[0]->Interfaces().Count();
    TInt i = 0;

    for ( ; i < count; i++ )
        {
        if ( tokenTypes[0]->Interfaces()[i].iUid == aRequiredInterface.iUid )
            {
            i = count + 1;
            }
        }

    tokenTypes.Close();

    if ( i == count )
        {
        User::Leave( KErrNotSupported );
        }

    switch ( aRequiredInterface.iUid )
        {
        case KInterfaceCertStore:   // Read Only Cert Store Interface.
            {
            iCertStoreIf = CWimCertStore::NewL( *this );
            aReturnedInterface = iCertStoreIf;
            break;
            }
        case KInterfaceWritableCertStore:
            {                       // Writable Cert Store Interface
            iCertStoreWritableIf = CWimCertStore::NewL( *this );
            aReturnedInterface = iCertStoreWritableIf;
            break;
            }
        case KInterfaceKeyStore:    // KeyStore Interface
            {
            iKeyStoreIf = CWimKeyStore::NewL( *this );
            aReturnedInterface = iKeyStoreIf;
            break;
            }
        case KCTInterfaceAuthenticationObject:
            {                       // Authentication Object Interface
            iAuthObjectListIf = CWimAuthenticationObjectList::NewL( *this );
            aReturnedInterface = iAuthObjectListIf;
            break;
            }
        default: // Should be checked in the caller function
            { 
            User::Leave( KErrCorrupt );             
            break;
            }
        }
    }
// -----------------------------------------------------------------------------
// CWimToken::DoCancelGetInterface()
// Nothing to do
// -----------------------------------------------------------------------------
//
TBool CWimToken::DoCancelGetInterface()
    {
    _WIMTRACE ( _L( "CWimToken::DoCancelGetInterface()" ) );
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CWimToken::ReferenceCount()
// Returns a reference to reference counter.
// -----------------------------------------------------------------------------
//
TInt& CWimToken::ReferenceCount()
    {
    return iCount;
    }

// -----------------------------------------------------------------------------
// CWimToken::NotifyOnRemoval( TRequestStatus& aStatus )
// Notifies the client when the token has been removed.
// -----------------------------------------------------------------------------
//
void CWimToken::NotifyOnRemoval( TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimToken::NotifyOnRemoval()" ) );
    iWimTokenListener->SetClientStatus( aStatus );
    }

// -----------------------------------------------------------------------------
// CWimToken::CancelNotify()
// Cancels the NotifyOnRemoval request.
// -----------------------------------------------------------------------------
//
void CWimToken::CancelNotify()
    {
    _WIMTRACE ( _L( "CWimToken::CancelNotify()" ) );
    if ( TokenRemoved() )
        {
        return;
        }
    iWimTokenListener->ClearClientStatus();
    }

// -----------------------------------------------------------------------------
// CWimToken::WimSecModule()
// Returns a pointer to current security module
// -----------------------------------------------------------------------------
//
CWimSecModule* CWimToken::WimSecModule()
    {
    return iWimSecModule;
    }

// -----------------------------------------------------------------------------
// CWimToken::TokenListener()
// Returns a pointer to token listener
// -----------------------------------------------------------------------------
//
CWimTokenListener* CWimToken::TokenListener()
    {
    return iWimTokenListener;
    }

// -----------------------------------------------------------------------------
// CWimToken::TokenRemoved()
//
// -----------------------------------------------------------------------------
//
TBool CWimToken::TokenRemoved()
    {
    if ( TokenListener()->TokenStatus() == KRequestPending )
        {
        return EFalse;
        }
    else
        {
        return ETrue;
        }
    }
// -----------------------------------------------------------------------------
// TCurrentTokenType::Accept( const CCTTokenTypeInfo& ) const
// Returns true if current token type is same as given token type
// -----------------------------------------------------------------------------
//
TBool TCurrentTokenType::Accept( const CCTTokenTypeInfo& aInfo ) const
    {
    if ( aInfo.Type().iUid == WIM_IMPLEMENTATION_UID )
        {
        return ETrue;
        }
    else
        {
        return EFalse;
        }   
    }               
    
