/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*     Mixin class for passing memory status updates to observers.
*
*/


#ifndef __DEVENCUI_MEMINFOOBSERVER_H__
#define __DEVENCUI_MEMINFOOBSERVER_H__

#include <DevEncConstants.h>

class MDevEncUiMemInfoObserver
    {
    public:
        virtual void UpdateInfo( TDevEncUiMemoryType aType,
                                 TUint aState,
                                 TUint aProgress = 0 ) = 0;
    };

#endif // __DEVENCUI_MEMINFOOBSERVER_H__
