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
* Description:  SWIM APDU Request header
*
*/


#ifndef SWIMAPDUREQHEADER_H
#define SWIMAPDUREQHEADER_H

//  INCLUDES
#include "SwimReaderConsts.h"


// CLASS DECLARATION

/**
*  Services for creating Request APDU header
*  Used by class CSwimReader
*
*  @lib SwimReader.lib
*  @since Series60 2.1
*/
class TSwimApduReqHeader  
    {
    
    public:  // Constructors and destructor

        /**
        * C++ default constructor.
        */
        TSwimApduReqHeader();

    public: // New functions

        /**
        * Return APDU data
        * @return Data
        */
        TDes8* Data();

        /**
        * Set Application type byte
        * @param aApp Application type
        * @return void
        */
        void SetAppType( TUint8 aApp );
        
        /**
        * Set CardReader byte
        * @param aCardReader Card reader byte
        * @return void
        */
        void SetCardReader( TUint8 aCardReader );
        
        /**
        * Set header
        * @param aServiceType Service type
        * @param aCardReader Card reader
        * @param aAppType Applicatin type
        * @param aPaddingByte Padding type
        * @return void
        */
        void SetHeader( TUint8 aServiceType = KSendApdu,
                        TUint8 aCardReader = KNoPreferredReader,
                        TUint8 aAppType = KAPDUAppTypeWIM,
                        TUint8 aPaddingByte = 0 );

        /**
        * operator=
        * @param aData
        * @return void
        */
        void operator=( const TDesC8& aData );
        
        /**
        * Return Padding Byte
        * @return Padding Byte
        */
        TUint8 PaddingByte();
        
        /**
        * Return ServiceType byte
        * @return Service Type
        */
        TUint8 ServiceType();
        
        /**
        * Set PaddingByte
        * @param aPaddingByte Padding type
        * @return void
        */
        void SetPaddingByte( TUint8 aPaddingByte );
        
        /**
        * Set ServiceType
        * @param aServiceType Service type
        * @return void
        */
        void SetServiceType( TUint8 aServiceType );
        
    private:  // Data
        // APDU data
        TBuf8<KMaxApduHeaderLen> iData;
    };

#endif      // SWIMAPDUREQHEADER_H

// End of File
