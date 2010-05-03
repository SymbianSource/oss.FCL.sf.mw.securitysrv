/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: autolockuseractivityservice_global.h
*
*/


#ifndef AUTOLOCKAUTOLOCKUSERACTIVITYSERVICE_GLOBAL_H
#define AUTOLOCKAUTOLOCKUSERACTIVITYSERVICE_GLOBAL_H

#include <qglobal.h>

#ifdef AUTOLOCKUSERACTIVITYSERVICE_LIB
    #define AUTOLOCKUSERACTIVITYSERVICE_EXPORT Q_DECL_EXPORT
#else
    #ifdef AUTOLOCKUSERACTIVITYSERVICE_TEST
        #define AUTOLOCKUSERACTIVITYSERVICE_EXPORT
    #else
        #define AUTOLOCKUSERACTIVITYSERVICE_EXPORT Q_DECL_IMPORT
    #endif
#endif

#endif // AUTOLOCKAUTOLOCKUSERACTIVITYSERVICE_GLOBAL_H
