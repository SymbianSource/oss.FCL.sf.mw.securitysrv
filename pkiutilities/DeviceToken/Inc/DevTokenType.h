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
* Description:   The header file of DevTokenType
*
*/



#ifndef __CDEVTOKENTYPE_H__
#define __CDEVTOKENTYPE_H__

#include <ct/ccttokentype.h>

class CDevTokenTypeSession; 

/**
 * DevToken Type
 * Derived from CCTTokenType to represent devicetoken type
 * 
 *  @lib DevTokenClient.dll
 *  @since S60 v3.2
 */
class CDevTokenType :  public CCTTokenType
    {
    public:
    
        /** 
        * Creates a client side representation of the CCTTokenType represented by the UID parameter
        * Called by ECom or directly accessible.  Initiates a session with the filetokens server, and
        * corresponding token type access server-side
        *
        *  @param aUid The UID of the appropriate token type (devcertstore, devkeystore)
        *  @return
        */
        IMPORT_C static CCTTokenType* NewL(TUid aUid);
    
    public:
    
        virtual ~CDevTokenType();
    
    public: 
         
        //  From MCTTokenType
        virtual void List(RCPointerArray<HBufC>& aTokens, TRequestStatus& aStatus);
        
        virtual void CancelList();
        
        virtual void OpenToken(const TDesC& aTokenInfo, MCTToken*& aToken, TRequestStatus& aStatus);
        
        virtual void OpenToken(TCTTokenHandle aHandle, MCTToken*& aToken, TRequestStatus& aStatus);
        
        virtual void CancelOpenToken();
    
    private:
    
        void OpenToken(MCTTokenType* aTokenType);
    
    private:
    
        CDevTokenType();
    
        void ConstructL(TUid aUid);
    
    public:
    
        TUid iUid;    
    
    private:
    
        CDevTokenTypeSession* iTokenTypeSession;
    };

#endif  //  __CDTTOKENTYPE_H__

//EOF

