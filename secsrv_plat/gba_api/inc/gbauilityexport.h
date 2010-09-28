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
* Description: dll export flag definition
*/

#ifndef GBAUTILITYEXPORT_H
#define GBAUTILITYEXPORT_H

#if defined(BUILD_QTGBA_DLL)
#define QTGBADLL_EXPORT Q_DECL_EXPORT
#else
#define QTGBADLL_EXPORT Q_DECL_IMPORT
#endif

#endif //GBAUTILITYEXPORT_H

