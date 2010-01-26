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
* Description:  Class for APDU request header
*
*/



// INCLUDE FILES 
#include    "SwimApduReqHeader.h"
#include    "WimTrace.h"            // for trace logging

#ifdef _DEBUG
#include    <flogger.h>
#endif


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// TSwimApduReqHeader::TSwimApduReqHeader
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
TSwimApduReqHeader::TSwimApduReqHeader()
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduReqHeader::TSwimApduReqHeader|Begin"));
    }

// -----------------------------------------------------------------------------
// TSwimApduReqHeader::Data
// Return APDU data
// -----------------------------------------------------------------------------
//
TDes8* TSwimApduReqHeader::Data()
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduReqHeader::Data|Begin"));
    return &iData;
    }

// -----------------------------------------------------------------------------
// TSwimApduReqHeader::operator=
// Operator =
// -----------------------------------------------------------------------------
//
void TSwimApduReqHeader::operator=( const TDesC8& /*aData*/ )
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduReqHeader::operator=|Begin"));
    }

// -----------------------------------------------------------------------------
// TSwimApduReqHeader::SetAppType
// Set Application type byte
// -----------------------------------------------------------------------------
//
void TSwimApduReqHeader::SetAppType( TUint8 aAppType )
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduReqHeader::SetAppType|Begin"));
    iData[KIndexAppType] = aAppType;
    }

// -----------------------------------------------------------------------------
// TSwimApduReqHeader::SetCardReader
// Set CardReader byte
// -----------------------------------------------------------------------------
//
void TSwimApduReqHeader::SetCardReader( TUint8 aCardReader )
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduReqHeader::SetCardReader|Begin"));
    iData[KIndexCardReader] = aCardReader;
    }

// -----------------------------------------------------------------------------
// TSwimApduReqHeader::SetHeader
// Set header for ADPU
// -----------------------------------------------------------------------------
//
void TSwimApduReqHeader::SetHeader(
    TUint8 aServiceType,
    TUint8 aCardReader, 
    TUint8 aAppType,
    TUint8 aPaddingByte )
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduReqHeader::SetHeader|Begin"));
    // Initialize data buffer so it can be accessed byte by byte.
    for ( TInt i = 0; i < KMaxApduHeaderLen; i++ )
        {
        if ( iData.Length() < iData.MaxLength() )
            {
            iData.Append( 0 );
            }
        else
            {
            iData[i] = 0;
            }
        }

    // Service type and card reader are used in all types of APDUs.
    SetServiceType( aServiceType );
    SetCardReader( aCardReader );

    switch ( aServiceType )
        {
        case KSendApdu:
            {
            // App type and padding byte are used only with SEND_APDU msg.
            SetAppType( aAppType );
            SetPaddingByte( aPaddingByte );

            iData.SetLength( KSendApduHeaderLen );
            break;
            }
        case KList:
            {
            iData.SetLength( KGetListHeaderLen );
            break;
            }
        default:
            {
            iData.SetLength( KMaxApduHeaderLen );
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// TSwimApduReqHeader::SetPaddingByte
// Set padding byte
// -----------------------------------------------------------------------------
//
void TSwimApduReqHeader::SetPaddingByte( TUint8 aPaddingByte )
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduReqHeader::SetPaddingByte|Begin"));
    iData[KIndexPaddingByte] = aPaddingByte;
    }

// -----------------------------------------------------------------------------
// TSwimApduReqHeader::SetServiceType
// Set ServiceType byte
// -----------------------------------------------------------------------------
//
void TSwimApduReqHeader::SetServiceType( TUint8 aServiceType )
    {
    _WIMTRACE(_L("WIM|SwimReader|TSwimApduReqHeader::SetServiceType|Begin"));
    iData[KIndexServiceType] = aServiceType;
    }

// End of File
