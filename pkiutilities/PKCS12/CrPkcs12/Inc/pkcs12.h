/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Header file of the PKCS#12 parser API
*
*/


#ifndef __CPKCS12_H__
#define __CPKCS12_H__

//  INCLUDES
#include "mpkcs12.h"
#include <e32std.h>
#include <e32base.h>
#include <f32file.h>

// FORWARD DECLARATIONS
class CCrPKCS12;
class CX509Certificate;
class CCrData;

// CLASS DECLARATION

/**
*  Class CPKCS12
*  The PKCS12 Library (CrPKCS12) provides opening functionality of a 
*  PKCS #12 file. PKCS #12 is a format used to store and protect user's
*  private data, i.e. his/her key pairs, certificates and other personal
*  information.
*
*  @lib crpkcs12.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( CPKCS12 ): public MPKCS12, public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CPKCS12* NewL();
        
    private:
        /**
        * Destructor.
        */
        virtual ~CPKCS12();

    public: // Functions from MPKCS12
    
        TBool IsPKCS12Data(const TDesC8& aBinaryData);

        /**
        * Executes actual parsing.
        * @param aPKCS12 PKCS#12 encoded data
        * @param aPassword Used to generate encryption key.
        * @return ?description
        */        
        void ParseL(const TDesC8& aPKCS12, const TDesC16& aPassword);
        
        /**
        * ?member_description.
        * @since Series ?XX ?SeriesXX_version
        * @param ?arg1 ?description
        * @return ?description
        */
        const CArrayPtr<CX509Certificate>& CACertificates() const;
        const CArrayPtr<CX509Certificate>& UserCertificates() const;
        const CArrayPtr<HBufC8>& PrivateKeys() const;
        
        // Returns number of SafeBags in PKCS #12 file.
        TUint SafeBagsCount() const;
        // Returns the number of iterations.
		TUint IterCount() const;
        
        /**
	    * Frees resources of the class
	    */
        void Release();                

    private:

        /**
        * C++ default constructor.
        */
        CPKCS12();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        void Reset();
                
        void HandleErrorL( TUint16 aError ) const; 
        
        static TBool IsASN1Tag(TInt aTag, const TDesC8& aBinaryData, TInt& aPos);
        
        static TBool IsExpectedData(const TDesC8& aBinaryData, TInt& aPos, const TDesC8& aExpectedData);

    private:    // Data
        CCrPKCS12* iCrPkcs12;        
        CCrData* iCrData;                 
    };

#endif      // __CPKCS12_H__   
            
// End of File
