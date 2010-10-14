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
    RDEBUG("0", 0);
    Cancel();
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::NewL()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifier* CSecuiDialogNotifier::NewL()
    {
		RDEBUG("0", 0);
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
    RDEBUG("0", 0);
    delete this;
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::RegisterL()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifier::TNotifierInfo CSecuiDialogNotifier::RegisterL()
    {
		RDEBUG("0", 0);
    return Info();
    }

// ---------------------------------------------------------------------------
// CSecuiDialogNotifier::Info()
// ---------------------------------------------------------------------------
//
CSecuiDialogNotifier::TNotifierInfo CSecuiDialogNotifier::Info() const
    {
		RDEBUG("0", 0);

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
    RDEBUG("0", 0);

    TRAPD( err, DoStartL( aBuffer, aReplySlot, aMessage ) );
    RDEBUG("err", err);
    if( err )
        {
        if( iSecuiDialogs && !iIsSecuiDialogsDeleted )
            {
            RDEBUG("0", 0);
            delete iSecuiDialogs;
            iSecuiDialogs = NULL;
            }
        if( !aMessage.IsNull() )
            {
            RDEBUG("0", 0);
            aMessage.Complete( err );
            }
        }

    RDEBUG("0x99", 0x99);
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
    RDEBUG("0", 0);
    if( iSecuiDialogs && !iIsSecuiDialogsDeleted )
        {
        RDEBUG("0", 0);
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
    RDEBUG("0", 0);
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
		RDEBUG("0", 0);

    iSecuiDialogs = CSecuiDialogs::NewL( iIsSecuiDialogsDeleted );
    iSecuiDialogs->StartLD( aBuffer, aReplySlot, aMessage );
    }

