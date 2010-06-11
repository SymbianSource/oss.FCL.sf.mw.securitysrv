#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
# All rights reserved.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, version 2.1 of the License.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, 
# see "http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html/".
#
# Description:
#

TEMPLATE=app
TARGET=Autolock

CONFIG += service
CONFIG += Hb

CONFIG += mobility
MOBILITY = publishsubscribe

XQSERVICE_ROOT=../..
include(../../xqservicebase.pri)
include(src/Autolock.pri)

LIBS+=-lxqservice -lxqserviceutil -lflogger
LIBS += -L../../../../../bin/release -lautolockuseractivityservice
LIBS += -lsecui -letelmm -letel -lcustomapi -lcentralrepository

SERVICE.FILE = service_conf.xml
SERVICE.OPTIONS = embeddable
SERVICE.OPTIONS += hidden

libFiles.sources = Autolock.exe 
libFiles.path = "!:\sys\bin"
DEPLOYMENT += libFiles

RESOURCES += Autolock.qrc

symbian*: {
				TARGET.CAPABILITY = CAP_APPLICATION
				TARGET.UID3 = 0x100059B5
				crmlFiles.sources = autolock.qcrml
				crmlFiles.path = /resource/qt/crml
				DEPLOYMENT += crmlFiles
}

BLD_INF_RULES.prj_exports += "./rom/Autolock.iby CORE_APP_LAYER_IBY_EXPORT_PATH(Autolock.iby)"
BLD_INF_RULES.prj_exports += "./rom/AutolockSrv.iby CORE_APP_LAYER_IBY_EXPORT_PATH(AutolockSrv.iby)"
BLD_INF_RULES.prj_exports += "./PubSub/SecurityUIsPrivatePSKeys.h |../../inc/securityuisprivatepskeys.h"
