#
# Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies). 
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
# Description: SecUi notification dialog plugin project.
#

TEMPLATE = lib
TARGET = secuinotificationdialogplugin
CONFIG += plugin
TRANSLATIONS = secui.ts

CONFIG += hb
CONFIG += mobility
MOBILITY = publishsubscribe

# directories
INCLUDEPATH += .
DEPENDPATH += .
INCLUDEPATH += $$APP_LAYER_SYSTEMINCLUDE
INCLUDEPATH += ../../../../inc

HEADERS += inc/secuinotificationdialogplugin.h \
    inc/secuinotificationdialog.h \
    inc/secuinotificationcontentwidget.h

SOURCES += src/secuinotificationdialogplugin.cpp \
    src/secuinotificationdialog.cpp \
    src/secuinotificationcontentwidget.cpp

RESOURCES += secuinotificationdialogplugin.qrc

symbian: {
    TARGET.EPOCALLOWDLLDATA = 1
    # NetworkControl NetworkServices needed for emergency calls
    TARGET.CAPABILITY = CAP_GENERAL_DLL NetworkControl NetworkServices
    TARGET.UID3 = 0x2102432C    # TODO: allocate UID

    pluginstub.sources = secuinotificationdialogplugin.dll
    pluginstub.path = /resource/plugins/devicedialogs
    DEPLOYMENT += pluginstub
    crml.sources = secuinotificationdialogplugin.qcrml
    crml.path = c:/resource/qt/crml
    DEPLOYMENT += crml
}
BLD_INF_RULES.prj_exports += \
  "$${LITERAL_HASH}include <platform_paths.hrh>" \
  "qmakepluginstubs/secuinotificationdialogplugin.qtplugin   /epoc32/data/z/pluginstub/secuinotificationdialogplugin.qtplugin"\
  "rom/secuinotificationdialogplugin.iby    CORE_APP_LAYER_IBY_EXPORT_PATH(secuinotificationdialogplugin.iby)"\
	"rom/secuinotificationdialogplugin_resources.iby LANGUAGE_APP_LAYER_IBY_EXPORT_PATH(secuinotificationdialogplugin_resources.iby)"
  
LIBS += -lxqservice
LIBS += -lphoneclient

tests {
    test.depends = sub-src
    test.commands += cd tsrc && $(MAKE) test
    autotest.depends = sub-src
    autotest.commands += cd tsrc && $(MAKE) autotest
    QMAKE_EXTRA_TARGETS += test autotest
}
