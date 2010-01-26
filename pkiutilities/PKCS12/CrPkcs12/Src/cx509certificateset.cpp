/*
* Copyright (c) 2000, 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   This file contains the implementation of 
*                CX509CertificateSet class. 
*
*/



//  INCLUDE FILES

#include "CX509CertificateSet.h"



// -----------------------------------------------------------------------------
// CX509CertificateSet
// Constructor
// -----------------------------------------------------------------------------
CX509CertificateSet::CX509CertificateSet( TInt aGranularity )
                     :CArrayPtrFlat<CX509Certificate>( aGranularity )
    {
    }

// -----------------------------------------------------------------------------
// CX509CertificateSet::ConstructL
// -----------------------------------------------------------------------------

void CX509CertificateSet::ConstructL()
    {
    }

// -----------------------------------------------------------------------------
// CX509CertificateSet::NewLC
// -----------------------------------------------------------------------------
CX509CertificateSet* CX509CertificateSet::NewLC( TInt aGranularity )
    {
    CX509CertificateSet* self = new (ELeave) CX509CertificateSet(aGranularity);
    CleanupStack::PushL(self);

    self->ConstructL();

    return self; 
    }

// -----------------------------------------------------------------------------
// CX509CertificateSet
// Destructor
// -----------------------------------------------------------------------------
CX509CertificateSet::~CX509CertificateSet()
	{
    ResetAndDestroy();
    }


// -----------------------------------------------------------------------------
// CX509CertificateSet::NewL
// -----------------------------------------------------------------------------
CX509CertificateSet* CX509CertificateSet::NewL( TInt aGranularity )
    {
    CX509CertificateSet* self = NewLC( aGranularity );
    CleanupStack::Pop();

    return self; 
    }

// -----------------------------------------------------------------------------
// CX509CertificateSet::DecodeCertsL
// This function reads binarydata given as parameter and decodes
// all certificates from it and puts them into CX509CertificateSet,
// what is obviously a set of X509Certificates.
// Parameters: const TDesC8& aBinaryData   Binarydata containing 
//                                         CX509Certificates.
// Returns:    Amount of certificates decoded.                                 
// -----------------------------------------------------------------------------
TUint CX509CertificateSet::DecodeCertsL(const TDesC8& aBinaryData)
	{
	TInt pos = NULL;//start at the start
    TUint amount = NULL;

    while ( pos < aBinaryData.Length() )
		{
        CX509Certificate *aTempCertificate = CX509Certificate::NewLC( aBinaryData, pos );
 
        AppendL( aTempCertificate );
		CleanupStack::Pop();
        amount++;
        }

    return amount; // amount of certificates in CertificateSet
	}

// End of file

