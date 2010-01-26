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
*
*/



#ifndef __DEVENCSTARTERSTARTER_H__
#define __DEVENCSTARTERSTARTER_H__

// INCLUDES
#include <e32base.h>

// FORWARD DECLARATIONS

class MAppStarterCallback
    {
    public:
        virtual void AppStarted( const TUint32& aUid )=0;
    };

class CDevEncStarterStarter : public CActive
    {
    public:
        static CDevEncStarterStarter* NewL( MAppStarterCallback* aCallback );
        static CDevEncStarterStarter* NewLC( MAppStarterCallback* aCallback );
        ~CDevEncStarterStarter();

        void StartAppL( const TUint32& aUid );
        TBool CanStartAppL( const TUint32& aUid );

    protected:
        void RunL();
        void DoCancel();
        TInt RunError( TInt aError );

    private:
    // Functions
        CDevEncStarterStarter( MAppStarterCallback* aCallback );

        void ConstructL();

    // Data
        TUint32 iAppUidStarting;
        MAppStarterCallback* iCallback; // parent
    };

#endif //__DEVENCSTARTERSTARTER_H__

// End of file
