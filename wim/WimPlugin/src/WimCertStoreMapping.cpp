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
* Description:  This implementation represents one entry in an array
*               where one mapping contains certificate info 
*               with trusted settings
*
*/


// INCLUDE FILES

#include "WimCertStoreMapping.h"
#include "WimTrace.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimCertStoreMapping::NewL()
// NewL constructor which uses NewLC constructor
// -----------------------------------------------------------------------------
//
CWimCertStoreMapping* CWimCertStoreMapping::NewL()
    {
    _WIMTRACE ( _L( "CWimCertStoreMapping::NewL()" ) );
    CWimCertStoreMapping* self = CWimCertStoreMapping::NewLC();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMapping::NewLC()
// NewLC constructor pushes pointer to cleanup stack
// -----------------------------------------------------------------------------
//
CWimCertStoreMapping* CWimCertStoreMapping::NewLC()
    {
    _WIMTRACE ( _L( "CWimCertStoreMapping::NewLC()" ) );
    CWimCertStoreMapping* self = new( ELeave ) CWimCertStoreMapping();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMapping::ConstructL()
// Create an array for certificate applications in this mapping
// -----------------------------------------------------------------------------
//
void CWimCertStoreMapping::ConstructL()
    {
    _WIMTRACE ( _L( "CWimCertStoreMapping::ConstructL()" ) );
    iCertificateApps = new( ELeave ) RArray<TUid>();
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMapping::CWimCertStoreMapping()
// Default constructor.
// -----------------------------------------------------------------------------
//
CWimCertStoreMapping::CWimCertStoreMapping()
    {   
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMapping::~CWimCertStoreMapping()
// Destructor
// -----------------------------------------------------------------------------
//
CWimCertStoreMapping::~CWimCertStoreMapping()
    {
    _WIMTRACE ( _L( "CWimCertStoreMapping::~CWimCertStoreMapping()" ) );
    // CCTCertInfo class release is done in CWimCertInfo class
    iEntry = NULL;

    if ( iCertificateApps )
        {
        iCertificateApps->Close();
        delete iCertificateApps;
        }
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMapping::SetEntryL()
// Sets given certificate info into mapping entry
// -----------------------------------------------------------------------------
//
void CWimCertStoreMapping::SetEntryL( CCTCertInfo* aCertInfo )
    {
    _WIMTRACE ( _L( "CWimCertStoreMapping::SetEntryL()" ) );
    __ASSERT_ALWAYS( aCertInfo, User::Leave( KErrArgument ) );
    
    if ( iEntry )
        {
        iEntry->Release();
        }
    iEntry = aCertInfo;
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMapping::SetCertificateAppsL()
// Sets certificate applications to mapping entry
// -----------------------------------------------------------------------------
//
void CWimCertStoreMapping::SetCertificateAppsL( RArray<TUid>* aCertificateApps )
    {
    _WIMTRACE ( _L( "CWimCertStoreMapping::SetCertificateAppsL()" ) );
    TInt count = aCertificateApps->Count();
    for ( TInt i = 0; i < count; i++ )
        {
        __ASSERT_ALWAYS( &aCertificateApps[i], User::Leave( KErrArgument ) );
        }
    iCertificateApps->Close();
    delete iCertificateApps;
    iCertificateApps = aCertificateApps;
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMapping::Entry() const
// Returns the certificate part of mapping 
// -----------------------------------------------------------------------------
//
CCTCertInfo* CWimCertStoreMapping::Entry() const
    {
    _WIMTRACE ( _L( "CWimCertStoreMapping::Entry()" ) );
    return iEntry;
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMapping::CertificateApps()
// Returns an array of applications from mapping entry the certificate supports
// -----------------------------------------------------------------------------
//
const RArray<TUid>& CWimCertStoreMapping::CertificateApps() const
    {
    _WIMTRACE ( _L( "CWimCertStoreMapping::CertificateApps()" ) );
    return *iCertificateApps;
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMapping::IsApplicable()
// Returns boolean value indicating if given applicaton is supported by
// the certificate of mapping entry
// -----------------------------------------------------------------------------
//
TBool CWimCertStoreMapping::IsApplicable( const TUid& aApplication ) const
    {
    _WIMTRACE ( _L( "CWimCertStoreMapping::IsApplicable()" ) );
    TInt count = iCertificateApps->Count();
    for ( TInt i = 0; i < count; i++ )
        {
        TUid app = ( *iCertificateApps )[i];
        if ( app == aApplication )
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMapping::Trusted()
// Returns boolean value indicating if the certificate in mapping 
// entry is trusted
// -----------------------------------------------------------------------------
//
TBool CWimCertStoreMapping::Trusted() const
    {
    _WIMTRACE ( _L( "CWimCertStoreMapping::Trusted()" ) );
    return iTrusted;
    }

// -----------------------------------------------------------------------------
// CWimCertStoreMapping::SetTrusted()
// Sets mapping entry to state of trusted or not trusted ( true/false )
// -----------------------------------------------------------------------------
//
void CWimCertStoreMapping::SetTrusted( TBool aTrusted )
    {
    _WIMTRACE ( _L( "CWimCertStoreMapping::SetTrusted()" ) );
    iTrusted = aTrusted;
    }
