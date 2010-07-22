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

#include "untrustedcertificateplugin.h"
#include "untrustedcertificatedialog.h"
#include "untrustedcertificatedefinitions.h"


// ----------------------------------------------------------------------------
// UntrustedCertificatePlugin::UntrustedCertificatePlugin()
// ----------------------------------------------------------------------------
//
UntrustedCertificatePlugin::UntrustedCertificatePlugin() : mError(KErrNone)
{
}

// ----------------------------------------------------------------------------
// UntrustedCertificatePlugin::~UntrustedCertificatePlugin()
// ----------------------------------------------------------------------------
//
UntrustedCertificatePlugin::~UntrustedCertificatePlugin()
{
}

// ----------------------------------------------------------------------------
// UntrustedCertificatePlugin::accessAllowed()
// ----------------------------------------------------------------------------
//
bool UntrustedCertificatePlugin::accessAllowed(const QString &deviceDialogType,
    const QVariantMap &parameters, const QVariantMap &securityInfo) const
{
    Q_UNUSED(deviceDialogType)
    Q_UNUSED(parameters)
    Q_UNUSED(securityInfo)

    // All clients are allowed to use.
    // TODO: should access be limited to certain clients?
    return true;
}

// ----------------------------------------------------------------------------
// UntrustedCertificatePlugin::createDeviceDialog()
// ----------------------------------------------------------------------------
//
HbDeviceDialogInterface *UntrustedCertificatePlugin::createDeviceDialog(
    const QString &deviceDialogType, const QVariantMap &parameters)
{
    Q_UNUSED(deviceDialogType)

    UntrustedCertificateDialog *deviceDialog = new UntrustedCertificateDialog(parameters);
    mError = deviceDialog->deviceDialogError();
    if (mError != KErrNone) {
        delete deviceDialog;
        deviceDialog = 0;
    }

    return deviceDialog;
}

// ----------------------------------------------------------------------------
// UntrustedCertificatePlugin::deviceDialogInfo()
// ----------------------------------------------------------------------------
//
bool UntrustedCertificatePlugin::deviceDialogInfo( const QString &deviceDialogType,
        const QVariantMap &parameters, DeviceDialogInfo *info) const
{
    Q_UNUSED(deviceDialogType);
    Q_UNUSED(parameters);

    info->group = GenericDeviceDialogGroup;
    info->flags = NoDeviceDialogFlags;
    info->priority = DefaultPriority;

    return true;
}

// ----------------------------------------------------------------------------
// UntrustedCertificatePlugin::deviceDialogTypes()
// ----------------------------------------------------------------------------
//
QStringList UntrustedCertificatePlugin::deviceDialogTypes() const
{
    QStringList types;
    types << KUntrustedCertificateDialog;
    return types;
}

// ----------------------------------------------------------------------------
// UntrustedCertificatePlugin::pluginFlags()
// ----------------------------------------------------------------------------
//
HbDeviceDialogPlugin::PluginFlags UntrustedCertificatePlugin::pluginFlags() const
{
    return NoPluginFlags;
}

// ----------------------------------------------------------------------------
// UntrustedCertificatePlugin::error()
// ----------------------------------------------------------------------------
//
int UntrustedCertificatePlugin::error() const
{
    return mError;
}

Q_EXPORT_PLUGIN2(untrustedcertdialog,UntrustedCertificatePlugin)

