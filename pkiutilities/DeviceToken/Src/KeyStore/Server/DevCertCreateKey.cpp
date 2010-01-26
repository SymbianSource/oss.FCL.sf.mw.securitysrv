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
* Description:   Implementation of DevCertKeyDataManager
*
*/


#include <bigint.h>
#include "DevCertCreateKey.h"
#include "DevTokenServerDebug.h"
#include "SimpleDHKey.h"


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CSimpleDHKey::NewL()
// ---------------------------------------------------------------------------
//
CSimpleDHKey* CSimpleDHKey::NewL(TInt aSize)
    {
    CSimpleDHKey* me = new (ELeave) CSimpleDHKey();
    CleanupStack::PushL(me);
    me->ConstructL(aSize);
    CleanupStack::Pop(me);
    return (me);
    }


// ---------------------------------------------------------------------------
// CSimpleDHKey::ConstructL()
// ---------------------------------------------------------------------------
//
void CSimpleDHKey::ConstructL(TInt aSize)
    {
    iKey = RInteger::NewRandomL(aSize - 1);             
    }


// ---------------------------------------------------------------------------
// CSimpleDHKey::~CSimpleDHKey()
// ---------------------------------------------------------------------------
//
CSimpleDHKey::~CSimpleDHKey()
    {
    iKey.Close();
    }


// ---------------------------------------------------------------------------
// CDevCertKeyCreator::CDevCertKeyCreator()
// ---------------------------------------------------------------------------
//
CDevCertKeyCreator::CDevCertKeyCreator()
: CActive(EPriorityStandard), iAction(EIdle)
    {
    CActiveScheduler::Add(this);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyCreator::~CDevCertKeyCreator()
// ---------------------------------------------------------------------------
//
CDevCertKeyCreator::~CDevCertKeyCreator()
    {
    Cancel(); 
    iCreatorThread.LogonCancel(iStatus);
    iCreatorThread.Close();
    delete iCreateData;
    }


// ---------------------------------------------------------------------------
// CDevCertKeyCreator::DoCreateKeyAsyncL()
//  Spin a thread to create an appropriate key, if successful, 
// left on CleanupStack
// ---------------------------------------------------------------------------
//
void CDevCertKeyCreator::DoCreateKeyAsyncL(CDevTokenKeyInfo::EKeyAlgorithm aAlgorithm, TInt aSize, TRequestStatus& aStatus)
    {
    if ( (aSize <= 0) ||
    (aAlgorithm==CDevTokenKeyInfo::EInvalidAlgorithm) || 
    ((aAlgorithm!=CDevTokenKeyInfo::ERSA) && (aAlgorithm!=CDevTokenKeyInfo::EDSA) && (aAlgorithm!=CDevTokenKeyInfo::EDH)) )
    User::Leave(KErrArgument);

    iClientStatus = &aStatus;
    *iClientStatus = KRequestPending;

    iCreateData = new (ELeave) CDevCertKeyCreatorData(aAlgorithm, aSize);

    //  OK, ready to start the async operation...do it in RunL
    iAction = EReadyToCreateKey;
    iStatus = KRequestPending;
    SetActive();
    TRequestStatus* stat = &iStatus;
    User::RequestComplete(stat, KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyCreator::CreatorThreadEntryPoint()
// HERE'S THE THREAD TO CREATE THE KEY
// Code cannot leave in here, but not as many traps as it looks
// ---------------------------------------------------------------------------
//
TInt CDevCertKeyCreator::CreatorThreadEntryPoint(TAny* aParameters)
    {
    CTrapCleanup* cleanup = CTrapCleanup::New();
    if (!cleanup)
    User::Exit(KErrNoMemory);

    #ifdef _DEBUG
    DevTokenServerDebug::PauseOOMTest();
    #endif

    ASSERT(aParameters);
    TInt result = KErrNone;
    CDevCertKeyCreatorData* createData = static_cast<CDevCertKeyCreatorData*>(aParameters);
    switch (createData->iKeyAlgorithm)
        {
        case(CDevTokenKeyInfo::ERSA):
            {// Currently, CRT signing is not supported, in case the key is to be used
            //  for such, create a standard (private) key as part of the pair
            TRAP(result, createData->iKey.iRSAKey = CRSAKeyPair::NewL(createData->iSize));
            }
        break;
        case (CDevTokenKeyInfo::EDSA):
            {
            TRAP(result, createData->iKey.iDSAKey = CDSAKeyPair::NewL(createData->iSize));
            }
        break;
        case (CDevTokenKeyInfo::EDH):
            {// Generate a number that's less than N. The snag is that
            //  we don't know what N is. We do know that it'll be of a
            //  particular size, so we can safely generate any number
            //  with less than iSize digits
            TRAP(result, createData->iKey.iDHKey = CSimpleDHKey::NewL(createData->iSize));        
            }
        break;
        default:
        ASSERT(EFalse);
        result = KErrArgument;
        }

    #ifdef _DEBUG
    DevTokenServerDebug::ResumeOOMTest();
    #endif

    delete cleanup;
    User::Exit(result);
    return (KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyCreator::GetCreatedRSAKey()
// ---------------------------------------------------------------------------
//
CRSAKeyPair* CDevCertKeyCreator::GetCreatedRSAKey()
    {// Check algorithm is as expected, return NULL if no key or wrong type
    if ( (!iCreateData) || (CDevTokenKeyInfo::ERSA!=iCreateData->iKeyAlgorithm) )
    return (NULL);
    else
    return (iCreateData->iKey.iRSAKey);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyCreator::GetCreatedDSAKey()
// ---------------------------------------------------------------------------
//
CDSAKeyPair* CDevCertKeyCreator::GetCreatedDSAKey()
    {// Check algorithm is as expected, return NULL if no key or wrong type
    if ( (!iCreateData) || (CDevTokenKeyInfo::EDSA!=iCreateData->iKeyAlgorithm) )
    return (NULL);
    else
    return (iCreateData->iKey.iDSAKey);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyCreator::GetCreatedDHKey()
// ---------------------------------------------------------------------------
//
void CDevCertKeyCreator::GetCreatedDHKey(RInteger& aDHKey)
    {
    ASSERT(iCreateData);
    ASSERT(CDevTokenKeyInfo::EDH==iCreateData->iKeyAlgorithm);    
    aDHKey = iCreateData->iKey.iDHKey->DHKey();
    }


// ---------------------------------------------------------------------------
// CDevCertKeyCreator::DoCancel()
// ---------------------------------------------------------------------------
//
void CDevCertKeyCreator::DoCancel()
    {// Only do the cancel if in the middle of creating a key.  Kill the thread.
    if (iAction!=EIdle)
        {
        TExitType exitType = iCreatorThread.ExitType();
        if (EExitPending==exitType) //  Still alive, so kill it
        iCreatorThread.Kill(KErrCancel);

        iAction = EIdle;
        }

    ASSERT(iClientStatus);
    User::RequestComplete(iClientStatus, KErrCancel);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyCreator::RunL()
// ---------------------------------------------------------------------------
//
void CDevCertKeyCreator::RunL()
    {
    ASSERT(iClientStatus);

    switch (iAction)
        {
        case (EReadyToCreateKey):
            {// Spin off the thread and pass it the parameter data, then stand by
            User::LeaveIfError(iCreatorThread.Create(KCreateKeyName, CreatorThreadEntryPoint, KDefaultStackSize, NULL, (TAny*)iCreateData));
            iStatus = KRequestPending;
            iCreatorThread.Logon(iStatus);
            iAction = ECreatedKey;
            SetActive();
            iCreatorThread.Resume();
            }
        break;
        case (ECreatedKey):
            {// Notify the caller
            ASSERT(iClientStatus);

            // May be OOM creating logon, in which case we should kill thread
            if (iStatus.Int() == KErrNoMemory)
                {
                TExitType exitType = iCreatorThread.ExitType();
                if (EExitPending==exitType) //  Still alive, so kill it
                iCreatorThread.Kill(KErrNone);
                }

            User::RequestComplete(iClientStatus, iStatus.Int());
            iAction = EIdle;
            }
        break;
        default:
        ASSERT(EFalse);
        }
    }


// ---------------------------------------------------------------------------
// CDevCertKeyCreator::RunError()
// ---------------------------------------------------------------------------
//
TInt CDevCertKeyCreator::RunError(TInt anError)
    {
    if (iClientStatus)
        {
        User::RequestComplete(iClientStatus, anError);
        }
    return (KErrNone);
    }


// ---------------------------------------------------------------------------
// CDevCertKeyCreator::CDevCertKeyCreatorData::CDevCertKeyCreatorData()
// ---------------------------------------------------------------------------
//
CDevCertKeyCreator::CDevCertKeyCreatorData::CDevCertKeyCreatorData(CDevTokenKeyInfo::EKeyAlgorithm aAlgorithm, TInt aSize)
: iSize(aSize), iKeyAlgorithm(aAlgorithm)
    {}


// ---------------------------------------------------------------------------
// CDevCertKeyCreator::CDevCertKeyCreatorData::~CDevCertKeyCreatorData()
// ---------------------------------------------------------------------------
//
CDevCertKeyCreator::CDevCertKeyCreatorData::~CDevCertKeyCreatorData()
    {
    if (iKeyAlgorithm==CDevTokenKeyInfo::ERSA)
        {
    	delete iKey.iRSAKey;
        }
    else if (iKeyAlgorithm==CDevTokenKeyInfo::EDSA)
        {
        delete iKey.iDSAKey;	
        }
    else if (iKeyAlgorithm==CDevTokenKeyInfo::EDH)
        {
    	delete iKey.iDHKey;
        }
    }
    
//EOF

