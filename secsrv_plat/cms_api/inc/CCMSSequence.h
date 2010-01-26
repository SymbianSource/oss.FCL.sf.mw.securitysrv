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
* Description:  ?Description
*
*/



#ifndef CCMSSequence_H
#define CCMSSequence_H

//  INCLUDES
#include 	"MCMSModule.h"
#include 	<asn1dec.h>
#include 	<asn1enc.h>

// CLASS DECLARATION
/**
*  Base class for sequence (or set) type of modules.
*
*  @lib cms.lib
*  @since 2.8
*/
class CCMSSequence : public CBase, public MCMSModule
    {
	public: // From MCMSModule
		/**
        * From MCMSModule
        * @since 2.8
        * @param aResult Allocates and sets result in this variable
        * @return ?description
        */
        void EncodeL( HBufC8*& aResult ) const;

    protected:  // New functions
       /**
        * C++ default constructor.
        */
        CCMSSequence();

        /**
        * Decode a generic ASN1 sequence
        * @since 2.8
        * @param aRawData raw-data to be splitted in ASN.1 Modules
        * @return Array of generic asn.1 modules.
        */
        IMPORT_C static CArrayPtr<TASN1DecGeneric>* DecodeSequenceLC( const TDesC8& aRawData );

        /**
		* Decode a generic ASN1 sequence, with a minimum/maximum number
		* of top level terms. Will leave with KErrArgument if number of terms
		* is not between valid range (including min and max)
		* @since 2.8
		* @param aRawData raw-data to be splitted in ASN.1 Modules
		* @param aMinTerms minimum of top level terms.
		* @param aMaxTerms maximum number of top level terms.
		* @return Array of generic asn.1 modules.
        */
        IMPORT_C static CArrayPtr<TASN1DecGeneric>* DecodeSequenceLC( const TDesC8& aRawData,
        															  TInt aMinTerms,
        															  TInt aMaxTerms );
		/**
         * Creates DER encoded descriptor and leaves it to 
		 * CleanupStack
		 * @since 2.8
		 * @param aEncoding base encoding to be encoded
		 * @return new heap buffer containing der encoded content
         */
		IMPORT_C static HBufC8* CreateDerEncodingL( CASN1EncBase* aEncoding );

    };

// taken from MMF, did not want to make dependency to MMF
template <class T>
class CleanupResetAndDestroy
	{
public:
	inline static void PushL(T& aRef);
private:
	static void ResetAndDestroy(TAny *aPtr);
	};
template <class T>
inline void CleanupResetAndDestroyPushL(T& aRef);
template <class T>
inline void CleanupResetAndDestroy<T>::PushL(T& aRef)
	{CleanupStack::PushL(TCleanupItem(&ResetAndDestroy,&aRef));}
template <class T>
void CleanupResetAndDestroy<T>::ResetAndDestroy(TAny *aPtr)
	{(STATIC_CAST(T*,aPtr))->ResetAndDestroy();}
template <class T>
inline void CleanupResetAndDestroyPushL(T& aRef)
	{CleanupResetAndDestroy<T>::PushL(aRef);}

#endif      // CCMSSequence_H

// End of File
