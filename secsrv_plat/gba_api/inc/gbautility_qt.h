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
* Description: Utility to perform GBA bootstrapping operation.
*              GBA = Generic Bootstrapping Architecture
*              Reference: 3GPP TS 33.220 (version 7.110 Release 7) at http://www.3gpp.org
*/

#ifndef GBAUTILITY_QT_H
#define GBAUTILITY_QT_H

#include <QObject>
#include <QString>
#include <QtCore/qdatetime.h>
#include <gbauilityexport.h>

const QString gUsim = "USIM";
const QString gSim  = "SIM";
const QString gIsim = "ISIM";  // Currently not supported.

enum GbaBootstrapFlag {
    // Default bootstrap: use the cached credentials if they have not expired.
    DefaultBootstrap = 0,
    // Do not use cached credentials, instead force bootstrapping with BSF.
    ForceBootstrap
};

enum GbaRunType {
    RunTypeNone = 0,
    RunType2gGba,       // 2G GBA: GBA-specific functions are carried out in the ME.
    RunType3gGbaMe,     // 3G GBA_ME: GBA-specific functions are carried out in the ME.
    RunType3gGbaU       // 3G GBA_U: GBA-specific functions are carried out in the UICC.
};

// Input data to GBA Bootstrapping operation.
typedef struct
{
    // Input: FQDN (Fully Qualified Domain Name) of NAF
    QString nafName;
    // Input: DefaultBootstrap or ForceBootstrap
    GbaBootstrapFlag bootstrapFlag;
    // Input: label of UICC application that user wants to use, gUsim ("USIM") for example;
    //   default UICC application is used if the label is empty.
    QString uiccLabel;                            // Currently not supported.
    // Input: security protocol identifier appended to NAF Name for key derivation
    QString protocolIdentifier;
    // Input: identifier of preferred internet access point for bootstrap;
    //   set -1 to use the default access point.
    qint32  accessPoint;
} GbaBootstrapInputData;

// Output data (credentials) from GBA Bootstrapping operation.
typedef struct
{
    // Output: B-TID
    QString bTid;
    // Output: Ks_NAF
    QByteArray ksNaf;
    // Output: lifetime
    QDateTime lifetime;
    // Output: IMPI
    QString impi;           // Currently not supported.
    // Output: GBA run-type
    GbaRunType gbaRunType;
    // Output: type of UICC application used
    QString uiccLabel;
} GbaBootstrapOutputData;


class CGbaUtilityBody;


// API class for GBA Bootstrapping operation.
class QTGBADLL_EXPORT GbaUtility : public QObject
{
    Q_OBJECT

public:
    enum GbaErrorStatus {
        GbaNoError = 0,             // No error; operation is successful.
        GbaErrorGeneral,            // Error: general error.
        GbaErrorArgument,           // Error: wrong argument value in method call.
        GbaErrorInUse,              // Error: an outstanding bootstrap request is in place.
        GbaErrorPermissionDenied,   // Error: permission to execute the operation is denied, e.g.,
                                    //        due to application not having enough security capability.
        GbaErrorNetworkConnection,  // Error: error in network connection.
        GbaErrorBootstrapFailed     // Error: bootstrapping with BSF has failed.
    };

    GbaUtility(QObject *parent = 0);

    virtual ~GbaUtility();

    // Perform GBA Bootstrapping operation. This is an asynchronous method and the "bootstrapCompleted" signal
    // will be sent with a success or failure indication when the operation is completed.
    // Only one Bootstrapping operation can be performed at a time.
    // On Symbian platform, application needs to have "ReadDeviceData" capability
    // for this method call to be successful.
    GbaErrorStatus bootstrap(const GbaBootstrapInputData *input, GbaBootstrapOutputData *output);

    // Cancel the current Bootstrapping operation.
    // The "bootstrapCompleted" signal will not be sent after cancellation.
    void cancelBootstrap();

    // Set BSF address. The BSF address set by this method will overwrite the one calculated from IMPI.
    // On Symbian platform, application needs to have "WriteDeviceData" capability
    // for this method call to be successful.
    GbaErrorStatus setBsfAddress(const QString &bsfAddress);

signals:
    // Signal to indicate that GBA Bootstrapping operation has completed.
    // The parameter "status" indicates the success or failure of the operation.
    // If it is successful, the output data (credentials) of the operation are returned in
    // the "GbaBootstrapOutputData" structure that application passed in the previous "bootstrap" method call.
    void bootstrapCompleted(GbaUtility::GbaErrorStatus status);

private:
    CGbaUtilityBody *gbaUtilityBody;

private:    // Friend class definitions
    friend class CGbaUtilityBody;
};

#endif // GBAUTILITY_QT_H
// EOF
