/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
#include "cpdevicelockplugin.h"
#include "cpdevicelockpluginview.h"
#include <cpsettingformentryitemdataimpl.h>



/*
 *****************************************************************
 * Name        : CpDeviceLockPlugin
 * Parameters  : None
 * Return value: None
 * Description : constructor
 *****************************************************************
 */
CpDeviceLockPlugin::CpDeviceLockPlugin()
{
}


/*
 *****************************************************************
 * Name        : ~CpDeviceLockPlugin
 * Parameters  : None
 * Return value: None
 * Description : destructor
 *****************************************************************
 */
CpDeviceLockPlugin::~CpDeviceLockPlugin()
{
}


/*
 *****************************************************************
 * Name        : createSettingFormItemData
 * Parameters  : CpItemDataHelper
 * Return value: QLsit<CpSettingFormItemData>*
 * Description : creates a data form for device lock settings
 *****************************************************************
 */
QList<CpSettingFormItemData*> CpDeviceLockPlugin::createSettingFormItemData(CpItemDataHelper &itemDataHelper) const
{
    return QList<CpSettingFormItemData*>() 
           << new CpSettingFormEntryItemDataImpl<CpDeviceLockPluginView>(itemDataHelper,tr("Device lock"), QString());
}

Q_EXPORT_PLUGIN2(cpdevicelockplugin, CpDeviceLockPlugin);
