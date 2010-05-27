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
#ifndef	CPCERTPLUGINLOADER_H
#define	CPCERTPLUGINLOADER_H

#include <qobject.h>
#include <cpplugininterface.h>
#include <QTranslator>
class CpCertPluginLoader : public QObject, public CpPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(CpPluginInterface)
public:
    CpCertPluginLoader();
    virtual ~CpCertPluginLoader();
      virtual QList<CpSettingFormItemData*>createSettingFormItemData(CpItemDataHelper &itemDataHelper) const;
private:
	  QTranslator* mTranslator;
};

#endif	//CPCERTPLUGINLOADER_H
