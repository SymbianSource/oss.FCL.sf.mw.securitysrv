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

#ifndef M_MTLSCONNECTIONOBSERVER_H
#define M_MTLSCONNECTIONOBSERVER_H

enum TTlsConnectionState
    {
    ENotInitialized = 0,
    EDisconnected,
    EIdle,
    EConnectingNetwork,
    EOpeningDelay,
    EResolvingHostName,
    EConnectingServer,
    EHandshaking,
    EConnecting,
    ESending,
    EReading,
    EAllDone
    };

class MTlsConnectionObserver
    {
    public:
        virtual void HandleNetworkEvent( TTlsConnectionState aEvent, TInt aError ) = 0;
        virtual void HandleTransferData( const TDesC8& aData, TInt aLength ) = 0;
    };

#endif // M_MTLSCONNECTIONOBSERVER_H
