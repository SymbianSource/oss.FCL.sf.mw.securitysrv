/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Provides functions for creating and setting encryption keys.
*
*/


#ifndef DEVENCKEYUTILS_H_
#define DEVENCKEYUTILS_H_

#include <e32base.h>
#include <e32des16.h>
#include <e32des8.h>

class CFileStore;
class CPBEncryptionData;
class CPBEncryptElement;
class CPBEncryptSet;
class RFs;
class CDevEncEngineBase;


// CLASS DECLARATION
/**
* Provides functions for creating and setting encryption keys.
*/
class CDevEncKeyUtils: public CBase
    {
    public:
        /**
        * C++ constructor.
        */
        IMPORT_C CDevEncKeyUtils();

        /**
        * Destructor.
        */
        virtual ~CDevEncKeyUtils();

        // To connect the engine 
        IMPORT_C TInt Connect();

        // To close the connection with the engine
        IMPORT_C void Close();

        /**
        * Creates a PKCS#5 key and takes it into use by loading it
        * into the security driver. Asynchronous function.
        * @param aStatus Contains system-wide error code on completion
        * @param aResult On successful completion, contains PKCS#5
        *   encrypted key (externalized encryption data and ciphertext)
        * @param aPassword The user password to use for the key
        * @param aLength The desired length of the key
        */
        IMPORT_C void CreateSetKey( TRequestStatus& aStatus,
                                    HBufC8*& aResult,
                                    const TDesC8& aPassword,
                                    const TInt aLength ) const;

        /**
        * Creates a PKCS#5 key and takes it into use by loading it
        * into the security driver. Asynchronous function. A random password
        * is generated and the resulting key is ignored. After completion,
        * the key exists only in the security driver and cannot be retrieved.
        * @param aStatus Contains system-wide error code on completion
        * @param aLength The desired length of the key
        */
        IMPORT_C void CreateSetKey( TRequestStatus& aStatus,
                                    const TInt aLength ) const;

        /**
        * Sets the specified PKCS#5 key in the security driver.
        * Asynchronous function.
        * @param aStatus Contains system-wide error code on completion
        * @param aPkcs5Key The encrypted PKCS#5 key data (externalized
        *   encryption data and ciphertext)
        * @param aPassword The user password to use for the key
        */
        IMPORT_C void SetKey( TRequestStatus& aStatus,
                              const TDesC8& aPkcs5Key,
                              const TDesC8& aPassword ) const;

        /**
        * Resets the key in the security driver. Asynchronous function.
        * @param aStatus Contains system-wide error code on completion
        */
        IMPORT_C void ResetKey( TRequestStatus& aStatus ) const;
        
    private:
    // Functions
        void DoCreateSetKeyL( const TDesC8& aPassword,
                              TInt aLength ) const;
        void DoCreateSetKeyL( HBufC8*& aResult,
                              const TDesC8& aPassword,
                              const TInt aLength ) const;
        void DoSetKeyL( const TDesC8& aPkcs5Key,
                        const TDesC8& aPassword ) const;
        
        void Pkcs5RemovePadding( TPtr8& aInput ) const;

        void GetNewFileStoreL( RFs& aFs,
                               TDes& aFileName,
                               CFileStore*& aStore ) const;
        
        void SaveKeyL( CFileStore* aStore,
                       const CPBEncryptElement* aSet,
                       const TDesC8& aCiphertext ) const;        
        
        void LoadKeyLC( RFs& aFs,
                        const TFileName& aFileName,
                        CPBEncryptionData*& aData,
                        HBufC8*& aCiphertext ) const;

        void InternalizeKeyL( CPBEncryptElement*& aElement,
                              const TDesC8& aPassword,
                              HBufC8*& aCiphertext,
                              const TDesC8& aSource ) const;

        void ExternalizeKeyL( const CPBEncryptElement* aElement,
                              const TDesC8& aCiphertext,
                              //HBufC8*& aResult ) const;
                              TDes8& aResult ) const;

        TBool ProcessHasCapability( TCapability aCapability ) const;

        void LoadDevEncEngineL();

        void UnloadDevEncEngine();

    // Data
    private:
       CDevEncEngineBase* iDevEncEngine;
       RLibrary iLibrary;
       TBool iConnect;
    };

#endif /*DEVENCKEYUTILS_H_*/

// End of file
