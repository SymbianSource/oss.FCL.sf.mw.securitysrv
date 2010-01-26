/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  class CGbaServerSession definition
*
*/


#ifndef GBASESSION_H
#define GBASESSION_H

#include <e32base.h>
#include "GbaServer.h"
#include "bootstrap.h" 
#include "GbaCommon.h"

class CBootstrapStateMachine;

class CGbaServerSession : public CSession2
    {
    public: 
        static CGbaServerSession* NewL();
        virtual ~CGbaServerSession();
        //Server reference
        CGbaServer* Server();
       
    private:
        void ConstructL();
        CGbaServerSession();
        //Handle the event cause when Bootstrapping is completed.
        void StateMachineCallBack( TInt aError ) ;
        void ServiceL(const RMessage2& aMessage);
        void CreateL() ;
        void RequestBootstrapL(const RMessage2& aMessage);

    private: 
        RMessage2  iMessage;
        C3GPPBootstrap* i3GPPBootstrap;
        TGBABootstrapInputParams    iGbaInputParams;
        TGBABootstrapOutputParams   iGbaOutputParams;
    
    friend class C3GPPBootstrap;
    };

#endif //GBASESSION_H
//EOF
