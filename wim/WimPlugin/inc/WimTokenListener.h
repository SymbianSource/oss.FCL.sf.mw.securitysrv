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
* Description:  Listener class for Wim token removal event
*
*/



#ifndef CWIMTOKENLISTENER_H
#define CWIMTOKENLISTENER_H

//  INCLUDES

#include "WimSecModule.h"
#include <e32base.h>

// FORWARD DECLARATIONS


// CLASS DECLARATION

/**
*  CWimTokenListener listens removal event from WimServer
*
*  @lib   WimPlugin
*  @since Series60 2.1
*/
class CWimTokenListener : public CActive
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @param aWimSecModule (IN) A pointer to current security module
        * @return An instance of this class
        */
        static CWimTokenListener* NewL( CWimSecModule* aWimSecModule );
        
        /**
        * Destructor.
        */
        virtual ~CWimTokenListener();


    public: // New functions

        /**
        * Starts system to listen token removal
        * @return void
        */
        void StartListening();

        /**
        * Set client status
        * @param aStatus (IN/OUT) Client status that wants to know if token 
        *        is removed
        * @return void
        */
        void SetClientStatus( TRequestStatus& aStatus );

        /**
        * Clear client status. This is done when framework's CancelNotify()
        * is called. The client won't get any message about token removal.
        * @return void
        */
        void ClearClientStatus();

        /**
        * Returns token status.
        * @return KRequestPending meaning that token is not removed, 
        *         KErrNone meaning that token has been removed, 
        */
        TInt TokenStatus();

    private: // From base class CActive

        void RunL();
        
        void DoCancel();

    private: // Constructors
    
        /**
        * Default constructor.
        * @param aWimSecModule (IN) A pointer to current security module
        */
        CWimTokenListener( CWimSecModule* aWimSecModule );

        // Second phase constructor
        void ConstructL();

    private:    
        
        // A pointer to current security module.
        // This class don't own the pointed object.
        CWimSecModule* iWimSecModule;

        // A pointer to UI's request status
        // This class don't own the pointed object.
        TRequestStatus* iClientStatus;

        // Status value of token
        TInt iTokenStatus;

    };

#endif      // CWIMTOKENLISTENER_H   
            
// End of File
