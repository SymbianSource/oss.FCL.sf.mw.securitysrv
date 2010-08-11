
#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description: 
#

TEMPLATE = lib
TARGET = cpadvancedsecplugin

CONFIG += hb plugin

LIBS += -lcpframework

include ( advancedsecplugin.pri )
include ( ../rom/cpsecplugins_rom.pri )

symbian: { 
	TARGET.CAPABILITY = CAP_ECOM_PLUGIN
	TARGET.UID3 = 0X2002E684
	INCLUDEPATH += $$MW_LAYER_SYSTEMINCLUDE
	INCLUDEPATH += ../inc
	TARGET.EPOCALLOWDLLDATA = 1
	LIBS += -lcertstore
	LIBS += -lCTFramework
	LIBS += -lX509
	LIBS += -lpkixcert
	LIBS += -lx509certnameparser
	LIBS += -lhash
	LIBS += -lcryptography
	LIBS += -lDevTokenClient
	LIBS += -lcrypto
	LIBS += -lefsrv

	PLUGIN_STUB_PATH = /resource/qt/plugins/controlpanel
		
	deploy.path = C:
	pluginstub.sources = $${TARGET}.dll
	pluginstub.path = $$PLUGIN_STUB_PATH
	DEPLOYMENT += pluginstub

    qtplugins.path = $$PLUGIN_STUB_PATH
    qtplugins.sources += qmakepluginstubs/$${TARGET}.qtplugin
     
    for(qtplugin, qtplugins.sources):BLD_INF_RULES.prj_exports += "./$$qtplugin  $$deploy.path$$qtplugins.path/$$basename(qtplugin)" 
	}

TRANSLATIONS= certificate_manager.ts
