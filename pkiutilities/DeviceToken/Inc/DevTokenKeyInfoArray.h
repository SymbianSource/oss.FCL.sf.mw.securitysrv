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
* Description:   The header file of DevTokenKeyInfoArray
*
*/



#ifndef __DEVTOKENKEYINFOARRAY_H__
#define __DEVTOKENKEYINFOARRAY_H__

#include "DevTokenDataTypes.h" //MDevTokenKeyInfoArray

/**
 * A class that wraps an array of key info pointers.
 * Implements the MDevTokenKeyInfoArray interface required for marshalling key info
 * arrays.  
 *
 *  @lib DevTokenClient.dll
 *  @since S60 v3.2
 */
class TDevTokenKeyInfoArray : public MDevTokenKeyInfoArray
    {
    public:
    
        TDevTokenKeyInfoArray(RMPointerArray<CCTKeyInfo>& aArray) : iArray(aArray) { }
        
        virtual TInt Count() { return iArray.Count(); }
        
        virtual CCTKeyInfo* operator[](TInt aIndex) { return static_cast<CCTKeyInfo*>(iArray[aIndex]); }
        
        virtual TInt Append(CCTKeyInfo* aInfo) { return iArray.Append(aInfo); }
        
        virtual void Close() { iArray.Close(); }
    
    private:
        
        RMPointerArray<CCTKeyInfo>& iArray;
    };
    
#endif //__DEVTOKENKEYINFOARRAY_H__

//EOF

