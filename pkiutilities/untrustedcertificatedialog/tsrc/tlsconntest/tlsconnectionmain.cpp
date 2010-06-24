/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description:  Secure connections test application
*
*/

#include "tlsconnectionapp.h"
#include <QDebug>
#include <e32base.h>                // CTrapCleanup


int DoMainL(int argc, char *argv[])
{
    TlsConnectionApplication app(argc, argv);
    return app.exec();
}

int main(int argc, char *argv[])
{
    qDebug() << "TLStest main begin";

    int err = 0;
    CTrapCleanup *cleanup = CTrapCleanup::New();
    if (cleanup) {
        TRAP(err, DoMainL(argc, argv));
        delete cleanup;
    }

    qDebug() << "TLStest main returns" << err;
    return err;
}

