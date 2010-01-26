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
* Description:  WIM Token handler
*
*/



#ifndef CWIMTOKENHANDLER_H
#define CWIMTOKENHANDLER_H

//  INCLUDES
#include <e32base.h>

// FORWARD DECLARATIONS
class CWimMemMgmt;
class CWimTimer;
class CWimUtilityFuncs;

// CLASS DECLARATION


/**
*  Class for token handling
*
*  @lib WimServer.lib
*  @since Series60 2.1
*/
class CWimTokenHandler : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimTokenHandler* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CWimTokenHandler();

    public: // New functions
        
        /**
        * Fetches WIM card info.
        * @param aMessage Encapsulates a client request.
        * @param aWimMgmt Pointer to memory management class.
        * @return void
        */
        void GetWIMInfoL( const RMessage2& aMessage,
                          CWimMemMgmt* const aWimMgmt ) const;

        /**
        * Checks if the WIM is already opened.
        * @param aMessage Encapsulates a client request.
        * @param aTimer Pointer to timer class.
        * @return void
        */
        void IsWIMOpenL( const RMessage2& aMessage,
                         CWimTimer* const aTimer, 
                         CWimMemMgmt* const aWimMgmt ) const;

        /**
        * Closes a connection to a WIM card.
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void CloseWIM( const RMessage2& aMessage ) const;

    private:

        /**
        * C++ default constructor.
        */
        CWimTokenHandler();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data
        // Pointer to utility class
        CWimUtilityFuncs* iWimUtilFuncs;
         
    };

#endif      // CWIMTOKENHANDLER_H   
            
// End of File
