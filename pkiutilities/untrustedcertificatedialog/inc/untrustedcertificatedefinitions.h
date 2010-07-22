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
* Description:  Parameter definitions for untrusted certificate dialog.
*               Untrusted certificate dialog is displayed for secure
*               connection (TLS) server authentication failure errors.
*
*/

#ifndef UNTRUSTEDCERTIFICATEDEFINITIONS_H
#define UNTRUSTEDCERTIFICATEDEFINITIONS_H

// Device dialog type for untrusted certificate dialog
const QString KUntrustedCertificateDialog = "com.nokia.untrustedcert/1.0";

// Corresponds to CServerAuthenticationFailureInput class
const QString KUntrustedCertEncodedCertificate = "cert";    // bytearray, mandatory
const QString KUntrustedCertServerName = "host";            // string, mandatory
const QString KUntrustedCertValidationError = "err";        // int (TValidationError), mandatory
const QString KUntrustedCertTrustedSiteStoreFail = "tss";   // any, prevents permanent acceptance

// Dialog return codes
const QString KUntrustedCertificateDialogResult = "result"; // int
const int KDialogRejected = 0;
const int KDialogAccepted = 1;
const int KDialogAcceptedPermanently = 2;

#endif // UNTRUSTEDCERTIFICATEDEFINITIONS_H

