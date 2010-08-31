/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Security dialog notifier
*
*/

#include "securitydialognotifier.h"     // CSecurityDialogNotifier
#include "securitydialogs.h"            // CSecurityDialogs
#include "securitydialogstrace.h"       // TRACE macro


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSecurityDialogNotifier::~CSecurityDialogNotifier()
// ---------------------------------------------------------------------------
//
CSecurityDialogNotifier::~CSecurityDialogNotifier()
    {
    TRACE( "CSecurityDialogNotifier::~CSecurityDialogNotifier" );
    Cancel();
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifier::NewL()
// ---------------------------------------------------------------------------
//
CSecurityDialogNotifier* CSecurityDialogNotifier::NewL()
    {
    CSecurityDialogNotifier* self = new( ELeave ) CSecurityDialogNotifier;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifier::Release()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifier::Release()
    {
    TRACE( "CSecurityDialogNotifier::Release" );
    delete this;
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifier::RegisterL()
// ---------------------------------------------------------------------------
//
CSecurityDialogNotifier::TNotifierInfo CSecurityDialogNotifier::RegisterL()
    {
    return Info();
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifier::Info()
// ---------------------------------------------------------------------------
//
CSecurityDialogNotifier::TNotifierInfo CSecurityDialogNotifier::Info() const
    {
    TNotifierInfo info;
    info.iUid = KUidSecurityDialogNotifier;
    info.iChannel = KUidSecurityDialogNotifier;
    info.iPriority = ENotifierPriorityAbsolute;
    return info;
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifier::StartL()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifier::StartL( const TDesC8& aBuffer, TInt aReplySlot,
        const RMessagePtr2& aMessage )
    {
    TRACE( "CSecurityDialogNotifier::StartL, begin" );

    TRAPD( err, DoStartL( aBuffer, aReplySlot, aMessage ) );
    TRACE( "CSecurityDialogNotifier::StartL, DoStartL err=%d", err );
    if( err )
        {
        if( iSecurityDialogs && !iIsSecurityDialogsDeleted )
            {
            TRACE( "CSecurityDialogNotifier::StartL, deleting iSecurityDialogs" );
            delete iSecurityDialogs;
            iSecurityDialogs = NULL;
            }
        if( !aMessage.IsNull() )
            {
            TRACE( "CSecurityDialogNotifier::StartL, completing message" );
            aMessage.Complete( err );
            }
        }

    TRACE( "CSecurityDialogNotifier::StartL, end" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifier::StartL()
// ---------------------------------------------------------------------------
//
TPtrC8 CSecurityDialogNotifier::StartL( const TDesC8& /*aBuffer*/ )
    {
    return KNullDesC8();
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifier::Cancel()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifier::Cancel()
    {
    TRACE( "CSecurityDialogNotifier::Cancel" );
    if( iSecurityDialogs && !iIsSecurityDialogsDeleted )
        {
        TRACE( "CSecurityDialogNotifier::Cancel, deleting iSecurityDialogs" );
        delete iSecurityDialogs;
        iSecurityDialogs = NULL;
        }
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifier::UpdateL()
// ---------------------------------------------------------------------------
//
TPtrC8 CSecurityDialogNotifier::UpdateL( const TDesC8& /*aBuffer*/ )
    {
    return KNullDesC8();
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifier::CSecurityDialogNotifier()
// ---------------------------------------------------------------------------
//
CSecurityDialogNotifier::CSecurityDialogNotifier()
    {
    TRACE( "CSecurityDialogNotifier::CSecurityDialogNotifier" );
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifier::ConstructL()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifier::ConstructL()
    {
    }

// ---------------------------------------------------------------------------
// CSecurityDialogNotifier::DoStartL()
// ---------------------------------------------------------------------------
//
void CSecurityDialogNotifier::DoStartL( const TDesC8& aBuffer, TInt aReplySlot,
        const RMessagePtr2& aMessage )
    {
    iSecurityDialogs = CSecurityDialogs::NewL( iIsSecurityDialogsDeleted );
    iSecurityDialogs->StartLD( aBuffer, aReplySlot, aMessage );
    }

