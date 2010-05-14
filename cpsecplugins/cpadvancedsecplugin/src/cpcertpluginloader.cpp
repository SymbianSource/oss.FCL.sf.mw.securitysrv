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

CpCertPluginLoader::CpCertPluginLoader()
	{
		mTranslator =q_check_ptr( new QTranslator());
		// to be uncommented after translation sumission.
		//  QString lang = QLocale::system().name();
		QString path = "z:/resource/qt/translations/";
		mTranslator->load(path + "certificate_management_en" );
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

CpSettingFormItemData *CpCertPluginLoader::createSettingFormItemData(CpItemDataHelper &itemDataHelper) const
		{
		 return new CpSettingFormEntryItemDataImpl<CpSecurityView>(
			itemDataHelper,tr("Advanced Security"), QString());    
		}

Q_EXPORT_PLUGIN2(cpcertpluginloader, CpCertPluginLoader);
