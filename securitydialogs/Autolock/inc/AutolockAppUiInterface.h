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
* Description: 
*     Interface to App Ui from helper classes
*
*/
#ifndef AUTOLOCKAPPUIINTERFACE_H
#define AUTOLOCKAPPUIINTERFACE_H

class MAutolockAppUiInterface
    {
    public:
        virtual TInt DeviceLockQueryStatus() = 0;
        virtual TInt DeviceLockStatus() = 0;
        virtual void CancelDeviceLockQuery() = 0;
    };

#endif // AUTOLOCKAPPUIINTERFACE_H
