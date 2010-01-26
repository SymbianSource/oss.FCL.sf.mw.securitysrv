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
* Description:   Implementation of DTClient
*
*/



#include "DTClient.h"
#include "DevTokenUtils.h"
#include "DevTokenClientSession.h"


// ======== MEMBER FUNCTIONS ========

// -----------------------------------------------------------------------------
// CDTClient::CDTClient()
// -----------------------------------------------------------------------------
//
CDTClient::CDTClient(TInt aUID,
           MCTToken& aToken, 
           RDevTokenClientSession& aClient)
  : CActive(EPriorityNormal),
    iToken(aToken),   
    iInterfaceUID(aUID),  
    iClientSession(aClient),
    iRequestPtr(NULL, 0, 0)
    {
    } 


// -----------------------------------------------------------------------------
// CDTClient::~CDTClient()
// -----------------------------------------------------------------------------
//
CDTClient::~CDTClient()
    {
    Cancel();     
    delete iRequestDataBuf;
    }


// -----------------------------------------------------------------------------
// CDTClient::TAsyncRequest::~TAsyncRequest()
// -----------------------------------------------------------------------------
//
CDTClient::TAsyncRequest::~TAsyncRequest()
    {
    __ASSERT_DEBUG(EIdle==iRequest, DevTokenPanic(ERequestOutstanding));
    }


// -----------------------------------------------------------------------------
// CDTClient::TAsyncRequest::operator()
// -----------------------------------------------------------------------------
//
void CDTClient::TAsyncRequest::operator()(TDevTokenMessages aRequest, 
                                          TRequestStatus* aStatus)
    {
    __ASSERT_DEBUG(EIdle==iRequest, DevTokenPanic(ERequestOutstanding));
    iRequest = aRequest; 
    iClientStatus = aStatus;
    *aStatus = KRequestPending;
    }


// -----------------------------------------------------------------------------
// CDTClient::TAsyncRequest::Complete()
// -----------------------------------------------------------------------------
//
void CDTClient::TAsyncRequest::Complete(TInt aCompletionResult)
    {
    __ASSERT_DEBUG(EIdle!=iRequest, DevTokenPanic(ENoRequestOutstanding));
    User::RequestComplete(iClientStatus, aCompletionResult);
    iRequest = EIdle;
    }


// -----------------------------------------------------------------------------
// CDTClient::TAsyncRequest::Cancel()
// -----------------------------------------------------------------------------
//
void CDTClient::TAsyncRequest::Cancel()
    {
    User::RequestComplete(iClientStatus, KErrCancel);
    iRequest = EIdle;
    }


// -----------------------------------------------------------------------------
// CDTClient::TAsyncRequest::FreeRequestBuffer()
// -----------------------------------------------------------------------------
//
void CDTClient::FreeRequestBuffer() const
    {
    delete iRequestDataBuf; 
    iRequestDataBuf = NULL;
    iRequestPtr.Set(NULL, 0, 0);
    }


// -----------------------------------------------------------------------------
// CDTClient::AllocRequestBuffer(TInt aReqdSize) const
// -----------------------------------------------------------------------------
//
TInt CDTClient::AllocRequestBuffer(TInt aReqdSize) const
    {
    ASSERT(aReqdSize > 0);
    TInt result = KErrNoMemory;

    FreeRequestBuffer();
    iRequestDataBuf = HBufC8::NewMax(aReqdSize);
    if ( iRequestDataBuf )
        {
        iRequestPtr.Set(iRequestDataBuf->Des());
        iRequestPtr.FillZ();
        result = KErrNone;
        }
    return result;
    }


// -----------------------------------------------------------------------------
// CDTClient::SendSyncRequestAndHandleOverflowL()
// Execute a synchronous request that returns a buffer of indetermintate length.
// If the initial buffer is too short, the server leaves with KErrOverflow and
// passes us the required length - the reuest is then re-sent.
// -----------------------------------------------------------------------------
// 
void CDTClient::SendSyncRequestAndHandleOverflowL(TDevTokenMessages aMessage,
                          TInt aInitialBufSize,
                          const TIpcArgs& aArgs) const
    {
    User::LeaveIfError(AllocRequestBuffer(aInitialBufSize));

    TInt err = iClientSession.SendRequest(aMessage, aArgs);
    if ( err == KErrOverflow )
        {
        TInt sizeReqd = 0;
        TPckg<TInt> theSize(sizeReqd);
        theSize.Copy(iRequestPtr);
        User::LeaveIfError(AllocRequestBuffer(sizeReqd));     
        err = iClientSession.SendRequest(aMessage, aArgs);
        }
    User::LeaveIfError(err);
    }


// -----------------------------------------------------------------------------
// CDTClient::DoCancel()
// -----------------------------------------------------------------------------
// 
void CDTClient::DoCancel()
    {
    iCurrentRequest.Cancel();
    }


// -----------------------------------------------------------------------------
// CDTClient::RunError()
// -----------------------------------------------------------------------------
// 
TInt CDTClient::RunError(TInt aError)
    {
    iCurrentRequest.Complete(aError);
    return KErrNone; // Handled
    }

//EOF

