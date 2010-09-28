#
# ============================================================================
#  Name        : indicatorautolockplugin.pro
#  Part of     : hb / 
#  Description : indicator autolock plugin
#  Version     : %version: 2 %
#
#  Copyright © 2009 Nokia.  All rights reserved.
#  This material, including documentation and any related computer
#  programs, is protected by copyright controlled by Nokia.  All
#  rights are reserved.  Copying, including reproducing, storing,
#  adapting or translating, any or all of this material requires the
#  prior written consent of Nokia.  This material also contains
#  confidential information which may not be disclosed to others
#  without the prior written consent of Nokia.
# ============================================================================
#

TEMPLATE = lib

CONFIG += Hb

TARGET = IndicatorAutolockPlugin
CONFIG += plugin
win32 {
    debug {
        DESTDIR = ../indicatorplugin/debug
    }
    else {
        DESTDIR = ../indicatorplugin/release
    }
}
else {
    DESTDIR = ../indicatorplugin
}

HEADERS += hbindicatorautolockplugin.h autolockindicators.h
SOURCES += hbindicatorautolockplugin.cpp

symbian {
    TARGET.EPOCALLOWDLLDATA=1
    TARGET.CAPABILITY = CAP_GENERAL_DLL

    hblib.sources = Hb.dll
    hblib.path = \sys\bin
    hblib.depends = "(0xEEF9EA38), 1, 0, 0, {\"Hb\"}"

    pluginstub.sources = indicatorautolockplugin.dll
    pluginstub.path = /resource/plugins/indicators
    DEPLOYMENT += pluginstub
}

!local {
    target.path = $${HB_PLUGINS_DIR}/indicators
    INSTALLS += target
}

BLD_INF_RULES.prj_exports += \
    "$${LITERAL_HASH}include <platform_paths.hrh>" \
    "rom/indicatorautolockplugin.iby CORE_APP_LAYER_IBY_EXPORT_PATH(indicatorautolockplugin.iby)"

symbian:MMP_RULES += SMPSAFE
