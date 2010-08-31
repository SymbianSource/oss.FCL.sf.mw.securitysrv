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
TARGET = srvauthfaildlglauncher
CONFIG += hb
DEPENDPATH += .
INCLUDEPATH += .

HEADERS += srvauthfaildlglauncher.h
SOURCES += srvauthfaildlglaunchermain.cpp srvauthfaildlglauncher.cpp

symbian {
    TARGET.CAPABILITY = CAP_APPLICATION
}

BLD_INF_RULES.prj_exports += \
    "$${LITERAL_HASH}include <platform_paths.hrh>" \
    "srvauthfaildlglauncher.iby CORE_APP_LAYER_IBY_EXPORT_PATH(srvauthfaildlglauncher.iby)" \
    "testCerts/RootCert.cer /epoc32/winscw/c/data/testCerts/RootCert.cer" \
    "testCerts/IntermediateCert.cer /epoc32/winscw/c/data/testCerts/IntermediateCert.cer" \
    "testCerts/ServerCert1.cer /epoc32/winscw/c/data/testCerts/ServerCert1.cer" \
    "testCerts/ServerCert2_noCN.cer /epoc32/winscw/c/data/testCerts/ServerCert2_noCN.cer" \
    "testCerts/ServerCert3_exp.cer /epoc32/winscw/c/data/testCerts/ServerCert3_exp.cer" \
    "testCerts/ServerCert4_exp2.cer /epoc32/winscw/c/data/testCerts/ServerCert4_exp2.cer" \
    "testCerts/ServerCert5_alg.cer /epoc32/winscw/c/data/testCerts/ServerCert5_alg.cer" \
    "testCerts/ServerCert6_key.cer /epoc32/winscw/c/data/testCerts/ServerCert6_key.cer" \
    "testCerts/SevverCert7_ocsp.cer /epoc32/winscw/c/data/testCerts/ServerCert7_ocsp.cer" \
    "testCerts/ServerCert8_aia.cer /epoc32/winscw/c/data/testCerts/ServerCert8_aia.cer" \
    "testCerts/ServerCert9_san.cer /epoc32/winscw/c/data/testCerts/ServerCert9_san.cer"
