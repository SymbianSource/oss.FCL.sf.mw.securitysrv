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
* Description:   This file contains header of the CX509CertificateSet class.
*
*/



#ifndef CERTIFICATESET
#define CERTIFICATESET

//  INCLUDES

#include <x509cert.h>                   // CX509Certificate
#include <x509certext.h>                // TKeyUsage

/**
*  Class CX509CertificateSet
*  Class CX509CertificateSet contains set of CX509Certificate objects.
*
*  @lib crpkcs12.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( CX509CertificateSet ): public CArrayPtrFlat<CX509Certificate>
    {
   /////////////////////////////////////////////
   // Constructors and destructors.
    private:
        CX509CertificateSet(TInt aGranularity);
        void ConstructL();
        
    public:
         static CX509CertificateSet* NewLC(TInt aGranularity);
         static CX509CertificateSet* NewL(TInt aGranularity);
         ~CX509CertificateSet();

        // Applies CX509Certificates into CX509CertificateSet
         TUint DecodeCertsL(const TDesC8& aBinaryData);
    };

#endif CERTIFICATESET