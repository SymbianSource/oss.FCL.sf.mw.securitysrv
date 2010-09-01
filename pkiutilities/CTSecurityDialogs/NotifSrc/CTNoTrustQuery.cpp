/*
* Copyright (c) 2006-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  
*
*/



// INCLUDE FILES

#include "CTNoTrustQuery.h"
#include "CTSecurityDialogsAO.h"
#include "CTCertificateQuery.h"
#include <aknlists.h>
#include <CTSecDlgs.rsg>
#include <uikon/eiksrvui.h>
#include <StringLoader.h>


// ================= MEMBER FUNCTIONS ==========================================

// -----------------------------------------------------------------------------
// CSecNotNoTrustQuery::CCTNoTrustQuery()
// -----------------------------------------------------------------------------
//
CCTNoTrustQuery::CCTNoTrustQuery(
    CCTSecurityDialogsAO& aNotifier,
    TBool& aRetVal,
    TRequestStatus& aClientStatus,
    HBufC* aServerName,
    TBool aShowPermAccept,
    TBool& aIsDeleted ):
    CAknQueryDialog( CAknQueryDialog::ENoTone ),
    iNotifier( aNotifier ),
    iRetVal( aRetVal ),
    iClientStatus( &aClientStatus ),
    iServerName( aServerName ),
    iShowPermAccept( aShowPermAccept ),
    iDeleted( aIsDeleted )
    {
    iDeleted = EFalse;
    }

// -----------------------------------------------------------------------------
// CSecNotNoTrustQuery::~CCTNoTrustQuery()
// -----------------------------------------------------------------------------
//
CCTNoTrustQuery::~CCTNoTrustQuery()
    {
    // Allow application switching again
    CEikonEnv* eikonEnv = CEikonEnv::Static();
    if( eikonEnv )
        {
        CEikServAppUi* eikServAppUi = static_cast<CEikServAppUi*>( eikonEnv->EikAppUi() );
        if( eikServAppUi )
            {
            eikServAppUi->SuppressAppSwitching( EFalse );
            }
        }

    // Complete the client request
    if( iClientStatus && *iClientStatus == KRequestPending )
        {
        iRetVal = EFalse;
        User::RequestComplete( iClientStatus, KErrNone );
        }
    
    // Notify that the dialog has been deleted
    iDeleted = ETrue;
    }

// -----------------------------------------------------------------------------
// CSecNotNoTrustQuery::OkToExitL()
// -----------------------------------------------------------------------------
//
TBool CCTNoTrustQuery::OkToExitL( TInt aButtonId )
    {
    if ( aButtonId == EAknSoftkeyOptions || aButtonId == EAknSoftkeyOk )
        {
        return OptionsMenuL();
        }
    else
        {
        iRetVal = EFalse;
        User::RequestComplete( iClientStatus, KErrNone );
        }
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CCTNoTrustQuery::PostLayoutDynInitL()
// -----------------------------------------------------------------------------
//
void CCTNoTrustQuery::PostLayoutDynInitL()
    {
    CAknQueryControl* control = QueryControl();
    if (control)
        control->StartAnimationL();
    ((CEikServAppUi*)(CEikonEnv::Static())->EikAppUi())->SuppressAppSwitching( ETrue );
    }

// -----------------------------------------------------------------------------
// CCTNoTrustQuery::OptionsMenuL()
// -----------------------------------------------------------------------------
TBool CCTNoTrustQuery::OptionsMenuL()
    {
    TBool ret = EFalse;
    CAknSinglePopupMenuStyleListBox* list =
        new( ELeave ) CAknSinglePopupMenuStyleListBox;
    CleanupStack::PushL( list );
    CAknPopupList* popupList = CAknPopupList::NewL(
        list, R_AVKON_SOFTKEYS_SELECT_CANCEL__SELECT, AknPopupLayouts::EMenuWindow );
    CleanupStack::PushL( popupList );
    list->ConstructL( popupList, CEikListBox::ELeftDownInViewRect );
    list->CreateScrollBarFrameL( ETrue );
    list->ScrollBarFrame()->SetScrollBarVisibilityL(
        CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );

    TInt resourceid;

    if ( iShowPermAccept )
        {
        resourceid = R_NOTRUST_MENUPANE;
        }
    else
        {
        resourceid = R_NOTRUST_MENUPANE_NO_PERM;
        }

     CDesCArrayFlat* items =
            CEikonEnv::Static()->ReadDesCArrayResourceL( resourceid );

    CTextListBoxModel* model = list->Model();
    model->SetOwnershipType( ELbmOwnsItemArray );
    model->SetItemTextArray( items );

    CleanupStack::Pop( popupList ); //popupList
    if ( popupList->ExecuteLD() )
        {
        if( !iDeleted )
            {
            TInt index = list->CurrentItemIndex();
            if ( index == 0 )       // Accept now
                {
                iRetVal = EServerCertAcceptedTemporarily;
                User::RequestComplete(iClientStatus, KErrNone);
                ret = ETrue;
                }
            else if (( index == 1 ) && ( resourceid == R_NOTRUST_MENUPANE ))  // Accept permanently
                {
    
                HBufC* prompt = StringLoader::LoadLC( R_QTN_HTTPSEC_QUERY_PERM_ACCEPT_TEXT, *iServerName );
    
                CAknMessageQueryDialog* note = CAknMessageQueryDialog::NewL( *prompt );
    
                note->PrepareLC( R_HTTPSEC_QUERY_PERM_ACCEPT );
                note->SetPromptL( *prompt );
    
                if ( note->RunLD() )
                    {
                    iRetVal = EServerCertAcceptedPermanently;
                    User::RequestComplete( iClientStatus, KErrNone );
                    ret = ETrue;
                    }
                else
                    {
                    ret = EFalse;
                    }
    
                CleanupStack::PopAndDestroy( prompt );
                }
            else if ((( index == 2 ) && ( resourceid == R_NOTRUST_MENUPANE ))  ||
                     (( index == 1 ) && ( resourceid == R_NOTRUST_MENUPANE_NO_PERM )) ) // Details
                {
                // We need to delay this implementation in other releases, because
                // separate ICD was taken to 3.0.
                CCTCertificateQuery* query =
                CCTCertificateQuery::NewL( iNotifier );
                query->ExecuteLD( R_NOTRUST_CERTIFICATE_QUERY );
                ret = EFalse;
                }
            else
                {
                iRetVal = EServerCertNotAccepted;
                User::RequestComplete( iClientStatus, KErrNone );
                ret = ETrue;
                }
            }
        else
            {
            ret = EFalse;
            }
        }
    else
        {
        ret = EFalse;
        }
    CleanupStack::PopAndDestroy( list );  // list
    return ret;
    }

