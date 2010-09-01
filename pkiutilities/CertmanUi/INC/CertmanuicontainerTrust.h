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
* Description:   Declaration of the CCertManUIContainerTrust class
*                Maintains correct list of trusted clients depending
*                on the certificate that was in focus in Authority
*                certificates view when Trust Settings view was entered.
*                Shows and changes correct Yes/No text for the Trusted client.
*
*/


#ifndef  CERTMANUICONTAINERTRUST_H
#define  CERTMANUICONTAINERTRUST_H

// INCLUDES
#include <aknradiobuttonsettingpage.h>

// CONSTANTS
const TInt KTrustSettingsResourceIndexWAP = 0;
const TInt KTrustSettingsResourceIndexMailAndImageConn = 1;
const TInt KTrustSettingsResourceIndexAppCtrl = 2;
const TInt KTrustSettingsResourceIndexJavaInstall = 3;
const TInt KTrustSettingsResourceIndexOCSPCheck = 4;
const TInt KTrustSettingsResourceIndexVPN = 5;
const TInt KTrustSettingsResourceIndexWidget = 6;
const TInt KTrustSettingsResourceIndexValueYes = 0;
const TInt KTrustSettingsResourceIndexValueNo = 1;

// FORWARD DECLARATIONS
class CCertificateAppInfoManager;

// CLASS DECLARATION

/**
*  CCertManUIContainerTrust container control class.
*
*  @lib certmanui.lib
*/
NONSHARABLE_CLASS( CCertManUIContainerTrust ):
    public CCoeControl, public MCoeControlObserver, public MEikListBoxObserver
    {

    /**
    * Container trust needs to get access to CertManUIViewTrust's DoActivateL
    */
    friend void CCertManUIViewTrust::DoActivateL( const TVwsViewId&, TUid, const TDesC8& );

    public: // functions

        /**
        * Overrided Default constructor
        */
        CCertManUIContainerTrust( CCertManUIViewTrust& aParent, CCertManUIKeeper& aKeeper );

        /**
        * Destructor.
        */
        ~CCertManUIContainerTrust();

        /**
        * Change focus
        */
        void FocusChanged( TDrawNow aDrawNow );

        /**
        * Draws listbox
        */
        void DrawListBoxL( TInt aCurrentPosition, TInt aTopItem, TInt aCertificateIndex );

        /**
        * Puts correct Trust Settings text to listbox and correct value (yes/no) to the setting,
        * asks the value from CheckCertificateClientTrustL
        * For X509 certificates, only one Application is currently supported (Application controller),
        * more applications maybe added later however, so one additional application has been left
        * in comments as an example.
        * WAP connection and Application installer have their names from resources and are thus localized,
        * other additional application get their names from the CertMan API with Name() function
        */
        void UpdateListBoxL( TInt aCertificateIndex );

        /**
        * Handles listbox events
        * Cannot be changed to non-leaving function.
        * L-function is required by the class definition, even if empty.
        */
        void HandleListBoxEventL( CEikListBox* aListBox, TListBoxEvent aEventType );

        /**
        * Changes the Trust Settings text from Yes to No and Vice versa
        * Updates the Trust Settings Listbox item value
        * Can be called only for those certificates that are located in ECrCertLocationCertMan !!!
        * Cannot be called for certificates located in ECrCertLocationWIMCard,
        *                       ECrCertLocationWIMURL,
        *                       ECrCertLocationPhoneMemory
        *                       ECrCertLocationPhoneMemoryURL
        * Certificate formats ECrX968Certificate ECrCertificateURL
        * are not visible in the Trust View
        */
        void UpdateTrustListboxItemL( CCTCertInfo& aEntry, TInt aCurrentTruster );

        /**
        * Puts correct client name and trust state to radiobutton setting page
        * Finds out which client was focused in Trust Settings view and calls ShowTrustChangeSettingPageL
        * This is accessed only by X509 certificates, located in CertMan
        * and having one or more clients
        * Must be public so that viewTrust can access
        */
        void PopupTrustChangeSettingPageL( TInt aCertificateIndex );

        /**
        * Checks if a certificate trusts a client
        * * Certificate format == ECrX509Certificate ECrX968Certificate ECrCertificateURL
        * are not supported if certificate location == ECrCertLocationWIMCard
        * * Certificate format == ECrX968Certificate ECrCertificateURL are not supported
        * if certificate location == ECrCertLocationCertMan
        * * Certificate location == ECrCertLocationWIMURL ECrCertLocationPhoneMemory
        * ECrCertLocationPhoneMemoryURL are not supported
        */
        TBool CheckCertificateClientTrustL( const TUid aClientTUid, CCTCertInfo& aEntry ) const;

        /**
        * Changes the Trust state of client in the CertManAPI
        */
        void ChangeTrustValueL( CCTCertInfo& aEntry, const TUid aClientUid );

        /**
        * Handles resource change
        */
        void HandleResourceChange( TInt aType );

    public: // data

        /**
        * To get hold of ViewTrust
        */
        CCertManUIViewTrust&    iParent;

        /**
        * To get hold of CertManUIKeeper
        */
        CCertManUIKeeper&   iKeeper;

        /**
        * To handle listbox
        * Must be public so that viewTrust can access
        */
        CEikTextListBox*    iListBox;

        /**
        * To get the certificate location
        * Must be public so that viewTrust can access
        */
        TInt  iCertificateLocation;

        /**
        * Application manager class, used to get applications in system.
        */
        CCertificateAppInfoManager*   iAppInfoManager;

        /**
        * Applications registered to system.
        */
        RArray<TCertificateAppInfo> iApps;

        /**
        * Client Uids of the certstore
        */
        RArray<TUid> iClientUids;

    private: // functions

        /**
        * Default constructor
        */
        CCertManUIContainerTrust();

        /**
        * EPOC default constructor.
        * @param aRect Frame rectangle for container.
        */
    void ConstructL(const TRect& aRect);

        /**
        * Creates listbox
        */
        void CreateListBoxL();

        /**
        * From CoeControl,SizeChanged.
        */
        void SizeChanged();

        /**
        * From CoeControl,CountComponentControls.
        */
        TInt CountComponentControls() const;

        /**
        * From CCoeControl,ComponentControl.
        */
        CCoeControl* ComponentControl( TInt aIndex ) const;

        /**
        * From CCoeControl,Draw.
        */
        void Draw( const TRect& aRect ) const;

        /**
        * Handles control events
        * Cannot be changed to non-leaving function.
        * L-function is required by the class definition, even if empty.
        */
        void HandleControlEventL( CCoeControl* aControl,TCoeEvent aEventType);

        /**
        * Handles the key events OK button, arrow keys.
        */
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

        /**
        * Sets empty list for listbox.
        */
        void SetEmptyListL( CEikListBox* aListBox );

        /**
        * Puts correct client names and yes/no state to settings page
        * Sets title text to a setting page
        */
        void ShowTrustChangeSettingPageL( const TUid aClientUid,
            TDesC& aClientName, CCTCertInfo& aEntry );

        void ChangeTrustChangeSettingSingleClickL();

        void InsertItemToTrustListL( const TInt index, const TUid id,
                                   CCTCertInfo& entry, CDesCArray* itemArray,
                                   TInt position );

        /**
        * Sets empty list for listbox.
        */
        TInt GetTrusterResId( TUid aTrusterUid );

        /**
        * This function is called when Help application is launched.
        * (other items were commented in a header).
        */
        void GetHelpContext( TCoeHelpContext& aContext ) const;


    private: //data

        /**
        * To get the names of the trusted clients from the resources
        */
        CDesCArrayFlat*   iTrustedClients;

        /**
        * To get the yes/no trust texts from the resources
        */
        CDesCArrayFlat*   iTrustValues;

    };

#endif // __CERTMANUICONTAINERTRUST_H

// End of File
