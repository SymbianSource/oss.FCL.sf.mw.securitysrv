/*
* Copyright (c) 2002-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Memory management function for WimServer to handle WIMI memory
*
*/


// INCLUDE FILES
#include    "WimMemMgmt.h"
#include    "Wimi.h"        // WIMI definitions
#include    "WimConsts.h"
#include    "WimTrace.h"
#include    "WimCleanup.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimMemMgmt::CWimMemMgmt
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimMemMgmt::CWimMemMgmt()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimMemMgmt::CWimMemMgmt | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimMemMgmt::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimMemMgmt::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimMemMgmt::ConstructL | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimMemMgmt::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimMemMgmt* CWimMemMgmt::NewL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimMemMgmt::NewL | Begin"));
    CWimMemMgmt* self = new( ELeave ) CWimMemMgmt;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// Destructor
CWimMemMgmt::~CWimMemMgmt()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimMemMgmt::~CWimMemMgmt | Begin"));

    TInt refsCount = iRefs.Count();
    for( TInt index = 0; index < refsCount; ++index )
        {
        TWimiAllocRef* allocRef = iRefs[ index ];
        if( allocRef )
            {
            if( allocRef->refType == EWimiRefpt )
                {
                _WIMTRACE2(_L("WIM | WIMServer | CWimMemMgmt::~CWimMemMgmt | -ref 0x%08x"), allocRef->ref);
                free_WIMI_Ref_t( allocRef->ref );
                }
            else
                {
                _WIMTRACE2(_L("WIM | WIMServer | CWimMemMgmt::~CWimMemMgmt | -refLst 0x%08x"), allocRef->refLst);
                free_WIMI_RefList_t( allocRef->refLst );
                }
            }
        }
    iRefs.ResetAndDestroy();

    if( iWimRef )
        {
        _WIMTRACE2(_L("WIM | WIMServer | CWimMemMgmt::~CWimMemMgmt, -ref 0x%08x"), iWimRef);
        free_WIMI_Ref_t( iWimRef );
        }
    }

// -----------------------------------------------------------------------------
// CWimMemMgmt::SetWIMRef
// Sets active memory reference to iWimRef member variable.
// -----------------------------------------------------------------------------
//
void CWimMemMgmt::SetWIMRef( WIMI_Ref_pt aWimRef )
    {
    __ASSERT_DEBUG( iWimRef == NULL, User::Invariant() );
    _WIMTRACE2(_L("WIM | WIMServer | CWimMemMgmt::SetWIMRef, +ref 0x%08x"), aWimRef);
    iWimRef = aWimRef;
    }

// -----------------------------------------------------------------------------
// CWimMemMgmt::GetWIMRef
// Fetches active reference.
// -----------------------------------------------------------------------------
//
WIMI_Ref_pt CWimMemMgmt::WimRef()
    {
    _WIMTRACE2(_L("WIM | WIMServer | CWimMemMgmt::GetWIMRef, iWimRef 0x%08x"), iWimRef);
    return iWimRef;
    }

// -----------------------------------------------------------------------------
// CWimMemMgmt::AppendWIMRefLstL
// Creates and appends a new WIMI_Alloc_Ref item to iRefs array.
// -----------------------------------------------------------------------------
//
void CWimMemMgmt::AppendWIMRefLstL( WIMI_RefList_t aRefLst )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimMemMgmt::AppendWIMRefLstL | Begin"));
    CleanupPushWimRefListL( aRefLst ); // take ownership first
    TWimiAllocRef* allocRef = new( ELeave ) TWimiAllocRef;
    CleanupStack::PushL( allocRef );
    allocRef->refLst = aRefLst;
    allocRef->refType = EWimiRefListt;
    iRefs.AppendL( allocRef );
    _WIMTRACE2(_L("WIM | WIMServer | CWimMemMgmt::AppendWIMRefLstL, +refLst 0x%08x"), aRefLst);
    CleanupStack::Pop( 2, aRefLst );    // allocRef, aRefLst
    }

// -----------------------------------------------------------------------------
// CWimMemMgmt::AppendWIMRefL
// Creates and appends a new WIMI_Alloc_Ref item to iRefs array.
// -----------------------------------------------------------------------------
//
void CWimMemMgmt::AppendWIMRefL( WIMI_Ref_pt aRef )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimMemMgmt::AppendWIMRefL | Begin"));
    CleanupPushWimRefL( aRef );    // take ownership first
    TWimiAllocRef* allocRef = new( ELeave ) TWimiAllocRef;
    CleanupStack::PushL( allocRef );
    allocRef->ref = aRef;
    allocRef->refType = EWimiRefpt;
    iRefs.AppendL( allocRef );
    _WIMTRACE2(_L("WIM | WIMServer | CWimMemMgmt::AppendWIMRefL, +ref 0x%08x"), aRef);
    CleanupStack::Pop( 2, aRef );   // allocRef, aRef
    }

// -----------------------------------------------------------------------------
// CWimMemMgmt::FreeRef
// Removes reference from the list and releases allocated memory.
// -----------------------------------------------------------------------------
//
void CWimMemMgmt::FreeRef( WIMI_Ref_pt aRef )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimMemMgmt::FreeRef | Begin"));
    TInt count = iRefs.Count();
    for( TInt index = 0; index < count; index++ )
        {
        TWimiAllocRef* allocRef = iRefs[ index ];
        if ( allocRef && ( allocRef->ref == aRef ) && ( allocRef->refType == EWimiRefpt ) )
            {
			_WIMTRACE2(_L("WIM | WIMServer | CWimMemMgmt::FreeRef, -ref 0x%08x"), aRef);
            iRefs.Remove( index );
            iRefs.Compress();
            free_WIMI_Ref_t( aRef );
            delete allocRef;
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CWimMemMgmt::FreeWIMRefs
// Frees the reference(s) pointed by a client.
// -----------------------------------------------------------------------------
//
void CWimMemMgmt::FreeWIMRefs( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimMemMgmt::FreeWIMRefsL | Begin"));
    TUint wimCount = aMessage.Int1();
    WIMI_Ref_pt ref = reinterpret_cast< WIMI_Ref_pt >( aMessage.Int0() );
    _WIMTRACE3(_L("WIM | WIMServer | CWimMemMgmt::FreeWIMRefsL | ref 0x%08x, wimCount %d"),
            ref, wimCount);

    for( TUint8 wimIndex = 0; wimIndex < wimCount; wimIndex++ )
        {
        _WIMTRACE2(_L("WIM | WIMServer | CWimMemMgmt::FreeWIMRefsL | index=%d"), wimIndex);
        FreeRef( ref );
        }

    _WIMTRACE(_L("WIM | WIMServer | CWimMemMgmt::FreeWIMRefsL | End"));
    aMessage.Complete( KErrNone );
    }

// -----------------------------------------------------------------------------
// CWimMemMgmt::FreeRefLst
// Frees the list of reference(s) pointed by a client.
// -----------------------------------------------------------------------------
//
void CWimMemMgmt::FreeRefLst( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimMemMgmt::FreeRefLst | Begin"));
    WIMI_Ref_pt* refList = ( WIMI_Ref_pt* )aMessage.Int0();
    TInt count = iRefs.Count();
    for( TUint8 index = 0; index < count; index++ )
        {
        TWimiAllocRef* allocRef = iRefs[ index ];
        if ( allocRef && ( allocRef->refLst == refList ) && ( allocRef->refType == EWimiRefListt ))
            {
            _WIMTRACE2(_L("WIM | WIMServer | CWimMemMgmt::FreeRefLst, -refList 0x%08x"), refList);
            iRefs.Remove( index );
            iRefs.Compress();
            free_WIMI_RefList_t( refList );
            delete allocRef;
            aMessage.Complete( KErrNone );
            return;
            }
        }
    _WIMTRACE(_L("WIM | WIMServer | CWimMemMgmt::FreeRefLst | not found") );
    aMessage.Complete( KErrNotFound );
    }

// -----------------------------------------------------------------------------
// CWimMemMgmt::ValidateWIMRefL
// -----------------------------------------------------------------------------
//
TBool CWimMemMgmt::ValidateWIMRefL( WIMI_Ref_pt aRef )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimMemMgmt::ValidateWIMRefL | Begin"));
    TInt count = iRefs.Count();
    for( TUint8 index = 0; index < count; index++ )
        {
        TWimiAllocRef* allocRef = iRefs[ index ];
        if( allocRef && ( allocRef->ref == aRef ) )
            {
            _WIMTRACE(_L("WIM | WIMServer | CWimMemMgmt::ValidateWIMRefL, valid"));
            return ETrue;
            }
        }
    _WIMTRACE(_L("WIM | WIMServer | CWimMemMgmt::ValidateWIMRefL, invalid"));
    return EFalse;
    }

//  End of File
