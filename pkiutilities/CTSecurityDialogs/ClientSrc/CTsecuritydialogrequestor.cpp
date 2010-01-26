/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Definition of dialog requester
*
*/


// INCLUDE FILES
#include "CTSecurityDialogRequestor.h"
#include "CTSecurityDialogDefs.h"
#include <secdlgimpldefs.h>

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CCTSecurityDialogs::CCTSecurityDialogs()
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CCTSecurityDialogRequestor::CCTSecurityDialogRequestor(MSecurityDialogResponse& aSecurityDialogs): 
	CActive(EPriorityStandard),
	iSecurityDialogs(aSecurityDialogs)
{
}

// -----------------------------------------------------------------------------
// CCTSecurityDialogs::~CCTSecurityDialogs()
// -----------------------------------------------------------------------------
//
CCTSecurityDialogRequestor::~CCTSecurityDialogRequestor()
{
	Cancel();
	iNotifier.Close();
}

// -----------------------------------------------------------------------------
// CCTSecurityDialogs::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CCTSecurityDialogRequestor* CCTSecurityDialogRequestor::NewL(MSecurityDialogResponse& aSecurityDialogs)
{
	CCTSecurityDialogRequestor* self = new (ELeave) CCTSecurityDialogRequestor(aSecurityDialogs);
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(); // self
	return self;
}

// -----------------------------------------------------------------------------
// CCTSecurityDialogs::ConstructL()
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogRequestor::ConstructL()
{
	User::LeaveIfError(iNotifier.Connect());
	CActiveScheduler::Add(this);
}

// -----------------------------------------------------------------------------
// CCTSecurityDialogs::RequestDialog()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogRequestor::RequestDialog(const TDesC8& aData, TDes8& aResponse)
    {
	iNotifier.StartNotifierAndGetResponse(iStatus, KUidSecurityDialogNotifier, aData, aResponse);
	iState = EMakingRequest;
	SetActive();
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogs::RequestVariableBufferDialog()
// -----------------------------------------------------------------------------
//    
void CCTSecurityDialogRequestor::RequestVariableBufferDialog( const TDesC8& aOperationData,
										const TDesC8& aVariableData,
										const RArray<TCTTokenObjectHandle>& aCertHandleList,
										TDes8& aResponse )
	{
	iOperationDataPtr = &aOperationData;
	iVariableDataPtr = &aVariableData;
	iCertHandleListPtr = &aCertHandleList;
	iResponsePtr = &aResponse;
	iState = EFillingVariableInputBuffer;
	SetActive();
	// Complete the AO immediately so that buffer processing can occur in RunL().
	TRequestStatus* statusPtr = &iStatus;
	User::RequestComplete( statusPtr, KErrNone );
	}    

// -----------------------------------------------------------------------------
// CCTSecurityDialogs::DoCancel()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogRequestor::DoCancel()
    {
	iNotifier.CancelNotifier(KUidSecurityDialogNotifier);
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogs::RunL()
// -----------------------------------------------------------------------------
//
void CCTSecurityDialogRequestor::RunL()
    {
    switch ( iState )
		{
		case EFillingVariableInputBuffer:
			{
			TInt operationDataSize = iOperationDataPtr->Size();
			TInt variableDataSize = iVariableDataPtr->Size();
			TInt arraySize = sizeof( TCTTokenObjectHandle ) * iCertHandleListPtr->Count();
			TInt bufferSize = operationDataSize + variableDataSize + arraySize;
			iInputBuffer = HBufC8::NewL( bufferSize );
			TPtr8 inputBufferPtr( iInputBuffer->Des() );
			inputBufferPtr.Append( *iOperationDataPtr );
			for ( TInt h = 0, total = iCertHandleListPtr->Count(); h < total; ++h )
				{
				const TCTTokenObjectHandle& certHandle = (*iCertHandleListPtr)[h];
				TPckgC<TCTTokenObjectHandle> certHandleBuf( certHandle );
				inputBufferPtr.Append( certHandleBuf );
				}
			inputBufferPtr.Append( *iVariableDataPtr );
			RequestDialog( *iInputBuffer, *iResponsePtr );
			break;
			}
		case EMakingRequest:
			{
			delete iInputBuffer;
			iInputBuffer = NULL;
			iSecurityDialogs.HandleResponse(iStatus.Int());
			break;
			}
		default:
			__ASSERT_DEBUG( EFalse, _L( "Invalid state" ) );
		}
    }

// -----------------------------------------------------------------------------
// CCTSecurityDialogs::RunError()
// -----------------------------------------------------------------------------
//
TInt CCTSecurityDialogRequestor::RunError(TInt aError)
{
	iSecurityDialogs.HandleResponse(aError);
	return KErrNone;
}

// End of File
