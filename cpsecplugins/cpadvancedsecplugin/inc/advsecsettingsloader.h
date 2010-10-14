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
 * Description:  Advanced security settings plugin loader
 *
 */
#ifndef	ADVSECSETTINGSLOADER_H
#define	ADVSECSETTINGSLOADER_H

#include <QObject>
#include <cpplugininterface.h>          // CpPluginInterface

class HbTranslator;


/**
 * Control panel plugin loader for advanced security settings.
 */
class AdvSecSettingsLoader : public QObject, public CpPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(CpPluginInterface)

public:     // constructor and destructor
    AdvSecSettingsLoader();
    ~AdvSecSettingsLoader();

public:     // new functions
    QList<CpSettingFormItemData*> createSettingFormItemData(
        CpItemDataHelper &itemDataHelper) const;

private:    // data
    HbTranslator* mTranslator;
};

#endif // ADVSECSETTINGSLOADER_H
