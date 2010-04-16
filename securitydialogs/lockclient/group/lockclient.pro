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
# Description:   
#                


TEMPLATE = lib
TARGET = lockclient 	

TARGET.UID3 = 0x2000B124

TARGET.CAPABILITY = CAP_GENERAL_DLL


DEPENDPATH += .
DEPENDPATH += ../src
DEPENDPATH += ../inc

INCLUDEPATH += .

CONFIG += hb

# Input
SOURCES += devicelockaccessapi.cpp \
           keyguardaccessapi.cpp \
           lockaccessextension.cpp

HEADERS = lockaccessextension.h

LIBS += -lxqservice
LIBS += -lxqserviceutil

symbian*: { 
	TARGET.EPOCALLOWDLLDATA = 1
	MMP_RULES -= "OPTION_REPLACE ARMCC --export_all_vtbl -D__QT_NOEFFECTMACRO_DONOTUSE"
	}
