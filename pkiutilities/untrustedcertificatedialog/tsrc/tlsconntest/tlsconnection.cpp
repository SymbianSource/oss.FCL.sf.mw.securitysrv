/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Secure connections test application
*
*/

#include "tlsconnection.h"
#include <e32debug.h>
#include <SecureSocket.h>
#include <ssl_internal.h>       // KSolInetSSL, KSoSSLDomainName

_LIT( KData, "GET index.html" );
_LIT( KSecureProtocol, "TLS1.0" );
_LIT( KCommDBIapId, "IAP\\Id" );
_LIT8( KNewLine, "\n" );

const TInt KRecvBufferLength = 0x4000;
const TInt KOneSecondInMicroseconds = 1000000;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------
// CTlsConnection::NewL
// ---------------------------------------------------------
//
CTlsConnection* CTlsConnection::NewL( MTlsConnectionObserver& aObs )
    {
    RDebug::Print( _L("CTlsConnection::NewL()") );
    CTlsConnection* self = new ( ELeave ) CTlsConnection( aObs );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CTlsConnection::~CTlsConnection()
// -----------------------------------------------------------------------------
//
CTlsConnection::~CTlsConnection()
    {
    RDebug::Print( _L("CTlsConnection::~CTlsConnection()") );
    Cancel();
    iDelayTimer.Close();
    CloseSession();
    delete iSendBuffer;
    delete iSendBufPtr;
    }

// -----------------------------------------------------------------------------
// CTlsConnection::ConnectL()
// -----------------------------------------------------------------------------
//
void CTlsConnection::ConnectL( const TDesC& aHostName, TInt aPort, TInt aDelay )
    {
    RDebug::Print( _L("CTlsConnection::Connect() host '%S', port %d, delay %d"),
        &aHostName, aPort, aDelay );
    CloseConnection();

    // Parameters
    if( iHostName )
        {
        delete iHostName;
        iHostName = NULL;
        }
    iHostName = aHostName.AllocL();
    iPort = aPort;
    iDelay = aDelay;

    // Save host name in 8-bit form for server certificate checking
    if( iHostNameForCertCheck )
        {
        delete iHostNameForCertCheck;
        iHostNameForCertCheck = NULL;
        }
    iHostNameForCertCheck = HBufC8::NewL( aHostName.Length() );
    TPtr8 ptr( iHostNameForCertCheck->Des() );
    ptr.Copy( aHostName );

    // Convert send data to 8-bit and add newline at the end
    if( iSendBuffer || iSendBufPtr )
        {
        delete iSendBuffer;
        iSendBuffer = NULL;
        delete iSendBufPtr;
        iSendBufPtr = NULL;
        }
    iSendBuffer = HBufC8::NewL( KData().Length() + 1 );
    iSendBufPtr = new( ELeave ) TPtr8( iSendBuffer->Des() );
    iSendBufPtr->Copy( KData );
    iSendBufPtr->Append( KNewLine );

    // Connect
    RDebug::Printf( "CTlsConnection::ConnectL iConnection.Open" );
    User::LeaveIfError( iConnection.Open( iSockServer ) );
    RDebug::Printf( "CTlsConnection::ConnectL iConnPref.SetDirection" );
    iConnPref.SetDirection( ECommDbConnectionDirectionOutgoing );
    if( iCurrentIap )
        {
        // uses the same IAP as before
        RDebug::Printf( "CTlsConnection::ConnectL SetIapId(%d)", iCurrentIap );
        iConnPref.SetIapId( iCurrentIap );
        iConnPref.SetDialogPreference( ECommDbDialogPrefDoNotPrompt );
        }
    else
        {
        // not defined, prompts the IAP from user
        RDebug::Printf( "CTlsConnection::ConnectL ECommDbDialogPrefPrompt" );
        iConnPref.SetDialogPreference( ECommDbDialogPrefPrompt );
        }
    StateChange( EConnectingNetwork );
    RDebug::Printf( "CTlsConnection::ConnectL iConnection.Start" );
    iConnection.Start( iConnPref, iStatus );
    SetActive();
    RDebug::Printf( "CTlsConnection::ConnectL end" );
    }

// -----------------------------------------------------------------------------
// CTlsConnection::Disconnect()
// -----------------------------------------------------------------------------
//
void CTlsConnection::Disconnect()
    {
    Cancel();
    CloseConnection();
    StateChange( EDisconnected );
    }

// -----------------------------------------------------------------------------
// CTlsConnection::RunL
// -----------------------------------------------------------------------------
//
void CTlsConnection::RunL()
    {
    if( iState == EReading && ( iStatus.Int() == KErrEof || iStatus.Int() == KErrSSLAlertCloseNotify ) )
        {
        RDebug::Printf( "CTlsConnection::RunL: EReading, EOF: iStatus = %d", iStatus.Int() );
        StateChange( EAllDone );
        }
    else if( iStatus.Int() == KErrNone )
        {
        switch ( iState )
            {
            case EConnectingNetwork:
                RDebug::Printf( "CTlsConnection::RunL: EConnectingNetwork" );
                if( !iCurrentIap )
                    {
                    iConnection.GetIntSetting( KCommDBIapId, iCurrentIap );
                    }
                if( iDelay )
                    {
                    TTimeIntervalMicroSeconds32 delay( iDelay * KOneSecondInMicroseconds );
                    iDelayTimer.After( iStatus, delay );
                    StateChange( EOpeningDelay );
                    }
                else
                    {
                    iConnection.ProgressNotification( iProgress, iStatus );
                    StateChange( EResolvingHostName );
                    }
                SetActive();
                break;

            case EOpeningDelay:
                iConnection.ProgressNotification( iProgress, iStatus );
                StateChange( EResolvingHostName );
                SetActive();
                break;

            case EResolvingHostName:
                {
                RDebug::Printf( "CTlsConnection::RunL: EResolvingHostName" );
                TInt err = iHostResolver.Open( iSockServer, KAfInet, KProtocolInetUdp, iConnection );
                User::LeaveIfError( err );
                iHostResolver.GetByName( *iHostName, iNameEntry, iStatus );
                StateChange( EConnectingServer );
                SetActive();
                }
                break;

            case EConnectingServer:
                {
                iRemoteAddr.SetFamily( KAfInet );
                iRemoteAddr = TInetAddr( iNameEntry().iAddr );
                iRemoteAddr.SetPort( iPort );

                RDebug::Printf( "CTlsConnection::RunL: EConnectingServer" );
                TInt err = iSocket.Open( iSockServer, KAfInet, KSockStream, KProtocolInetTcp, iConnection );
                RDebug::Printf( "CTlsConnection::RunL: iSocket.Open ret %d", err );
                User::LeaveIfError( err );
                iSocket.Connect( iRemoteAddr, iStatus );
                StateChange( EHandshaking );
                SetActive();
                }
                break;

            case EHandshaking:
                {
                RDebug::Printf( "CTlsConnection::RunL: EHandshaking" );
                if( iSecureSocket )
                    {
                    delete iSecureSocket;
                    iSecureSocket = NULL;
                    }
                iSecureSocket = CSecureSocket::NewL( iSocket, KSecureProtocol );
                iSecureSocket->SetDialogMode( EDialogModeAttended );
                // Server name must be set, otherwise CCTSecurityDialogsAO fails with KErrArgument
                iSecureSocket->SetOpt( KSoSSLDomainName, KSolInetSSL, *iHostNameForCertCheck );
                iSecureSocket->StartClientHandshake( iStatus );
                StateChange( EConnecting );
                SetActive();
                }
                break;

            case EConnecting:
                {
                RDebug::Printf( "CTlsConnection::RunL: EConnecting" );
                // Check if connection is closed
                const TInt stage( iProgress().iStage );
                if( stage == KConnectionClosed || stage == KLinkLayerClosed )
                    {
                    StateChange( EDisconnected );
                    }
                else
                    {
                    iConnection.ProgressNotification( iProgress, iStatus );
                    StateChange( ESending );
                    SetActive();
                    }
                }
                break;

            case ESending:
                RDebug::Printf( "CTlsConnection::RunL: ESending" );
                iSecureSocket->Send( *iSendBufPtr, iStatus, iSendLength );
                StateChange( EReading );
                SetActive();
                break;

            case EReading:
                iTotalRecvCount += iRecvLength();
                RDebug::Printf( "CTlsConnection::RunL: EReading, read %d bytes", iTotalRecvCount );
                iObserver.HandleTransferData( iRecvBuffer->Des(), iTotalRecvCount );

                iRecvBufPtr->Zero();
                iSecureSocket->RecvOneOrMore( *iRecvBufPtr, iStatus, iRecvLength );
                SetActive();
                break;

            default:
                break;
            }
        }
    else
        {
        RDebug::Printf( "CTlsConnection::RunL: ERROR iStatus %d", iStatus.Int() );
        CloseConnection();
        StateChange( EDisconnected, iStatus.Int() );
        }
    }

// ---------------------------------------------------------------------------
// CTlsConnection::DoCancel()
// ---------------------------------------------------------------------------
//
void CTlsConnection::DoCancel()
    {
    RDebug::Printf( "CTlsConnection::DoCancel(), iState=%d", iState );

    switch( iState )
        {
        case EConnectingNetwork:
            iConnection.Stop();
            break;

        case EOpeningDelay:
            iDelayTimer.Cancel();
            iConnection.Stop();
            break;

        case EResolvingHostName:
            iConnection.CancelProgressNotification();
            iConnection.Stop();
            break;

        case EConnectingServer:
            iHostResolver.Cancel();
            iConnection.Stop();
            break;

        case EHandshaking:
            iSocket.CancelConnect();
            iConnection.Stop();
            break;

        case EConnecting:
            if( iSecureSocket )
                {
                iSecureSocket->CancelHandshake();
                delete iSecureSocket;
                iSecureSocket = NULL;
                }
            iSocket.Close();
            iConnection.Stop();
            break;

        case ESending:
            if( iSecureSocket )
                {
                iSecureSocket->CancelSend();
                delete iSecureSocket;
                iSecureSocket = NULL;
                }
            iSocket.Close();
            iConnection.Stop();
            break;

        case EReading:
            if( iSecureSocket )
                {
                iSecureSocket->CancelRecv();
                delete iSecureSocket;
                iSecureSocket = NULL;
                }
            iSocket.Close();
            iConnection.Stop();
            break;

        case EAllDone:
        case EIdle:
        case EDisconnected:
        default:
            break;
        }

    StateChange( EDisconnected );
    }


// -----------------------------------------------------------------------------
// CTlsConnection::RunError()
// -----------------------------------------------------------------------------
//
TInt CTlsConnection::RunError( TInt aError )
    {
    RDebug::Printf( "CTlsConnection::RunError, aError: %d", aError );
    CloseConnection();
    StateChange( EDisconnected, aError );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CTlsConnection::CTlsConnection()
// -----------------------------------------------------------------------------
//
CTlsConnection::CTlsConnection( MTlsConnectionObserver& aObs ) :
        CActive( CActive::EPriorityStandard ), iObserver( aObs ), iState( ENotInitialized )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CTlsConnection::ConstructL()
// -----------------------------------------------------------------------------
//
void CTlsConnection::ConstructL()
    {
    RDebug::Printf( "CTlsConnection::ConstructL begin" );

    User::LeaveIfError( iDelayTimer.CreateLocal() );
    RDebug::Printf( "CTlsConnection::ConstructL iSockServer" );
    User::LeaveIfError( iSockServer.Connect() );
    RDebug::Printf( "CTlsConnection::ConstructL iConnection" );
    User::LeaveIfError( iConnection.Open( iSockServer ) );
    RDebug::Printf( "CTlsConnection::ConstructL buffers" );
    iRecvBuffer = HBufC8::NewL( KRecvBufferLength );
    iRecvBufPtr = new( ELeave ) TPtr8( iRecvBuffer->Des() );
    StateChange( EIdle );

    RDebug::Printf( "CTlsConnection::ConstructL end" );
    }

// ---------------------------------------------------------------------------
// CTlsConnection::CloseConnection()
// ---------------------------------------------------------------------------
//
void CTlsConnection::CloseConnection()
    {
    RDebug::Printf( "CTlsConnection::CloseConnection begin" );
    if( iSecureSocket )
        {
        delete iSecureSocket;
        iSecureSocket = NULL;
        }
    iSocket.Close();
    iHostResolver.Close();
    if( iConnection.SubSessionHandle() )
        {
        if( iState >= EConnectingNetwork )
            {
            TInt err = iConnection.Stop();
            if( err != KErrNone )
                {
                RDebug::Printf( "iConnection.Stop() failed: %d", err );
                }
            }
        iConnection.Close();
        }
    RDebug::Printf( "CTlsConnection::CloseConnection end" );
    }

// ---------------------------------------------------------------------------
// CTlsConnection::CloseSession()
// ---------------------------------------------------------------------------
//
void CTlsConnection::CloseSession()
    {
    RDebug::Printf( "CTlsConnection::CloseSession" );
    CloseConnection();
    iSockServer.Close();
    delete iHostName;
    iHostName = NULL;
    delete iHostNameForCertCheck;
    iHostNameForCertCheck = NULL;
    delete iRecvBuffer;
    iRecvBuffer = NULL;
    delete iRecvBufPtr;
    iRecvBufPtr = NULL;
    StateChange( EDisconnected );
    }

// -----------------------------------------------------------------------------
// CTlsConnection::StateChange()
// -----------------------------------------------------------------------------
//
void CTlsConnection::StateChange( TTlsConnectionState aNewState, TInt aError )
    {
    RDebug::Printf( "CTlsConnection::StateChange, aNewState=%d, aError=%d", aNewState, aError );
    if( aNewState != iState && iState != EAllDone )
        {
        iState = aNewState;
        iObserver.HandleNetworkEvent( aNewState, aError );
        }
    }

// ---------------------------------------------------------------------------
// CTlsConnection::ReadL()
// ---------------------------------------------------------------------------
//
void CTlsConnection::ReadL()
    {
    RDebug::Printf( "CTlsConnection::ReadL(), iState=%d", iState );

    TPtr8 recvBuffer = iRecvBuffer->Des();
    recvBuffer.Zero();
    iSecureSocket->RecvOneOrMore( recvBuffer, iStatus, iRecvLength );
    SetActive();
    }

