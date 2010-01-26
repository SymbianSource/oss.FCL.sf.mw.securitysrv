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
* Description:  Header file of the abstract PKCS#12 parser API
*
*/


#ifndef __MPKCS12_H__
#define __MPKCS12_H__

//  INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <f32file.h>

// CONSTANTS
// Minimum data length needed to recognize PKCS#12 data.
const TInt KPKCS12DataMinLength = 49;

// FORWARD DECLARATIONS
class CX509Certificate;

// CLASS DECLARATION

/**
*  Class MPKCS12
*  The MPKCS12 abstract base class for opening  a PKCS #12 file. 
*  PKCS #12 is a format used to store and protect user's
*  private data, i.e. his/her key pairs, certificates and other personal
*  information.
*
*  @since Series 60 3.0
*/
class MPKCS12
    {        
    public: // Functions
           
        /**
        * Executes actual parsing.
        * @param aPKCS12 PKCS#12 encoded data
        * @param aPassword Used to generate encryption key.
        * @return ?description
        */
        virtual void ParseL(const TDesC8& aPKCS12, const TDesC16& aPassword) = 0;
        
        /**
        * ?member_description.
        * @since Series ?XX ?SeriesXX_version
        * @param ?arg1 ?description
        * @return ?description
        */
        virtual const CArrayPtr<CX509Certificate>& CACertificates() const = 0;
        virtual const CArrayPtr<CX509Certificate>& UserCertificates() const = 0;        
        virtual const CArrayPtr<HBufC8>& PrivateKeys() const = 0;
        
        // Returns number of SafeBags in PKCS #12 file.
        virtual TUint SafeBagsCount() const = 0;
        // Returns the number of iterations.
		virtual TUint IterCount() const = 0;

        /**
	    * Frees resources of the MPKCS12 class
	    */
	    virtual void Release()=0;	   	    	    
	    
	    /**
        * Recognizes PKCS#12 data
        * @param aPKCS12 PKCS#12 encoded data        
        * @return ETrue if data is recognized as PKCS12,
        *         otherwise EFalse.
        */
        virtual TBool IsPKCS12Data(const TDesC8& aBinaryData) = 0;
        
    protected:  // Destructor       
        
        /**
        * Destructor.
        */
        inline virtual ~MPKCS12()
            {};
                 
    };


/**
 * Factory for creating the relevant concrete subclass of the pkcs#12 base class.
 */
class PKCS12Factory
	{
public:
	/**
	 * Creates an instance of a subclass of MPKCS12. Implement to create
	 * the appropriate PKCS#12 parser
	 * 
	 * @return	An object that implements MPKCS12 functions
	 */
	IMPORT_C static MPKCS12* CreateL();	    
	};

#endif      // __MPKCS12_H__   
            
// End of File
