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
* Description:  Core class for all Smart Card aware applications to use when 
*                communicating with the card.
*
*/



// INCLUDE FILES
#include    "Scard.h"
#include    "ScardBase.h"
#include    "ScardClsv.h"
#include    "ScardComm.h"
#include    "ScardConnectionRequirement.h"
#include    "WimTrace.h"

#ifdef _DEBUG   // for logging
#include    "ScardLogs.h"
#include    <flogger.h> 
#endif

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardComm::CScardComm
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardComm::CScardComm( RScard* aScard )
    : iScard( aScard )
    {
    _WIMTRACE(_L("WIM|Scard|CScardComm::CScardComm|Begin"));
    }

// -----------------------------------------------------------------------------
// CScardComm::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardComm::ConstructL(
    TScardConnectionRequirement& aRequirement,
    TScardReaderName& aReaderName,
    TRequestStatus& aStatus,
    const TInt32 aTimeOut )
    {
    _WIMTRACE(_L("WIM|Scard|CScardComm::ConstructL|Begin"));
    aStatus = KRequestPending;
    TAny* p[KMaxMessageArguments];
    p[0] = p[1] = p[2] = p[3] = reinterpret_cast<TAny*>( 0 );

    p[3] = reinterpret_cast<TAny*>( aTimeOut );

    TInt mode( 0 );
    
    if ( aRequirement.iNewReaders )
        {
        mode |= KNewReadersOnly;
        }
    if ( aRequirement.iNewCards )
        {
        mode |= KNewCardsOnly;
        }

    //  Reader requirements
    if ( aRequirement.iExplicitReader )
        {
        mode |= KExplicitReader;
        aReaderName.Copy( *aRequirement.iExplicitReader );
        }
    else if ( aRequirement.iExcludedReader )
        {
        mode |= KExcludedReader;
        aReaderName.Copy( *aRequirement.iExcludedReader );
        }
    else
        {
        mode |= KAnyReader;
        aReaderName.Copy( _L( "" ) );
        }
    
    p[1] = static_cast<TAny*>( &aReaderName );
    
    //  Card requirements
    if ( aRequirement.iATRBytes )
        {
        mode |= KATRSpesified;
        p[2] = reinterpret_cast<TAny*>( aRequirement.iATRBytes );
        }
    else if ( aRequirement.iAIDBytes )
        {
        mode |= KApplicationSpesified;
        p[2] = reinterpret_cast<TAny*>( aRequirement.iAIDBytes );
        }
    else
        {
        mode |= KAnyCard;
        }
#ifdef _DEBUG
    RFileLogger::WriteFormat( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, _L( "CScardComm::ConstructL mode: 0x%x \n" ), 
        mode );
#endif

    //  Set the connection mode
    p[0] = reinterpret_cast<TAny*>( mode );
    TIpcArgs args( p[0], &aReaderName, p[2], p[3] ); 
    iScard->SendReceive( EScardServerConnectToReader, args, aStatus );
    }

// -----------------------------------------------------------------------------
// CScardComm::NewL
// Two-phased constructor, asynchronous version.
// -----------------------------------------------------------------------------
//
EXPORT_C CScardComm* CScardComm::NewL(
    RScard* aScard,
    TScardConnectionRequirement& aRequirement,
    TScardReaderName& aReaderName,
    TRequestStatus& aStatus,
    const TInt32 aTimeOut )
    {
    _WIMTRACE(_L("WIM|Scard|CScardComm::NewL|Begin"));
    __ASSERT_ALWAYS( aScard, 
        User::Panic( _L( "Null pointer" ), KScPanicNullPointer ) );
    __ASSERT_ALWAYS( aTimeOut >= 0, User::Leave( KScErrBadArgument ) );
    
    CScardComm* self = new( ELeave ) CScardComm( aScard );
    
    CleanupStack::PushL( self );
    self->ConstructL( aRequirement, aReaderName, aStatus, aTimeOut );
    CleanupStack::Pop( self );

    return self;
    }
 
// Destructor
EXPORT_C CScardComm::~CScardComm()
    {
    _WIMTRACE(_L("WIM|Scard|CScardComm::~CScardComm|Begin"));
    if ( iScard )
        {
        DisconnectFromReader();
        }
    }

// -----------------------------------------------------------------------------
// CScardComm::TransmitToCard
// Transmit data to the card
// -----------------------------------------------------------------------------
//
EXPORT_C void CScardComm::TransmitToCard(
    const TDesC8& aCommand,
    TDes8& aResponse,
    TRequestStatus& aStatus,
    const TInt32 aTimeOut,
    const TInt8 aChannel ) const
    {
    _WIMTRACE(_L("WIM|Scard|CScardComm::TransmitToCard|Begin"));
#ifdef _DEBUG
    RFileLogger::HexDump( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, 
        _S( "CScardComm::TransmitToCard command APDU:\n" ), 0,
        aCommand.Ptr(), aCommand.Length() );
#endif
    
    if ( aTimeOut < 0 ) // Check if timeout value is valid
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete( status, KErrArgument );
        return; // Return right away
        }

    aStatus = KRequestPending;
    
    TIpcArgs args( &aCommand,
                   &aResponse,
                   aTimeOut,
                   aChannel );

    iScard->SendReceive( EScardServerTransmitToCard, args, aStatus );
    }

// -----------------------------------------------------------------------------
// CScardComm::CancelTransmit
// Cancel transmitting
// -----------------------------------------------------------------------------
//
EXPORT_C void CScardComm::CancelTransmit() const
    {
    _WIMTRACE(_L("WIM|Scard|CScardComm::CancelTransmit|Begin"));
    TIpcArgs args( TIpcArgs::ENothing );
    iScard->SendReceive( EScardServerCancelTransmit, args );
    }

// -----------------------------------------------------------------------------
// CScardComm::GetATRL
// This function fetches the answer-to-reset bytes of the SC currently in the 
// card reader. If an error occurs during the operation, the  length of the 
// ATR bytes is set to zero. 
// -----------------------------------------------------------------------------
//

EXPORT_C void CScardComm::GetATRL( TScardATR& aATR ) const
    {
    _WIMTRACE(_L("WIM|Scard|CScardComm::GetATRL|Begin"));

    TIpcArgs args( ( TAny* )&aATR );
    TInt err = iScard->SendReceive( EScardServerGetATR, args );

    if ( err )
        {
        User::Leave( err );
        }
    }

// -----------------------------------------------------------------------------
// CScardComm::GetCapabilitiesL
// Get proprietary parameters from the reader handler. 
// -----------------------------------------------------------------------------
//
EXPORT_C void CScardComm::GetCapabilitiesL(
    const TInt32 aTag,
    TDes8& aValue,
    const TInt32 aTimeOut ) const
    {
    _WIMTRACE(_L("WIM|Scard|CScardComm::GetCapabilitiesL|Begin"));
    __ASSERT_ALWAYS( aTimeOut >= 0, User::Leave( KScErrBadArgument ) );
    TAny* p[KMaxMessageArguments];
    p[0] = reinterpret_cast<TAny*>( 
        static_cast<TInt>( ( ( aTag & 0xff00L ) >> 8 ) ) ); //Bits 8-15 from tag
    p[1] = reinterpret_cast<TAny*>( 
        static_cast<TInt>( ( aTag & 0x00ffL ) ) );  //First 8 bits from tag
    p[2] = static_cast<TAny*>( &aValue );
    p[3] = reinterpret_cast<TAny*>( aTimeOut );
    
    TIpcArgs args( p[0], p[1], &aValue, p[3] );
    
    TInt err = iScard->SendReceive( EScardServerGetCapabilities, args );
    
    if ( err ) // Scard error code may be positive
        {
        User::Leave( err );
        }
    }

// -----------------------------------------------------------------------------
// CScardComm::Scard
// Return Scard object contained within this object.
// -----------------------------------------------------------------------------
//
EXPORT_C RScard* CScardComm::Scard() const
    {
    _WIMTRACE(_L("WIM|Scard|CScardComm::Scard|Begin"));
    return iScard;
    }

// -----------------------------------------------------------------------------
// CScardComm::ManageChannel
// Channel management function.
// -----------------------------------------------------------------------------
//
EXPORT_C void CScardComm::ManageChannel(
    const TScChannelManagement aCommand,
    const TInt8 aArgument,
    TDes8& aResponseBuffer,
    TRequestStatus& aStatus,
    const TInt32 aTimeOut ) const
    {
    _WIMTRACE(_L("WIM|Scard|CScardComm::ManageChannel|Begin"));
    
    if ( aTimeOut < 0 ) // Check if timeout value is valid
        {
        TRequestStatus* status = &aStatus;
        User::RequestComplete( status, KErrArgument );
        return; // Return right away
        }

    TAny* p[KMaxMessageArguments];
    
    p[0] = p[1] = p[2] = p[3] = reinterpret_cast<TAny*>( 0 );

    p[0] = reinterpret_cast<TAny*>( aCommand );

    TIpcArgs args( p[0], p[1], p[2], p[3] );    
    
    switch ( aCommand )
        {
        case EOpenAnyChannel:
            {
#ifdef _DEBUG
            RFileLogger::Write( KScardLogDir, KScardLogFileName, 
                EFileLoggingModeAppend, 
                _L( "CScardComm::ManageChannel -- Open Any channel" ) );
#endif

            args.Set( 1, &aResponseBuffer );
            args.Set( 2, aTimeOut );
            break;
            }
        
        case ECloseChannel:
            {
#ifdef _DEBUG
            RFileLogger::Write( KScardLogDir, KScardLogFileName, 
                EFileLoggingModeAppend, 
                _L( "CScardComm::ManageChannel -- Close channel" ) );
#endif

            args.Set( 1, aArgument );
            args.Set( 2, aTimeOut );
            
            break;
            }
        
        case EChannelStatus:
            {
            args.Set( 1, &aResponseBuffer );
            break;
            }
        
        default:
            {
            User::Panic( _L( "Command not supported" ), 
                KScPanicNotSupported );
            break;
            }
        }

    iScard->SendReceive( EScardServerManageChannel, args, aStatus );
    }

// -----------------------------------------------------------------------------
// CScardComm::DisconnectFromReader
// Close connection
// -----------------------------------------------------------------------------
//
void CScardComm::DisconnectFromReader() const
    {
    _WIMTRACE(_L("WIM|Scard|CScardComm::DisconnectFromReader|Begin"));
#ifdef _DEBUG
    RFileLogger::Write( KScardLogDir, KScardLogFileName, 
        EFileLoggingModeAppend, _L( "CScardComm::DisconnectFromReader" ) );
#endif

    TIpcArgs args( TIpcArgs::ENothing );
    iScard->SendReceive( EScardServerDisconnectFromReader, args );
    }

//  End of File  
