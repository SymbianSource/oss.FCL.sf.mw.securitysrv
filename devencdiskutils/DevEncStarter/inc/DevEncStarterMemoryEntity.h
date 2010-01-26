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
*     Class for managing different types of memory on a very abstract level.
*
*/


#ifndef DEVENCSTARTER_MEMORYENTITY_H
#define DEVENCSTARTER_MEMORYENTITY_H

#include <e32base.h>

#include <DevEncConstants.h>
#include "DevEncUiMemInfoObserver.h"
#include <DevEncDiskStatusObserver.h>

class CDevEncSession;
class CDevEncUiInfoObserver;

class CDevEncStarterMemoryEntity : public CBase,
                                   public MDiskStatusObserver
    {
    public:
        /**
         * Constructs the CDevEncStarterMemoryEntity using the NewLC method, popping
         * the constructed object from the CleanupStack before returning it.
         *
         * @return The newly constructed CDevEncStarterMemoryEntity
         */
    	static CDevEncStarterMemoryEntity* NewL( TDevEncUiMemoryType aType );

        /**
         * Constructs the CDevEncStarterMemoryEntity using the constructor and ConstructL
         * method, leaving the constructed object on the CleanupStack before returning it.
         *
         * @return The newly constructed CDevEncStarterMemoryEntity
         */
    	static CDevEncStarterMemoryEntity* NewLC( TDevEncUiMemoryType aType );

    	/**
         * Adds an observer to this object's list of observers.
         * @param aInfoObserver The observer to add
         * @leave Symbian error code
         */
        void AddObserverL( MDevEncUiMemInfoObserver* aInfoObserver );

        /**
         * Removes an observer from this object's list of observers.
         * Any errors are ignored.
         * @param aInfoObserver The observer to remove
         */
        void RemoveObserver( MDevEncUiMemInfoObserver* aInfoObserver );

        /**
         * Called periodically. Not intended for use by other classes.
         * @param aPtr Pointer to an instance of this class.
         */
        static TInt ProgressTick( TAny* aPtr );

        /**
         * Called periodically. Not intended for use by other classes.
         * @param aPtr Pointer to an instance of this class.
         */
        static TInt PollTick( TAny* aPtr );
        
        /**
        * Destructor.
        */
        virtual ~CDevEncStarterMemoryEntity();

        void Cancel();

        /**
         * Gets the state of this memory entity.
         * @return the current state
         */
        TUint State() const;
        
        /**
         * Sets the internal state and calls UpdateMemoryInfo()
         * @param aState the new state
         **/
        virtual void SetState( TUint aState );

        /**
         * Sends the current memory state information to all observers.
         * @return none
         */
        virtual void UpdateMemoryInfo();
        
        /*
         * From MDiskStatusObserver
         */
        void DiskStatusChangedL( TInt aStatus );

    private:
    // functions

        /**
         * C++ Constructor
         **/
        CDevEncStarterMemoryEntity( TDevEncUiMemoryType aType );

        /**
         * Second-phase constructor
         **/
        virtual void ConstructL();

        /**
         * Starts a timer to periodically update the memory state in the UI
         * @param aInterval the polling interval
         **/
        void StartPolling( TTimeIntervalMicroSeconds32 aInterval );

        /**
         * Starts a timer to periodically update the memory state in the UI
         * @param aInterval the polling interval
         **/
        void DoStartPollingL( TTimeIntervalMicroSeconds32 aInterval );

        /**
         * Fetches the encryption state and progress. Called periodically.
         **/
        virtual void CheckProgress();

        /**
         * Performs periodical processing of timeouts. Calls CheckProgress().
         **/
        virtual void DoProgressTick();

        /**
         * Performs periodical processing of timeouts.
         * Checks if the encryption status has changed.
         **/
        virtual void DoPollTick();

    // Data

        /** Owned */
        CPeriodic* iPeriodic;

        /** Owned */
        RArray<MDevEncUiMemInfoObserver*> iObservers;

        /** Owned */
        CDevEncSession* iSession;
        
        /** Owned */
        CDiskStatusObserver* iDiskStatusObserver;

        TUint iState;
        TUint iPrevState;
        const TDevEncUiMemoryType iType;
        TInt iPercentDone;
        TInt iPrevPercentDone;
    };

#endif	// DEVENCSTARTER_MEMORYENTITY_H
