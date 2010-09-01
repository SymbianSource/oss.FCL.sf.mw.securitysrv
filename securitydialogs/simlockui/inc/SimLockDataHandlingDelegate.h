/*
* ============================================================================
*  Name         : SimLockDataHandlingDelegate.h     
*  Part of      : Sim Lock UI Application
*  Description  : Handles reading and writing of Sim Lock data including
                  unlocking the Sim Lock.
*  Version      : 
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

#ifndef SIMLOCKDATAHANDLINGDELEGATE_H
#define SIMLOCKDATAHANDLINGDELEGATE_H

// System Includes
#include <e32base.h>

// Forward Declarations
class RMmCustomAPI;
struct TSimPathStr;
struct TSimLockHeadStr;
class CActiveSchedulerWait;
class TIsiReceiveC;
class CPeriodic;

/**
 *  CSimLockDataHandlingDelegate
 *
 *  Delegate class to handle communication with lower layers of S60 and Symbian,
 *  and also the native( ISA ) layer.
 *
 *  @lib euser.lib
 *  @lib iscapi.lib
 *  @lib isimessage.lib
 *  @lib etel.lib
 *  @lib etelmm.lib
 *  @lib customapi.lib
 */
class CSimLockDataHandlingDelegate : public CActive
    {
public: // Public Constructor / Destructor

    /**
     * Two-phased constructor
     *
     */
    static CSimLockDataHandlingDelegate* NewL( RMmCustomAPI& aCustomAPI );

    /**
     * Destructor
     *
     */
    virtual ~CSimLockDataHandlingDelegate();

public: // Public API

     /**
      * Open the Sim Lock( type 1 ) given a passcode.
      *
      * @param aPassword TDesC of the password to open lock
      * @return Indicates the result of the Sim Lock open attempt
      */
    TInt OpenSimLock( const TDesC& aPassword );

     /**
      * Determine if the Sim Lock is open.
      *
      * @return Indicates if Sim Lock is open
      */
    TBool IsSimLockOpen() const;


private: // Private Constructors

     /**
      * C++ constructor.
      *
      * @param aCustomAPI ETel custom API object
      */
    CSimLockDataHandlingDelegate( RMmCustomAPI& aCustomAPI );

     /**
      * Symbian OS default constructor.
      */
    void ConstructL();


private: // From CActive

    /**
     * Active object run method
     * @see CActive
     */
    virtual void RunL();

    /**
     * Active object cancel method
     * @see CActive
     */
    virtual void DoCancel();
    
private: // Local Member API
       
    /**
     * CPeriodic Callback function for elapsed timer
     *
     * @param aUnused requred for interface
     * @return required but unused
     */
    static TInt TimerElapsed(TAny* aUnused);

    /**
     * Pause execution and allow active scheduler to run until the current outstanding
     * request complets
     *
     */
    void CompleteRequestWithTimeout();

private:    // Member Data
    /**
     * Indicates if the Sim Lock is open per the last read from the Sim Lock
     * server.
     */
    TBool iLockIsOpen;

    /**
     * Handle to ETel MultiMode Custom API server
     */
    RMmCustomAPI& iCustomAPI;

    /**
     * Help with asynchronous requests
     */
    CActiveSchedulerWait* iSchedulerWait;

    /**
     * Indicates that Sim Lock data has been read
     */
    TBool iDataHasBeenRead;

    /**
     * Timeout handling timer
     * owns
     */
    CPeriodic* iTimer;

    };

#endif //SIMLOCKDATAHANDLINGDELEGATE_H

// End of File

