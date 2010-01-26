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
* Description:   Implementation of the CSecurityDialogBase class.
*
*/


// INCLUDE FILES
#include    "SecurityDialogBase.h"
#include <s32mem.h>

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSecurityDialogBase::CSecurityDialogBase
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CSecurityDialogBase::CSecurityDialogBase()
    {
    }

// -----------------------------------------------------------------------------
// CSecurityDialogBase::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CSecurityDialogBase::ConstructL()
    {
    iRequester = CCTSecurityDialogRequestor::NewL( *this );
    iSenderBuffer = CBufFlat::NewL( KCTSecDialSendBufLen );
    }

    
// Destructor
CSecurityDialogBase::~CSecurityDialogBase()
    {
    if ( iRequester )
        {
		iRequester->Cancel();
        }
	delete iRequester;
    delete iSenderBuffer;
    delete iBufferData; 
    }

// -----------------------------------------------------------------------------
// CSecurityDialogBase::HandleResponse
// HandleResponse handles the dialog Requestor's responses to dialogs 
// and updates the client component's variables.
// -----------------------------------------------------------------------------
//
void CSecurityDialogBase::HandleResponse( TInt aResult )
    {
	User::RequestComplete(iClientStatus, aResult);
    }

// -----------------------------------------------------------------------------
// CSecurityDialogBase::InitClientStatus
// InitClientStatus initialises the client's RequestStatus object.
// -----------------------------------------------------------------------------
//
void CSecurityDialogBase::InitClientStatus( TRequestStatus& aStatus )
	{
	iClientStatus = &aStatus;
	*iClientStatus = KRequestPending;
	}
// -----------------------------------------------------------------------------
// CSecurityDialogBase::ShowNoteL
// -----------------------------------------------------------------------------
//

void CSecurityDialogBase::ShowNoteL(
                            TRequestStatus& aStatus, 
                            TInt aDlgType,  
                            const TDesC& aDynamic)
    {

	InitClientStatus( aStatus );
	
	iSenderBuffer->Reset();
	RBufWriteStream stream;
	CleanupClosePushL( stream );
	stream.Open( *iSenderBuffer );

	stream.WriteInt32L( aDlgType );
	stream.WriteInt32L( aDynamic.Length() );
	stream << aDynamic;

	stream.WriteInt32L(0);
	stream << KNullDesC;

    delete iBufferData;
    iBufferData = NULL;
	iBufferData = HBufC8::NewL(iSenderBuffer->Ptr(0).Size());
	iBufferData->Des() = iSenderBuffer->Ptr(0);
	CleanupStack::PopAndDestroy(); // stream.close;

	RequestDialog(*iBufferData);
    }

// -----------------------------------------------------------------------------
// CSecurityDialogBase::RequestDialog
// -----------------------------------------------------------------------------
//
void CSecurityDialogBase::RequestDialog(const TDesC8& aData)
    {
    iRequester->RequestDialog(aData, iDlgResponseBuf);
    }

// -----------------------------------------------------------------------------
// CSecurityDialogBase::Requester
// -----------------------------------------------------------------------------
//    
CCTSecurityDialogRequestor& CSecurityDialogBase::Requester()
    {
    return *iRequester;
    }
// -----------------------------------------------------------------------------
// CSecurityDialogBase::CheckError
// -----------------------------------------------------------------------------
//
void CSecurityDialogBase::CheckError( TInt aError )
    {
    if (aError)
        {
        User::RequestComplete(iClientStatus, aError);
        }
    }

//  End of File  
