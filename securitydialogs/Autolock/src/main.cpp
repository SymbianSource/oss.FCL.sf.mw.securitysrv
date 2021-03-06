/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, version 2.1 of the License.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, 
* see "http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html/".
*
* Description:
*
*/

// #include "xqservicelog.h"
#include <QDebug>

#include <QApplication>
#include "Autolock.h"
#include "../PubSub/securityuisprivatepskeys.h"

#include <hbapplication.h>
#include <hbmainwindow.h>
#include <apgwgnam.h>
#include <eikenv.h>

int main(int argc, char **argv)
{
    // qInstallMsgHandler(XQSERVICEMESSAGEHANDLER);
    // XQSERVICE_DEBUG_PRINT(" ================== xxxx Autolock::main");
    qDebug() << "================== xxxx QApplication Autolock::main";
    
    
    // Need to check whether process is already running. This happens if it's started from Stater, and 
    // before fully initialized, it's started by API through QtHighway
    TSecurityPolicy readPolicy(ECapabilityReadDeviceData);
    TSecurityPolicy writePolicy(ECapabilityWriteDeviceData);
    int ret = RProperty::Define(KPSUidSecurityUIs,
            KSecurityUIsLockInitiatorUID, RProperty::EInt, TSecurityPolicy(TSecurityPolicy::EAlwaysPass),
            writePolicy);
		qDebug() << "KSecurityUIsLockInitiatorUID ret=" << ret;
    int myInitiatorUID = 0;
    int err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsLockInitiatorUID, myInitiatorUID);
		qDebug() << "KSecurityUIsLockInitiatorUID err=" << err;
		qDebug() << "KSecurityUIsLockInitiatorUID myInitiatorUID=" << myInitiatorUID;
		err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsLockInitiatorUID, 0);

    // it takes about 3 seconds to start it, on device
    QApplication a( argc, argv );
    // hide server from TaskSwitcher
    CEikonEnv * env = CEikonEnv::Static();
    if(env) {
        env->RootWin().SetOrdinalPosition(0, ECoeWinPriorityNeverAtFront);

        CApaWindowGroupName *wgName = CApaWindowGroupName::NewLC(env->WsSession());    
        wgName->SetHidden(ETrue); // hides us from FSW and protects us from OOM FW etc.
        wgName->SetSystem(ETrue); // Allow only application with PowerManagement cap to shut us down    
        // wgName->SetCaptionL(KRunningAppServerName);
        wgName->SetWindowGroupName(env->RootWin());
        CleanupStack::PopAndDestroy(wgName);
    }

    Autolock *cl = new Autolock();
    // qDebug() << " Autolock::main cl->show";
    // cl->show();
    // qDebug() << " Autolock::main cl->hide";
    cl->hide();
    // qDebug() << " Autolock::main cl->lower";
    cl->lower();
    qDebug() << " Autolock::main cl->lower";
    err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsLockInitiatorUID, 1);
    int rv = a.exec();
    qDebug() << " Autolock::main cl->exec";
    delete cl;
    qDebug() << " Autolock::main cl->delete";
    return rv;
}

