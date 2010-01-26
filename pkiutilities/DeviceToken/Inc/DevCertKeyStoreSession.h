/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   The header file of DevCertKeyStoreSession
*
*/



#ifndef __DEVTOKENKEYSTORESESSION_H__
#define __DEVTOKENKEYSTORESESSION_H__

#include "DevTokenServer.h"

class CDevCertKeyStoreConduit;
class CDevCertOpenedKeySrv;

/**
 * Encapsulates an opened key object and its handle.
 */
struct TObjectIndex
    {
    TInt iHandle;
    CDevCertOpenedKeySrv* iObject;
    };

/**
 * A keystore session.
 * Handles client requests by passing them to the keystore conduit.  Keeps track
 * of uids/passphrases for the session.
 *
 *  @lib 
 *  @since S60 v3.2
 */
class CDevCertKeyStoreSession : public CDevTokenServerSession
    {
    public:
        
        /**
        * Create a new session object.
        *
        * @param aConduit The keystore conduit used to service user requests.
        * @return 
        */
        static CDevCertKeyStoreSession* NewL(CDevCertKeyStoreServer& aServer );
        
        virtual ~CDevCertKeyStoreSession();

        TInt AddOpenedKeyL(CDevCertOpenedKeySrv& aObject);
        
        void RemoveOpenedKeyL(TInt aHandle);
        
        CDevCertOpenedKeySrv* OpenedKey(TInt aHandle);

        /**
        * Determine if this session has a key open.
        *
        * @param aHandle The key data handle - not the session's open key handle!
        * @return ETrue if has open key
        */
        TBool HasOpenKey(TInt aHandle);

        CDevCertKeyStoreSession(CDevCertKeyStoreServer& aServer );
    
    private:
       
        virtual void DoServiceL(const RMessage2& aMessage);
    
    private:
        
        CDevCertKeyStoreServer& iServer;
        TInt iLastHandle;
        RArray<TObjectIndex> iOpenedKeys;
    };

#endif
