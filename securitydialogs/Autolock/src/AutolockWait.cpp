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
#include	<eikenv.h>
#include	<eikappui.h>
#include	"AutolockWait.h"



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
// CSystemLock::ConstructL()	
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
// CWait::StartWaitForRequest
// 
// ----------------------------------------------------------
//
TInt CWait::WaitForRequestL()
    {	
	CWaitAbsorbingControl* absorbing = CWaitAbsorbingControl::NewLC();
	SetActive();
	iWait.Start();
	CleanupStack::PopAndDestroy(absorbing);
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
// class CWaitAbsorbingControl
//
CWaitAbsorbingControl::CWaitAbsorbingControl()
	{
	}

CWaitAbsorbingControl::~CWaitAbsorbingControl()
	{
	if (iCoeEnv && iAppUi)
		iAppUi->RemoveFromStack(this);
	}

CWaitAbsorbingControl* CWaitAbsorbingControl::NewLC()
	{
	CWaitAbsorbingControl* self= new(ELeave) CWaitAbsorbingControl();
	CleanupStack::PushL(self);
	self->ConstructL();
	return self;
	}

void CWaitAbsorbingControl::ConstructL()
	{
	CreateWindowL();
	SetExtent(TPoint(0,0), TSize(0,0));
	ActivateL();
	SetPointerCapture(ETrue);
	ClaimPointerGrab(ETrue);
	iAppUi=iEikonEnv->EikAppUi();
	iAppUi->AddToStackL(this, ECoeStackPriorityEnvironmentFilter);
	}

TKeyResponse CWaitAbsorbingControl::OfferKeyEventL(const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/)
	{
	return EKeyWasConsumed;
	}

// End of file
