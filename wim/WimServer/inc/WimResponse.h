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
* Description:  This is a header file for class that is used a transaction id
*                for the callback functions
*
*/


#ifndef WIMRESPONSE_H
#define WIMRESPONSE_H

//  INCLUDES
#include "Wimi.h"       // WIMI definitions
#include "WimOpcode.h"

#include <e32std.h>

// CLASS DECLARATION

/**
*  WIMI response.
*  Holds messages for completing after WIMI response.
*  
*  @since Series60 2.1
*/
class CWimResponse : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Constructor.
        */
        CWimResponse( RMessage2 aMessage );

        /**
        * Destructor.
        */
        virtual ~CWimResponse();
    
    public: // New functions

        /**
        * Delete myself
        * @return void
        */
        void CompleteMsgAndDelete();

    public:     // Data
    
        // Message to be completed
        RMessage2     iMessage;
        // Status of the WIMI call
        WIMI_STAT    iStatus; 
        // Any internal error 
        TInt         iError;
        // Operation code
        TWimServRqst iOpcode;

    };

#endif      // WIMRESPONSE_H

// End of File



