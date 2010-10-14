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
 * Description:  Advanced security settings loader
 *
 */

#include "advsecsettingsloader.h"     // AdvSecSettingsLoader
#include "advsecsettingsmainview.h"   // AdvSecSettingsMainView
#include <cpsettingformentryitemdataimpl.h>     // CpSettingFormEntryItemDataImpl
#include <HbApplication>                        // qApp
#include <HbTranslator>

const QString KTranslationsPath = "/resource/qt/translations/";
const QString KTranslationsFile = "certificate_manager";


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// AdvSecSettingsLoader::AdvSecSettingsLoader()
// ---------------------------------------------------------------------------
//
AdvSecSettingsLoader::AdvSecSettingsLoader()
{
    mTranslator = new HbTranslator(KTranslationsPath, KTranslationsFile);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsLoader::~AdvSecSettingsLoader()
// ---------------------------------------------------------------------------
//
AdvSecSettingsLoader::~AdvSecSettingsLoader()
{
    delete mTranslator;
    mTranslator = 0;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsLoader::createSettingFormItemData()
// ---------------------------------------------------------------------------
//
QList<CpSettingFormItemData*> AdvSecSettingsLoader::createSettingFormItemData(
    CpItemDataHelper &itemDataHelper) const
{
    CpSettingFormEntryItemData *advancedSecuritySettingsItem =
            new CpSettingFormEntryItemDataImpl<AdvSecSettingsMainView>(
                    CpSettingFormEntryItemData::ButtonEntryItem,
                    itemDataHelper, hbTrId("txt_certificate_manager_setlabel_advanced_security"));
    advancedSecuritySettingsItem->setContentWidgetData("textAlignment",
            QVariant( Qt::AlignHCenter | Qt::AlignVCenter) );
    advancedSecuritySettingsItem->setContentWidgetData("objectName",
            "advancedSecuritySettingsButton" );
    return QList<CpSettingFormItemData *>() << advancedSecuritySettingsItem;
}


Q_EXPORT_PLUGIN2(cpadvancedsecplugin, AdvSecSettingsLoader);

