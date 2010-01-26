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
* Description:
*
*/
#ifndef DEVENCSTARTERMMCOBSERVER_H_
#define DEVENCSTARTERMMCOBSERVER_H_

#include "DevEncMmcObserver.h"
#include "DevEncStarterUtils.h"
#include "DevEncUiMemInfoObserver.h"
#include <f32file.h>

// Forward declarations
class CDevEncSession;
//class CDevEncStarterUtils;
class CDevEncStarterMemoryEntity;
class CRepository;
class TMmcInfo;

class CDevEncStarterMmcObserver :  public CBase,
                                   public MMemoryCardObserver,
                                   public MDevEncUiMemInfoObserver,
                                   public MUtilsCallback
                                 
    {
    public:
        static CDevEncStarterMmcObserver* NewL(
                                              CDevEncStarterUtils*& aUtils );
        ~CDevEncStarterMmcObserver();

    // From MMemoryCardObserver

        void MMCStatusChangedL();
        
        /**
         * Called periodically. Not intended for use by other classes.
         * @param aPtr Pointer to an instance of this class.
         */
        static TInt PollTick( TAny* aPtr );

        /**
        * From MDevEncUiMemInfoObserver.
        */
        void UpdateInfo( TDevEncUiMemoryType aType,
                         TUint aState,
                         TUint aProgress );

        /**
        * From MUtilsCallback.
        */
        void AppStarted( const TUint32& aUid );
        
    private:
    // Functions
        CDevEncStarterMmcObserver( CDevEncStarterUtils*& aUtils );
        void ConstructL();   
        TInt GetInfo( TMmcInfo& aInfo );
        void LogInfo( const TMmcInfo& aInfo ) const;
        TInt CheckMMC();

        TBool NeedToStartUi( TDevEncUiMemoryType aType,
                             TUint aState );

        TBool NeedToBlockPowerKey();

    // Data
        /** Owned */
        CMmcObserver* iObserver;

        /** Owned */
        CRepository* iCrSettings;
        
        RFs iFs;

        /** Owned */
        RArray<CDevEncStarterMemoryEntity*> iMemEntities;

        TUint iMmcEncState;
        TUint iPhoneEncState;
        TInt iMmcStatus;

        /** Not owned */
        CDevEncStarterUtils*& iUtils;
        
        TInt32 iPowerKeyHandle;
    };

#endif /*DEVENCSTARTERMMCOBSERVER_H_*/
