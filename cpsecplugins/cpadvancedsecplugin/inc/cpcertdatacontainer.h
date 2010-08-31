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
* Description:   Declaration of the CCertManUIKeeper class
*
*/

#ifndef  CPCERTDATACONTAINER_H
#define  CPCERTDATACONTAINER_H

// INCLUDES
#include <unifiedcertstore.h>
#include <mctwritablecertstore.h>
#include <unifiedkeystore.h>

// FORWARD DECLARATIONS
class CpCertManUISyncWrapper;
class CpCertManUICertData;

// CONSTANTS
const TUid KCMDeviceCertStoreTokenUid = { 0x101FB668 };
const TUid KCMDeviceKeyStoreTokenUid = { 0x101FB66A };
const TUid KCMTrustedServerTokenUid = { 0x101FB66F };
const TUid KCMFileKeyStoreTokenUid = { 0x101F7333 };
const TUid KCMFileCertStoreTokenUid = { 0x101F501A };


// CLASS DECLARATION

/**
*  CpCertDataContainer retrieves and holds certificates.
*
*/
class CpCertDataContainer : public CBase
    {
    public: // functions

        /**
        * Default constructor
        */
		CpCertDataContainer();

        /**
        * Destructor.
        */
        ~CpCertDataContainer();

        /**
        * NewL
        */
        static class CpCertDataContainer* NewL();

        /**
        * NewLC
        */
        static class CpCertDataContainer* NewLC();

        CUnifiedCertStore*& CertManager();

        CUnifiedKeyStore*& KeyManager();

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

    public: //data

        /**
        * To store CA certificate entries for Authority and Trust view
        * Includes label from x509certnameparser.
        */
        RMPointerArray<CpCertManUICertData>     iCALabelEntries;

        /**
        * To store CA certificate entries for Authority and Trust view
        * Includes label from x509certnameparser.
        */
        RMPointerArray<CpCertManUICertData>     iUserLabelEntries;

        /**
        * To store Peer certificate entries for Trusted Site view
        * Includes label from x509certnameparser.
        */
        RMPointerArray<CpCertManUICertData>     iPeerLabelEntries;

        /**
        * To store Peer certificate entries for Device certificate view
        * Includes label from x509certnameparser.
        */
        RMPointerArray<CpCertManUICertData>     iDeviceLabelEntries;

        /**
        * Used to wrap asynchronous calls
        */
        CpCertManUISyncWrapper*          iWrapper;

        /**
        * File session handle.
        */
        RFs                             iRfs;

    private: // functions

        void ConstructL();

    private: //data

        /**
        * To list certificates with criteria.
        */
        CCertAttributeFilter*   iFilter;

        /**
        * To list keys with criteria.
        */

        TCTKeyAttributeFilter*    iKeyFilter;

        /**
        * UnifiedCertStore
        */
        CUnifiedCertStore*    iStore;

        /**
        * UnifiedKeyStore
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

   };

/**
*  Small helper class to put label and certificate in same class.
*
*/
class CpCertManUICertData : public CBase
    {
    public:

	CpCertManUICertData();

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
        ~CpCertManUICertData();

    };


#endif // CPCERTDATACONTAINER_H
