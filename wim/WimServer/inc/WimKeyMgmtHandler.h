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
* Description:  Wim key management handler
*
*/



#ifndef CWIMKEYMGMTHANDLER_H
#define CWIMKEYMGMTHANDLER_H

//  INCLUDES
#include <e32base.h>

// FORWARD DECLARATIONS
class CWimMemMgmt;
class CWimUtilityFuncs;

// CLASS DECLARATION

/**
*  WIM key management handler
*  
*
*  @since Series60 2.6
*/
class CWimKeyMgmtHandler : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimKeyMgmtHandler* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CWimKeyMgmtHandler();

    public: // New functions
        
        /**
        * Checks if the given key exists.
        * @param    aMessage Client message
        * @return   void
        */
        void DoesKeyExistL( const RMessage2& aMessage );

        /**
        * Fetches the details of the given key.
        * @param    aMessage Client message
        * @return   void
        */
        void GetKeyDetailsL( const RMessage2& aMessage );

        /**
        * Fetches the list of keys in WIM.
        * @param    aMessage Client message
        * @param    aWimMgmt Pointer to CWimMemMgmt class
        * @return   void
        */
        void GetKeyListL( const RMessage2& aMessage,
                          CWimMemMgmt* aWimMgmt ) const;

    private:

        /**
        * C++ default constructor.
        */
        CWimKeyMgmtHandler();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data
        // Pointer to Utility functions class
        CWimUtilityFuncs* iWimUtilFuncs;

    };

#endif      // CWIMKEYMGMTHANDLER_H 
            
// End of File
