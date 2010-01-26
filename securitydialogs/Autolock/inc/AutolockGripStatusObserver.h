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

//CONSTANTS
const TInt KCancelKeyCode( 165 );

class MAutolockGripStatusObserver
    {
    public:
        /**
         * Implement this method to be notified when grip status
         * changes.
         */
        IMPORT_C virtual TInt DeviceLockQueryStatus() = 0;
        IMPORT_C virtual TInt DeviceLockStatus() = 0;
    };

class CAutolockGripStatusObserver : public CActive
    {
    public:
        IMPORT_C static CAutolockGripStatusObserver* NewL( MAutolockGripStatusObserver* aObserver, RWsSession& aSession );

        void RunL();
        void DoCancel();
        ~CAutolockGripStatusObserver();
    
    private:
        void ConstructL( MAutolockGripStatusObserver* aObserver );   
        CAutolockGripStatusObserver( RWsSession& aSession );
        void GripStatusChangedL( TInt aGripStatus );

    private:
        MAutolockGripStatusObserver* iObserver;
        RProperty iGripStatus;
        RWsSession& iSession;
    };


#endif 
