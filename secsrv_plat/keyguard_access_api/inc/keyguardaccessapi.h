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
* Description:  Interface to access keyguard.
 *
*/


#ifndef __KEYGUARDACCESSAPI_H__
#define __KEYGUARDACCESSAPI_H__

#include <e32base.h>

class RLockAccessExtension;

/**
 *  RKeyguardAccessApi class.
 *  Client used to access/configure keyguard features.
 *
 *  @lib   lockapp
 *  @since 5.0
 */
class CKeyguardAccessApi : public CBase
    {
    public:

        /**
         * Two-phased constructor.
         */
        IMPORT_C static CKeyguardAccessApi* NewL( );

        /**
         * Destructor.
         */
        IMPORT_C ~CKeyguardAccessApi( );

    private:

        /**
         * Constructor for performing 1st stage construction
         */
        CKeyguardAccessApi( );

        /**
         * Default constructor for performing 2nd stage construction
         */
        void ConstructL( );

    public:

        /**
         * Returns true if phone is keys are locked. Both keyguard
         * and devicelock cannot be simultaneously enabled.
         * @return true if either keyguard or devicelock is enabled.
         */
        IMPORT_C TBool IsKeylockEnabled();

        /**
         * Has keyguard been activated.
         * @return true if keyguard is enabled.
         */
        IMPORT_C TBool IsKeyguardEnabled();

        /**
         * Enable keyguard. Only works if devicelock is not enabled.
         * @param  aWithNote should informing note be shown
         * @return KErrNone if succeeded
         *         KErrAlreadyExists if already enabled
         *         KErrPermissionDenied if devicelock is activated
         *         Other errors
         */
        IMPORT_C TInt EnableKeyguard( TBool aWithNote );

        /**
         * Disable keyguard. Only works if devicelock is not enabled.
         * @param  aWithNote should informing note be shown
         * @return KErrNone if succeeded
         *         KErrAlreadyExists if already disabled
         *         KErrPermissionDenied if devicelock is activated
         *         Other errors
         */
        IMPORT_C TInt DisableKeyguard( TBool aWithNote );

        /**
         * Offer to enable keyguard. Shows a query note to user.
         * @return KErrNone if succeeded
         *         KErrPermissionDenied if keyguard/devicelock already activated
         *         KErrCancel if user replies "No"
         *         Other errors
         */
        IMPORT_C TInt OfferKeyguard();

        /**
         * Shows note informing that keys are locked.
         * Only works if keyguard is already enabled.
         * @return KErrNone if succeeded
         *         KErrPermissionDenied if keyguard is not activated
         *         Other errors
         */
        IMPORT_C TInt ShowKeysLockedNote();


        /**
         * Test functions, return KErrNotSupported in release builds.
         */
        IMPORT_C TInt TestInternal( );

        IMPORT_C TInt TestDestruct( );

        IMPORT_C TInt TestPanicClient( );

        /**
         * Confirms that the server is running
         * @return KErrNone if succeeded
         */
        IMPORT_C TInt AutolockStatus( TInt aCommand, TInt aParam);

    private:

        RLockAccessExtension* iLockAccessExtension;

    };

#endif // __KEYGUARDACCESSAPI_H__
