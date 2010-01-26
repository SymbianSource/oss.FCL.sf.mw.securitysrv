/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of the CPKIDialog class
*
*/


// INCLUDE FILES
#include "CPKIDlg.h"
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

EXPORT_C MPKIDialog* PKIDialogFactory::CreateNoteL()
    {
    return CPKIDialog::NewL();
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CPKIDialog::CPKIDialog
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CPKIDialog::CPKIDialog()
    {
    }
// -----------------------------------------------------------------------------
// CPKIDialog::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CPKIDialog::ConstructL()
    {
    iRequester = CCTSecurityDialogRequestor::NewL( *this );
    iSenderBuffer = CBufFlat::NewL( KCTSecDialSendBufLen );
    }
// -----------------------------------------------------------------------------
// CPKIDialog::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CPKIDialog* CPKIDialog::NewL()
    {
    CPKIDialog* self = new( ELeave ) CPKIDialog;
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CPKIDialog::~CPKIDialog()
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
// CPKIDialog::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CPKIDialog::UserAuthentication(
    const RArray<TCTTokenObjectHandle>& aCertHandleList, 
    TCTTokenObjectHandle& aCertHandle,
    TRequestStatus& aStatus )
    {
	iCurrentOperation = EUserAuthentication;
	InitClientStatus( aStatus );

	TSignInput signInput;
	signInput.iOperation = EUserAuthentication;
	signInput.iCertHandleArrayTotal = aCertHandleList.Count();
	TPckgC<TSignInput> signInputBufPtr( signInput );
	// iSignInputBuf is persistent because further input processing occurs in the AO before
	// calling the notifier.
	iSignInputBuf.Copy( signInputBufPtr );
	iVariableDataPtr.Set( NULL, 0 );
	iClientCertInfoHandlePtr = &aCertHandle;

	iRequester->RequestVariableBufferDialog( iSignInputBuf, iVariableDataPtr,
											aCertHandleList, iCertInfoHandleBuf );
    }
    
// -----------------------------------------------------------------------------
// CPKIDialog::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//    
void CPKIDialog::UserAuthenticationText( 
    const TDesC& aTextToSign,
    const RArray<TCTTokenObjectHandle>& aCertHandleList, 
    TCTTokenObjectHandle& aCertHandle,
    TRequestStatus& aStatus )
    {
    iCurrentOperation = EUserAuthenticationText;
	InitClientStatus( aStatus );

	TInt textSize = aTextToSign.Size();
	TSignInput signInput;
	signInput.iOperation = EUserAuthenticationText;
	signInput.iVariableDataSize = textSize;
	signInput.iCertHandleArrayTotal = aCertHandleList.Count();
	TPckgC<TSignInput> signInputBufPtr( signInput );
	// iSignInputBuf is persistent because further input processing occurs in the AO before
	// calling the notifier.
	iSignInputBuf.Copy( signInputBufPtr );
	const TUint8* textToSignPtr = reinterpret_cast<const TUint8*>( aTextToSign.Ptr() );
	iVariableDataPtr.Set( textToSignPtr, textSize );

	iClientCertInfoHandlePtr = &aCertHandle;

	iRequester->RequestVariableBufferDialog( iSignInputBuf, iVariableDataPtr,
											aCertHandleList, iCertInfoHandleBuf );
    }

// -----------------------------------------------------------------------------
// CPKIDialog::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CPKIDialog::RequestWithTokenHandleL( 
    const TDesC& aText,
    const TCTTokenObjectHandle& aTokenHandle,
    TRequestStatus& aStatus )    
    {
    InitClientStatus(aStatus);

    iSenderBuffer->Reset();
	RBufWriteStream stream;
	CleanupClosePushL( stream );
	stream.Open( *iSenderBuffer );

	stream.WriteInt32L( iCurrentOperation );
	if (0 < aText.Length())
	    {
	    stream.WriteInt32L( aText.Length() );
	    stream << aText;    
	    }
    // new stuff begin
	delete iBufferData;
	TInt inputBufferSize = 
	    iSenderBuffer->Ptr(0).Size() + sizeof(TCTTokenObjectHandle);
	
	iBufferData = HBufC8::NewL( inputBufferSize );
	TPtr8 inputBufferPtr( iBufferData->Des() );
	inputBufferPtr = iSenderBuffer->Ptr(0);
	
	TPckgC<TCTTokenObjectHandle> handleBuf( aTokenHandle );
	inputBufferPtr.Append( handleBuf );
	
    // new stuff end
	CleanupStack::PopAndDestroy(); // stream.close;
	iRequester->RequestDialog(*iBufferData, iDlgResponseBuf);
    }
    
// -----------------------------------------------------------------------------
// CPKIDialog::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//        
void CPKIDialog::CreateCSR( 
    const TDesC& aSubject,
    const TCTTokenObjectHandle& aKeyHandle,
    TRequestStatus& aStatus )
    {
    iCurrentOperation = ECreateCSR;
    TRAPD(ret, RequestWithTokenHandleL(aSubject, aKeyHandle, aStatus));
    if (ret!=KErrNone)
        {
        User::RequestComplete(iClientStatus, ret);
        }
        
    }

// -----------------------------------------------------------------------------
// CPKIDialog::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CPKIDialog::CertificateDetails( 
    const TCTTokenObjectHandle& aCertHandle,
    TRequestStatus& aStatus  )
    {
    iCurrentOperation = ECertDetails;
    TRAPD(ret, RequestWithTokenHandleL(KNullDesC, aCertHandle, aStatus));
    if (ret!=KErrNone)
        {
        User::RequestComplete(iClientStatus, ret);
        }
    }

// -----------------------------------------------------------------------------
// CPKIDialog::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CPKIDialog::SaveCertificate( 
    TCertificateFormat aFormat,
    TCertificateOwnerType aCertificateOwnerType, 
    const TDesC8& aCert, TRequestStatus& aStatus)
    {
    TRAPD(ret, DoSaveCertificateL(aFormat, aCertificateOwnerType, aCert, aStatus));
    if (ret!=KErrNone)
        {
        User::RequestComplete(iClientStatus, ret);
        }
    }

// -----------------------------------------------------------------------------
// CPKIDialog::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CPKIDialog::DoSaveCertificateL( 
    TCertificateFormat aFormat,
    TCertificateOwnerType aCertificateOwnerType, 
    const TDesC8& aCert, TRequestStatus& aStatus)
    {
    iCurrentOperation = ESaveCert;
	InitClientStatus( aStatus );
	
	TSaveCertInput saveCertInput;
	saveCertInput.iOperation = ESaveCert;
	saveCertInput.iCertFormat = aFormat;
	saveCertInput.iCertOwnerType = aCertificateOwnerType;
	saveCertInput.iDataSize = aCert.Size();
	
	TPckgC<TSaveCertInput> saveCertInputBuf( saveCertInput );
	
	//const TUint8* textToSignPtr = reinterpret_cast<const TUint8*>( aCert.Ptr() );
    	    
    // new stuff begin
	delete iBufferData;
	TInt inputBufferSize = aCert.Size() + sizeof(TSaveCertInput);
	
	iBufferData = HBufC8::NewL( inputBufferSize );
	TPtr8 inputBufferPtr( iBufferData->Des() );
    inputBufferPtr.Append( saveCertInputBuf );
	inputBufferPtr.Append(aCert);
	iRequester->RequestDialog(*iBufferData, iDlgResponseBuf);
    }
    
// -----------------------------------------------------------------------------
// CPKIDialog::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//   
void CPKIDialog::DeleteCertificate( 
    const TCTTokenObjectHandle& aCertHandle,
    TRequestStatus& aStatus  )
    {
    iCurrentOperation = EDeleteCert;
    TRAPD(ret, RequestWithTokenHandleL(KNullDesC, aCertHandle, aStatus));
    if (ret!=KErrNone)
        {
        User::RequestComplete(iClientStatus, ret);
        }
    }
            
// -----------------------------------------------------------------------------
// CPKIDialog::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CPKIDialog::SaveSignedText( 
    const TDesC& aSignedText, 
    TRequestStatus& aStatus )
    {
    TCTTokenObjectHandle tokenHandle;
    iCurrentOperation = ESaveReceipt;
    TRAPD(ret, RequestWithTokenHandleL(aSignedText, tokenHandle, aStatus));
    if (ret!=KErrNone)
        {
        User::RequestComplete(iClientStatus, ret);
        }
    }
// -----------------------------------------------------------------------------
// CPKIDialog::?member_function
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CPKIDialog::PINBlockedInfo( 
    const TPINLabel& aLabel, 
    TRequestStatus& aStatus )
    {
    iCurrentOperation = EPinCodeBlockedInfo;
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
	iRequester->RequestDialog(*iBufferData, iDlgResponseBuf);
    }

// -----------------------------------------------------------------------------
// CPKIDialog::Note
// ?implementation_description
// (other items were commented in a header).
// -----------------------------------------------------------------------------
//
void CPKIDialog::Note( 
    MPKIDialog::TNoteType aNoteType, 
    TRequestStatus& aStatus )
    {
    ShowNoteL( aStatus, aNoteType );
    }
    
// -----------------------------------------------------------------------------
// CPKIDialog::ShowNoteL
// -----------------------------------------------------------------------------
//
void CPKIDialog::ShowNoteL(
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

    iRequester->RequestDialog(*iBufferData, iDlgResponseBuf);
    }
    
    
// -----------------------------------------------------------------------------
// CPKIDialog::Release
//
// -----------------------------------------------------------------------------
//
void CPKIDialog::Release()
    {
    delete this;
    }
    
// -----------------------------------------------------------------------------
// CPKIDialog::InitClientStatus
// InitClientStatus initialises the client's RequestStatus object.
// -----------------------------------------------------------------------------
//
void CPKIDialog::InitClientStatus( TRequestStatus& aStatus )
	{
	iClientStatus = &aStatus;
	*iClientStatus = KRequestPending;
	} 
	   
// -----------------------------------------------------------------------------
// CPKIDialog::HandleResponse
//
// -----------------------------------------------------------------------------
//    
void CPKIDialog::HandleResponse(TInt aResult)
    {
    switch ( iCurrentOperation )
        {
        case EUserAuthenticationText:
        case EUserAuthentication:
			{
			*iClientCertInfoHandlePtr = iCertInfoHandleBuf();
			break;
			}
		default:
		    // Do nothing, just client status complete.
		    break;
		    
        }
	User::RequestComplete(iClientStatus, aResult);
    }

//  End of File  
