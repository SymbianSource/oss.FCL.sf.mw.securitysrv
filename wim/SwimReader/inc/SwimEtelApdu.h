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
* Description:  Etel references
*
*/



#ifndef ETELAPDU_H
#define ETELAPDU_H

//  INCLUDES
#include <rmmcustomapi.h>   // For CustomAPI services

//  CONSTANTS 
// For logging
#ifdef _DEBUG
_LIT( KSwimReaderLogDir, "Scard" );
_LIT( KSwimReaderLogFileName, "Log.txt" );
#endif

// FORWARD DECLARATIONS
class RTelServer;


// CLASS DECLARATION

/**
*  Functionality to send Apdus through Etel
*  
*
*  @lib SwimReader.lib
*  @since Series60 2.1
*/
class RApdu : public RTelSubSessionBase
    {
    public: // Constructors and destructor
    
        /**
        * C++ default constructor.
        */
        RApdu();
    
    public: // New functions
        
        /**
        * Open Etel connection
        * @param aSession RTelServer session
        * @param aName    Name of the phone
        * @return S60 error code
        */
        TInt Open( RTelServer& aSession, const TDesC& aName );

        /**
        * Close connection to Etel server
        * @return void
        */
        void Close();
        
        /**
        * Transmit message through Etel (via CustomAPI)
        * @param  aStatus Request's status
        * @param  aMsg    Message to be transmitted
        * @return void
        */
        void APDUReq( TRequestStatus& aStatus, RMmCustomAPI::TApdu& aMsg );
        
        /**
        * Cancel transmit message through Etel (via CustomAPI)
        * @return void
        */
        void CancelAPDUReq();
        
    private:

        /**
        * Symbian 2nd phase constructor
        */
        void ConstructL();

        /**
        * Destruct 
        * @return void
        */
        void Destruct();

    private:    // Data
        //pointer to customAPI instance. Owned.
        RMmCustomAPI* iCustomApi;
        RMobilePhone iPhone;
    };

#endif      // ETELAPDU_H

// End of File
