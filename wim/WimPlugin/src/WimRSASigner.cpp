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
* Description:  Implementation of Wim RSA signer interface
*
*/


#include "WimKeyStore.h"
#include "WimRSASigner.h"
#include <hash.h>
#include <mctkeystoreuids.h>            // KRSARepudiableSignerUID


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimRSASigner::CWimRSASigner()
//
// -----------------------------------------------------------------------------
//
CWimRSASigner::CWimRSASigner( CWimKeyStore& aClient )
		: MCTSigner<CRSASignature*>( aClient.Token() ),iClient( aClient )
	{                               
	iHandle.iTokenHandle = aClient.Token().Handle();
	}

// -----------------------------------------------------------------------------
// CWimRSASigner* CWimRSASigner::NewL()
// Creates new Instance of CWimRSASigner
// -----------------------------------------------------------------------------
//
CWimRSASigner* CWimRSASigner::NewL( CWimKeyStore& aClient )
    {
    CWimRSASigner* self = new( ELeave ) CWimRSASigner( aClient );
    return self;
    }


// -----------------------------------------------------------------------------
// CWimRSASigner::~CWimRSASigner()
// Destructor
// -----------------------------------------------------------------------------
//
CWimRSASigner::~CWimRSASigner()
	{
    delete iLabel;
	delete iDigest;
        delete iDigestBuf;
	}

// -----------------------------------------------------------------------------
// CWimRSASigner::SetLabelL()
// Sets label for signin key object
// -----------------------------------------------------------------------------
//
void CWimRSASigner::SetLabelL( const TDesC& aLabel )
    {
    iLabel = aLabel.AllocL();
    }

// -----------------------------------------------------------------------------
// CWimRSASigner::SetObjectId()
// Sets object id -points to a certain key
// -----------------------------------------------------------------------------
//
void CWimRSASigner::SetObjectId( const TInt aObjectId )
    {
    iHandle.iObjectId = aObjectId;
    }

// -----------------------------------------------------------------------------
// CWimRSASigner::Release()
// Release object
// -----------------------------------------------------------------------------
//
void CWimRSASigner::Release()
	{
	MCTTokenObject::Release();
	}

// -----------------------------------------------------------------------------
// CWimRSASigner::Label() const
// Returns the object's human-readable label
// -----------------------------------------------------------------------------
//
const TDesC& CWimRSASigner::Label() const
	{
	return *iLabel;
	}

// -----------------------------------------------------------------------------
// CWimRSASigner::Token()
// Returns a reference to the associated token
// -----------------------------------------------------------------------------
//
MCTToken& CWimRSASigner::Token() const
	{
	return iClient.Token();
	}

// -----------------------------------------------------------------------------
// CWimRSASigner::Type()
// Returns a UID representing the type of the token object. The
// meanings of possible UIDs should be documented in the
// documentation for the interface that returns them.
// -----------------------------------------------------------------------------
//
TUid CWimRSASigner::Type() const
	{
	return KRSARepudiableSignerUID;
	}

// -----------------------------------------------------------------------------
// CWimRSASigner::Handle()
// Returns a handle for the object. The primary purpose of the
// handle is to allow token objects to be 'passed' between
// processes. See TCTTokenObjectHandle for more details.
// -----------------------------------------------------------------------------
//
TCTTokenObjectHandle CWimRSASigner::Handle() const
	{
	return iHandle;
	}

// -----------------------------------------------------------------------------
// CWimRSASigner::SignMessage()
// Sign some data. The data is hashed before the signature is created using 
// the SHA-1 algorithm.
// -----------------------------------------------------------------------------
//
void CWimRSASigner::SignMessage( const TDesC8& aPlaintext, 
  				                 CRSASignature*& aSignature, 
  				                 TRequestStatus& aStatus )
	{
    if( iDigest )
        {
        delete iDigest;
        iDigest = NULL;
        }
    if( iDigestBuf )
        {
        delete iDigestBuf;
        iDigest = NULL;
        }

	// Hash the data on the client side
	TRAPD( err, iDigest = CSHA1::NewL() );

	if ( err == KErrNone )
		{
	    iDigest->Update( aPlaintext );
	    TRAP( err, iDigestBuf = HBufC8::NewL( iDigest->HashSize() ) );
	    
	    if (err != KErrNone )
	       {
	       TRequestStatus* status = &aStatus;
		     User::RequestComplete( status, err );
	       return;
	    	 }
	    
	    TPtr8 tempPtr = iDigestBuf->Des();
	    tempPtr.Copy( iDigest->Final() );	
	    Sign( *iDigestBuf, aSignature, aStatus );
		}
    else
        {
		TRequestStatus* status = &aStatus;
		User::RequestComplete( status, err );
        }
	}

// -----------------------------------------------------------------------------
// CWimRSASigner::Sign()
// Perform a raw signing operation.
// -----------------------------------------------------------------------------
//
void CWimRSASigner::Sign( const TDesC8& aPlaintext, 
  				          CRSASignature*& aSignature, 
  				          TRequestStatus& aStatus )
	{
	iClient.NonRepudiableRSASign( Handle(), aPlaintext, aSignature, aStatus );
	}

// -----------------------------------------------------------------------------
// CWimRSASigner::CancelSign()
// Cancel an ongoing sign
// -----------------------------------------------------------------------------
//
void CWimRSASigner::CancelSign()
	{
    iClient.Cancel();
	}

// -----------------------------------------------------------------------------
// CWimRSASigner::DoRelease()
// Releases this object.
// -----------------------------------------------------------------------------
//
void CWimRSASigner::DoRelease()
	{
	delete this;
	}
