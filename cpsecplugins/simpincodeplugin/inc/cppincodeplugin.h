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

#ifndef CPPINCODEPLUGIN_H
#define CPPINCODEPLUGIN_H

// System includes
#include <QObject>

// User includes
#include <cpplugininterface.h>

// Class declaration
class CpPinCodePlugin : public QObject, public CpPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(CpPluginInterface)
    
public:    
    CpPinCodePlugin();
    virtual ~CpPinCodePlugin();
    virtual QList<CpSettingFormItemData *> createSettingFormItemData(
        CpItemDataHelper &itemDataHelper) const;
};

#endif //CPPINCODEPLUGIN_H
