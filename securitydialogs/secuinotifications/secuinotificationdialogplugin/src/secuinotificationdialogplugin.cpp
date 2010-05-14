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
* Description: SecUi notification plugin.
*
*/

#include "secuinotificationdialogplugin.h"
#include "secuinotificationdialog.h"
#include "secuinotificationdialogpluginkeys.h"

// This plugin implements one device dialog type
static const struct {
    const char *mTypeString;
} dialogInfos[] = {
    {SECUINOTIFICATIONDIALOG}
};


// ----------------------------------------------------------------------------
// SecUiNotificationDialogPlugin::SecUiNotificationDialogPlugin()
// ----------------------------------------------------------------------------
//
SecUiNotificationDialogPlugin::SecUiNotificationDialogPlugin() : mError(KNoError)
{
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialogPlugin::~SecUiNotificationDialogPlugin()
// ----------------------------------------------------------------------------
//
SecUiNotificationDialogPlugin::~SecUiNotificationDialogPlugin()
{
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialogPlugin::accessAllowed()
// ----------------------------------------------------------------------------
//
bool SecUiNotificationDialogPlugin::accessAllowed(const QString &deviceDialogType,
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
// SecUiNotificationDialogPlugin::createDeviceDialog()
// ----------------------------------------------------------------------------
//
HbDeviceDialogInterface *SecUiNotificationDialogPlugin::createDeviceDialog(
    const QString &deviceDialogType, const QVariantMap &parameters)
{
    //  Create device dialog widget
    Q_UNUSED(deviceDialogType)

    SecUiNotificationDialog *deviceDialog = new SecUiNotificationDialog(parameters);
    mError = deviceDialog->deviceDialogError();
    if (mError != KNoError) {
        delete deviceDialog;
        deviceDialog = 0;
    }

    return deviceDialog;
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialogPlugin::deviceDialogInfo()
// ----------------------------------------------------------------------------
//
bool SecUiNotificationDialogPlugin::deviceDialogInfo( const QString &deviceDialogType,
        const QVariantMap &parameters, DeviceDialogInfo *info) const
{
    // Return device dialog flags
    Q_UNUSED(deviceDialogType);
    Q_UNUSED(parameters);

    //info->group = DeviceNotificationDialogGroup;	// TODO this should be SecurityGroup , but it's still not available, Commented out by 10.1 Integration
	info->group = SecurityGroup;	// Added by 10.1 Integration... It's working better with this layer.
    info->flags = NoDeviceDialogFlags;
    info->priority = DefaultPriority;

    return true;
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialogPlugin::deviceDialogTypes()
// ----------------------------------------------------------------------------
//
QStringList SecUiNotificationDialogPlugin::deviceDialogTypes() const
{
    // Return device dialog types this plugin implements

    QStringList types;
    const int numTypes = sizeof(dialogInfos) / sizeof(dialogInfos[0]);
    for(int i = 0; i < numTypes; ++i) {
        types.append(dialogInfos[i].mTypeString);
    }

    return types;
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialogPlugin::pluginFlags()
// ----------------------------------------------------------------------------
//
HbDeviceDialogPlugin::PluginFlags SecUiNotificationDialogPlugin::pluginFlags() const
{
    // Return plugin flags
    return NoPluginFlags;
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialogPlugin::error()
// ----------------------------------------------------------------------------
//
int SecUiNotificationDialogPlugin::error() const
{
    // Return last error
    return mError;
}

Q_EXPORT_PLUGIN2(secuinotificationdialogplugin,SecUiNotificationDialogPlugin)
