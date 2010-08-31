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
# Description:  Secure connections test application
#

TEMPLATE = app
TARGET = tlsconntest
CONFIG += hb
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += tlsconnection.h tlsconnectionobserver.h tlsconnectionapp.h tlsconntest.loc
SOURCES += tlsconnectionmain.cpp tlsconnection.cpp tlsconnectionapp.cpp
LIBS += -lesock -lcommdb -linsock -linetprotutil -lsecuresocket

symbian {
    BLD_INF_RULES.prj_exports += "tlsconntest.loc APP_LAYER_LOC_EXPORT_PATH(tlsconntest.loc)"
    
    TARGET.UID2 = 0x100039CE
    TARGET.UID3 = 0xE8DC94B1        # random uid
    TARGET.CAPABILITY = CAP_APPLICATION
}

BLD_INF_RULES.prj_exports += \
  "$${LITERAL_HASH}include <platform_paths.hrh>" \
  "tlsconntest.iby CORE_APP_LAYER_IBY_EXPORT_PATH(tlsconntest.iby)"

 