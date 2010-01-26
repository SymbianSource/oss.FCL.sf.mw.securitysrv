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
* Description:   Implementation of DevTokenKeyInfo
*
*/



#include "DevTokenDataTypes.h"

// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CDevTokenKeyInfo* CDevTokenKeyInfo::NewL(RReadStream& aStream)
// ---------------------------------------------------------------------------
// 
EXPORT_C CDevTokenKeyInfo* CDevTokenKeyInfo::NewL(RReadStream& aStream)
    {
    CDevTokenKeyInfo* me = new (ELeave) CDevTokenKeyInfo();
    me->CleanupPushL();
    me->ConstructL(aStream);
    CleanupStack::Pop(me);
    return me;
    }


// ---------------------------------------------------------------------------
// CDevTokenKeyInfo::SetUsePolicy(const TSecurityPolicy& aPolicy)
// ---------------------------------------------------------------------------
//
EXPORT_C void CDevTokenKeyInfo::SetUsePolicy(const TSecurityPolicy& aPolicy)
    {
    iUsePolicy = aPolicy;
    }


// ---------------------------------------------------------------------------
// CDevTokenKeyInfo::SetManagementPolicy(const TSecurityPolicy& aPolicy)
// ---------------------------------------------------------------------------
//
EXPORT_C void CDevTokenKeyInfo::SetManagementPolicy(const TSecurityPolicy& aPolicy)
    {
    iManagementPolicy = aPolicy;
    }


// ---------------------------------------------------------------------------
// CDevTokenKeyInfo::SetPKCS8AttributeSet(HBufC8* aPKCS8AttributeSet)
// ---------------------------------------------------------------------------
//
EXPORT_C void CDevTokenKeyInfo::SetPKCS8AttributeSet(HBufC8* aPKCS8AttributeSet)
    {
    delete iPKCS8AttributeSet;
    iPKCS8AttributeSet = aPKCS8AttributeSet;
    }

//EOF

