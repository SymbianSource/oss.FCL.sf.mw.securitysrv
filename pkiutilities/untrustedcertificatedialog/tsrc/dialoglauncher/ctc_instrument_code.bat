@echo off
rem
rem Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
rem All rights reserved.
rem This component and the accompanying materials are made available
rem under the terms of "Eclipse Public License v1.0"
rem which accompanies this distribution, and is available
rem at the URL "http://www.eclipse.org/legal/epl-v10.html".
rem
rem Initial Contributors:
rem Nokia Corporation - initial contribution.
rem
rem Contributors:
rem
rem Description:  TLS untrusted certificate dialog test code.
rem

echo ----------------------------------------------------------------------
echo.
echo Instrumenting code (armv5 urel and winscw udeb, decision coverage)
echo.
echo ----------------------------------------------------------------------
echo.
pushd ..\..
if exist MON.sym del MON.sym
if exist MON.dat del MON.dat
if exist profile.txt del profile.txt
call qmake
call make distclean
call qmake
call ctcwrap -i d -2comp -C "EXCLUDE+*.UID.CPP" -C "EXCLUDE+moc_*.cpp" sbs -c armv5_urel
call ctcwrap -i d -2comp -C "EXCLUDE+*.UID.CPP" -C "EXCLUDE+moc_*.cpp" sbs -c winscw_udeb
popd

