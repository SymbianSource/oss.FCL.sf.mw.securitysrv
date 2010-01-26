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
* Description:  Inline function for smart card server.
*
*/



// INCLUDE FILES
#include "ScardSession.h"
#include "ScardNotifyRegistry.h"
#include "ScardAccessControl.h"
#include "ScardAccessControlRegistry.h"
#include "ScardReaderRegistry.h"
#include "ScardResourceRegistry.h"
#include "ScardConnectionRegistry.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardServer::NotifyRegistry
// return pointer to registry
// -----------------------------------------------------------------------------
//
inline CScardNotifyRegistry* CScardServer::NotifyRegistry() const
    {
    return iNotifyRegistry;
    }

// -----------------------------------------------------------------------------
// CScardServer::FindAccessControl
// Return the access controller for the reader
// -----------------------------------------------------------------------------
//
inline CScardAccessControl* CScardServer::FindAccessControl(
    const TReaderID aReaderID ) const
    {
    return iControlRegistry->FindAccessController( aReaderID );
    }

// -----------------------------------------------------------------------------
// CScardServer::FactoryRegistry
// Return factory register
// -----------------------------------------------------------------------------
//
inline CScardReaderRegistry* CScardServer::FactoryRegistry() const
    {
    return iReaderFactoryRegistry;
    }

// -----------------------------------------------------------------------------
// CScardServer::ConnectionRegistry
// Return pointer to CScardConnectionRegistry
// -----------------------------------------------------------------------------
//
inline CScardConnectionRegistry* CScardServer::ConnectionRegistry() const
    {
    return iConnectionRegistry;
    }

// -----------------------------------------------------------------------------
// CScardServer::NotifyObserver
// Return pointer to observer
// -----------------------------------------------------------------------------
//
inline MScardNotifyObserver* CScardServer::NotifyObserver()
    {
    return iNotifyRegistry;
    }

// -----------------------------------------------------------------------------
// CScardServer::ReaderService
// Return service pointer
// -----------------------------------------------------------------------------
//
inline MScardReaderService* CScardServer::ReaderService()
    {
    return this;
    }

// -----------------------------------------------------------------------------
// CScardServer::ReaderID
// Scan database to see ID belonging to this name
// -----------------------------------------------------------------------------
//
inline const TReaderID CScardServer::ReaderID(
    const TScardReaderName& aFriendlyName ) const
    {
    return iReaderFactoryRegistry->ReaderID( aFriendlyName );
    }

// -----------------------------------------------------------------------------
// CScardServer::FriendlyName
// Scan the database for the name belonging to this ID
// -----------------------------------------------------------------------------
//
inline const TScardReaderName CScardServer::FriendlyName(
    const TReaderID aReaderID ) const
    {
    return iReaderFactoryRegistry->FriendlyName( aReaderID );
    }

// -----------------------------------------------------------------------------
// CScardServer::ReaderSupported
// Scan the database to see if we support this reader type
// -----------------------------------------------------------------------------
//
inline const TBool CScardServer::ReaderSupported(
    const TScardReaderName& aReaderName ) const
    {
    return iReaderFactoryRegistry->IsSupported( aReaderName );
    }

// -----------------------------------------------------------------------------
// CScardServer::AccessRegistry
// Return pointer to CScardAccessControlRegistry
// -----------------------------------------------------------------------------
//
inline CScardAccessControlRegistry* CScardServer::AccessRegistry() const
    {
    return iControlRegistry;
    }

// -----------------------------------------------------------------------------
// CScardServer::ReaderRegistry
// Return pointer to server's CScardReaderRegistry
// -----------------------------------------------------------------------------
//
inline CScardReaderRegistry* CScardServer::ReaderRegistry() const
    {
    return iReaderFactoryRegistry;
    }

// -----------------------------------------------------------------------------
// CScardServer::ConnectToReaderL
// Connect to reader
// -----------------------------------------------------------------------------
//
inline void CScardServer::ConnectToReaderL(
    CScardSession* aSession,
    const RMessage2& aMessage )
    {
    iConnectionRegistry->ConnectToReaderL( aSession, aMessage );
    }

// -----------------------------------------------------------------------------
// CScardServer::CardEvent
// Handle card event
// -----------------------------------------------------------------------------
//
inline void CScardServer::CardEvent(
    const TScardServiceStatus aEvent,
    const TReaderID aReaderID )
    {
    iResourceRegistry->CardEvent( aEvent, aReaderID );
    }

// End of File
