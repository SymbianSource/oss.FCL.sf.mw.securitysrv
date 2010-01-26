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
* Description:   The header file of DevToken
*
*/



#ifndef __DEVTOKEN_H__
#define __DEVTOKEN_H__

#include <ct.h>
#include "DevTokenClientSession.h"
#include "DevTokenTypesEnum.h"


/**
 * Devicetokens client side representation of MCTToken, 
 * provides client-side reference counting
 * which calls server with ECloseToken request when appropriate.
 * 
 * Created by CDevTokenTypeSession as part of OpenToken call
 * Uses RDevTokenClientSession to communicate with DevTokens server
 *
 *  @lib DevTokenClient.dll
 *  @since S60 v3.2
 */
class CDevToken : public CBase, public MCTToken
    {
    public:
    
        static MCTToken* NewL(EDevTokenEnum aTokenTypeVal, MCTTokenType* aTokenType, RDevTokenClientSession& aClient);
    
    public: //  From MCTToken
    
        virtual MCTTokenType& TokenType();
    
        virtual const TDesC& Label();
    
        virtual TCTTokenHandle Handle();
    
    protected:  //  From MCTToken
    
        virtual TInt& ReferenceCount();
    
        virtual void DoGetInterface(TUid aRequiredInterface, MCTTokenInterface*& aReturnedInterface, TRequestStatus& aStatus);
    
        virtual TBool DoCancelGetInterface();
    
        virtual const TDesC& Information(TTokenInformation aRequiredInformation);
    
    private:
    
        CDevToken(EDevTokenEnum aTokenTypeVal, MCTTokenType* aTokenType, RDevTokenClientSession& aClient);
    
    private:
    
        EDevTokenEnum iTokenEnum;
    
        MCTTokenType* iTokenType;
    
        TInt iRefCount;
    
        RDevTokenClientSession& iClientSession;
    };

#endif  //__DEVTOKEN_H__

//EOF

