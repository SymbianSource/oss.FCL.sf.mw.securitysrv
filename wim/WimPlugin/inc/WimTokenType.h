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
* Description:  This class is instantiated via ECom for a particular token type
*
*/


#ifndef CWIMTOKENTYPE_H
#define CWIMTOKENTYPE_H

//  INCLUDES

#include "WimSecModuleMgr.h"
#include <e32base.h>
#include <ct.h>


/**
*  This refers to a software module that supports the instantiation of a tokens
*
*  @lib   WimPlugin
*  @since Series60 2.1
*/ 

class CWimTokenType :   public CCTTokenType
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimTokenType* NewL();

        
        /**
        * Destructor.
        */
        virtual ~CWimTokenType();

    public: // Functions from base class MCTTokenType
        
        /**
        * Lists all the tokens of this type. Caller owns all parameters and
        * is responsible to destroy all objects in the received RCPointerArray.
        * @param  aTokens (OUT) The returned tokens are added to this array.
        * @param  aStatus (IN/OUT). Asyncronous return status: 
        *         KErrNone, if no errors occurred
        *         KErrNotFound, if Wim information not found
        *         KErrCancel, if list operation cancelled
        *         Any other system wide error code
        * @return void
        */
        void List( RCPointerArray<HBufC>& aTokens, 
                            TRequestStatus& aStatus );

        /**
        * Cancel a list operation.
        * @return void
        */
        void CancelList();

        /**
        * Opens a specified token with token info.
        * The caller owns all the parameters.
        * @param  aTokenInfo (IN) The info of the required token (token label)
        * @param  aToken (OUT) The returned token.
        * @param  aStatus (IN/OUT) Asynchronous return status.  
        *         KErrNone, if token opening succeeded
        *         KErrNotFound, if token not found by aTokenInfo
        *         Any other system wide error code
        * @return void
        */
        void OpenToken( const TDesC& aTokenInfo, 
                                 MCTToken*& aToken,
                                 TRequestStatus& aStatus );
    
        /**
        * Opens a specified token with handle
        * The caller owns all the parameters.
        * @param  aHandle (IN) The handle of the required token
        * @param  aToken (OUT) The returned token.
        * @param  aStatus (IN/OUT) Asynchronous return status.  
        *         KErrNone, if token opening succeeded
        *         KErrNotFound, if token not found by aHandle
        *         Any other system wide error code
        * @return void
        */
        void OpenToken( TCTTokenHandle aHandle, 
                                 MCTToken*& aToken,
                                 TRequestStatus& aStatus );
    
         /**
        * Cancel an OpenToken operation.
        * @return void
        */
        void CancelOpenToken();      

    private: // Constructors

        /**
        * Second phase constructor
        */
        void ConstructL();

        /**
        * Default constructor
        */
        CWimTokenType();

    private: // Data
        
        // A pointer to security module handler.
        // The ownership of this object belongs to this class
        CWimSecModuleMgr* iWimSecModuleMgr;

    };

#endif      // CWIMTOKENTYPE_H  
            
// End of File
