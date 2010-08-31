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
* Description: SecUi notification plugin class.
*
*/

#ifndef SECUINOTIFICATIONDIALOGPLUGIN_H
#define SECUINOTIFICATIONDIALOGPLUGIN_H

#include <hbdevicedialogplugin.h>       // HbDeviceDialogPlugin


/**
 * Software install notification plugin.
 * Implements the HbDeviceDialogPlugin to show SW install confirmatoin dialogs.
 */
class SecUiNotificationDialogPlugin : public HbDeviceDialogPlugin
{
    Q_OBJECT

public:
    SecUiNotificationDialogPlugin();
    ~SecUiNotificationDialogPlugin();

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
    Q_DISABLE_COPY(SecUiNotificationDialogPlugin)

    int mError;
};

#endif // SECUINOTIFICATIONDIALOGPLUGIN_H
