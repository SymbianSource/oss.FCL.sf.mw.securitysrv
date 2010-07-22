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

#ifndef C_TESTTLSCONNECTION_H
#define C_TESTTLSCONNECTION_H

#include <es_sock.h>                    // RConnection
#include <in_sock.h>                    // TInetAddr
#include <commdbconnpref.h>             // TCommDbConnPref
#include "tlsconnectionobserver.h"      // MTlsConnectionObserver, TTlsConnectionState

class CSecureSocket;


class CTlsConnection : public CActive
    {
    public:     // new functions
        static CTlsConnection* NewL( MTlsConnectionObserver& aObs );
        ~CTlsConnection();
        void ConnectL( const TDesC& aHostName, TInt aPort, TInt aDelay );
        void Disconnect();

    protected:  // from CActive
        void RunL();
        void DoCancel();
        TInt RunError( TInt aError );

    private:    // constructors
        CTlsConnection( MTlsConnectionObserver& aObs );
        void ConstructL();

    private:    // new functions
        void CloseConnection();
        void CloseSession();
        void StateChange( TTlsConnectionState aNewState, TInt aError = KErrNone );
        void ReadL();

    private:    // data
        MTlsConnectionObserver& iObserver;
        RConnection iConnection;
        RSocketServ iSockServer;
        TCommDbConnPref iConnPref;
        TUint32 iCurrentIap;
        TNifProgressBuf iProgress;
        HBufC* iHostName;
        HBufC8* iHostNameForCertCheck;
        TInt iPort;
        TInt iDelay;
        RHostResolver iHostResolver;
        TNameEntry iNameEntry;
        TInetAddr iRemoteAddr;
        RSocket iSocket;
        CSecureSocket* iSecureSocket;
        HBufC8* iRecvBuffer;
        TPtr8* iRecvBufPtr;
        TSockXfrLength iRecvLength;
        HBufC8* iSendBuffer;
        TPtr8* iSendBufPtr;
        TSockXfrLength iSendLength;
        TInt iTotalRecvCount;
        TTlsConnectionState iState;
        RTimer iDelayTimer;
    };


#endif // C_TESTTLSCONNECTION_H

