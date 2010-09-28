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
* Description:  Secui dialog notifier
*
*/

#include "secuidialognotifier.h"     // CSecuiDialogNotifier
#include "secuidialogs.h"            // CSecuiDialogs
#include "secuidialogstrace.h"       // TRACE macro


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::~CSecuiDialogNotifier()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifier::~CSecuiDialogNotifier()
    {
    TRACE( "CSecuiDialogNotifier::~CSecuiDialogNotifier" );
    Cancel();
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::NewL()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifier* CSecuiDialogNotifier::NewL()
    {
		TRACE( "CSecuiDialogNotifier::NewL, 1 =%d", 1 );
    CSecuiDialogNotifier* self = new( ELeave ) CSecuiDialogNotifier;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::Release()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifier::Release()
    {
    TRACE( "CSecuiDialogNotifier::Release" );
    delete this;
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::RegisterL()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifier::TNotifierInfo CSecuiDialogNotifier::RegisterL()
    {
		TRACE( "CSecuiDialogNotifier::RegisterL, 1 =%d", 1 );
    return Info();
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::Info()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifier::TNotifierInfo CSecuiDialogNotifier::Info() const
    {
		TRACE( "CSecuiDialogNotifier::Info, 1 =%d", 1 );

    TNotifierInfo info;
    static const TUid KUidSecuiDialogNotifier = { 0x10005988 };
    info.iUid = KUidSecuiDialogNotifier;
    const TUid KSecAuthChannel = {0x00000602};
    info.iChannel = KSecAuthChannel;
    info.iPriority = ENotifierPriorityAbsolute;
    return info;
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::StartL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifier::StartL( const TDesC8& aBuffer, TInt aReplySlot,
        const RMessagePtr2& aMessage )
    {
    TRACE( "CSecuiDialogNotifier::StartL, begin" );

    TRAPD( err, DoStartL( aBuffer, aReplySlot, aMessage ) );
    TRACE( "CSecuiDialogNotifier::StartL, DoStartL err=%d", err );
    if( err )
        {
        if( iSecuiDialogs && !iIsSecuiDialogsDeleted )
            {
            TRACE( "CSecuiDialogNotifier::StartL, deleting iSecuiDialogs" );
            delete iSecuiDialogs;
            iSecuiDialogs = NULL;
            }
        if( !aMessage.IsNull() )
            {
            TRACE( "CSecuiDialogNotifier::StartL, completing message" );
            aMessage.Complete( err );
            }
        }

    TRACE( "CSecuiDialogNotifier::StartL, end" );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::StartL()
// ---------------------------------------------------------------------------
//
TPtrC8 CSecuiDialogNotifier::StartL( const TDesC8& /*aBuffer*/ )
    {
    return KNullDesC8();
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::Cancel()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifier::Cancel()
    {
    TRACE( "CSecuiDialogNotifier::Cancel" );
    if( iSecuiDialogs && !iIsSecuiDialogsDeleted )
        {
        TRACE( "CSecuiDialogNotifier::Cancel, deleting iSecuiDialogs" );
        delete iSecuiDialogs;
        iSecuiDialogs = NULL;
        }
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::UpdateL()
// ---------------------------------------------------------------------------
//
TPtrC8 CSecuiDialogNotifier::UpdateL( const TDesC8& /*aBuffer*/ )
    {
    return KNullDesC8();
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::CSecuiDialogNotifier()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifier::CSecuiDialogNotifier()
    {
    TRACE( "CSecuiDialogNotifier::CSecuiDialogNotifier" );
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::ConstructL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifier::ConstructL()
    {
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::DoStartL()
// ---------------------------------------------------------------------------
//
void CSecuiDialogNotifier::DoStartL( const TDesC8& aBuffer, TInt aReplySlot,
        const RMessagePtr2& aMessage )
    {
		TRACE( "CSecuiDialogNotifier::DoStartL, 1 =%d", 1 );

    iSecuiDialogs = CSecuiDialogs::NewL( iIsSecuiDialogsDeleted );
    iSecuiDialogs->StartLD( aBuffer, aReplySlot, aMessage );
    }

