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
# Description:  Test application for untrusted certificate dialog.
#

TEMPLATE = app
TARGET = dialoglauncher
CONFIG += hb
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += dialoglauncher.h
SOURCES += dialoglaunchermain.cpp dialoglauncher.cpp

symbian {
    TARGET.CAPABILITY = CAP_APPLICATION
}

BLD_INF_RULES.prj_exports += \
  "$${LITERAL_HASH}include <platform_paths.hrh>" \
  "dialoglauncher.iby CORE_APP_LAYER_IBY_EXPORT_PATH(dialoglauncher.iby)" \
  "test_cert_expired.crt /epoc32/winscw/c/data/testCerts/test_cert_expired.crt" \
  "test_cert_server.crt /epoc32/winscw/c/data/testCerts/test_cert_server.crt" \
  "test_cert_sha2.crt /epoc32/winscw/c/data/testCerts/test_cert_sha2.crt"

