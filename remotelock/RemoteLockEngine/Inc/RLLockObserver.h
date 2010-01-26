/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  The header file of unlocking event observer
*
*/


#ifndef     RLLOCKOBSERVER_H
#define     RLLOCKOBSERVER_H

#include    <e32base.h>
#include    <e32property.h>

// CONSTANTS 

// MACROS 

// FORWARD DECLARATIONS 
class MRLLockObserverHandler;

// LOCAL FUNCTION PROTOTYPES 

// MEMBER FUNCTIONS 


// CLASS DECLARATION
/**
* Observe the unlocking event
*
* @since S60 3.2
*/
class CRLLockObserver: public CActive
    {
    public:
       /**
        * Two-Phased constructor
        *
        * @since S60 3.2
        * @param aHandler Client implemented observer handler that will be
        *                 called when phone are unlocked. 
        * @return
        */
        static CRLLockObserver* NewL( MRLLockObserverHandler* aHandler );
    
        ~CRLLockObserver();    
    
    private:
    
        /**
        * Starts asynchronic listening 
        * KCoreAppUIsAutolockStatus/
        * KPSUidAutolockStatusValue event
        *
        * @Since S60 3.2
        * @return KErrNone: if no errors
        *         KErrInUse: if already listening
        */
        TInt Start();            
    
    private: 

        CRLLockObserver( MRLLockObserverHandler* aHandler );
        
        void ConstructL();
        
    private: // from CActive
         
        void RunL();
    
        void DoCancel();
        
    private: // data
    
        RProperty                iProperty;
        MRLLockObserverHandler*  iHandler; //not owned
    };

// CLASS DECLARATION

/**
* Abstract interface class declares a callback function for 
* remotelock observer to call when the phone is unlocked. User
* derives his class from this and implements HandleUnlockEvent().
*
* @since S60 3.2
*/
class MRLLockObserverHandler 
    {
    public:
    
    /**
    * The function is called when the phone is unlocked.
    *
    * @since S60 3.2
    */
    virtual void HandleUnlockEvent() = 0;
    };

#endif //RLLOCKOBSERVER_H
// End of file
