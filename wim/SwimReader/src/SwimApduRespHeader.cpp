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
* Description:  Class for APDU response header
*
*/



// INCLUDE FILES 
#include    "SwimApduRespHeader.h"
#include    "WimTrace.h"            // for trace logging

#ifdef _DEBUG
#include <flogger.h>
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// TSwimApduRespHeader::TSwimApduRespHeader
// Constructor
// -----------------------------------------------------------------------------
//
TSwimApduRespHeader::TSwimApduRespHeader( const TDesC8& aData )
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduRespHeader::TSwimApduRespHeader|Begin"));
    Copy( aData );
    }

// -----------------------------------------------------------------------------
// TSwimApduRespHeader::AnyByte
// Return byte in given index
// -----------------------------------------------------------------------------
//
TUint8 TSwimApduRespHeader::AnyByte( TUint8 aIndex )
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduRespHeader::AnyByte|Begin"));
    if ( iData.Length() > aIndex )
        {
        return iData[aIndex];
        }
    else
        {
        return 0;
        }
    }

// -----------------------------------------------------------------------------
// TSwimApduRespHeader::Copy
// Copy data
// -----------------------------------------------------------------------------
//
void TSwimApduRespHeader::Copy( const TDesC8& aData )
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduRespHeader::Copy|Begin"));
    // If max length is shorter than length of given data,
    // copy only max length amount of data.
    if ( KMaxApduHeaderLen >= aData.Length() ) 
        {
        iData = aData;
        }
    else
        {
        iData = aData.Left( KMaxApduHeaderLen );
        }
    }

// -----------------------------------------------------------------------------
// TSwimApduRespHeader::DataLength
// Return data length
// -----------------------------------------------------------------------------
//
TUint16 TSwimApduRespHeader::DataLength()
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduRespHeader::DataLength|Begin"));
    TUint8  serviceType, shortRet;
    TUint16 ret = 0;

    serviceType = ServiceType();

    if ( serviceType == KList )
        {
        shortRet = ShortDataLength();
        ret = shortRet;
        }
    else if ( iData.Length() > 13 )
        {
        ret = TUint16( iData[12] << 8 );
        ret = TUint16( ret | iData[13] );
        }

    return ret;
    }

// -----------------------------------------------------------------------------
// TSwimApduRespHeader::operator=
// Operator =
// -----------------------------------------------------------------------------
//
void TSwimApduRespHeader::operator=( const TDesC8& aData )
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduRespHeader::operator=|Begin"));
    Copy( aData );
    }

// -----------------------------------------------------------------------------
// TSwimApduRespHeader::ServiceType
// Return Service type byte
// -----------------------------------------------------------------------------
//
TUint8 TSwimApduRespHeader::ServiceType()
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduRespHeader::ServiceType|Begin"));
    return AnyByte( KIndexRespServiceType );
    }

// -----------------------------------------------------------------------------
// TSwimApduRespHeader::ShortDataLength
// Return ShortDataLength byte
// -----------------------------------------------------------------------------
//
TUint8 TSwimApduRespHeader::ShortDataLength()
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduRespHeader::ShortDataLength|Begin"));
    if ( ServiceType() == KSendApdu )
        {
        return 0;
        }
    else
        {
        return AnyByte( KIndexRespSDataLength );
        }
    }

// -----------------------------------------------------------------------------
// TSwimApduRespHeader::Status
// Return status byte
// -----------------------------------------------------------------------------
//
TUint8 TSwimApduRespHeader::Status()
    {
    _WIMTRACE2(_L("WIM|SwimReader|TSwimApduRespHeader::Status|status=%d"), AnyByte(KIndexRespStatus));
    return AnyByte( KIndexRespStatus );
    }

// End of File
