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
* Description:  Header for utility functions
*
*/



#ifndef CWIMUTILITYFUNCS_H
#define CWIMUTILITYFUNCS_H

//  INCLUDES
#include "WimConsts.h"      //Error codes
#include "Wimi.h"           //WIMI definitions

// DATA TYPES

// Type of request
enum TWimReqType
    {
    EWimMgmtReq
    };

// Request type and transaction id
struct TWimReqTrId
    {
    TAny*       iReqTrId;
    TWimReqType iReqType;
    };


// CLASS DECLARATION

/**
*  Utility functions for WimServer
*
*  @since Series 60 2.1
*/
class CWimUtilityFuncs : public CBase  
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimUtilityFuncs* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CWimUtilityFuncs();

    public: // New functions
        
        /**
        * Map WIMI error to WIM errors
        * @param aStatus WIMI error code
        * @return WIM error code 
        */
        static TInt MapWIMError( WIMI_STAT aStatus );

        /**
        * Get SIM/SWIM state
        * @return SIM/SWIM state: KErrNone if OK
        *                         If not OK some other system wide state
        */
        static TInt SimState();

        /**
        * Utility function that reads a descriptor from the clients address
        * space. The caller has to free the returned HBufC8.
        * @param aIndex Message slot index to be read
        * @param aMessage Message
        * @return Pointer to allocated HBufC8 buffer.
        */
        HBufC8* DesLC( const TInt aIndex,
                       const RMessage2& aMessage ) const;
   
       
        /**
        * Creates a new ReqTrId item.
        * @param aAny Pointer to TAny
        * @param aReqType Request type
        * @return Structure which contains transaction ID and request type
        */
        TWimReqTrId* TrIdLC( TAny* aAny, TWimReqType aReqType ) const;

        /**
        * Maps WIMI's certificate location value to WimServer's location type
        * TWimCertificateCDF
        * @param aCertCDF WIMI's certificate location value
        * @return WimServer's location value
        */
        TUint8 MapCertLocation( const TUint8 aCertCDF ) const;

    private:

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * C++ default constructor.
        */
        CWimUtilityFuncs();

    };

#endif      // CWIMUTILITYFUNCS_H 
            
// End of File
