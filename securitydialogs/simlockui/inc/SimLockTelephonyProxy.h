/*
* ============================================================================
*   Name       : SimLockTelephonyProxy.h
*  Part of     : Sim Lock UI Telephony Proxy
*  Description : Wrap asynchronous calls to Core Telephony
*  Version     : 
*  
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Build info file for Ado domain appinstall 
* ============================================================================
*/

#ifndef SIMLOCK_TELEPHONY_PROXY_H
#define SIMLOCK_TELEPHONY_PROXY_H

// System Includes
#include <e32base.h>
#include <etel3rdparty.h> // CTelephony members

// Forward Declarations
class CPeriodic;

/**
 *  CSimLockTelephonyProxy
 *
 *  Act as proxy for asynchronous calls to CTelephony
 *
 *  @lib euser.lib
 *  @lib etel3rdparty.lib
 */
class CSimLockTelephonyProxy : public CActive
    {
public: // Public Constructor/Destructor

    /**
     * NewL
     * Construct a CSimLockTelephonyProxy
     * using two phase construction, and return a pointer to the created object
     * @return a pointer to the created instance of CSimLockTelephonyProxy
     */
    static CSimLockTelephonyProxy* NewL();

    /**
     * Destructor
     */
    ~CSimLockTelephonyProxy();

public: // Public API

    /**
     * Find out if a call is in progress.
     *
     * @return Whether or not a call is in progress
     */
    TBool IsCallInProgress();

private: // From CActive

    /**
     * Active object run method
     */
    virtual void RunL();
    
    /** 
     * Active object cancel method
     */
    virtual void DoCancel();

 private: // Private Constructors

    /**
     * C++ constructor.
     *     
     */
    CSimLockTelephonyProxy();

    /**
     * Symbian OS default constructor.
     */
    void ConstructL();

private: // Private Member Functions

    /**
     * CPeriodic Callback function for elapsed timer
     *
     * @param aClientObject pointer to CSimLockTelephonyProxy client object
     * @return required but unused
     */
    static TInt TimerElapsed(TAny* aClientObject);

    /**
     * Complete an asynchronous request synchronously with a hardcoded timeout
     */
    void CompleteRequestWithTimeout();

private: // Member Data

    /**
     * Telephony indicators( used to detect ongoing voice call )
     */
    CTelephony::TIndicatorV1 iIndicators;

    /**
     * Telephony indicator package as received from CTelephony
     */
    CTelephony::TIndicatorV1Pckg iIndicatorPackage;

     /**
     * ETel 3rd Party API( used to detect ongoing voice call )
     * owns
     */
    CTelephony* iTelephony;

    /**
     * Active scheduler wait API
     * owns
     */
    CActiveSchedulerWait* iSchedulerWait;

    /**
     * Request timeout timer
     * owns
     */
    CPeriodic* iTimer;

    };

#endif // SIMLOCK_TELEPHONY_PROXY_H

// End of file.
