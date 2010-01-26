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
* Description:  SWIM APDU Response Header
*
*/


#ifndef SWIMAPDURESPHEADER_H
#define SWIMAPDURESPHEADER_H

//  INCLUDES
#include "SwimReaderConsts.h"

// CLASS DECLARATION

/**
*  Services for parse APDU responce header
*  Used by class CSwimReader
*
*  @lib SwimReader.lib
*  @since Series60 2.1
*/
class TSwimApduRespHeader  
    {
    
    public:  // Constructors and destructor

        /**
        * C++ default constructor.
        @ param aData Data
        */
        TSwimApduRespHeader( const TDesC8& aData );

    public: // New functions

        /**
        * operator=
        * @param aData Data to be assigned
        * @return void
        */
        void operator=( const TDesC8& aData );
        
        /**
        * Return byte in given index
        * @param aIndex Index of byte
        * @return TUint8
        */
        TUint8 AnyByte( TUint8 aIndex );

        /**
        * Copy data
        * @param aData
        * @return void
        */
        void Copy( const TDesC8& aData );
        
        /**
        * Return Data length
        * @return Data Length
        */
        TUint16 DataLength();

        /**
        * Return Service type byte
        * @return TUint8
        */
        TUint8 ServiceType();

        /**
        * Return short data length
        * @return Lenght
        */
        TUint8 ShortDataLength();
        
        /**
        * Return Status byte
        * @return Status byte
        */
        TUint8 Status();

    private:  //Data
        // APDU data
        TBuf8<KMaxApduHeaderLen> iData;
    };

#endif      // SWIMAPDURESPHEADER_H

// End of File
