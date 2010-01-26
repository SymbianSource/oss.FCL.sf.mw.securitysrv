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

#ifndef AUTOLOCKFPSOBSERVER_H_
#define AUTOLOCKFPSOBSERVER_H_

#include <e32base.h>
#include <e32property.h>
#include <w32std.h>
#include <hwrmdomainpskeys.h>


class MAutolockFpsStatusObserver
    {
    public:
        /**
         * Implement this method to be notified when Fps status
         * changes.
         */
        IMPORT_C virtual TInt DeviceFpsLock(TInt iStatus) = 0;
        IMPORT_C virtual TInt DeviceLockStatus() = 0;
    };

class CAutolockFpsStatusObserver : public CActive
    {
    public:
        IMPORT_C static CAutolockFpsStatusObserver* NewL( MAutolockFpsStatusObserver* aObserver, RWsSession& aSession );

        void RunL();
        void DoCancel();
        ~CAutolockFpsStatusObserver();
    
    private:
        void ConstructL( MAutolockFpsStatusObserver* aObserver );   
        CAutolockFpsStatusObserver( RWsSession& aSession );
        void FpsStatusChangedL( TInt aFpsStatus );

    private:
        MAutolockFpsStatusObserver* iObserver;
        RProperty iFpsStatus;
        RWsSession& iSession;
    };


#endif 
