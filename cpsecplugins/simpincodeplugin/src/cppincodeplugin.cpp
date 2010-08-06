/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 *
 * Description:
 *
 */

// System includes
#include <cpsettingformentryitemdataimpl.h>
#include <../../inc/cpsecplugins.h>

// User includes
#include "cppincodeplugin.h"
#include "cppincodepluginview.h"

/*!
    \class CpPinCodePlugin
    \brief Pin Code Setting plugin class

    This class is used to create PIN code setting
*/

// ======== LOCAL FUNCTIONS ========

/*!
   Constructor
*/
CpPinCodePlugin::CpPinCodePlugin()
{
RDEBUG("0", 0);
}

/*!
   Destructor
*/
CpPinCodePlugin::~CpPinCodePlugin()
{
}

/*!
   Create PinCode setting
*/
QList<CpSettingFormItemData *> CpPinCodePlugin::createSettingFormItemData(
    CpItemDataHelper &itemDataHelper) const
{
		RDEBUG("0", 0);
		RDEBUG("using icon", 1);
    return  QList<CpSettingFormItemData *>()
        << new CpSettingFormEntryItemDataImpl<CpPinCodePluginView>
        (itemDataHelper, tr("PIN code"), QString(), HbIcon(QString("qtg_large_pin_code")) );
}

/*!
   Export plugin
*/
Q_EXPORT_PLUGIN2(cppincodeplugin, CpPinCodePlugin);
