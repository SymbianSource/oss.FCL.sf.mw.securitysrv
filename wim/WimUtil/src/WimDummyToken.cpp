/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Token and TokenType implementation for TrustSettingsStore
*
*/



// INCLUDE FILES
#include    "WimDummyToken.h"
#include    "WimTrace.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimDummyToken::CWimDummyToken
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimDummyToken::CWimDummyToken( MCTTokenType& aTokenType )
    : iTokenType( aTokenType )
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimDummyToken::CWimDummyToken | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimDummyToken::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimDummyToken::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimDummyToken::ConstructL | Begin"));
    iCount = -1; // -1 instead of 0 because of different construction method
    }

// -----------------------------------------------------------------------------
// CWimDummyToken::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimDummyToken* CWimDummyToken::NewL( MCTTokenType& aTokenType )
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimDummyToken::NewL | Begin"));
    CWimDummyToken* self = new( ELeave ) CWimDummyToken( aTokenType );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// Destructor
CWimDummyToken::~CWimDummyToken()
    {
    _WIMTRACE(_L("WIM | WIMUtil | CWimDummyToken::~CWimDummyToken | Begin"));
    delete iSerialNumber;
    }

// -----------------------------------------------------------------------------
// CWimDummyToken::Label()
// Get the label from current token
// -----------------------------------------------------------------------------
//
const TDesC& CWimDummyToken::Label()
    {
    _WIMTRACE ( _L( "CWimDummyToken::Label()" ) );
    return KNullDesC;
    }

// -----------------------------------------------------------------------------
// CWimDummyToken::Information()
// Returns the specified information string about the token
// -----------------------------------------------------------------------------
//
const TDesC& CWimDummyToken::Information( 
    TTokenInformation aRequiredInformation )
    {
    _WIMTRACE ( _L( "CWimDummyToken::Information()" ) );

    switch ( aRequiredInformation ) 
        {
        case ESerialNo: // Returns the Serialnumber from current token
            {
            return *iSerialNumber;
            }
        default:    // Returns empty string 
            {
            break;
            }
        }

    return KNullDesC;
    }

// -----------------------------------------------------------------------------
// MCTTokenType& CWimDummyToken::TokenType()
// Returns the associated token type.
// -----------------------------------------------------------------------------
//
MCTTokenType& CWimDummyToken::TokenType()
    {
    _WIMTRACE ( _L( "CWimDummyToken::TokenType()" ) );
    return iTokenType;
    }

// -----------------------------------------------------------------------------
// TCTTokenHandle CWimDummyToken::Handle()
// Returns the token's handle. TCTTokenHandle defines a handle to a subclass 
// of the MCTToken class
// WimSecModule()->TokenNumber retrieves the token actually exists. 
// Values 0..7 are for hardware tokens. Value 255 is for SoftId-token.
// -----------------------------------------------------------------------------
//
TCTTokenHandle CWimDummyToken::Handle()
    {
    _WIMTRACE ( _L( "CWimDummyToken::Handle()" ) );
    return TCTTokenHandle( TokenType().Type(), 0 );
    }

// -----------------------------------------------------------------------------
// CWimDummyToken::DoGetInterface()
// Returns a valid interface and KErrNone, or interface = NULL and 
// KErrNotSupported if it isn't supported by this token
// -----------------------------------------------------------------------------
// 
void CWimDummyToken::DoGetInterface(
    TUid /*aRequiredInterface*/,
    MCTTokenInterface*& /*aReturnedInterface*/,
    TRequestStatus& /*aStatus*/ )
    {
    _WIMTRACE ( _L( "CWimDummyToken::DoGetInterface()" ) );
    }

// -----------------------------------------------------------------------------
// CWimDummyToken::DoCancelGetInterface()
// Nothing to do
// -----------------------------------------------------------------------------
//
TBool CWimDummyToken::DoCancelGetInterface()
    {
    _WIMTRACE ( _L( "CWimDummyToken::DoCancelGetInterface()" ) );
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CWimDummyToken::ReferenceCount()
// Returns a reference to reference counter.
// -----------------------------------------------------------------------------
//
TInt& CWimDummyToken::ReferenceCount()
    {
    _WIMTRACE ( _L( "CWimDummyToken::ReferenceCount()" ) );
    return iCount;
    }

// -----------------------------------------------------------------------------
// CWimDummyToken::NotifyOnRemoval( TRequestStatus& aStatus )
// Notifies the client when the token has been removed.
// -----------------------------------------------------------------------------
//
void CWimDummyToken::NotifyOnRemoval( TRequestStatus& /*aStatus*/ )
    {
    _WIMTRACE ( _L( "CWimDummyToken::NotifyOnRemoval()" ) );
    }

// -----------------------------------------------------------------------------
// CWimDummyToken::CancelNotify()
// Cancels the NotifyOnRemoval request.
// -----------------------------------------------------------------------------
//
void CWimDummyToken::CancelNotify()
    {
    _WIMTRACE ( _L( "CWimDummyToken::CancelNotify()" ) );
    }

// -----------------------------------------------------------------------------
// CWimDummyToken::SetSerialNumber()
// Sets serial number for token
// -----------------------------------------------------------------------------
//
void CWimDummyToken::SetSerialNumber( TDesC& aSerialNumber )
    {
    _WIMTRACE ( _L( "CWimDummyToken::SetSerialNumber()" ) );
    if ( iSerialNumber )
        {
        delete iSerialNumber;
        iSerialNumber = NULL;
        }

    TRAPD( err, iSerialNumber = HBufC::NewL( aSerialNumber.Length() ) );
    if ( err != KErrNone )
        {
        return;
    	  }
    TPtr serialNbr = iSerialNumber->Des();
    serialNbr.Copy( aSerialNumber );
    }


// ============== TokenType ====================================================

// -----------------------------------------------------------------------------
// CWimDummyTokenType::CWimDummyTokenType()
// Default constructor
// -----------------------------------------------------------------------------
//
CWimDummyTokenType::CWimDummyTokenType()
    {
    _WIMTRACE ( _L( "CWimDummyToken::CWimDummyTokenType()" ) );
    }

// -----------------------------------------------------------------------------
// CWimDummyTokenType::ConstructL()
// Second phase constructor
// -----------------------------------------------------------------------------
//
void CWimDummyTokenType::ConstructL()
    {
    _WIMTRACE ( _L( "CWimDummyTokenType::ConstructL()" ) );
    }

// -----------------------------------------------------------------------------
// CWimDummyTokenType::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimDummyTokenType* CWimDummyTokenType::NewL() 
    {
    _WIMTRACE ( _L( "CWimDummyToken::NewL()" ) );
    CWimDummyTokenType* that = new( ELeave ) CWimDummyTokenType();
    CleanupStack::PushL( that );
    that->ConstructL();
    CleanupStack::Pop();
    return that;
    }

// -----------------------------------------------------------------------------
// CWimDummyTokenType::~CWimDummyTokenType()
// Destructor.
// -----------------------------------------------------------------------------
//
CWimDummyTokenType::~CWimDummyTokenType()
    {
    _WIMTRACE ( _L( "CWimDummyTokenType::~CWimDummyTokenType()" ) );

    }

// -----------------------------------------------------------------------------
// CWimDummyTokenType::List()
// List all the tokens of this type.
// -----------------------------------------------------------------------------
//
void CWimDummyTokenType::List(
    RCPointerArray<HBufC>& /*aTokens*/,
    TRequestStatus& /*aStatus*/ )
    {
    _WIMTRACE ( _L( "CWimDummyToken::List()" ) );
    }

// -----------------------------------------------------------------------------
// CWimDummyTokenType::CancelList()
// Cancel a list operation.
// -----------------------------------------------------------------------------
//
void CWimDummyTokenType::CancelList()
    {
    _WIMTRACE ( _L( "CWimDummyTokenType::CancelList" ) );
    }

// -----------------------------------------------------------------------------
// CTokenTypeImplementation::OpenToken() 
// Opens new token, which name is specified in aTokenInfo
// This is not true asynchronous function.
// -----------------------------------------------------------------------------
//
void CWimDummyTokenType::OpenToken(
    const TDesC& /*aTokenInfo*/,
    MCTToken*& /*aToken*/,
    TRequestStatus& /*aStatus*/ )
    { 
    _WIMTRACE ( _L( "CWimDummyTokenType::OpenToken with name" ) );
    }

// -----------------------------------------------------------------------------
// CWimDummyTokenType::OpenToken()
// Open new token, The handle of the required token is used to get needed 
// information. Before opening new token we need to compare token numbers.
// This is not true asynchronous function.
// -----------------------------------------------------------------------------
//
void CWimDummyTokenType::OpenToken(
    TCTTokenHandle /*aHandle*/,
    MCTToken*& /*aToken*/,
    TRequestStatus& /*aStatus*/ )
    {
    _WIMTRACE ( _L( "CWimDummyTokenType::OpenToken with handle" ) );
    }
// -----------------------------------------------------------------------------
// CWimDummyTokenType::CancelOpenToken()
// Cancel an OpenToken operation.
// -----------------------------------------------------------------------------
//
void CWimDummyTokenType::CancelOpenToken()
    {
    _WIMTRACE ( _L( "CWimDummyTokenType::CancelOpenToken" ) );
    }


//  End of File  
