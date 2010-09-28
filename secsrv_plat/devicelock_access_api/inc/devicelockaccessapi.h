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
* Description:  Interface to access devicelock.
 *
*/


#ifndef __DEVICELOCKACCESSAPI_H__
#define __DEVICELOCKACCESSAPI_H__

#include <e32base.h>

class RLockAccessExtension;

/**
 *  Three possible reason for devicelock: manual, remote, timer
 *
 *  @lib   lockapp
 *  @since 5.0
 */
enum TDevicelockReason
    {
    EDevicelockManual = 1,
    EDevicelockRemote
    };

/**
 *  RDevicelockAccessApi class.
 *  Client used to access/configure devicelock features.
 *
 *  @lib   lockapp
 *  @since 5.0
 */
class CDevicelockAccessApi : public CBase
    {
    public:

        /**
         * Two-phased constructor.
         */
        IMPORT_C static CDevicelockAccessApi* NewL( );

        /**
         * Destructor.
         */
        IMPORT_C ~CDevicelockAccessApi( );

    private:

        /**
         * Constructor for performing 1st stage construction
         */
        CDevicelockAccessApi( );

        /**
         * Default constructor for performing 2nd stage construction
         */
        void ConstructL( );

    public:

        /**
         * Returns true if phone is keys are locked. Both keyguard
         * and devicelock cannot be simultaneously enabled.
         * @return ETrue if either keyguard or devicelock is enabled.
         */
        IMPORT_C TBool IsKeylockEnabled( );

        /**
         * Has devicelock been activated.
         * @return ETrue if devicelock is enabled.
         */
        IMPORT_C TBool IsDevicelockEnabled( );

        /**
         * Enable devicelock.
         * @return KErrNone if succeeded
         *         KErrAlreadyExists if already enabled
         *         KErrPermissionDenied if no required capabilities
         *         Other errors
         */
        IMPORT_C TInt EnableDevicelock( TDevicelockReason aReason = EDevicelockManual );

        /**
         * Offer to enable devicelock. Shows a query note to user, and then a unlock-query.
         * @return KErrNone if succeeded
         *         KErrAlreadyExists if already enabled
         *         KErrPermissionDenied if no required capabilities
         *         Other errors
         */
        IMPORT_C TInt OfferDevicelock( );

        /**
         * Disable devicelock. Shows a unlock-query to user.
         * @return KErrNone if succeeded
         *         KErrAlreadyExists if already enabled
         *         KErrPermissionDenied if no required capabilities
         *         Other errors
         */
        IMPORT_C TInt DisableDevicelock( );

    private:

        RLockAccessExtension* iLockAccessExtension;

    };

#endif // __DEVICELOCKACCESSAPI_H__
