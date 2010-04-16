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
*
* Description: SecUi notification keys.
*
*/

#ifndef SECUINOTIFICATIONPLUGINKEYS_H
#define SECUINOTIFICATIONPLUGINKEYS_H

#include <QString>

// Device dialog type
#define SECUINOTIFICATIONDIALOG "com.nokia.secuinotificationdialog/1.0"

// Keys for the parameters passed to notification framework plugin
const QString KDialogTitle = "title";
const QString KApplicationName = "application";
const QString KQueryType = "type";
const QString KApplicationIcon = "icon";
const QString KSupplier = "supplier";
const QString KMemorySelection = "memory";
const QString KCertificates = "certificates";
const QString KDrmDetails = "drmDetails";
const QString KCodeTop = "codeTop";

// Keys for the return values passed back to calling application
const QString KResultAccepted = "accepted";         // bool
const QString KSelectedMemoryIndex = "memory";      // int
const QString KCodeTopIndex = "codeTop";      // int

// Error values
const int KNoError = 0;

#endif // SECUINOTIFICATIONPLUGINKEYS_H
