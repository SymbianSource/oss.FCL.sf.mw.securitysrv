/*
* Copyright (c) 2003-2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Includes, classes, constants and strings used by CertManUI
*
*/


#ifndef  CERTMANUICOMMON_H
#define  CERTMANUICOMMON_H

// INCLUDES
#include <eikmenub.h>     // for menu and cba support
#include <aknnavide.h>
#include <akntabgrp.h>
#include <akntitle.h>
#include <aknlists.h>
#include <aknViewAppUi.h>
#include <barsread.h>     // for TResourceReader
#include <StringLoader.h>
#include <featmgr.h>

// _LITS
_LIT( KCertManUIInsertDescriptor, "\t" );
_LIT( KCertManUIFormatDescriptorTrustSettings, "\t%S\t\t" );
_LIT( KCertManUIDetailsViewEnter, "\n" );
_LIT( KCertManUIDetailsViewEnterEnter, "\n\n" );
_LIT( KCertManUIDetailsViewEnterHyphenEnter, " - " );
_LIT( KCertManUIDetailsViewHexaFormat, "%02X" );
_LIT( KCertManUICertDataIndexFormat, "%d" );

// FORWARD DECLARATIONS
class CCertManUIContainerAuthority;
class CCertManUIContainerPersonal;

// CONST TINTS

const TInt KCertTypeAuthority = 1;
const TInt KCertTypeTrustedSite = 2;
const TInt KCertTypePersonal = 3;
const TInt KCertTypeDevice = 4;

// For setting the correct title for CertManUI
const TInt KViewTitleCertManUI = 1;
const TInt KViewTitleCertManUIAuthority = 2;
const TInt KViewTitleCertManUITrustedSite = 3;
const TInt KViewTitleCertManUIPersonal = 4;
const TInt KViewTitleCertManUIDevice = 5;


const TUid KCMFileCertStoreTokenUid = { 0x101F501A };
const TUid KCMTrustedServerTokenUid = { 0x101FB66F };
const TUid KCMDeviceCertStoreTokenUid = { 0x101FB668 };

const TUid KCMFileKeyStoreTokenUid = { 0x101F7333 };
const TUid KCMDeviceKeyStoreTokenUid = { 0x101FB66A };


// Granularity of the created iconarrays
LOCAL_D const TInt KGranularity = 4;

// "dd/mm/yyyy0"
const TInt KMaxLengthTextDateString = 11;
// HH0
const TInt KMaxLengthTextSerialNumberFormatting = 3;
// CertLabel, Issuer, Owner max length 510 bytes
const TInt KMaxLengthTextCertLabel = 510;
// CertLabel, Issuer, Owner max visible length
const TInt KMaxLengthTextCertLabelVisible = 200;
// CertWimLabel, KeyLabel, KeyWimLabel max length 255 bytes
const TInt KMaxLengthTextKeyWimLabel = 256;
// Maximum length of dynamic strings is 4092,
// plus 13 resource strings with average lenght of 50 makes 650
// total 4742 rounded up to 5000
const TInt KMaxLengthTextDetailsBody = 5000;
// Max length for ave_list_setting_pane_1 is 52 and
// for ave_list_setting_pane_2 51 (formatting 7) , total 103 + end mark
const TInt KMaxLengthTextSettListOneTwo = 104;
// Max length for ave_list_setting_pane_1 is 52
const TInt KMaxLengthTextSettListOne = 52;
// Maximum length of a certificate
const TInt KMaxCertificateLength = 5000;
const TInt KMaxKeyLength = 10000;

const TInt KWIMCertStoreUid ( 0x101F79D9 );

// FEATURE FLAGS

#define WIM_ENABLED ( FeatureManager::FeatureSupported( KFeatureIdWim ) )

// DATA TYPES

enum TCertmanUiPanics
    {
    EBadDialogSelector,
    EDoubleDialogClose
    };

enum TCertmanUiDialogs
    {
    ECertmanUiDeleteDialog,
    ECertmanUiWaitDialog
    };

#endif //   CERTMANUICOMMON_H

// End of File
