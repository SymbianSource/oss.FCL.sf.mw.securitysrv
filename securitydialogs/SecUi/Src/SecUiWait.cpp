/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description: 
*    
*
*/



#include    <e32base.h>
#include    <eikenv.h>
#include    <eikappui.h>
#include    "SecUiWait.h"



// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CWait::NewL()    
// 
// ----------------------------------------------------------
// 
CWait* CWait::NewL()
    {
    CWait* self = new(ELeave) CWait();
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }
//
// ----------------------------------------------------------
// CWait::ConstructL()    
// 
// ----------------------------------------------------------
// 
void CWait::ConstructL()    
    {    
    CActiveScheduler::Add(this);            
    }    
//
// ----------------------------------------------------------
// CWait::CWait()
// 
// ----------------------------------------------------------
//
CWait::CWait() : CActive(0)
    { 
    }
//
// ----------------------------------------------------------
// CWait::~CWait()
// Destructor
// ----------------------------------------------------------
//
CWait::~CWait()
    {
		Cancel();
    }
//
// ----------------------------------------------------------
// CWait::WaitForRequestL()
// 
// ----------------------------------------------------------
//
TInt CWait::WaitForRequestL()
    {    
    SetActive();
    iWait.Start();
    return iStatus.Int();
    }
//
// ----------------------------------------------------------
// CWait::RunL()
// 
// ----------------------------------------------------------
// 
void CWait::RunL()
    {        
    if(iWait.IsStarted())        
        iWait.AsyncStop();
    }
//
// ----------------------------------------------------------
// CWait::DoCancel()
// Cancels code request
// ----------------------------------------------------------
//
void CWait::DoCancel()
    {
    if(iWait.IsStarted())
        iWait.AsyncStop();
    }

//
// ----------------------------------------------------------
// CWait::SetRequestType
// Sets active request type
// ----------------------------------------------------------
//
void CWait::SetRequestType(TInt aRequestType)
{
    iRequestType = aRequestType;
}

//
// ----------------------------------------------------------
// CWait::GetRequestType
// Gets active request type
// ----------------------------------------------------------
//
TInt CWait::GetRequestType()
{
    return iRequestType;
}
//
// class CAutolockQuery
//
CAutolockQuery::CAutolockQuery()
    {
    }

CAutolockQuery::~CAutolockQuery()
    {
    
    }

CAutolockQuery* CAutolockQuery::NewLC()
    {
    return NULL;
    }

void CAutolockQuery::ConstructL()
    {
    
    }

TKeyResponse CAutolockQuery::OfferKeyEventL(const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/)
    {
    return EKeyWasConsumed; 
    }
/***********/
//
// class CWaitAbsorbingControl
//
CWaitAbsorbingControl::CWaitAbsorbingControl()
    {
    }

CWaitAbsorbingControl::~CWaitAbsorbingControl()
    {
    
    }

CWaitAbsorbingControl* CWaitAbsorbingControl::NewLC()
    {
    return NULL;
    }

void CWaitAbsorbingControl::ConstructL()
    {
    
    }

TKeyResponse CWaitAbsorbingControl::OfferKeyEventL(const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/)
    {
    return EKeyWasConsumed; 
    }

// End of file
