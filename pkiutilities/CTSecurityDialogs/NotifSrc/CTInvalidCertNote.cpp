/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Info note shown for invalid and revoked certificates
*
*/



#include "CTInvalidCertNote.h"
#include "CTCertificateQuery.h"     // CCTCertificateQuery
#include <uikon/eiksrvui.h>         // CEikServAppUi
#include <CTSecDlgs.rsg>


// ======== MEMBER FUNCTIONS ========


// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
//
CCTInvalidCertificateNote::CCTInvalidCertificateNote( 
    CCTSecurityDialogsAO& aNotifier,
    TRequestStatus& aClientStatus ) :
    iNotifier( aNotifier ),
    iClientStatus( &aClientStatus )
    {
    }


// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
//
CCTInvalidCertificateNote::~CCTInvalidCertificateNote()
    {
    static_cast<CEikServAppUi*>(iEikonEnv->EikAppUi())->SuppressAppSwitching( EFalse );
    }


// ---------------------------------------------------------------------------
// CCTInvalidCertificateNote::OkToExitL()
// ---------------------------------------------------------------------------
//
TBool CCTInvalidCertificateNote::OkToExitL( TInt aButtonId )
    {
    if( aButtonId == EAknSoftkeyShow )
        {
        ShowDetailsL();
        return EFalse;
        }
    else
        {
        User::RequestComplete( iClientStatus, KErrNone );
        }
    return ETrue;
    }


// ---------------------------------------------------------------------------
// CCTInvalidCertificateNote::PostLayoutDynInitL()
// ---------------------------------------------------------------------------
//
void CCTInvalidCertificateNote::PostLayoutDynInitL()
    {
    CAknNoteDialog::PostLayoutDynInitL();
    static_cast<CEikServAppUi*>(iEikonEnv->EikAppUi())->SuppressAppSwitching(ETrue);
    }


// ---------------------------------------------------------------------------
// CCTInvalidCertificateNote::ShowDetailsL()
// ---------------------------------------------------------------------------
//
void CCTInvalidCertificateNote::ShowDetailsL()
    {
    CCTCertificateQuery* query = CCTCertificateQuery::NewL( iNotifier );
    query->ExecuteLD( R_NOTRUST_CERTIFICATE_QUERY );
    }


// end of file
