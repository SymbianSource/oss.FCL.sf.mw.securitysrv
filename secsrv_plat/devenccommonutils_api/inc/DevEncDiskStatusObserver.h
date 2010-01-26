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


#ifndef DEVENCDISKSTATUSOBSERVER_H_
#define DEVENCDISKSTATUSOBSERVER_H_

#include <e32base.h>
#include <e32property.h>
#include <f32file.h>

class MDiskStatusObserver
    {
    public:
        /**
         * Implement this method to be notified when disk status
         * changes.
         */
        IMPORT_C virtual void DiskStatusChangedL( TInt aNfeStatus ) = 0;
    };

class CDiskStatusObserver : public CActive
    {
    public:
        IMPORT_C static CDiskStatusObserver* NewL( MDiskStatusObserver* aObserver, TDriveNumber aDriveNumber );

        void RunL();
        void DoCancel();
        ~CDiskStatusObserver();
    
    private:
        void ConstructL( MDiskStatusObserver* aObserver, TDriveNumber aDriveNumber );   
        CDiskStatusObserver();
        TBool IsEncryptionOperationOngoing();

    private:
        MDiskStatusObserver* iObserver;
        RProperty iNfeStatus;
        TDriveNumber iDrive;
    };

#endif /*DEVENCDISKSTATUSOBSERVER_H_*/
