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
* Description:   Implementation of the CDigSigningNote class
*
*/


// INCLUDE FILES
#include "CDigSigningNote.h"
#include "CTSecurityDialogRequestor.h"
#include "CTSecurityDialogDefs.h"
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

EXPORT_C MDigSigningNote* DigSigningNoteFactory::CreateNoteL()
    {
    return CDigSigningNote::NewL();
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CDigSigningNote::CDigSigningNote
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CDigSigningNote::CDigSigningNote(): CSecurityDialogBase()
    {
    }
// -----------------------------------------------------------------------------
// CDigSigningNote::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CDigSigningNote::ConstructL()
    {
    iSenderBuffer = CBufFlat::NewL( KCTSecDialSendBufLen );
    CSecurityDialogBase::ConstructL();
    }
// -----------------------------------------------------------------------------
// CDigSigningNote::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CDigSigningNote* CDigSigningNote::NewL()
    {
    CDigSigningNote* self = new( ELeave ) CDigSigningNote;
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CDigSigningNote::~CDigSigningNote()
    {
	delete iSenderBuffer;
	delete iBufferData;
    }


// -----------------------------------------------------------------------------
// CDigSigningNote::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CDigSigningNote::SaveSignedText( 
    const TDesC& aSignedText, 
    TRequestStatus& aStatus )
    {
    InitClientStatus(aStatus);

    iSenderBuffer->Reset();
	RBufWriteStream stream;
	CleanupClosePushL( stream );
	stream.Open( *iSenderBuffer );

	stream.WriteInt32L( ESaveReceipt );
    stream.WriteInt32L( aSignedText.Length() );
	stream << aSignedText;

    // new stuff begin
	delete iBufferData;
	iBufferData = NULL;
	iBufferData = HBufC8::NewL( iSenderBuffer->Ptr(0).Size() );
	iBufferData->Des() = iSenderBuffer->Ptr(0);
    // new stuff end
	CleanupStack::PopAndDestroy(); // stream.close;
	RequestDialog(*iBufferData);
    }
// -----------------------------------------------------------------------------
// CDigSigningNote::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CDigSigningNote::ShowPINBlockedInfo( 
    const TPINLabel& aLabel, 
    TRequestStatus& aStatus )
    {
    InitClientStatus(aStatus);

    iSenderBuffer->Reset();
	RBufWriteStream stream;
	CleanupClosePushL( stream );
	stream.Open( *iSenderBuffer );

	stream.WriteInt32L( EPinCodeBlockedInfo );
	stream << aLabel;
    // new stuff begin
	delete iBufferData;
	iBufferData = NULL;
	iBufferData = HBufC8::NewL( iSenderBuffer->Ptr(0).Size() );
	iBufferData->Des() = iSenderBuffer->Ptr(0);
    // new stuff end
	CleanupStack::PopAndDestroy(); // stream.close;
	RequestDialog(*iBufferData);
    }

// -----------------------------------------------------------------------------
// CDigSigningNote::ShowNote
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CDigSigningNote::ShowNote( 
    MDigSigningNote::TNoteType aNoteType, 
    TRequestStatus& aStatus )
    {
    ShowNoteL( aStatus, aNoteType );
    }
// -----------------------------------------------------------------------------
// CDigSigningNote::Release
//
// -----------------------------------------------------------------------------
//
void CDigSigningNote::Release()
    {
    delete this;
    }

//  End of File  
