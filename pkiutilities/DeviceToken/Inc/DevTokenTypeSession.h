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
* Description:   The header file of DevTokenTypeSession
*
*/



#ifndef __DEVTOKENTYPESESSION_H__
#define __DEVTOKENTYPESESSION_H__

#include <ct/ccttokentype.h>
#include "DevTokenClientSession.h"

/**
 * Token type session.
 * It is owned and created by the CDevTokenType so is not released until
 * all references to the tokentype object are relinquished.  It creates
 * RDevTokenClientSession, with which it communicates with the filetokens
 * server.
 *
 * @lib DevTokenClient.dll
 * @since S60 v3.2
 */
class CDevTokenTypeSession : public CBase
    {
    public:
    
        virtual ~CDevTokenTypeSession();
    
    public:
    
        static CDevTokenTypeSession* NewL(TUid aUid);
    
    public:
    
        void ListTokensL(RCPointerArray<HBufC>& aTokens);
    
        void OpenTokenL(const TDesC& aTokenInfo, MCTTokenType* aTokenType, MCTToken*& aToken);
    
        void OpenTokenL(TCTTokenHandle aHandle, MCTTokenType* aTokenType, MCTToken*& aToken);
    
    private:
    
        CDevTokenTypeSession(TUid aUid);
    
        void ConstructL();
    
    private:
    
        RDevTokenClientSession iClientSession;
    
        TUid iUid;
    
        EDevTokenEnum iTokenId;
    };

#endif  //  __DEVTOKENTYPESESSION_H__

//EOF

