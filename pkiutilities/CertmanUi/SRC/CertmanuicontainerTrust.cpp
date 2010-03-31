/*
* Copyright (c) 2003-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Implementation of class CCertManUIContainerTrust.
*                Maintains correct list of trusted clients depending
*                on the certificate that was in focus in Authority
*                certificates view when Trust Settings view was entered.
*                Shows and changes correct Yes/No text for the Trusted client.
*
*/


// INCLUDE FILES
#include <certificateapps.h>
#include <mctcertapps.h>
#include <mctcertstore.h>
#include <csxhelp/cm.hlp.hrh>   // for help context of CertManUI
#include <certmanui.rsg>

#include "CertmanuiKeeper.h"
#include "CertmanuiviewTrust.h"
#include "CertmanuicontainerTrust.h"
#include "Certmanuitrustids.h"
#include "CertmanuiSyncWrapper.h"
#include "CertManUILogger.h"
#include "CertmanuiCommon.h"
#include "CertManUILogger.h"

#include "Certmanuidll.h"     // for applicationUID


const TInt KMidpCertStoreUid = 0x101F5B71;

// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// void CloseAndDeleteRArray( TAny* aPointer )
// Function frees resources of a RArray.
// Returns: void
// -----------------------------------------------------------------------------
//
void CloseAndDeleteRArray( TAny* aPointer )
    {
    ((RArray<TUid>*)aPointer)->Close();
    delete ( (RArray<TUid>*)aPointer );
    }

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCertManUIContainerTrust::CCertManUIContainerTrust(
//                              CCertManUIViewTrust& aParent,
//                              CCertManUIKeeper& aKeeper)
// Constructor with parent
// ---------------------------------------------------------
//
CCertManUIContainerTrust::CCertManUIContainerTrust(
    CCertManUIViewTrust& aParent,
    CCertManUIKeeper& aKeeper )
    : iParent ( aParent ), iKeeper ( aKeeper )
    {
    CERTMANUILOGGER_WRITE_TIMESTAMP(
        "CCertManUIContainerTrust::CCertManUIContainerTrust" );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::~CCertManUIContainerTrust()
// Destructor
// ---------------------------------------------------------
//
CCertManUIContainerTrust::~CCertManUIContainerTrust()
    {
    CERTMANUILOGGER_ENTERFN(
        " CCertManUIContainerTrust::~CCertManUIContainerTrust" );

    delete iListBox;
    delete iTrustedClients;
    delete iTrustValues;
    delete iAppInfoManager;
    iClientUids.Close();

    CERTMANUILOGGER_LEAVEFN(
        " CCertManUIContainerTrust::~CCertManUIContainerTrust" );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::ConstructL(const TRect& aRect)
// EPOC two phased constructor
// ---------------------------------------------------------
//
void CCertManUIContainerTrust::ConstructL(const TRect& aRect)
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrust::ConstructL" );

    CreateWindowL();
    CreateListBoxL();
    SetRect(aRect);
    iTrustedClients = iCoeEnv->ReadDesC16ArrayResourceL(
          R_CERTMANUI_VIEW_TRUST_SETTINGS_LISTBOX );
    iTrustValues = iCoeEnv->ReadDesC16ArrayResourceL(
          R_CERTMANUI_TRUST_SETTINGS_VALUES );

    TRAPD( error, iAppInfoManager =
        CCertificateAppInfoManager::NewL( iKeeper.iRfs, EFalse ) );
    if ( error != KErrNone )
        {
        if ( error == KErrNotSupported
            || error == KErrCorrupt
            || error == KErrNotReady
            || error == KErrArgument
            || error == KErrNotFound )
            {
            iKeeper.ShowErrorNoteL( error );
            User::Exit( KErrNone );
            }
        else
            {
            User::Leave( error );
            }
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrust::ConstructL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::SizeChanged()
// Called by framework when the view size is changed
// ---------------------------------------------------------
//
void CCertManUIContainerTrust::SizeChanged()
    {
    iListBox->SetRect( Rect() );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::FocusChanged(TDrawNow aDrawNow)
// ---------------------------------------------------------
//
void CCertManUIContainerTrust::FocusChanged( TDrawNow aDrawNow )
    {
    if ( iListBox )
        {
        iListBox->SetFocus( IsFocused(), aDrawNow );
        }
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::HandleListBoxEventL(
//      CEikListBox* /*aListBox*/, TListBoxEvent /*aEventType*/)
// Cannot be changed to non-leaving function.
// L-function is required by the class definition, even if empty.
// ---------------------------------------------------------
//
void CCertManUIContainerTrust::HandleListBoxEventL(
    CEikListBox* /*aListBox*/, TListBoxEvent aEventType )
    {
    CERTMANUILOGGER_ENTERFN(
      " CCertManUIContainerTrust::HandleListBoxEventL" );

    switch( aEventType )
        {
        case EEventItemSingleClicked:
            ChangeTrustChangeSettingSingleClickL();
            break;
        default:
            {
            break;
            }
        }

    CERTMANUILOGGER_LEAVEFN(
      " CCertManUIContainerTrust::HandleListBoxEventL" );
    }


// ---------------------------------------------------------
// CCertManUIContainerTrust::CountComponentControls() const
// ---------------------------------------------------------
//
TInt CCertManUIContainerTrust::CountComponentControls() const
    {
    // return number of controls inside this container
    return 1;
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::ComponentControl(TInt /*aIndex*/) const
// ---------------------------------------------------------
//
CCoeControl* CCertManUIContainerTrust::ComponentControl(
    TInt /*aIndex*/ ) const
    {
    return iListBox;
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::Draw(const TRect& aRect) const
// ---------------------------------------------------------
//
void CCertManUIContainerTrust::Draw( const TRect& aRect ) const
    {
    CWindowGc& gc = SystemGc();
    gc.SetPenStyle( CGraphicsContext::ENullPen );
    gc.SetBrushColor( KRgbGray );
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    gc.DrawRect( aRect );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::HandleControlEventL(
//    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
// Cannot be changed to non-leaving function.
// L-function is required by the class definition, even if empty.
// ---------------------------------------------------------
//
void CCertManUIContainerTrust::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/ )
    {
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::OfferKeyEventL(const TKeyEvent& aKeyEvent,
//      TEventCode aType)
// Handles the key events OK button, arrow keys.
// ---------------------------------------------------------
//
TKeyResponse CCertManUIContainerTrust::OfferKeyEventL(
    const TKeyEvent& aKeyEvent,
    TEventCode aType )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrust::OfferKeyEventL" );

    TBool doRet = EFalse;

    switch ( aKeyEvent.iCode )
        {
        case EKeyOK:
            {
            if ( !iKeeper.iWrapper->IsActive() )
                {
                iParent.ChangeTrustL();
                }
            break;
            }
        case EKeyLeftArrow:
        case EKeyRightArrow:
            {
            // Listbox takes all event even if it doesn't use them
            doRet = ETrue;
            break;
            }
        default:
            {
            break;
            }
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrust::OfferKeyEventL" );

    if ( doRet )
        {
        return EKeyWasConsumed;
        }

    return iListBox->OfferKeyEventL( aKeyEvent, aType );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::CreateListBoxL()
// Creates listbox
// ---------------------------------------------------------
//
void CCertManUIContainerTrust::CreateListBoxL()
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrust::CreateListBoxL" );

    iListBox = new ( ELeave ) CAknSettingStyleListBox;
    iListBox->SetContainerWindowL( *this );
    iListBox->ConstructL( this, EAknListBoxSelectionList );
    iListBox->CreateScrollBarFrameL( ETrue );
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(
          CEikScrollBarFrame::EOff,CEikScrollBarFrame::EAuto );
    iListBox->SetListBoxObserver( this );

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrust::CreateListBoxL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::DrawListBoxL(
//      TInt aCurrentPosition, TInt aTopItem, TInt aCertificateIndex)
// Draws listbox
// ---------------------------------------------------------
//
void CCertManUIContainerTrust::DrawListBoxL(
    TInt aCurrentPosition, TInt aTopItem, TInt aCertificateIndex)
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrust::DrawListBoxL" );

    iListBox->SetTopItemIndex( aTopItem );
    iListBox->SetCurrentItemIndex( aCurrentPosition );
    UpdateListBoxL( aCertificateIndex );
    ActivateL();
    DrawNow();

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrust::DrawListBoxL" );
    }

// ---------------------------------------------------------------------------
// CCertManUIContainerTrust::UpdateListBoxL(TInt aCertificateIndex)
// Puts correct Trust Settings text to listbox and correct value (yes/no)
// to the setting, asks the value from CheckCertificateClientTrust
// For X509 certificates, only one Application is currently supported
// (Application controller), more applications maybe added later however,
// so one additional application has been left in comments as an example.
// WAP connection and Application installer have their names from resources
// and thus localized,  other additional application get their names from
// TCertificateAppInfo's Name() function.
// ---------------------------------------------------------------------------
//
void CCertManUIContainerTrust::UpdateListBoxL( TInt aCertificateIndex )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrust::UpdateListBoxL" );

    SetEmptyListL( iListBox );

    iApps.Reset();
    iClientUids.Reset();

    TInt clientCount = 0;

    // Use certificate index from previous view
    CCTCertInfo* entry = iKeeper.iCALabelEntries[ aCertificateIndex ]->iCAEntry;

    // Here MCTCertApps should be used to get client options, but it is not properly
    // supported in any cert store. It should be possible to get the MCTCertApps
    // through token's GetInterface function.
    if ( entry->Token().TokenType().Type() == TUid::Uid( KTokenTypeFileCertstore ) )
        {
        if ( !iAppInfoManager )
            {
            User::Leave( KErrArgument );
            }
        iApps = iAppInfoManager->Applications();
        for ( TInt i = 0; i < iApps.Count(); i++ )
            {
            if ( iApps[i].Id() != KCertManUIViewTrustJavaInstallingId &&
                 iApps[i].Id() != KCertManUIViewTrustApplicationControllerId )
                {
                iClientUids.Append( iApps[i].Id() );
                }
            }
        }
    else
        {
        iKeeper.iWrapper->GetApplicationsL(
                iKeeper.CertManager(), *entry, iClientUids );
        }

    // This is needed because MIDPCertStore does not support MCTCertApps
    if ( iClientUids.Count() == 0 &&
        entry->Token().TokenType().Type() == TUid::Uid( KMidpCertStoreUid ) )
        {
        iClientUids.Append( KCertManUIViewTrustJavaInstallingId );
        }

    clientCount = iClientUids.Count();


    MDesCArray* itemList = iListBox->Model()->ItemTextArray();
    CDesCArray* itemArray = STATIC_CAST( CDesCArray*, itemList );
    if ( EX509Certificate == entry->CertificateFormat() )
        {
        TInt resIndex = KErrNotFound;
        TInt position = 0;
        if ( iClientUids.Count() > 0 )
            {
            for ( TInt k = 0; k < clientCount; k++ )
                {
                resIndex = GetTrusterResId( iClientUids[k] );
                if ( resIndex != KErrNotFound )
                    {
                    InsertItemToTrustListL(
                        resIndex, iClientUids[k], *entry, itemArray, position );
                    position++;
                    }
                }
            }
        }
    else
        {
        return;
        }

    iListBox->HandleItemAdditionL();
    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrust::UpdateListBoxL" );
    }

// ---------------------------------------------------------------------------
// CCertManUIContainerTrust::GetTrusterResId( TUid aTrusterUid )
// ---------------------------------------------------------------------------
//
TInt CCertManUIContainerTrust::GetTrusterResId( TUid aTrusterUid )
    {
    TInt resIndex = 0;
    if ( aTrusterUid == KCertManUIViewTrustApplicationControllerId )
        {
        resIndex = KTrustSettingsResourceIndexAppCtrl;
        }
    else if ( aTrusterUid == KCertManUIViewTrustMailAndImageConnId )
        {
        resIndex = KTrustSettingsResourceIndexMailAndImageConn;
        }
    else if ( aTrusterUid == KCertManUIViewTrustJavaInstallingId )
        {
        resIndex = KTrustSettingsResourceIndexJavaInstall;
        }
    else if ( aTrusterUid == KCertManUIViewOCSPCheckInstallingId )
        {
        resIndex = KTrustSettingsResourceIndexOCSPCheck;
        }
    else if ( aTrusterUid == KCertManUIViewTrustVPNId )
        {
        resIndex = KTrustSettingsResourceIndexVPN;
        }
    else if ( aTrusterUid == KCertManUIViewTrustWidgetInstallingId )
        {
        resIndex = KTrustSettingsResourceIndexWidget;
        }
    else
        {
        resIndex = KErrNotFound;
        }

    return resIndex;
    }


// ---------------------------------------------------------------------------
// CCertManUIContainerTrust::InsertItemToTrustListL(const TInt aIndex,
//                                              const TUid aId,
//                                              CCTCertInfo& aEntry,
//                                              CDesCArray* aItemArray,
//                                              TInt aPosition)
// This is helper function for UpdateListBoxL. Inserts trustsetting item to
// listbox.
// ---------------------------------------------------------------------------
//
void CCertManUIContainerTrust::InsertItemToTrustListL(
    const TInt aIndex, const TUid aId,
    CCTCertInfo& aEntry, CDesCArray* aItemArray,
    TInt aPosition )
    {
    TBuf<KMaxLengthTextSettListOneTwo> item;
    TBool trustState = EFalse;

    item = (*iTrustedClients)[ aIndex ];
    trustState = CheckCertificateClientTrustL( aId, aEntry );
    if ( trustState )
        {
        // item gets its beginning and end
        // from the resources using indexes
        item += (*iTrustValues)
                  [ KTrustSettingsResourceIndexValueYes ];
        }
    else
        {
        item += (*iTrustValues)
                  [ KTrustSettingsResourceIndexValueNo ];
        }
    aItemArray->InsertL( aPosition, item );
    }

// ---------------------------------------------------------------------------
// CCertManUIContainerTrust::CheckCertificateClientTrustL(const TUid aClientTUid,
// CCTCertInfo& aEntry) const
// Checks if a certificate trusts a client
// * Certificate format == ECrX509Certificate ECrX968Certificate ECrCertificateURL
// are not supported if certificate location == ECrCertLocationWIMCard
// * Certificate format == ECrX968Certificate ECrCertificateURL are not supported
// if certificate location == ECrCertLocationCertMan
// * Certificate location == ECrCertLocationWIMURL ECrCertLocationPhoneMemory
// ECrCertLocationPhoneMemoryURL are not supported
// ---------------------------------------------------------------------------
//
TBool CCertManUIContainerTrust::CheckCertificateClientTrustL(
    const TUid aClientTUid, CCTCertInfo& aEntry ) const
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrust::CheckCertificateClientTrust" );

    TBool trustSettingTrusted = EFalse;
    TCertificateFormat format = aEntry.CertificateFormat();

    if ( format == EX509Certificate )
        {

        RArray<TUid> trusterUids;
        CleanupClosePushL( trusterUids );

        //Owned by iKeeper.
        CUnifiedCertStore*& store = iKeeper.CertManager();
        // Get trusting applications' Uids for the one selected
        // certificate entry
        iKeeper.iWrapper->GetApplicationsL( store, aEntry, trusterUids );


        // get the number of trusting applications for one
        // selected certificate entry
        TInt trusterCount = trusterUids.Count();
        // in this loop, for every trusting application in one
        // selected certificate entry
        for ( TInt i = 0; i < trusterCount && !trustSettingTrusted; i++ )
            {
            // put the trusting application's Uid to TUid truster
            if ( aClientTUid == trusterUids[i] )
                {
                trustSettingTrusted = ETrue;
                }
            }
        CleanupStack::PopAndDestroy();  // close trusterUids
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrust::CheckCertificateClientTrust" );

    return trustSettingTrusted;
    }

// ---------------------------------------------------------------------------
// CCertManUIContainerTrust::SetEmptyListL(CEikListBox* aListBox)
// Sets empty list for listbox.
// ---------------------------------------------------------------------------
//
void CCertManUIContainerTrust::SetEmptyListL( CEikListBox* aListBox )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrust::SetEmptyListL" );

    // Sets empty list.
    HBufC* emptyListText = iCoeEnv->AllocReadResourceLC(
        R_TEXT_RESOURCE_VIEW_TRUST_SETTINGS_EMPTY_LIST );
    aListBox->View()->SetListEmptyTextL( *emptyListText );
    CleanupStack::PopAndDestroy();  // emptyListText

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrust::SetEmptyListL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::UpdateTrustListboxItemL(CCrCertEntry& aEntry,
//                                              TInt aCurrentTruster)
// Updates the Trust Settings Listbox item value
// Can be called only for those certificates that are
// located in ECrCertLocationCertMan !!!
// Cannot be called for certificates located in ECrCertLocationWIMCard,
//                        ECrCertLocationWIMURL,
//                        ECrCertLocationPhoneMemory
//                        ECrCertLocationPhoneMemoryURL
// Certificate formats ECrX968Certificate ECrCertificateURL
// are not visible in the Trust View
// ---------------------------------------------------------
//
void CCertManUIContainerTrust::UpdateTrustListboxItemL(CCTCertInfo& aEntry,
                                                       TInt aCurrentTruster)
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrust::UpdateTrustListboxItemL" );

    MDesCArray* itemList = iListBox->Model()->ItemTextArray();
    CDesCArray* itemArray = STATIC_CAST( CDesCArray*, itemList );
    TCertificateFormat format = aEntry.CertificateFormat();
    TUid id = KCertManUINullId;

    TInt poppableItems = 0;

    TInt index = 0;

    if ( format == EX509Certificate )
        {
        id = iClientUids[ aCurrentTruster ];
        index = aCurrentTruster;
        }

    if ( id != KCertManUINullId )
        {
        TBuf<KMaxLengthTextSettListOneTwo> item;

        if ( id == KCertManUIViewTrustApplicationControllerId )
            {
            item = (*iTrustedClients)[ KTrustSettingsResourceIndexAppCtrl ];
            }
        else if ( id == KCertManUIViewTrustMailAndImageConnId )
            {
            item =
             (*iTrustedClients)[ KTrustSettingsResourceIndexMailAndImageConn ];
            }
        else if ( id == KCertManUIViewTrustJavaInstallingId )
            {
            item =
             (*iTrustedClients)[ KTrustSettingsResourceIndexJavaInstall ];
            }
        else if ( id == KCertManUIViewOCSPCheckInstallingId )
            {
            item =
             (*iTrustedClients)[ KTrustSettingsResourceIndexOCSPCheck ];
            }
        else if ( id == KCertManUIViewTrustVPNId )
            {
            item =
             (*iTrustedClients)[ KTrustSettingsResourceIndexVPN ];
            }
        else
            {
            if ( iApps.Count() > 0 )
                {
                const TName& name = iApps[index].Name();
                TInt fieldLength = name.Length();
                TBuf<KMaxLengthTextSettListOneTwo> item2;
                if ( fieldLength >= KMaxLengthTextSettListOne )
                    {
                    item2 = name.Mid( 0, KMaxLengthTextSettListOne );
                    }
                else
                    {
                    item2 = name;
                    }
                item.Format( KCertManUIFormatDescriptorTrustSettings, &item2 );
                }
            }

        if ( item.Length() )
            {
            if ( CheckCertificateClientTrustL(id, aEntry) )
                {
                item += (*iTrustValues)[ KTrustSettingsResourceIndexValueYes ];
                }
            else
                {
                item += (*iTrustValues)[ KTrustSettingsResourceIndexValueNo ];
                }
            // need to Delete first the old Yes/No entry
            itemArray->Delete( aCurrentTruster );
            itemArray->InsertL( aCurrentTruster, item );
            }
        }

    CleanupStack::PopAndDestroy( poppableItems );

    iListBox->HandleItemAdditionL();

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrust::UpdateTrustListboxItemL" );
    }

// ---------------------------------------------------------------------------
// CCertManUIContainerTrust::ShowTrustChangeSettingPageL(const TUid aClientUid,
//                            TDesC& aClientName,
//                            CCTCertInfo& aEntry)
// Puts correct client names and yes/no state to settings page
// Sets title text to a setting page
// ---------------------------------------------------------------------------
//
void CCertManUIContainerTrust::ShowTrustChangeSettingPageL(
    const TUid aClientUid, TDesC& aClientName, CCTCertInfo& aEntry )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrust::ShowTrustChangeSettingPageL" );

    TBool trustState = CheckCertificateClientTrustL( aClientUid, aEntry );
    TInt oldTrustStatus = 1; // Trust set to No value
    if ( trustState )
        {
        oldTrustStatus = 0; // Trust set to Yes value
        }
    TInt newTrustStatus = oldTrustStatus;
    CDesCArrayFlat* items = iCoeEnv->ReadDesC16ArrayResourceL(
            R_CERTMANUI_VIEW_TRUST_YES_NO_SETTING_PAGE_LBX );
    CleanupStack::PushL( items );

    CAknRadioButtonSettingPage* dlg = new(ELeave) CAknRadioButtonSettingPage(
            R_CERTMANUI_VIEW_TRUST_YES_NO_SETTING_PAGE, newTrustStatus, items );

    dlg->SetSettingTextL( aClientName );
    dlg->ExecuteLD( CAknSettingPage::EUpdateWhenChanged );
    CleanupStack::PopAndDestroy( items );

    if ( newTrustStatus != oldTrustStatus )
        {
        TInt currentTruster = iListBox->CurrentItemIndex();
        ChangeTrustValueL( aEntry, aClientUid );
        UpdateTrustListboxItemL( aEntry, currentTruster );
        }

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrust::ShowTrustChangeSettingPageL" );
    }

// ---------------------------------------------------------------------------
// CCertManUIContainerTrust::ChangeTrustChangeSettingSingleClickL()
// ---------------------------------------------------------------------------
//
void CCertManUIContainerTrust::ChangeTrustChangeSettingSingleClickL()
    {
    if( !iKeeper.iWrapper->IsActive() )
        {
        TInt certIndex = iKeeper.iCurrentCACertForTrustSettings;

        if( certIndex >= 0 && certIndex < iKeeper.iCALabelEntries.Count() )
            {
            CCTCertInfo* entry = iKeeper.iCALabelEntries[ certIndex ]->iCAEntry;
            if( entry && entry->IsDeletable() )
                {
                TInt currentTrustSetting = iListBox->CurrentItemIndex();
                if( currentTrustSetting >= 0 && currentTrustSetting < iClientUids.Count() )
                    {
                    TUid uid = iClientUids[ currentTrustSetting ];
                    ChangeTrustValueL( *entry, uid );
                    UpdateTrustListboxItemL( *entry, currentTrustSetting );
                    }
                }
            }
        }
    }

// ---------------------------------------------------------------------------
// CCertManUIContainerTrust::PopupTrustChangeSettingPageL(
//      TInt aCertificateIndex)
// Finds out which client was focused in Trust Settings view and calls
// ShowTrustChangeSettingPageL This is accessed only by X509S
// certificates, located in CertMan and having one or more clients
// Must be public so that viewTrust can access
// ---------------------------------------------------------------------------
//
void CCertManUIContainerTrust::PopupTrustChangeSettingPageL(
    TInt aCertificateIndex )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrust::PopupTrustChangeSettingPageL" );

    // Use certificate index from previous view
    CCTCertInfo* entry =
        iKeeper.iCALabelEntries[ aCertificateIndex ]->iCAEntry;

    // if the certificate is read-only, trust settings cannot be changed
    if( !entry->IsDeletable() )
    	{
        CERTMANUILOGGER_LEAVEFN( "- PopupTrustChangeSettingPageL - read-only" );
    	return;
    	}

    TUid id = KCertManUINullId;
    TInt poppableItems = 0;

    TBuf<KMaxLengthTextSettListOne> clientName;

    // get position of the focus in the Trust Settings view
    TInt currentPosition = iListBox->CurrentItemIndex();
    id = iClientUids[ currentPosition ];

    if ( id != KCertManUINullId )
        {
        // Native installation
        if ( id == KCertManUIViewTrustApplicationControllerId )
            {
            clientName =
                    (*iTrustedClients)[ KTrustSettingsResourceIndexAppCtrl ];
            }
        // TLS
        else if ( id == KCertManUIViewTrustMailAndImageConnId )
            {
            clientName =
                  (*iTrustedClients)[
                  KTrustSettingsResourceIndexMailAndImageConn ];
            }
        // Midlet installation
        else if ( id == KCertManUIViewTrustJavaInstallingId )
            {
            clientName =
                  (*iTrustedClients)[
                  KTrustSettingsResourceIndexJavaInstall ];
            }
        // OCSP
        else if ( id == KCertManUIViewOCSPCheckInstallingId )
            {
            clientName =
                  (*iTrustedClients)[
                  KTrustSettingsResourceIndexOCSPCheck ];
            }
        // VPN
        else if ( id == KCertManUIViewTrustVPNId )
            {
            clientName =
                  (*iTrustedClients)[
                  KTrustSettingsResourceIndexVPN ];
            }
        else
            {
            if ( iApps.Count() > 0 )
                {
                const TName& name = iApps[currentPosition].Name();
                TInt fieldLength = name.Length();
                if ( fieldLength >= KMaxLengthTextSettListOne )
                    {
                    clientName = name.Mid(0, KMaxLengthTextSettListOne);
                    }
                else
                    {
                    clientName = name;
                    }
                }
            }
        }

    if ( id != KCertManUINullId )
        {
        clientName.TrimLeft();
        clientName.TrimRight();
        ShowTrustChangeSettingPageL( id, clientName, *entry );
        }

    CleanupStack::PopAndDestroy( poppableItems );  // clients

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrust::PopupTrustChangeSettingPageL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::ChangeTrustValueL(CCTCertInfo& aEntry)
// Changes the Trust state of client in the CertManAPI
// ChangeTrustValueL is called only for certificates located
// in certman.
// ---------------------------------------------------------
//
void CCertManUIContainerTrust::ChangeTrustValueL(
    CCTCertInfo& aEntry, const TUid aClientUid )
    {
    CERTMANUILOGGER_ENTERFN( " CCertManUIContainerTrust::ChangeTrustValueL" );

    RArray<TUid> trusterUids;
    CleanupClosePushL( trusterUids );

    iKeeper.iWrapper->GetApplicationsL(
        iKeeper.CertManager(), aEntry, trusterUids );

    RArray<TUid> newUids;
    CleanupClosePushL( newUids );

    // Go through all applications and if aClientId does not exist, add it and
    // update certificates data.
    TBool add = ETrue;
    for ( TInt i = 0; i < trusterUids.Count(); i++ )
        {
        if ( aClientUid == trusterUids[i] )
            {
            add = EFalse;
            }
        else
            {
            newUids.Append( trusterUids[i] );
            }
        }

    if ( add )
        {
        newUids.Append( aClientUid );
        }

    iKeeper.iWrapper->SetApplicabilityL(
        iKeeper.CertManager(), aEntry, newUids );

    CleanupStack::PopAndDestroy(2);  // trusterUids, newUids

    CERTMANUILOGGER_LEAVEFN( " CCertManUIContainerTrust::ChangeTrustValueL" );
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::HandleResourceChange
//
// ---------------------------------------------------------
//
void CCertManUIContainerTrust::HandleResourceChange(TInt aType)
    {
    CCoeControl::HandleResourceChange(aType);

    if (aType == KEikDynamicLayoutVariantSwitch)
        {
        TRect mainPaneRect;
        AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, mainPaneRect);
        SetRect(mainPaneRect);
        DrawNow();
        }
    }

// ---------------------------------------------------------
// CCertManUIContainerTrust::GetHelpContext
// This function is called when Help application is launched.
// (other items were commented in a header).
// ---------------------------------------------------------
//
void CCertManUIContainerTrust::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidCM;
    aContext.iContext = KCM_HLP_TRUST_SETTINGS;
    }

// End of File

