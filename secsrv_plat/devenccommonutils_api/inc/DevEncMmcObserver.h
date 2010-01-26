/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies).
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
*     
*
*/


#ifndef __MMCOBSERVER_H__
#define __MMCOBSERVER_H__

#include <e32base.h>

class RFs;

class MMemoryCardObserver
    {
    public:
        /**
         * Implement this method to be notified when MMC status
         * changes.
         */
        virtual void MMCStatusChangedL() = 0;
    };

class CMmcObserver : public CActive
    {
    public:
        IMPORT_C static CMmcObserver* NewL( MMemoryCardObserver* aObserver,
                                   RFs* aFileServerSession );
        IMPORT_C void StartObserver();

        // TBI
        IMPORT_C TBool MMCCurrentlyInserted();

        void RunL();
        void DoCancel();
        ~CMmcObserver();
    
    private:
        void ConstructL( MMemoryCardObserver* aObserver,
                         RFs* aFileServerSession );   
        CMmcObserver();
        MMemoryCardObserver* iObserver;
        RFs* iFsSession;
        CAsyncCallBack* iAsyncCallBack;
    };

#endif // __MMCOBSERVER_H__

// End of File
