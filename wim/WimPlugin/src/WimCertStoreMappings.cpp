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
* Description:  This implementation represents an array of mapping entries 
*               (certificate infos) with trusted settings
*
*/


// INCLUDE FILES

#include "WimCertStoreMappings.h"
#include "WimTrace.h"
//#include "WimDebug.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimCertStoreMappings::NewL()
// Two-phased constructor
// -----------------------------------------------------------------------------
//
CWimCertStoreMappings* CWimCertStoreMappings::NewL()
    {
    _WIMTRACE ( _L( "CWimCertStoreMappings::NewL()" ) );
    CWimCertStoreMappings* self = new( ELeave ) CWimCertStoreMappings();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMappings::CWimCertStoreMappings()
// Default constructor
// -----------------------------------------------------------------------------
//
CWimCertStoreMappings::CWimCertStoreMappings()
    {
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMappings::ConstructL()
// Second phase constructor
// -----------------------------------------------------------------------------
//
void CWimCertStoreMappings::ConstructL()
    {
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMappings::~CWimCertStoreMappings()
// Destructor. 
// -----------------------------------------------------------------------------
//
CWimCertStoreMappings::~CWimCertStoreMappings()
    {
    _WIMTRACE ( _L( "CWimCertStoreMappings::~CWimCertStoreMappings()" ) );
    iMappings.ResetAndDestroy();
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMappings::Count()
// Returns the count of mapping entries from the internal array.
// -----------------------------------------------------------------------------
//
TInt CWimCertStoreMappings::Count() const
    {
    return iMappings.Count();
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMappings::AddL()
// Adds an mapping entry to the internal array.
// -----------------------------------------------------------------------------
//
void CWimCertStoreMappings::AddL( const CWimCertStoreMapping* aEntry )
    {
    _WIMTRACE ( _L( "CWimCertStoreMappings::AddL()" ) );
    User::LeaveIfError( iMappings.Append( aEntry ) );
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMappings::Index()
// Returns the index of mapping entry from the internal array using given
// certificate info.
// -----------------------------------------------------------------------------
//
TInt CWimCertStoreMappings::Index( const CCTCertInfo& aCertInfo )
    {
    _WIMTRACE ( _L( "CWimCertStoreMappings::Index()" ) );
    TInt count = iMappings.Count();
    TInt ix = KErrNotFound;
    for ( TInt i = 0; i < count; i++ )
        {
        CWimCertStoreMapping* mapping = ( iMappings )[i];
        if ( aCertInfo == *( mapping->Entry() ) )
            {
            ix = i;
            i = count; // End this loop
            }
        }
    return ix;
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMapping* CWimCertStoreMappings::Mapping()
// Returns the mapping entry from the internal array using given index
// -----------------------------------------------------------------------------
//
CWimCertStoreMapping* CWimCertStoreMappings::Mapping( TInt aIndex )
    {
    _WIMTRACE ( _L( "CWimCertStoreMappings::Mapping()" ) );
    return ( iMappings )[aIndex];
    }

// -----------------------------------------------------------------------------
// CCTCertInfo& CWimCertStoreMappings::Entry()
// Returns the certificate info from mapping entry using given index
// -----------------------------------------------------------------------------
//
const CCTCertInfo& CWimCertStoreMappings::Entry( TInt aIndex )
    {
    _WIMTRACE ( _L( "CWimCertStoreMappings::Entry()" ) );
    CWimCertStoreMapping* mapping = ( iMappings )[aIndex];
    return *mapping->Entry();
    }

// -----------------------------------------------------------------------------
// CCTCertInfo& CWimCertStoreMappings::EntryByHandleL()
// Returns the certificate info from the mapping entry using given handle
// -----------------------------------------------------------------------------
//
const CCTCertInfo& CWimCertStoreMappings::EntryByHandleL( TInt aHandle ) const
    {
    _WIMTRACE ( _L( "CWimCertStoreMappings::EntryByHandleL()" ) );
    CCTCertInfo* info = NULL;
    TInt count = iMappings.Count();
    for ( TInt i = 0; i < count; i++ )
        {
        CWimCertStoreMapping* mapping = ( iMappings )[i];
        if ( aHandle == mapping->Entry()->Handle().iObjectId )
            {
            info = mapping->Entry();
            i = count; // End this loop
            }
        }
    if ( !info )
        {
        User::Leave( KErrNotFound );
        }
    return *info;
    }

// End of file
