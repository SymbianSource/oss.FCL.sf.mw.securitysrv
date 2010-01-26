/*
* Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Declaration of the CCertManUIKeeper class
*
*/


#ifndef  CERTMANUIKEEPER_H
#define  CERTMANUIKEEPER_H

// INCLUDES
#include <unifiedcertstore.h>
#include <mctwritablecertstore.h>
#include <unifiedkeystore.h>
#include <AknProgressDialog.h>
#include <ConeResLoader.h>

// FORWARD DECLARATIONS
class CCrCertEntry;
class CCertManUISyncWrapper;
class CCertManUICertData;
class CCertManUiDialogs;
class CEikColumnListBox;
class CAknTabGroup;
class CAknNavigationDecorator;
class CAknNavigationControlContainer;
class CCertManUICertificateHelper;
class CCertManUIWaitDialog;

// CLASS DECLARATION

/**
*  CCertManUIKeeper view control class.
*
*  @lib certmanui.lib
*/
NONSHARABLE_CLASS( CCertManUIKeeper ): public CCoeControl, public MCoeControlObserver
    {
    public: // functions

        /**
        * Default constructor
        */
        CCertManUIKeeper();

        /**
        * Destructor.
        */
        ~CCertManUIKeeper();

        /**
        * NewL
        */
        static class CCertManUIKeeper* NewL(const TRect& aRect, TUid aViewId);

        /**
        * NewLC
        */
        static class CCertManUIKeeper* NewLC(const TRect& aRect, TUid aViewId);

        /**
        * Activates the WimTitle and CertManUI Trust Settings title
        */
        void ActivateTitleL(TInt aCurrentTitle);

        /**
        * Returns *iCertStore
        */
        CUnifiedCertStore*& CertManager();


        /**
        * Initializes CertManUI Keeper
        */
        void InitializeL(const TRect& aRect, TUid aViewId);

        /**
        * Returns *iKeyStore
        */
        CUnifiedKeyStore*& KeyManager();

        /**
        * Added here because FocusChanged need to be included
        * in every control derived from CCoeControl that can have listbox
        */
        void FocusChanged(TDrawNow aDrawNow);

        /**
        * Checks that cacerts.dat file is not corrupt
        * and returns CA certificates from it.
        */
        void RefreshCAEntriesL();

        /**
        * Checks that cacerts.dat file is not corrupt
        * and returns user certificates from it.
        */
        void RefreshUserCertEntriesL();

        /**
        * Returns peer certificates.
        */
        void RefreshPeerCertEntriesL();

        /**
        * Returns peer certificates.
        */
        void RefreshDeviceCertEntriesL();

        /**
        * Shows error note
        */
        void ShowErrorNoteL(TInt aError);

        /**
        * Displays wait dialog
        */
        void StartWaitDialogL(TInt aDialogSelector);

        /**
        * Closes wait dialog
        */
        void CloseWaitDialogL();

        /**
        * Check whether local feature is on
        */
        TBool IsLocalFeatureSupported( const TInt aFeature ) const;

    public: //data

        /**
        * Added here because FocusChanged need to be included
        * in every control derived from CCoeControl that can have listbox
        */
        CEikColumnListBox*    iListBox;


        /**
        * To store CA certificate entries for Authority and Trust view
        * Includes label from x509certnameparser.
        */
        RMPointerArray<CCertManUICertData>     iCALabelEntries;

        /**
        * To store CA certificate entries for Authority and Trust view
        * Includes label from x509certnameparser.
        */
        RMPointerArray<CCertManUICertData>     iUserLabelEntries;

        /**
        * To store Peer certificate entries for Trusted Site view
        * Includes label from x509certnameparser.
        */
        RMPointerArray<CCertManUICertData>     iPeerLabelEntries;

        /**
        * To store Peer certificate entries for Device certificate view
        * Includes label from x509certnameparser.
        */
        RMPointerArray<CCertManUICertData>     iDeviceLabelEntries;

        /**
        * Used to wrap asynchronous calls
        */
        CCertManUISyncWrapper*          iWrapper;

        /**
        * File session handle.
        */
        RFs                             iRfs;

        /**
        * To keep track of the currupted state of cacerts.dat
        * ETrue = not corrupted
        * EFalse = corrupted
        */
        TBool   iCertsDatOK;

        /**
        * To know when CertManUI is started the first time
        * ETrue = startup
        * EFalse = not startup
        */
        TBool   iStartup;

         /**
        * Certificate helper class
        */
        CCertManUICertificateHelper*    iCertificateHelper;

    private: // functions

        /**
        * EPOC default constructor.
        * @param aRect Frame rectangle for container.
        */
        void ConstructL(const TRect& aRect, TUid aViewId);

        /**
        * Opens resource file.
        */
        void AddResourceFileL();

        /**
        * Handles control events
        * Cannot be changed to non-leaving function.
        * L-function is required by the class definition, even if empty.
        */
        void HandleControlEventL( CCoeControl* aControl,TCoeEvent aEventType );

        /**
        * Called when a key is pressed
        * Cannot be changed to non-leaving function.
        * L-function is required by the class definition, even if empty.
        */
        TKeyResponse OfferKeyEventL(
                const TKeyEvent& aKeyEvent, TEventCode aType);

        /**
        * Read local variation settings
        */
        void ReadLocalVariationL();


    private: //data

        /**
        * To store the previous view id inside CertManUI
        */
        TUid    iPreviousViewId;

        /**
        * To store the previous view id outside CertManUI, KSettListSecurityViewId
        */
        TUid    iPreviousAppViewId;

        /**
        * To list certificates with criteria.
        */
        CCertAttributeFilter*   iFilter;

        /**
        * To list certificates with criteria.
        */

        TCTKeyAttributeFilter*    iKeyFilter;

        /**
        * UnifiedCertStore
        */
        CUnifiedCertStore*    iStore;

        /**
        * UnifiedCertStore
        */
        CUnifiedKeyStore*   iKeyStore;

        /**
        * To store CA certificate entries.
        */
        RMPointerArray<CCTCertInfo>     iCAEntries;

        /**
        * To store User certificate entries.
        */
        RMPointerArray<CCTCertInfo>     iUserEntries;

        /**
        * To store Peer certificate entries.
        */
        RMPointerArray<CCTCertInfo>     iPeerEntries;

        /**
        * To store Device certificate entries.
        */
        RMPointerArray<CCTCertInfo>     iDeviceEntries;

        /**
        * To store Key entries
        */
        RMPointerArray<CCTKeyInfo>      iKeyEntries;

        /*
        * Wait dialog
        */
        CCertManUIWaitDialog*           iCertmanUIDialog;

        /**
        * Local variantion mask
        */
        TInt    iLocalFeaturesMask;

        /**
        * Resource loader
        */
        RConeResourceLoader iResourceLoader;


   };

/**
*  Small helper class to put label and certificate in same class.
*
*  @lib certmanui.lib
*/
NONSHARABLE_CLASS( CCertManUICertData ): public CBase
    {
    public:

        CCertManUICertData();

        void Release();

        HBufC* iCAEntryLabel;
        HBufC* iUserEntryLabel;
        HBufC* iPeerEntryLabel;
        HBufC* iDeviceEntryLabel;
        CCTCertInfo* iCAEntry;
        CCTCertInfo* iUserEntry;
        CCTCertInfo* iPeerEntry;
        CCTCertInfo* iDeviceEntry;

    private:
        ~CCertManUICertData();

    };


/**
 * Helper function to close wait dialog if leave happens.
 * CCertManUIKeeper::StartWaitDialogL() brings wait dialog visible.
 * Wait dialog must be closed by calling CCertManUIKeeper::CloseWaitDialogL().
 * If code leaves after StartWaitDialogL() and before CloseWaitDialogL(),
 * the wait dialog is never closed.
 */
void CleanupCloseWaitDialogPushL( CCertManUIKeeper& aKeeper );


#endif // __CERTMANUIKEEPER_H

// End of File
