/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
*     Declares UI class for application.
*
*/

#ifndef AUTOLOCKGRIPOBSERVER_H_
#define AUTOLOCKGRIPOBSERVER_H_

#include <e32base.h>
#include <e32property.h>
#include <w32std.h>
#include <hwrmdomainpskeys.h>

class MAutolockAppUiInterface;

class CAutolockGripStatusObserver : public CActive
    {
    public:
        IMPORT_C static CAutolockGripStatusObserver* NewL( MAutolockAppUiInterface* aObserver, RWsSession& aSession );

        void RunL();
        void DoCancel();
        ~CAutolockGripStatusObserver();
    
    private:
        void ConstructL( MAutolockAppUiInterface* aObserver );   
        CAutolockGripStatusObserver( RWsSession& aSession );
        void GripStatusChangedL( TInt aGripStatus );

    private:
        MAutolockAppUiInterface* iObserver;
        RProperty iGripStatus;
        RWsSession& iSession;
    };


#endif 
