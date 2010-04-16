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
* Description:  Implementation of CDevEncKeyUtils
*
*/


// INCLUDE FILES

// Class includes
#include "DevEncDef.h"
#include "DevEncKeyUtils.h"
#include "DevEncLog.h"
#include "DevEncUids.hrh"


#include <pbe.h>
#include <pbedata.h>
//#include <PathInfo.h> // for system path literals
#include <pkcs5kdf.h>
#include <s32file.h>
#include <s32mem.h>
#include <tconvbase64.h>

#include <DevEncEngineConstants.h>
#include <DevEncEngineBase.h>

// --------------------------------------------------------------------------
// CDevEncKeyUtils::Connect()
//
// --------------------------------------------------------------------------
EXPORT_C TInt CDevEncKeyUtils::Connect()
    {
    TRAPD( err, LoadDevEncEngineL() );
    return err;
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::Connect()
//
// --------------------------------------------------------------------------
EXPORT_C void CDevEncKeyUtils::Close()
    {
    UnloadDevEncEngine();
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::CDevEncKeyUtils()
//
// --------------------------------------------------------------------------
EXPORT_C CDevEncKeyUtils::CDevEncKeyUtils(): iConnect( EFalse )
    {
    // No implementation necessary
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::CDevEncKeyUtils()
//
// --------------------------------------------------------------------------
CDevEncKeyUtils::~CDevEncKeyUtils()
    {
    
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::CreateSetKey()
//
// --------------------------------------------------------------------------
EXPORT_C void CDevEncKeyUtils::CreateSetKey( TRequestStatus& aStatus,
                                             HBufC8*& aResult,
                                             const TDesC8& aPassword,
                                             const TInt aLength ) const
    {
    TInt error( KErrNone );
    if ( ! ProcessHasCapability( ECapabilityDiskAdmin ) )
        {
        DFLOG( "Process does not have DiskAdmin capability" );
        error = KErrAccessDenied;
        }
    else if ( aPassword.Length() > KMaxPasswordLength ||
              aPassword.Length() < KMinPasswordLength )
        {
        DFLOG( "CDevEncKeyUtils::CreateSetKey Invalid password length" );
        error = KErrArgument;
        }
    else if ( aLength > KEncryptionKeyLength )
        {
        DFLOG( "CDevEncKeyUtils::CreateSetKey Invalid key length" );
        error = KErrArgument;
        }
    else
        {
        TRAP( error, DoCreateSetKeyL( aResult, aPassword, aLength ) );
        DFLOG2( "CDevEncKeyUtils::CreateSetKey result %d", error );
        }
    TRequestStatus* statusPtr = &aStatus;
    User::RequestComplete( statusPtr, error );
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::CreateSetKey()
//
// --------------------------------------------------------------------------
EXPORT_C void CDevEncKeyUtils::CreateSetKey( TRequestStatus& aStatus,
                                             const TInt aLength ) const
    {
    TInt error( KErrNone );

    HBufC8* password( NULL );

    if ( ! ProcessHasCapability( ECapabilityDiskAdmin ) )
        {
        DFLOG( "Process does not have DiskAdmin capability" );
        error = KErrAccessDenied;
        }
    if ( aLength > KEncryptionKeyLength )
        {
        DFLOG( "CDevEncKeyUtils::CreateSetKey Invalid key length" );
        error = KErrArgument;
        }

    if ( !error )
        {
        TRAP( error, password = HBufC8::NewL( KMaxPasswordLength ) );
        if ( error )
            {
            DFLOG2( "Pwd buf alloc error %d", error );
            }
        }

    if ( !error )
        {
        // Get some random password
        TPtr8 passwordPtr = password->Des();
        passwordPtr.SetLength( KMaxPasswordLength );
        if( iConnect )
            {
            iDevEncEngine->RandomDataGet( passwordPtr, KMaxPasswordLength );
            }
        
        DFLOG( "Random password allocated" );
        
        TRAP( error, DoCreateSetKeyL( *password,
                                      aLength ) );
        DFLOG2( "CDevEncKeyUtils::CreateSetKey result %d", error );
        }

    // Cleanup on demand
    if ( password )
        {
        delete password;
        }
    
    TRequestStatus* statusPtr = &aStatus;
    User::RequestComplete( statusPtr, error );
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::DoCreateSetKey()
//
// --------------------------------------------------------------------------
void CDevEncKeyUtils::DoCreateSetKeyL( const TDesC8& aPassword,
                                       TInt aLength ) const
    {
    DFLOG2( ">>CDevEncKeyUtils::DoCreateSetKeyL, length %d", aLength );

    // Get some random key data
    HBufC8* clearKey = HBufC8::NewLC( aLength );
    TPtr8 clearKeyPtr = clearKey->Des();
    clearKeyPtr.SetLength( aLength );

    if( iConnect )
        {
        iDevEncEngine->RandomDataGet( clearKeyPtr, KMaxPasswordLength );
        }
    
    // if supply KDF, must also supply salt len and iteration count
    CPBEncryptElement* encryption = CPBEncryptElement::NewLC( aPassword,
                                                              ECipherDES_CBC );
    CPBEncryptor* encryptor = encryption->NewEncryptLC();

    HBufC8* ciphertextTemp = HBufC8::NewLC( encryptor->MaxFinalOutputLength( clearKey->Length() ) ); 
    TPtr8 ciphertext = ciphertextTemp->Des();   
    encryptor->ProcessFinalL( *clearKey, ciphertext );

    // ENCRYPTION DONE

    DFLOG( "DoCreateSetKeyL, Key in plaintext:" );
    RDebug::RawPrint( clearKeyPtr );
    DFLOG( "DoCreateSetKeyL, Key in ciphertext:" );
    RDebug::RawPrint( ciphertext );
    DFLOG( "DoCreateSetKeyL, Password:" );
    RDebug::RawPrint( aPassword );
    
    if( iConnect )
        {
        // Take the new key in use
        iDevEncEngine->TakeKeyInUseL( *clearKey );
        }
    
    // Destroy the evidence
    CleanupStack::PopAndDestroy( ciphertextTemp );
    CleanupStack::PopAndDestroy( encryptor );
    CleanupStack::PopAndDestroy( encryption );
    CleanupStack::PopAndDestroy( clearKey );
    };

// --------------------------------------------------------------------------
// CDevEncKeyUtils::DoCreateSetKey()
//
// --------------------------------------------------------------------------
void CDevEncKeyUtils::DoCreateSetKeyL( HBufC8*& aResult,
                                       const TDesC8& aPassword,
                                       TInt aLength ) const
    {
    DFLOG2( ">>CDevEncKeyUtils::DoCreateSetKeyL, length %d", aLength );

    // Get some random key data
    HBufC8* clearKey = HBufC8::NewLC( aLength );
    TPtr8 clearKeyPtr = clearKey->Des();
    clearKeyPtr.SetLength( aLength );
    
    if( iConnect )
        {
        iDevEncEngine->RandomDataGet( clearKeyPtr, KMaxPasswordLength );
        }
    
    // if supply KDF, must also supply salt len and iteration count
    CPBEncryptElement* encryption = CPBEncryptElement::NewLC( aPassword,
                                                              ECipherDES_CBC );
    CPBEncryptor* encryptor = encryption->NewEncryptLC();

    HBufC8* ciphertextTemp = HBufC8::NewLC( encryptor->MaxFinalOutputLength( clearKey->Length() ) ); 
    TPtr8 ciphertext = ciphertextTemp->Des();   
    encryptor->ProcessFinalL( *clearKey, ciphertext );

    // ENCRYPTION DONE

    DFLOG( "DoCreateSetKeyL, Key in plaintext:" );
    RDebug::RawPrint( clearKeyPtr );
    DFLOG( "DoCreateSetKeyL, Key in ciphertext:" );
    RDebug::RawPrint( ciphertext );
    DFLOG( "DoCreateSetKeyL, Password:" );
    RDebug::RawPrint( aPassword );
    
    if( iConnect )
        {
        // Take the new key in use
        iDevEncEngine->TakeKeyInUseL( *clearKey );
        }

    // If we got this far, the operation was successful.
    // Give the caller a copy of the encrypted key
    // Externalize the key data and ciphertext
    HBufC8* result = HBufC8::NewLC( ciphertext.Length() +
                                   KMaxPasswordLength +
                                   50 // should be enough for encryption data
                                   );
    TPtr8 resultPtr = result->Des();
    ExternalizeKeyL( encryption, ciphertext, resultPtr );
    
    // Encode key to base64 before returning it
    aResult = HBufC8::NewL( resultPtr.Length()*4/3+3 );
    TPtr8 returnPtr = aResult->Des();
    returnPtr.SetLength( 0 );
    TBase64 b64codec;
    b64codec.Encode( *result, returnPtr );
    
    // Destroy the evidence
    CleanupStack::PopAndDestroy( result );
    CleanupStack::PopAndDestroy( ciphertextTemp );
    CleanupStack::PopAndDestroy( encryptor );
    CleanupStack::PopAndDestroy( encryption );
    CleanupStack::PopAndDestroy( clearKey );
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::SetKey()
//
// --------------------------------------------------------------------------
EXPORT_C void CDevEncKeyUtils::SetKey( TRequestStatus& aStatus,
                                       const TDesC8& aPkcs5Key,
                                       const TDesC8& aPassword ) const
    {
    TInt error( KErrNone );

    if ( ! ProcessHasCapability( ECapabilityDiskAdmin ) )
        {
        DFLOG( "Process does not have DiskAdmin capability" );
        error = KErrAccessDenied;
        }
    else
        {
        TRAP( error, DoSetKeyL( aPkcs5Key, aPassword ) );
        DFLOG2( "CDevEncKeyUtils::SetKey result %d", error );
        }
    TRequestStatus* statusPtr = &aStatus;
    User::RequestComplete( statusPtr, error );
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::DoSetKey()
//
// --------------------------------------------------------------------------
void CDevEncKeyUtils::DoSetKeyL( const TDesC8& aPkcs5Key,
                                 const TDesC8& aPassword ) const
    {
    // Decode the base64 encoded key
    HBufC8* decodedKey = HBufC8::NewLC( aPkcs5Key.Length()*3/4 );
    TPtr8 keyPtr = decodedKey->Des();
    keyPtr.SetLength( 0 );
    TBase64 b64codec;
    b64codec.Decode( aPkcs5Key, keyPtr );

    // Read the parameters and ciphertext from the input
    CPBEncryptElement* encryption( NULL );
    HBufC8* ciphertext( NULL );
    InternalizeKeyL( encryption, aPassword, ciphertext, *decodedKey );
    CleanupStack::PopAndDestroy( decodedKey );
    CleanupStack::PushL( encryption );
    CleanupStack::PushL( ciphertext );

    // Decrypt and take key in use
    CPBDecryptor* decryptor = encryption->NewDecryptLC();
    HBufC8* plaintextTemp =
        HBufC8::NewLC( decryptor->MaxOutputLength( (*ciphertext).Size() ) ); 
    TPtr8 plaintext = plaintextTemp->Des(); 
    decryptor->Process( *ciphertext, plaintext );

    Pkcs5RemovePadding( plaintext );
    
    if( iConnect )
        {
        // Take the new key in use
        iDevEncEngine->TakeKeyInUseL( plaintext );
        }

    DFLOG( "DoSetKeyL, Key in plaintext:" );
    RDebug::RawPrint( plaintext );
    DFLOG( "DoSetKeyL, Key in ciphertext:" );
    RDebug::RawPrint( *ciphertext );
    DFLOG( "DoSetKeyL, Password:" );
    RDebug::RawPrint( aPassword );

    
    CleanupStack::PopAndDestroy( plaintextTemp );
    CleanupStack::PopAndDestroy( decryptor );
    CleanupStack::PopAndDestroy( ciphertext );
    CleanupStack::PopAndDestroy( encryption );
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::ResetKey()
//
// --------------------------------------------------------------------------
EXPORT_C void CDevEncKeyUtils::ResetKey( TRequestStatus& aStatus ) const
    {
    TInt error( KErrNone );

    if ( ! ProcessHasCapability( ECapabilityDiskAdmin ) )
        {
        DFLOG( "Process does not have DiskAdmin capability" );
        error = KErrAccessDenied;
        }
    else
        {
        TBuf8<KEncryptionKeyLength> nullKey;
        nullKey.FillZ( KEncryptionKeyLength );
        TRAP( error, 
        	  if( iConnect )
                  {
        		  // Take the new key in use
        		  iDevEncEngine->TakeKeyInUseL( nullKey );
        		  }  
        	);
        DFLOG2( "CDevEncKeyUtils::TakeKeyInUseL result %d", error );
        }
    TRequestStatus* statusPtr = &aStatus;
    User::RequestComplete( statusPtr, error );
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::GetNewFileStoreL()
//
// --------------------------------------------------------------------------
void CDevEncKeyUtils::GetNewFileStoreL( RFs& aFs,
                                        TDes& aFileName,
                                        CFileStore*& aStore ) const
    {
    // Leaves with KErrAlreadyExists if file exists from before
    aStore = CPermanentFileStore::CreateL( aFs,
                                           aFileName,
                                           EFileRead | EFileWrite );
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::SaveKeyL()
//
// --------------------------------------------------------------------------
void CDevEncKeyUtils::SaveKeyL( CFileStore* aStore,
                                const CPBEncryptElement* aElement,
                                const TDesC8& aCiphertext ) const
    {
    RStoreWriteStream write;
    
    aStore->SetTypeL( aStore->Layout() );
    
    //write the encryption data to a new stream
    write.CreateLC( *aStore );
    aElement->EncryptionData().ExternalizeL( write );
    write.CommitL();
    CleanupStack::PopAndDestroy(); //CreateLC()

    //write the cyphertext to a new stream
    write.CreateLC( *aStore );
    write << aCiphertext;
    write.CommitL();
    CleanupStack::PopAndDestroy(); //CreateLC()

    aStore->Commit();
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::LoadKeyLC()
//
// --------------------------------------------------------------------------
void CDevEncKeyUtils::LoadKeyLC( RFs& aFs,
                                 const TFileName& aFileName,
                                 CPBEncryptionData*& aData,
                                 HBufC8*& aCiphertext ) const
    {
    //prepare to read the streams back in, creating a new TPBEncryptionData
    RStoreReadStream read;
    // open the next PFS
    CFileStore *store = CPermanentFileStore::OpenLC( aFs,
                                                 aFileName,
                                                 EFileRead );
    TStreamId dataStreamId( 1 ); // we know it was the first stream written
    read.OpenLC( *store, dataStreamId );
//    CleanupStack::Pop();
    //read in Encryption data
    aData = CPBEncryptionData::NewL( read );
    CleanupStack::Pop(); // read
    read.Close();
    CleanupStack::PushL( aData );

    //read in ciphertext key
    TStreamId cipherId( 2 ); // we know it was the second stream written
    read.OpenLC( *store, cipherId );
    CleanupStack::Pop();
    aCiphertext = HBufC8::NewL( read, 10000 ); //some large number
    read.Close();

    CleanupStack::Pop( aData );
    CleanupStack::PopAndDestroy( store );
    CleanupStack::PushL( aData );
    CleanupStack::PushL( aCiphertext );
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::ExternalizeKeyL()
//
// --------------------------------------------------------------------------
void CDevEncKeyUtils::ExternalizeKeyL( const CPBEncryptElement* aElement,
                                       const TDesC8& aCiphertext,
                                       //HBufC8*& aResult ) const
                                       TDes8& aResult ) const
    {
    RDesWriteStream write;
    write.Open( aResult );
    write.PushL();
    aElement->EncryptionData().ExternalizeL( write );
    write << aCiphertext;
    write.CommitL();
    write.Pop();
    write.Close();
    DFLOG( "CDevEncKeyUtils::ExternalizeKeyL done" );
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::InternalizeKeyL()
//
// --------------------------------------------------------------------------
void CDevEncKeyUtils::InternalizeKeyL( CPBEncryptElement*& aElement,
                                       const TDesC8& aPassword,
                                       HBufC8*& aCiphertext,
                                       const TDesC8& aSource ) const
    {
    RDesReadStream read;
    read.Open( aSource );
    read.PushL();
    CPBEncryptionData* data = CPBEncryptionData::NewLC( read );
    aElement = CPBEncryptElement::NewLC( *data, aPassword );
    aCiphertext = HBufC8::NewL( aSource.Length() );
    TPtr8 cipherTextPtr = aCiphertext->Des();
    read >> cipherTextPtr;
    CleanupStack::Pop( aElement );
    CleanupStack::Pop( data );
    read.Pop();
    read.Close();
    DFLOG( "CDevEncKeyUtils::InternalizeKeyL done" );
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::Pkcs5RemovePadding()
//
// --------------------------------------------------------------------------
void CDevEncKeyUtils::Pkcs5RemovePadding( TPtr8& aInput ) const
    {
    // From RFC 2898:
    //    The padding string PS consists of 8-(||M|| mod 8) octets
    //    each with value 8-(||M|| mod 8). The padding string PS will
    //    satisfy one of the following statements:
    //
    //            PS = 01, if ||M|| mod 8 = 7 ;
    //            PS = 02 02, if ||M|| mod 8 = 6 ;
    //            ...
    //            PS = 08 08 08 08 08 08 08 08, if ||M|| mod 8 = 0.
    // So the last byte shows how much padding there is
    DFLOG( "CDevEncKeyUtils::Pkcs5RemovePadding" );
    TInt paddingBytes = aInput[ aInput.Length() - 1 ];
    
    if ( paddingBytes <= 0 ||
         paddingBytes > 8  ||
         paddingBytes > aInput.Length() )
        {
        return;
        }

    TInt delPos( aInput.Length() - paddingBytes - 1 );
    TInt delLen( aInput.Length() - delPos );
    aInput.Delete( delPos, delLen );
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::ProcessHasCapability()
//
// --------------------------------------------------------------------------
TBool CDevEncKeyUtils::ProcessHasCapability( TCapability aCapability ) const
    {
    RProcess process;
    return process.HasCapability( aCapability );
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::LoadDevEncEngineL()
//
// --------------------------------------------------------------------------
void CDevEncKeyUtils::LoadDevEncEngineL()
    {
    FLOG(" CDevEncKeyUtils::LoadDevEncEngineL >> ");
    
    if (!iDevEncEngine)
        {
         iConnect = EFalse;
         TInt err = iLibrary.Load(KEncryptionDll);
         if (err != KErrNone)
             {
             FLOG2("Error in finding the library... %d", err);
             if (err == KErrNotFound)
                 err = KErrNotSupported;
             User::Leave(err);
             }
         TLibraryFunction entry = iLibrary.Lookup(1);
         
         if (!entry)
             {
             FLOG("Error in loading the library...");
             User::Leave(KErrBadLibraryEntryPoint);
             }
         iDevEncEngine = (CDevEncEngineBase *) entry();
         iConnect = ETrue;
        }
    FLOG(" CDevEncKeyUtils::LoadDevEncEngineL << ");
    }

// --------------------------------------------------------------------------
// CDevEncKeyUtils::UnloadDevEncEngine()
//
// --------------------------------------------------------------------------
void CDevEncKeyUtils::UnloadDevEncEngine()
    {
    FLOG(" CDevEncKeyUtils::UnloadDevEncEngineL >> ");
    
    if (iDevEncEngine)
        {
        iDevEncEngine->Close();
        delete iDevEncEngine;
        iDevEncEngine = NULL;
        iLibrary.Close();
        }
    iConnect = EFalse;
    
    FLOG(" CDevEncKeyUtils::UnloadDevEncEngineL << ");
    }

// End of file
