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
* Description:  Registry of WimServer sessions
*
*/



#ifndef CWIMSESSIONREGISTRY_H
#define CWIMSESSIONREGISTRY_H

//  INCLUDES
#include <e32base.h>


// FORWARD DECLARATIONS
class CWimSession;


// CLASS DECLARATION

/**
* This class holds array of sessions of server.
*
* @since Series60 2.1
*/
class CWimSessionRegistry : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimSessionRegistry* NewL();

        /**
        * Destructor.
        */
        virtual ~CWimSessionRegistry();

    public:  // New functions

        /**
        * Add session to array
        * @param aSession Session to be added
        * @return void
        */
        void AddSessionL( CWimSession* aSession );

        /**
        * Get array of sessions
        * @param aSessions Session array
        * @return void
        */
        void GetSessionsL( RArray<CWimSession*>& aSessions ) const;

        /**
        * Remove session from sessions array
        * @param aSession Session to be removed
        * @return void
        */
        void RemoveSession( CWimSession* aSession );

    private:
    
        /**
        * C++ default constructor.
        */
        CWimSessionRegistry();
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:
        // Array of sessions
        RArray<CWimSession*> iSessions;
        
    };

#endif      // CWIMSESSIONREGISTRY_H

