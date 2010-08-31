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

#include <cpsettingformentryitemdataimpl.h>

#include "cpcertpluginloader.h"
#include "cpsecurityview.h"
#include <hbapplication.h>
#include <../../inc/cpsecplugins.h>

CpCertPluginLoader::CpCertPluginLoader()
	{
		RDEBUG("0", 0);
		mTranslator = q_check_ptr( new QTranslator());
		QString path = "z:/resource/qt/translations/";
		QString appName = "certificate_manager_";
		QString lang = QLocale::system().name();
		mTranslator->load(path + appName + lang);
		qApp->installTranslator(mTranslator);		
	}

CpCertPluginLoader::~CpCertPluginLoader()
	{
	    if (mTranslator)
        {
        if (mTranslator->isEmpty() == false)
            qApp->removeTranslator(mTranslator);
        delete mTranslator;
        }	
	}

QList<CpSettingFormItemData*> CpCertPluginLoader::createSettingFormItemData(CpItemDataHelper &itemDataHelper) const
		{
		RDEBUG("0", 0);
  return QList<CpSettingFormItemData*>() 
           << new CpSettingFormEntryItemDataImpl<CpSecurityView>(itemDataHelper,tr("Advanced Security"), QString());			
		}

Q_EXPORT_PLUGIN2(cpcertpluginloader, CpCertPluginLoader);
