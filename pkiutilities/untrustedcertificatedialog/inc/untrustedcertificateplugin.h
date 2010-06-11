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
* Description:  Device dialog plugin that shows untrusted certificate
*               dialog for TLS server authentication failure errors.
*
*/

#ifndef UNTRUSTEDCERTIFICATEPLUGIN_H
#define UNTRUSTEDCERTIFICATEPLUGIN_H

#include <hbdevicedialogplugin.h>


/**
 * Untrusted certificate plugin.
 * Device dialog plugin to show untrusted certificate dialog.
 * See UntrustedCertificateDialog.
 */
class UntrustedCertificatePlugin : public HbDeviceDialogPlugin
{
    Q_OBJECT

public:
    UntrustedCertificatePlugin();
    ~UntrustedCertificatePlugin();

public:     // from HbDeviceDialogPlugin
    bool accessAllowed(const QString &deviceDialogType, const QVariantMap &parameters,
            const QVariantMap &securityInfo) const;
    HbDeviceDialogInterface *createDeviceDialog(const QString &deviceDialogType,
            const QVariantMap &parameters);
    bool deviceDialogInfo(const QString &deviceDialogType, const QVariantMap &parameters,
            DeviceDialogInfo *info) const;
    QStringList deviceDialogTypes() const;
    PluginFlags pluginFlags() const;
    int error() const;

private:
    Q_DISABLE_COPY(UntrustedCertificatePlugin)

    int mError;
};

#endif // UNTRUSTEDCERTIFICATEPLUGIN_H
