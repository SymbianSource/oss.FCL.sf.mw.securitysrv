/*
* Copyright (c) 2003-2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Header of class used to parse and validate a certificate or PKCS#12 PFX.
*
*/


#ifndef CERTPARSER_H
#define CERTPARSER_H

//  INCLUDES
#include <e32base.h>
#include <mpkcs12.h>

// CONSTANTS
const TInt KHashLength( 20 );
// CertSaver panic category
_LIT( KCertSaverPanic, "Certificate saver" );
// Panic reasons
const TInt KPanicNullPointer( 0 );
const TInt KErrWrongCryptoLib( 1 );
const TInt KErrExitApp( -333333 );

// FORWARD DECLARATIONS
class CCertificate;
class CX509Certificate;
class CX500DistinguishedName;
class CPKCS12;
class RFs;
class CEikonEnv;

// CLASS DECLARATION

/**
*  CCertParser - X.509 certificate and PKCS#12 PFX parsing class.
*/
class CCertParser :public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CCertParser* NewL();

        /**
        * Destructor.
        */
        virtual ~CCertParser();

    public: // New functions

        enum TCertType
            {
            ETypeX509,
            ETypeURL,
            ETypeX509CA,
            ETypeX509Peer,
            ETypePKCS12,
            ETypeCorrupt
            };

        /**
        * Sets parsers state according to aBuffer. If certificate or PKCS#12
        * is corrupted, any of the query functions can't be called.
        * @param aBuffer, buffer containing the certificate
        * @return Type of certificate, can also be corrupted.
        */
        void SetContentL( RFile& aFile );

        /**
        * Returns the type of the certificate.
        * @return TCertType, type of certificate.
        */
        TCertType CertType() const;

        /**
        * Returns the content of the certificate.
        * Returned data is valid so long as state of this object
        * is not modified.
        * @return TPtrC8, the certificate
        */
        const TPtrC8 CertificateBuf() const;

        /**
        * Returns the X.509 certificate object.
        * Returned data is valid so long as state of this object
        * is not modified.
        * @return TPtrC8, the certificate
        */
        const CX509Certificate& Certificate() const;


        /**
        * Returns the private key of the PKCS#12 PDU.
        * Returned data is valid so long as state of this object
        * is not modified.
        * @return TPtrC8, the certificate
        */
        const CArrayPtr<HBufC8>& Keys() const;

        /**
        * Returns the CA certificates from the PKCS#12 PDU.
        * Returned data is valid so long as state of this object
        * is not modified.
        * @return TPtrC8, the certificate
        */
        const CArrayPtr<CX509Certificate>& CACertificates() const;

        /**
        * Returns the user certificates from the PKCS#12 PDU.
        * Returned data is valid so long as state of this object
        * is not modified.
        * @return TPtrC8, the certificate
        */
        const CArrayPtr<CX509Certificate>& UserCertificates() const;

    public: // Functions from base classes

    protected:  // New functions

    protected:  // Functions from base classes

    private:

        /**
        * C++ default constructor.
        */
        CCertParser();

        /**
        * By default EPOC constructor is private.
        */
        void ConstructL();

        TInt CheckIfX509CertificateL( const TDesC8& aCert );

        void CreatePKCS12L();
        TBool CheckIfPKCS12L( const TDesC8& aPKCS12, const TDesC& aFileName );

        TInt GetPasswordL( TDes& aPassword, const TDesC& aFileName );

        void ShowErrorNoteL( TInt aResourceID );

    public:     // Data

    protected:  // Data

    private:    // Data
        TCertType         iCertType;    // The type of the response
        CX509Certificate* iCert;
        MPKCS12*          iPKCS12;
        RLibrary          iLibrary;
        CEikonEnv*        iEikEnv;

    public:     // Friend classes
    protected:  // Friend classes
    private:    // Friend classes

    };

#endif      // CERTPARSER_H

// End of File
