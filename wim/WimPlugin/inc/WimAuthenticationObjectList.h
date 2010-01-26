/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Authentication objects can be listed through this interface
*
*/


#ifndef CWIMAUTHENTICATIONOBJECTLIST_H
#define CWIMAUTHENTICATIONOBJECTLIST_H

//  INCLUDES

#include <mctauthobject.h>

// FORWARD DECLARATIONS

class CWimToken;
class CWimPin;
class CWimAuthenticationObject;

// CLASS DECLARATION

/**
*  Represents an interface to authentication objects
*
*
*  @lib   WimPlugin
*  @since Series 60 2.1
*/

class CWimAuthenticationObjectList : public CActive, 
                                     public MCTAuthenticationObjectList
    {
    public:

        /**
        * Default constructor
        * @param  aToken  A reference to current token
        * @return A pointer to this class
        */
        static CWimAuthenticationObjectList* NewL( CWimToken& aToken );

        /**
        * Destructor
        */
        virtual ~CWimAuthenticationObjectList();

    public: // From base class MCTTokenInterface
        
        /**
        * Returns a reference to current token.
        * @return A reference to current token.
        */
        MCTToken& Token();

    public: // From base class MCTAuthenticationObjectList

        /**
        * Lists authentication objects.
        * The caller of this function owns all parameters.
        * Altought asynchronous look and behaviour this call is 
        * synchronous.
        * @param  aAuthObjects (OUT) The returned authentication objects 
        *         will be appended to this array.
        * @param  aStatus (IN/OUT) This will be completed with the final 
        *         status code.
        *         KErrNone, if no errors detected. Note: thought none 
        *         authentication objects are found, it is also KErrNone
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void List( RMPointerArray<MCTAuthenticationObject>& aAuthObjects,
                   TRequestStatus& aStatus );

        /**
        * Cancels an ongoing List operation. Because List is synchronous, 
        * there is nothing to do here.
        * @return void
        */
        void CancelList();

    protected:  // From base class MCTTokenInterface
        
        virtual void DoRelease();

    private: // Constructors

        /**
        * Default constructor
        * @param  aToken  (IN) A reference to current token
        */
        CWimAuthenticationObjectList( CWimToken& aToken );

        // Second phase constructor
        void ConstructL();
    
    private: //from CActive 
       
       /**
        * Different phases are handled here.
        * @return void
        */
        void RunL();
        
        /**
        * Cancellation function
        * @return void
        */
        void DoCancel();

        /**
        * The active scheduler calls this function if this active 
        * object's RunL() function leaves. 
        * @param aError -Error code which caused this event
        * @return TInt  -Error code to active scheduler, is always KErrNone.
        */ 
        TInt RunError( TInt aError );    

    private:
    
        /**
        * Leavable authentication objects list function.
        * @return void
        */
        void CreateListL( TRequestStatus& aStatus );

        /**
        * Leavable authentication objects creation function.
        * @param  aToken (IN) Current token
        * @param  aPinModule (IN) Pin manager module
        * @param  aLabel (IN) A label for authentication object.
        * @param  aType (IN) Type of this authenticaion object.
        * @param  aHandle (IN) A handle of this authentication object.
        * @param  aStatus (IN) The status of this authentication object.
        * @return Pointer to CWimAuthenticationObject
        */
        CWimAuthenticationObject* MakeAuthObjectL( CWimToken& aToken,
                                                   CWimPin& aPinModule,
                                                   const TDesC& aLabel,
                                                   TUid aType,
                                                   TUint32 aHandle,
                                                   TUint32 aStatus );
        /**
        * Leavable authentication objects list function.
        * The caller of this function owns all parameters.
        * @param  aAuthObjects (OUT) The returned authentication objects 
        *         will be appended to this array.
        * @return void
        */
        void DoListL( RMPointerArray<MCTAuthenticationObject>& aAuthObjects );

        /**
        * Returns true or false indicating if token is removed or not
        * @return Boolean true or false
        */
        TBool TokenRemoved();
        
    
    private:
    
    enum TAuthObjPhase
            {
            ECreateList,
            ECreateListDone,
            EDoList
            };   
            

    private: // Data

        // A reference to current token.
        CWimToken& iToken;

        // Array of pointers to PIN modules. Not owned.
        const CArrayPtrFlat<CWimPin>* iPinNRs; 
        
        // An internal array of listed authentication objects.
        RPointerArray<CWimAuthenticationObject> iAuthObjects;
        
        //Used to handle Authobj interface making
        TAuthObjPhase                   iPhase;
        //Client status is stored here 
        TRequestStatus*                 iClientStatus;
        
        //Not Owned
        RMPointerArray<MCTAuthenticationObject>* iClientAuthObjects;
    };


#endif      // CWIMAUTHENTICATIONOBJECTLIST_H   
            
// End of File
