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
* Description:   Implementation of DevCertKeyEncryptor
*
*/


#include <ecom/ecom.h>
#include "DevCertKeyEncryptor.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CDevCertKeyEncryptor::NewL()
// ---------------------------------------------------------------------------
//
CDevCertKeyEncryptor* CDevCertKeyEncryptor::NewL()
    {
    CDevCertKeyEncryptor* self = NewLC();
    CleanupStack::Pop( self );
    return self;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyEncryptor::NewLC()
// ---------------------------------------------------------------------------
//
CDevCertKeyEncryptor* CDevCertKeyEncryptor::NewLC()
    {
    CDevCertKeyEncryptor* self = new(ELeave)CDevCertKeyEncryptor();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyEncryptor::~CDevCertKeyEncryptor()
// ---------------------------------------------------------------------------
//
CDevCertKeyEncryptor::~CDevCertKeyEncryptor()
    {
    REComSession::DestroyedImplementation(iDtorIDKey);  

    if(iKeyEncryptor)
        {
        iKeyEncryptor->Release(); 
        }
    }


// ---------------------------------------------------------------------------
// CDevCertKeyEncryptor::CreateImplementationL()
// ---------------------------------------------------------------------------
//
MKeyEncryptor* CDevCertKeyEncryptor::CreateImplementationL()
    {
    TAny* ptr = REComSession::CreateImplementationL(iImplementationUID, iDtorIDKey);
    iKeyEncryptor = static_cast<MKeyEncryptor*>(ptr);
    return iKeyEncryptor;   
    }


// ---------------------------------------------------------------------------
// CDevCertKeyEncryptor::IsPluginExistL()
// ---------------------------------------------------------------------------
//
TBool CDevCertKeyEncryptor::IsPluginExistL()
    {
    RImplInfoPtrArray array;    
    REComSession::ListImplementationsL(KKeyEncryptorInterfaceUID, array);

    if( array.Count())
        { 
        iImplementationUID  = array[0]->ImplementationUid();
        array.ResetAndDestroy();
        return ETrue;
        }
    else
        {
        array.ResetAndDestroy();  
        return EFalse;
        }
    }


// ---------------------------------------------------------------------------
// CDevCertKeyEncryptor::CDevCertKeyEncryptor()
// ---------------------------------------------------------------------------
//
CDevCertKeyEncryptor::CDevCertKeyEncryptor()
    {
    }


// ---------------------------------------------------------------------------
// CDevCertKeyEncryptor::ConstructL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyEncryptor::ConstructL()
    {
    }

//EOF

