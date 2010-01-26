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
* Description:  This class is instantiated via ECom for a particular token type
*
*/


// INCLUDE FILES

#include "WimTokenType.h"
#include "WimToken.h"
#include "WimImplementationUID.hrh"
#include <ecom/implementationproxy.h>
#include "WimTrace.h"
//#include "WimDebug.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimTokenType::CWimTokenType()
// Default constructor
// -----------------------------------------------------------------------------
//
CWimTokenType::CWimTokenType()
    {
    }

// -----------------------------------------------------------------------------
// CWimTokenType::ConstructL()
// Second phase constructor
// -----------------------------------------------------------------------------
//
void CWimTokenType::ConstructL()
    {
    _WIMTRACE ( _L( "CWimTokenType::ConstructL()" ) );
    iWimSecModuleMgr = CWimSecModuleMgr::NewL(); 
    }

// -----------------------------------------------------------------------------
// CWimTokenType::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimTokenType* CWimTokenType::NewL() 
    {
    CWimTokenType* self = new( ELeave ) CWimTokenType();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimTokenType::~CWimTokenType()
// Destructor.
// -----------------------------------------------------------------------------
//
CWimTokenType::~CWimTokenType()
    {
    _WIMTRACE ( _L( "CWimTokenType::~CWimTokenType()" ) );
    delete iWimSecModuleMgr;
    }

// -----------------------------------------------------------------------------
// CWimTokenType::List()
// List all the tokens of this type.
// -----------------------------------------------------------------------------
//
void CWimTokenType::List( RCPointerArray<HBufC>& aTokens,  
                          TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimTokenType::List" ) );
    aStatus = KRequestPending;
    iWimSecModuleMgr->GetEntries( aTokens, aStatus );
    }

// -----------------------------------------------------------------------------
// CWimTokenType::CancelList()
// Cancel a list operation.
// -----------------------------------------------------------------------------
//
void CWimTokenType::CancelList()
    {
    _WIMTRACE ( _L( "CWimTokenType::CancelList" ) );
    iWimSecModuleMgr->CancelList();
    }

// -----------------------------------------------------------------------------
// CTokenTypeImplementation::OpenToken() 
// Opens new token, which name is specified in aTokenInfo
// This is not true asynchronous function.
// -----------------------------------------------------------------------------
//
void CWimTokenType::OpenToken( const TDesC& aTokenInfo, 
                               MCTToken*& aToken,
                               TRequestStatus& aStatus )
    { 
    _WIMTRACE ( _L( "CWimTokenType::OpenToken with name" ) );
    TInt err   = KErrNone;
    TInt i     = 0;

    aStatus    = KRequestPending;

    // Get the right Wim and pass it to CWimToken to be created.
    // Stack Wim count for efficiency reasons
    TInt count = iWimSecModuleMgr->WimCount(); 
    for ( i = 0; i < count && err == KErrNone; i++ ) 
        {
        HBufC* label = NULL;    // from heap
        // Get Wim label
        TRAP( err, label = iWimSecModuleMgr->WimLabelByIndexL( i ).AllocL() );
        if ( err == KErrNone ) 
            {
            // If labels match...
            if ( !aTokenInfo.Compare( label->Des() ) )
                {
                // Create token
                TRAP( err, aToken = CWimToken::NewL( iWimSecModuleMgr->GetWimSecModuleByIndexL( i ), *this ) );
                i = count + 1; // This completes for loop and separates
                               // from normal for loop end (i == count)
                if ( err == KErrNone )
                    {
                    IncReferenceCount();
                    }
                }
            }
        delete label;
        }

    if ( i == count && err == KErrNone ) // Not match found
        {
        err = KErrNotFound;
        }

    TRequestStatus* r = &aStatus;
    User::RequestComplete( r, err );

    }

// -----------------------------------------------------------------------------
// CWimTokenType::OpenToken()
// Open new token, The handle of the required token is used to get needed 
// information. Before opening new token we need to compare token numbers.
// This is not true asynchronous function.
// -----------------------------------------------------------------------------
//
void CWimTokenType::OpenToken( TCTTokenHandle aHandle,
                               MCTToken*& aToken,
                               TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimTokenType::OpenToken with handle" ) );
    TInt err   = KErrNone;
    TInt i     = 0;
    TInt count = 0;

    aStatus = KRequestPending; 

    // Current tokentype and requested token's tokentype must be the same
    if ( Type() == aHandle.iTokenTypeUid )
        {
        // Count of security modules:
        count = iWimSecModuleMgr->WimCount();
        // For effectivity reasons, stack the given handle
        TInt mytokennumber = aHandle.iTokenId;      
        // Let's go through all Wims and try to match handles
        for ( i = 0; i < count && err == KErrNone; i++ )
            {
            // Stack Wim handle
            TInt histokennumber = 0;    
            TRAP( err, histokennumber = iWimSecModuleMgr->
                                        WimTokenNumberByIndexL( i ) );
            if ( err == KErrNone )
                {
                if ( mytokennumber == histokennumber )
                    {   
                    HBufC* label = NULL;// From heap
                    // Let's get Wim label and use another 
                    // API function to get token info
                    TRAP( err, label = iWimSecModuleMgr->
                                       WimLabelByIndexL( i ).AllocL() );
                    if ( err == KErrNone )
                        {
                        // The completion of the next statement is handled 
                        // in the associated function
                        OpenToken( label->Des(), aToken, aStatus );
                        delete label;
                        return; // Must quit here, RequestComplete is done
                        }
                    delete label;   // Here we go if error in last TRAP
                    i = count + 1;  // This completes for loop and
                                    // separates from normal for loop end
                    }
                }
            } // for loop...
        } // if ( Type()...

    if ( i == count && err == KErrNone ) // No match found 
        {
        err = KErrNotFound;
        }

    TRequestStatus* r = &aStatus;
    User::RequestComplete( r, err );

    }
// -----------------------------------------------------------------------------
// CWimTokenType::CancelOpenToken()
// Cancel an OpenToken operation.
// -----------------------------------------------------------------------------
//
void CWimTokenType::CancelOpenToken()
    {
    _WIMTRACE ( _L( "CWimTokenType::CancelOpenToken" ) );
    // It's not really async, so you can't cancel.
    }

// -----------------------------------------------------------------------------
// TImplementationProxy ImplementationTable[] 
// An array listing the NewL functions of all the plugins.
// -----------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] = 
    {
    {{WIM_IMPLEMENTATION_UID},     (TProxyNewLPtr) CWimTokenType::NewL} //CARD-WIM
    };

// -----------------------------------------------------------------------------
// TImplementationProxy* ImplementationGroupProxy()
// This function is needed by ECom and is the only one exported function
// -----------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* 
                ImplementationGroupProxy( TInt& aTableCount )
    {
    aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );

    return ImplementationTable;
    }


