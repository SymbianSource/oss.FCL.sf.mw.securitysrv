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
* Description:   The header file of SimpleDHKey
*
*/


#ifndef __SIMPLEDHKEY_H__
#define __SIMPLEDHKEY_H__

#include <e32base.h>

/**
 * Wrapper class because we differ from crypto on what a DH key is
 *
 *  @lib
 *  @since S60 v3.2
 */
class CSimpleDHKey : public CBase
    {
    public:
        
        static CSimpleDHKey* NewL(TInt aSize);
        
        ~CSimpleDHKey();
    
    public:
        
        inline RInteger& DHKey() {return (iKey);};
    
    private:
        
        CSimpleDHKey() {};
        
        void ConstructL(TInt aSize);
    
    private:
        
        RInteger iKey;
    };

#endif //__SIMPLEDHKEY_H__