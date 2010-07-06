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

#include "secuinotificationdebug.h"
#include "secuinotificationdialogplugin.h"
#include "secuinotificationdialog.h"
#include "secuinotificationdialogpluginkeys.h"

#include <etelmm.h>

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
		RDEBUG("0", 0);
    Q_UNUSED(deviceDialogType)
    Q_UNUSED(parameters)
    Q_UNUSED(securityInfo)

    // All clients are allowed to use.
    // should access be limited to certain clients or capabilities ? Not for now.
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
    RDEBUG("0", 0);
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
		// For some unknown reason, this function doesn't print the signature 
		RDEBUG("0", 0);
		#ifdef _DEBUG
		RDebug::Printf( "SecUiNotificationDialogPlugin::deviceDialogInfo=%x", 0 );
		#endif

    Q_UNUSED(deviceDialogType);

		info->group = SecurityGroup;
    info->flags = NoDeviceDialogFlags;
    info->priority = DefaultPriority;

		// The unlock-query must have higher priority, to get over Telephony.
		if (parameters.contains(KQueryType)) {
				#ifdef _DEBUG
				RDebug::Printf( "SecUiNotificationDialogPlugin::deviceDialogInfo KQueryType=%x", 1 );
				#endif
        int iqueryType = parameters.value(KQueryType).toUInt();
        RDEBUG("iqueryType", iqueryType);
				#ifdef _DEBUG
				RDebug::Printf( "SecUiNotificationDialogPlugin::deviceDialogInfo iqueryType=%x", iqueryType );
				#endif
				if( (iqueryType & 0xFFFF) == RMobilePhone::ESecurityCodePhonePassword )
					{
					RDEBUG("CriticalGroup", CriticalGroup);
					#ifdef _DEBUG
					RDebug::Printf( "SecUiNotificationDialogPlugin::deviceDialogInfo CriticalGroup=%x", CriticalGroup );
					#endif
					info->group = CriticalGroup;
					}
				}
    // Return device dialog flags

    return true;
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialogPlugin::deviceDialogTypes()
// ----------------------------------------------------------------------------
//
QStringList SecUiNotificationDialogPlugin::deviceDialogTypes() const
{
		RDEBUG("0", 0);
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
		RDEBUG("0", 0);
    // Return plugin flags
    return NoPluginFlags;
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialogPlugin::error()
// ----------------------------------------------------------------------------
//
int SecUiNotificationDialogPlugin::error() const
{
		RDEBUG("mError", mError);
    // Return last error
    return mError;
}

Q_EXPORT_PLUGIN2(secuinotificationdialogplugin,SecUiNotificationDialogPlugin)
