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
* Description:   This file contains the implementation of CCrPKCS12 class. 
*
*/



//  INCLUDE FILES
#include "crpkcs12.h"
#include <crdefs.h>      // CCrDefines
#include <e32std.h>
#include <x509certext.h>

const TInt KX509Version3 = 3;

// -----------------------------------------------------------------------------
// CPKCS12SyncWrapper
// Constructor
// -----------------------------------------------------------------------------
//
CCrPKCS12::CPKCS12SyncWrapper::CPKCS12SyncWrapper(): CActive( EPriorityStandard )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CPKCS12SyncWrapper
// Destructor
// -----------------------------------------------------------------------------
//
CCrPKCS12::CPKCS12SyncWrapper::~CPKCS12SyncWrapper()
    {
    if (iSecDlg)
        {
		iSecDlg->Release();
        }
    }

// -----------------------------------------------------------------------------
// CPKCS12SyncWrapper
// EnterPassword wrapper function.
// -----------------------------------------------------------------------------
//
TInt CCrPKCS12::CPKCS12SyncWrapper::EnterPasswordL(const TPINParams& aPINParams, TBool aRetry, TPINValue& aPINValue)
    {
    iSecDlg = SecurityDialogFactory::CreateL();
    iSecDlg->EnterPIN(aPINParams, aRetry, aPINValue, iStatus);
    return SetActiveAndWait();
    }

// -----------------------------------------------------------------------------
// CPKCS12SyncWrapper::SetActiveAndWait
// 
// -----------------------------------------------------------------------------
//
TInt CCrPKCS12::CPKCS12SyncWrapper::SetActiveAndWait()
    {
    SetActive();
    iWait.Start();
    return iStatus.Int();
    }

// -----------------------------------------------------------------------------
// CPKCS12SyncWrapper::DoCancel
// 
// -----------------------------------------------------------------------------
//
void CCrPKCS12::CPKCS12SyncWrapper::DoCancel()
    {
    }

// -----------------------------------------------------------------------------
// CPKCS12SyncWrapper::RunL
// 
// -----------------------------------------------------------------------------
//
void CCrPKCS12::CPKCS12SyncWrapper::RunL()
    {
    iWait.AsyncStop();
    }

// -----------------------------------------------------------------------------
// CCrPKCS12
// Constructor
// This function constructs CCrPKCS12 object
// Return Values:  None
// -----------------------------------------------------------------------------
CCrPKCS12::CCrPKCS12():iIter(1), iPrivateKeyIdArray(1), iSafeBagsCount(0)
    {
    // Default iteration count is 1.
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::ConstructL
// This function initializes CCrPKCS12 object's member objects.
// Parameters:     None
// Return Values:  None
// -----------------------------------------------------------------------------
void CCrPKCS12::ConstructL()
    {
  	iberSet = CCrBerSet::NewL( 1 );
    iUserCertificates = CX509CertificateSet::NewL( 1 );
    iCACertificates = CX509CertificateSet::NewL( 1 );
    iUserCertificateBuffer = new (ELeave) CArrayPtrFlat<TDesC8>(1);
    iCACertificateBuffer = new (ELeave) CArrayPtrFlat<TDesC8>(1);
    iPKCS8PrivateKeyArray = new (ELeave) CArrayPtrFlat<HBufC8>(1);
    }

// -----------------------------------------------------------------------------
// CCrPKCS12
// Destructor
// This function destructs CCrPKCS12 object
// Return Values:  None
// -----------------------------------------------------------------------------
CCrPKCS12::~CCrPKCS12()
    {    
    delete iberSet;
    delete iUserCertificates;
    delete iCACertificates;
    
    if (iUserCertificateBuffer)
        {
        iUserCertificateBuffer->ResetAndDestroy();
        delete iUserCertificateBuffer;
        }
    if (iCACertificateBuffer)
        {
        iCACertificateBuffer->ResetAndDestroy();
        delete iCACertificateBuffer;
        }
    delete iMac;
    delete iSalt;
    delete iPassWord;    
    delete iContentInfo;
    delete iBags;    
    delete iDecryptionKey;
        
    if (iPKCS8PrivateKeyArray)
        {        
        iPKCS8PrivateKeyArray->ResetAndDestroy();    
        delete iPKCS8PrivateKeyArray;
        }
    iPrivateKeyIdArray.ResetAndDestroy();
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::NewLC
// This function implements the two-phase construction of CCrPKCS12
// class. The function uses standard constructor to reserve memory
// for pkcs12-object, stores a pointer to the object into clean up
// stack, and returns the pointer to the object.
// Parameters:     None.
// Return Values:  CCrPKCS12*     Pointer to the CCrPKCS12 object.
// -----------------------------------------------------------------------------
CCrPKCS12* CCrPKCS12::NewLC()
    {
    CCrPKCS12* self = new ( ELeave ) CCrPKCS12();
    CleanupStack::PushL( self );

    self->ConstructL();

    return self; 
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::NewL
// This function implements the two-phase construction of CCrPKCS12
// class. The function reserves memory for pkcs12-object and
// returns pointer to that object. This function uses NewLC to create
// the object and store it to cleanup stack. Finally the object is popped
// from clean up stack.
// Parameters:     None.
// Return Values:  CCrPKCS12*    Pointer to the CCrPKCS12 object.
// -----------------------------------------------------------------------------
CCrPKCS12* CCrPKCS12::NewL()
    {
    CCrPKCS12* self = NewLC();
    CleanupStack::Pop();

    return self; 
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::Open
// Opens pkcs12-file from location given in parameter afilePath,
// then it makes a CCrBer::berSet from it. Then it starts testing
// berObjects from berSet. When it comes long enough, needed
// information can be get and stored to CCrPKCS12-Class' members.
// Parameters: CCrData& aPkcs12File    PKCS #12 file.
//             TBuf<48> aPassWord      password.
// Returns:    TCrStatus               Value of error.
// -----------------------------------------------------------------------------
TCrStatus CCrPKCS12::OpenL(CCrData&       aPkcs12File, 
                          const TDesC16& aPassword)
    {
    TCrStatus status = KCrOK;
    TInt errData = KErrNone;
   
    if (!iberSet)
        {
        User::Leave(KErrGeneral);
        }

    // Number of extracted BER objects.
    TInt numberOfBerObjects = 0;
    // make berSet, set of berObjects
    TRAP( errData,numberOfBerObjects = iberSet->OpenL( &aPkcs12File,KOpenAllLevels ));        
    if ( errData < KErrNone )
        {
        return KCrPkcs12 | KCrBerLibraryError;
        }
    // number of ber objects must be > 0.
    if (numberOfBerObjects < 1)
        {
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }
 
    iObjectNum = KFirstObject;

    // Now we have the first object
    iberObject = iberSet->At( iObjectNum );
           
    // File opened successfully, let's start testing berObjects.

    // 1. Test, is berObject Sequence?
	if ( !iberObject->IsSeqOrSet( iberObject->Type() ) )			
	    {
		return KCrPkcs12 | KCrNotValidPkcs12Object;
        }

    iObjectNum++;
    if (numberOfBerObjects < iObjectNum)
        {
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }
    iberObject = iberSet->At(iObjectNum );
                
    // 2. Test, is an integer?
    if ( iberObject->Type() != KBerInteger )					
        {
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }
    
    TInt version = 0;
    TRAP( errData,version = iberObject->GetIntegerL() );       
    if ( errData < KErrNone )
        {
        return KCrPkcs12 | KCrBerLibraryError;
        }

    // 3. Test, is version 3?
    if ( version != KVersion3 )				
        {
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }

    // Is there still ber objects?
    iObjectNum++;
    if (numberOfBerObjects < iObjectNum)
        {
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }
    iberObject = iberSet->At(iObjectNum );
    
    // 4. Test, is Seq?
    if ( !iberObject->IsSeqOrSet( iberObject->Type() ) )			
        {
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }

    // Remember where we are.
    TInt place = iObjectNum;

    // Is there still ber objects?
    iObjectNum++;
    if (numberOfBerObjects < iObjectNum)
        {
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }

    iberObject = iberSet->At(iObjectNum );
    
    CCrData* contentInfo = NULL;
    TRAP (errData, contentInfo = CCrData::NewL(EFalse));
    if (errData < KErrNone)
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrNoMemory; 
        }
        
    if ( !ReadContentInfo(*contentInfo ) )
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrNotPasswordBasedEncryption;
        }

    iberObject = iberSet->At( place );

    iObjectNum += JumpNextObjectAtSameLevel();

    iObjectNum++;
    if (numberOfBerObjects < iObjectNum)
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }
    iberObject = iberSet->At( iObjectNum );
    
    // 5. Test, is Seq?
    if ( !iberObject->IsSeqOrSet( iberObject->Type() ) )			
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }
    
    iObjectNum++;
    if (numberOfBerObjects < iObjectNum)
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }
    iberObject = iberSet->At(iObjectNum );
             
    // 6. Test, is Seq?
    if ( !iberObject->IsSeqOrSet( iberObject->Type() ) )			
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }

    iObjectNum++;
    if (numberOfBerObjects < iObjectNum)
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }
    iberObject = iberSet->At(iObjectNum ); 

     // 6. Test, is Seq?
    if ( iberObject->IsSeqOrSet( iberObject->Type() ) )
        {
        iObjectNum++;
        if (numberOfBerObjects < iObjectNum)
            {
            delete contentInfo;
            contentInfo = NULL;
            return KCrPkcs12 | KCrNotValidPkcs12Object;
            }
         iberObject = iberSet->At(iObjectNum );
        }

    // 7. Test, is OID?
    if ( iberObject->Type() != KBerOid)			
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }
 
    HBufC* oid = NULL;

    TRAP(errData, oid = iberObject->GetOidL() );
        
    if ( errData < KErrNone )
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrBerLibraryError;
        }

    // 8. What is used HMAC-algorithm??
    if ( *oid == KCrSha1 )										
        {  
        iHMACalgorithm = ECrSHA1; 
        }
    else if ( *oid == KCrMD5 )
        {
        iHMACalgorithm = ECrMD5;
        }   
    else
        {
        delete oid;
        oid = NULL;
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrNotSupportedHMACalgorithm;
        }

    delete oid;
    oid = NULL;

    iObjectNum++;
    if (numberOfBerObjects < iObjectNum)
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }
 
    iberObject = iberSet->At(iObjectNum);
    
    // If Octet String, read mac // Case in Outlook
    if (iberObject->Type() == KBerOctetString)
        {
        }
    else
        {
        // 9. Test, is Null Tag?
        if ( iberObject->Type() != KBerNull )
            {
            delete contentInfo;
            contentInfo = NULL;
            return KCrPkcs12 | KCrNotValidPkcs12Object;
            }
        
        iObjectNum++;
        if (numberOfBerObjects < iObjectNum)
            {
            delete contentInfo;
            contentInfo = NULL;
            return KCrPkcs12 | KCrNotValidPkcs12Object;
            }
        iberObject = iberSet->At(iObjectNum);

        // If Octet String, read mac
        if ( iberObject->Type() != KBerOctetString )
            {
            delete contentInfo;
            contentInfo = NULL;
            return EFalse;
            }
        } 
        
    // This is where needed data can be get
    TRAP( errData,iMac = iberObject->GetOctetStringL() );
        
    if ( errData < KErrNone )
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrBerLibraryError;
        }
    
    iObjectNum++;
    if (numberOfBerObjects < iObjectNum)
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }
    iberObject = iberSet->At(iObjectNum );

    // If Octet String, read salt, if OctetString can be found,
    // validate of this file is OK.
    if ( iberObject->Type() != KBerOctetString )
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrNotValidPkcs12Object;
        }

    TRAP( errData,iSalt = iberObject->GetOctetStringL() );        
    if ( errData < KErrNone )
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrBerLibraryError;
        }

    // Here's optional integer value
    if ( iberSet->Count() > ++iObjectNum )
        {
        iberObject = iberSet->At( iObjectNum );

        // If here is an integer, then read the number of iterations,
        // otherwise leave DEFAULT value ( = 1, given in constructor).
        if ( iberObject->Type() == KBerInteger )	
            {
            TRAP( errData,iIter = iberObject->GetIntegerL() );
            
            if ( errData < KErrNone )
                {
                delete contentInfo;
                contentInfo = NULL;
                return KCrPkcs12 | KCrBerLibraryError;
                }
            }
        }
    
    
    // Verify Mac.
    TBool result = EFalse;
    TRAP(errData, result = VerifyMacL(aPassword));
    if ( errData < KErrNone )
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrWrongPassWordOrCorruptedFile;
        }
    if ( result)
        {
       // If password was OK, open ContentInfo
        if ( !UnpackContentInfo(*contentInfo))
            {
            delete contentInfo;
            contentInfo = NULL;
            return KCrPkcs12 | KCrNotPasswordBasedEncryption;
            } 
        
        delete contentInfo;
        contentInfo = NULL;

        iSafeBagsCount = DecodeSafeBagsL();

        if ( !iSafeBagsCount )
            {
            return KCrPkcs12 | KCrNotPasswordBasedEncryption;
            } 
        }
    // If macs weren't the same, password was wrong
    else
        {
        delete contentInfo;
        contentInfo = NULL;
        return KCrPkcs12 | KCrWrongPassWordOrCorruptedFile;
        }

    return status;    
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::ReadContentInfo
// Reads ContentInfo.
// Parameters:     
// Return Values:  
// -----------------------------------------------------------------------------
TBool CCrPKCS12::ReadContentInfo( CCrData& aContentInfo )
    {
    // If this object isn't OID, or if it isn't type pkcs7Data, return EFalse
    if ( iberObject->Type() != KBerOid )
        {
        return EFalse;
        }

    HBufC* oid = NULL;
    TInt errData = KErrNone;
    
    TRAP( errData,oid = iberObject->GetOidL());
    if ( errData < KErrNone )
        {
        delete oid;
        oid = NULL;
        return EFalse;
        }

    if ( *oid != Kpkcs7Data )
        {
        delete oid;
        oid = NULL;
        return EFalse;
        }

    delete oid;
    oid = NULL;

    iberObject = iberSet->At( ++iObjectNum );

    if ( iberObject->Type() != KBerImplicitConstructed )
        {
        return EFalse;
        }

    // Now, let's open Constructed data.
    iberObject = iberSet->At( ++iObjectNum );

    // Outlook or Netscape
    if (iberObject->Type() == KBerConstructedBit + KBerOctetString ||
        iberObject->Type() == KBerOctetString)
        { 
        // Outlook
        if (iberObject->Type() == KBerOctetString )
            {
            TRAP (errData, iberObject->GetOctetStringL(aContentInfo));
            if (errData < KErrNone)
                {
                return EFalse;
                }
            }
        // Netscape
        else  // iberObject->Type() == KBerConstructedBit + KBerOctetString
            {
            TRAP(errData,iberObject->OpenConstructedEncodingL(aContentInfo));
            if ( errData < KErrNone )
                {
                return EFalse;
                }
            }

        // Size of contentData
        TInt size = 0;
        aContentInfo.Seek( ESeekCurrent,size );

        // ContentInfo's size -> Buffer's allocated size
        TRAP( errData,iContentInfo = HBufC8::NewL( size ) );
        if ( errData < KErrNone )
            {
            return EFalse;
            }

        // Make pointer to iContentInfo
        TPtr8 pContentInfo = iContentInfo->Des();

        // Copy readed ContentInfo to aContentInfo (parameter)
        aContentInfo.Read( pContentInfo );
       
        // Go back to start
        size = 0;
        aContentInfo.Seek( ESeekStart,size );
        return ETrue;
        }
    else
        {
        return EFalse;
        }    
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::UnpackContentInfo
// This function unpacks ContentInfo. First, it tests from what level
// SafeContents begin and after that it goes through ContentData.
// All sequences, that begin at that level, are tested (via OID) and
// Unpacked.
// Parameters: CCrData& aContentData. CCrData containing ContentInfo.             
// Return Values:  TBool   If OK, returns ETrue and if not, EFalse
// -----------------------------------------------------------------------------
TBool CCrPKCS12::UnpackContentInfo( CCrData& aContentData )
    {
    TInt objectNum = KFirstObject;
    TInt count     = 0;
    TInt errData   = KErrNone;
    HBufC* oid = NULL;

    CCrBerSet* set = NULL;

    TRAP( errData,set = CCrBerSet::NewL( 1 ) );

    if ( errData < KErrNone )
        {
        delete set;
        set = NULL;
        return EFalse;
        }

    // make berSet, set of berObjects
    TRAP( errData,set->OpenL( &aContentData,KOpenAllLevels ) );

    if ( errData < KErrNone )
        {
        delete set;
        set = NULL;
        return EFalse;
        }

    count = set->Count();
    
    CCrBer* object = set->At( objectNum );
        
    // 1. Test, is seq?
    if ( !object->IsSeqOrSet( object->Type() ) )
	    {
        delete set;
        set = NULL;
		return EFalse;
        }

    object = set->At( ++objectNum );
    
	TInt i(0);
    for (i = objectNum ; i < count ; i++)
        {
        // If there exists object, then check
        //this object
        if ( i < count )
            {
            object = set->At( i );
            }

        // Next check type of SafeContent
        if ( object->Type() == KBerOid )
            {
            // Get object identifier
            TRAP( errData, oid = object->GetOidL() );

            if( errData < KErrNone )
                {
                delete set;
                set = NULL;
                return EFalse;
                }

            if ( *oid == Kpkcs7Data )
                {
                // Unpack PKCS #7 data
                object = set->At( ++i );
                
                // Case it's just an octetstring
                // NOT TESTED!
                if ( object->Type() != KBerImplicitConstructed &&
                     object->Type() == KBerOctetString )
                    {
                    HBufC8* pkcs7Data = NULL;

                    TRAP( errData, pkcs7Data = object->GetOctetStringL() );
                    if ( errData < KErrNone )
                        {
                        delete oid;
                        oid = NULL;
                        delete set;
                        set = NULL;
                        return EFalse;
                        }
                    // Riddance from warnings
                    if ( pkcs7Data == NULL ) { delete pkcs7Data; }
                    
                    }
                else
                    {
                    object = set->At( ++i );
                    
                    if ( (object->Type() ==
                            (KBerConstructedBit + KBerOctetString)) ||  
                         (object->Type() == KBerOctetString))
                        {
                        CCrData* pkcs7Data = NULL;
                        TRAP (errData, pkcs7Data = CCrData::NewL(EFalse));
                        if (errData < KErrNone)
                            {
                            delete pkcs7Data;
                            pkcs7Data = NULL;
                            delete oid;
                            oid = NULL;
                            delete set;
                            set = NULL;
                            return EFalse; 
                            }

                        // Netscape
                        if (object->Type() ==
                                (KBerConstructedBit + KBerOctetString))
                            {

                            TRAP( errData,
                                 object->OpenConstructedEncodingL(*pkcs7Data));                            
                            if ( errData < KErrNone )
                                {
                                delete pkcs7Data;
                                pkcs7Data = NULL;
                                delete oid;
                                oid = NULL;
                                delete set;
                                set = NULL;
                                return EFalse;
                                }
                            }
                        // Outlook
                        else if (object->Type() == KBerOctetString)
                            {
                            TRAP (errData, object->GetOctetStringL(*pkcs7Data));
                            if ( errData < KErrNone )
                                {
                                delete pkcs7Data;
                                pkcs7Data = NULL;
                                delete oid;
                                oid = NULL;
                                delete set;
                                set = NULL;
                                return EFalse;
                                }
                            }
             
                        else
                            {
                            delete pkcs7Data;
                            pkcs7Data = NULL;
                            delete oid;
                            oid = NULL;
                            delete set;
                            set = NULL;
                            return EFalse;
                            }

                        TBool result = EFalse;
                        TRAP (errData, result = UnpackPkcs7DataL(*pkcs7Data));
                        if ( !result || errData < KErrNone )
                            {
                            delete pkcs7Data;
                            pkcs7Data = NULL;
                            delete oid;
                            oid = NULL;
                            delete set;
                            set = NULL;
                            return EFalse;
                            }
                        delete pkcs7Data;
                        pkcs7Data = NULL;
                        }
                    }
                }
            else if ( *oid == Kpkcs7EncryptedData )
                {
                TInt currentPlace = 0;
                // Mark, where we are now
                aContentData.Seek( ESeekCurrent,currentPlace );

                // Unpack PKCS #7 encrypted Data
                TBool result = EFalse;
                TRAP(errData, result = UnpackPkcs7EncryptedDataL( aContentData ) );
                if (!result || errData < KErrNone)
                    {
                    delete oid;
                    oid = NULL;
                    delete set;
                    set = NULL;
                    return EFalse;
                    }
                                  
                // Go back where we were and continue
                aContentData.Seek( ESeekStart,currentPlace );
                }
            delete oid;
            oid = NULL;
            }
        }
    if( i < count )
        {
        object = set->At( i );
        }

    delete set;
    set = NULL;
    // Now iBags contains all SafeBags.
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::UnpackPkcs7DataL
// Unpacks Pkcs7Data.
// Parameters: CCrData& aPkcs7Data     
// Return Values:  TBool   If OK, returns ETrue and if not, EFalse
// -----------------------------------------------------------------------------
TBool CCrPKCS12::UnpackPkcs7DataL( CCrData& aPkcs7Data )
    {
    TInt size = 0;

    // Size fo aPkcs7Data
    aPkcs7Data.Seek( ESeekCurrent,size );
    // Add this bag to iBags

    // Allocate needed amount of memory for iBags
    // If iBags-buffer is NULL
    if ( iBags == NULL)
        {
        iBags = HBufC8::NewL( size );
        }
    // If iBags isn't NULL
    else
        {
        // Re-allocated space for iBags is
        // size of the current iBags buffer + size of aPkcs7Dat
        iBags = iBags->ReAllocL(iBags->Size() + size);
        }
    // Append aPkcs7Data to iBags
    TPtr8 pBags = iBags->Des();
    HBufC8* tempBuf = HBufC8::NewLC(size);
    TPtr8 tempPtr = tempBuf->Des();
    aPkcs7Data.Read(tempPtr);
    pBags.Append(tempPtr);
    CleanupStack::PopAndDestroy(); // tempBuf

    return ETrue;
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::UnpackPkcs7EncryptedDataL
// Unpacks Pkcs7EncryptedData.
// Parameters: CCrData& aPkcs7EncryptedData     
// Return Values:  TBool   If OK, returns ETrue and if not, EFalse
// -----------------------------------------------------------------------------  
TBool CCrPKCS12::UnpackPkcs7EncryptedDataL( CCrData& aPkcs7EncryptedData )
    {
    TInt objectNum = KFirstObject;
    //TInt count     = 0;
    HBufC* oid = NULL;
    HBufC8* pkcs7EncryptedDataBuf = NULL;
    CCrBerSet* set = NULL;
    CCrBer* object;

    TInt numberOfItemsInCStack = 0;

    set = CCrBerSet::NewL(1);
    CleanupStack::PushL(set);
    numberOfItemsInCStack++;

    // make berSet, set of berObjects
    set->OpenL( &aPkcs7EncryptedData,KOpenAllLevels );

    //count = set->Count();    

    // Now we should have a sequence here
    object = set->At( ++objectNum );
        
    // 1. Test, is seq?
    if ( !object->IsSeqOrSet( object->Type() ) )			
	    {
        CleanupStack::PopAndDestroy(numberOfItemsInCStack); // set
        numberOfItemsInCStack = 0;
        return EFalse;
        }
    
    object = set->At( ++objectNum );
    
    // 2. Test, is integer?
    if ( object->Type() != KBerInteger )
        {
        CleanupStack::PopAndDestroy(numberOfItemsInCStack); // set
        numberOfItemsInCStack = 0;
        return EFalse;
        }

    // 3. Test, is version 1?
    if ( object->GetIntegerL() != KEdVer0 )
        {
        CleanupStack::PopAndDestroy(numberOfItemsInCStack); // set
        numberOfItemsInCStack = 0;
        return EFalse;
        }

    object = set->At( ++objectNum );
        
    // 4. Test, is seq?
    if ( !object->IsSeqOrSet( object->Type() ) )			
	    {
        CleanupStack::PopAndDestroy(numberOfItemsInCStack); // set
        numberOfItemsInCStack = 0;
		return EFalse;
        }

    object = set->At( ++objectNum );
        
    // 5. Test, is OID?
    if ( object->Type() != KBerOid )
        {
        CleanupStack::PopAndDestroy(numberOfItemsInCStack); // set
        numberOfItemsInCStack = 0;
        return EFalse;
        }

    // Get OID.
    oid = object->GetOidL();
    CleanupStack::PushL(oid);
    numberOfItemsInCStack++; // 2 set + oid
    // Is it pkcs7Data?
    if ( *oid != Kpkcs7Data )
        {
        CleanupStack::PopAndDestroy(numberOfItemsInCStack); // set + oid
        numberOfItemsInCStack = 0;
        return EFalse;
        }

    CleanupStack::PopAndDestroy(); // oid
    numberOfItemsInCStack--; // 1

    object = set->At( ++objectNum );
        
    // 6. Test, is seq?
    if ( !object->IsSeqOrSet( object->Type() ) )
	    {
        CleanupStack::PopAndDestroy(numberOfItemsInCStack); // set
        numberOfItemsInCStack = 0;
		return EFalse;
        }

    object = set->At( ++objectNum );

    // Here should be encryption algorithm
    if ( object->Type() != KBerOid )
        {
        CleanupStack::PopAndDestroy(numberOfItemsInCStack); // set
        numberOfItemsInCStack = 0;
        return EFalse;
        }

    // Get used encryption algorithm
    TInt algorithm = 0;

    oid = object->GetOidL();
    CleanupStack::PushL(oid);
    numberOfItemsInCStack++; // 2 set + oid

    algorithm = GetAlgorithmL( oid );
    CleanupStack::PopAndDestroy(); // oid
    numberOfItemsInCStack--; // 1

    object = set->At( ++objectNum );
        
    // 7. Test, is seq?
    if ( !object->IsSeqOrSet(object->Type()))			
	    {
        CleanupStack::PopAndDestroy(numberOfItemsInCStack); // set
        numberOfItemsInCStack = 0;
		return EFalse;
        }
    
    object = set->At( ++objectNum );

    // Here should be salt
    if ( object->Type() != KBerOctetString )
        {
        CleanupStack::PopAndDestroy(numberOfItemsInCStack); // set
        numberOfItemsInCStack = 0;
        return EFalse;
        }

    HBufC8* salt = NULL;    
    salt = object->GetOctetStringL();
    CleanupStack::PushL(salt);
    numberOfItemsInCStack++; // 2 set + salt

    object = set->At( ++objectNum );
    
    // Here should be an iteration count
    if ( object->Type() != KBerInteger)
        {
        CleanupStack::PopAndDestroy(numberOfItemsInCStack); // set + salt
        numberOfItemsInCStack = 0;
        return EFalse;
        }

    // By default, iteration count is set to 1
    TInt iter = 1;
    iter = object->GetIntegerL();

    // Next we have encrypted data

    object = set->At( ++objectNum );
    
    // If it's an octet string, read it.
    if (object->Type() == KBerOctetString)
        {
        pkcs7EncryptedDataBuf = object->GetOctetStringL();
        }

    // If it's an Outlook's KBerImplicit, read it.
    if (object->Type() == KBerImplicit)
        {
        pkcs7EncryptedDataBuf = object->GetContentStringLC();
        numberOfItemsInCStack++;
        }

    TBool stop = EFalse;

    if (object->Type() == KBerImplicitConstructed )
        {
        object = set->At( ++objectNum );
        
        for (TInt i = objectNum ; i < set->Count() && !stop ; i++)
            {
            if ( object->Type() == KBerOctetString )
                {
                // First octet string into buffer...
                if ( i == objectNum )
                    {
                    pkcs7EncryptedDataBuf = object->GetOctetStringL();
                    CleanupStack::PushL(pkcs7EncryptedDataBuf);
                    numberOfItemsInCStack++;
                    }
                // ...and rest of octet strings. 
                // New object's content is added to the
                // pkcs7EncryptedDataBuf 
                else
                    {
                    object = set->At( i );                    
                    
                    // Re-allocate buffer
                    __ASSERT_DEBUG(pkcs7EncryptedDataBuf, User::Invariant());
                    TInt currentSize = pkcs7EncryptedDataBuf->Size();
                    CleanupStack::Pop(); // pkcs7EncryptedDataBuf
                    numberOfItemsInCStack--;
                  
                    pkcs7EncryptedDataBuf = pkcs7EncryptedDataBuf->ReAllocL(currentSize + object->ContentLen());
                    CleanupStack::PushL(pkcs7EncryptedDataBuf);
                    numberOfItemsInCStack++;
                
                    // Get new OctetString
                    HBufC8* octetString = NULL;
                    octetString = object->GetOctetStringL();
                    TPtr8 pOctetString = octetString->Des();

                    // Copy it into buffer
                    TPtr8 pPkcs7EncryptedData = pkcs7EncryptedDataBuf->Des();
                    pPkcs7EncryptedData.Append( pOctetString );

                    // Delete OctetString
                     delete octetString;
                     octetString = NULL;                    
                    }
                }
            else if ( object->Type() == KBerEndOfContent )
                {
                stop = ETrue;
                }
            }
        }
    // OK, now we have buffer to be decrypted, used algorithm, salt and
    // an iteration count. Next we decrypt buffer and append it to iBags.
	
	DecryptPkcs7EncryptedDataL( pkcs7EncryptedDataBuf,salt,iter,algorithm );

    CleanupStack::PopAndDestroy(numberOfItemsInCStack);
    numberOfItemsInCStack = 0;

    return ETrue;
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::DecryptPkcs7EncryptedDataL
// Decrypts Pkcs7EncryptedData and adds it to iBags.
// Parameters:     
// Return Values:  TBool   If OK, returns ETrue and if not, EFalse
// -----------------------------------------------------------------------------
TBool CCrPKCS12::DecryptPkcs7EncryptedDataL( HBufC8* aPkcs7EncryptedData,
                                             HBufC8* aSalt,
                                             TInt aIter,
                                             TInt aAlgorithm)
    {
    HBufC8* certificates = HBufC8::NewLC(aPkcs7EncryptedData->Length());    
    TPtr8 ptrCertificates = certificates->Des();    
    DecryptDataL(*aPkcs7EncryptedData, *aSalt, aIter, aAlgorithm, ptrCertificates);
  
    if (iBags)
        {
        iBags = iBags->ReAllocL(iBags->Size() + ptrCertificates.Size());
        }
    else
        {
        iBags = HBufC8::NewL(ptrCertificates.Size());
        }

    TPtr8 pIBag = iBags->Des();
    pIBag.Append(ptrCertificates);
         
    CleanupStack::PopAndDestroy(certificates);    
    return ETrue;
    }
// -----------------------------------------------------------------------------
// CCrPKCS12::DecodeSafeBagsL
// Decodes SafeBags and if needed, also decrypts them.
// Parameters:
// Return Values:  TBool       If OK, returns ETrue and if not, EFalse
// -----------------------------------------------------------------------------
TInt CCrPKCS12::DecodeSafeBagsL()
    {
    TInt numberOfItemsInCStack = 0;
    TInt bagCount = 0;

    CCrData* safeBags = CCrData::NewLC(EFalse);
    ++numberOfItemsInCStack;
    
    TPtr8 piBags = iBags->Des();
    safeBags->Write( piBags );

    // Go back to start
    TInt start = 0;
    safeBags->Seek( ESeekStart,start );

    CCrBerSet* set = NULL;
    CCrBer* object = NULL;
    
    set = CCrBerSet::NewLC( 1 );
    ++numberOfItemsInCStack;
    
    set->OpenL( safeBags,KOpenAllLevels );    

    TInt count = set->Count();

    // We have to check first only for a private key, because we need
    // it's LocalKeyId, so we can identify a corresponding certificate.
	TInt i(0); 
    for (i = 0 ; i < count ; i++)
        {
        object = set->At( i );

        if ( object->Type() == KBerOid )
            {
            
            HBufC* oid = object->GetOidL();
            CleanupStack::PushL(oid);

            if ( *oid == Kpkcs8ShroudedKeyBag )
                {
                bagCount++;
                DecryptShroudedKeybagL( * safeBags );                    
                }
                
            if ( *oid == KkeyBag )
                {
                bagCount++;
                
                ExtractKeybagL( * safeBags );
                } 
            
            CleanupStack::PopAndDestroy(oid);            
            oid = NULL;
            }
        }

    // And now for certificates.
    for (i = 0 ; i < count ; i++)
        {
        object = set->At( i );

        if ( object->Type() == KBerOid )
            {            
            HBufC* oid = object->GetOidL();
            CleanupStack::PushL(oid);
                        
            if ( *oid == KcertBag )
                {
                bagCount++;
                DecodeCertBagL( * safeBags );
                }

            CleanupStack::PopAndDestroy(oid);            
            oid = NULL;
            }
        }

    CleanupStack::PopAndDestroy(numberOfItemsInCStack, safeBags);

    // Return amount fo safebags
    return bagCount;
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::ExtractKeybagL
// Extract Keybag.
// Parameters:     
// Return Values:
// -----------------------------------------------------------------------------
void CCrPKCS12::ExtractKeybagL( CCrData& aSafeBag )
    {
    TInt numberOfItemsinCStack = 0;
    CCrBerSet* set = NULL;

    set = CCrBerSet::NewLC( 1 );
    ++numberOfItemsinCStack;
    
    CCrBer* object = NULL;
    TInt objectNum = KFirstObject;

    set->OpenL( &aSafeBag,KOpenAllLevels );
    
    object = set->At( objectNum );

   if ( object->Type() != KBerImplicitConstructed )
        {
        User::Leave( KErrArgument );
        }

    HBufC8* PKCS8PrivateKey = object->GetOctetStringL();
    iPKCS8PrivateKeyArray->AppendL( PKCS8PrivateKey );

    if ( ++objectNum < set->Count() )
        {
        object = set->At( objectNum );        
        iPrivateKeyIdArray.Append( GetLocalKeyId( aSafeBag ) );
        }

    CleanupStack::PopAndDestroy( numberOfItemsinCStack, set );
    
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::DecryptShroudedKeybagL
// DecryptShroudedKeybag.
// Parameters:     
// Return Values:
// -----------------------------------------------------------------------------
void CCrPKCS12::DecryptShroudedKeybagL( CCrData& aSafeBag )
    {
    TInt numberOfItemsinCStack = 0;
    CCrBerSet* set = NULL;
    TInt errData = KErrNone;

    set = CCrBerSet::NewLC( 1 );
    ++numberOfItemsinCStack;
    
    CCrBer* object = NULL;
    TInt objectNum = KFirstObject;

    set->OpenL( &aSafeBag,KOpenAllLevels );
    
    object = set->At( objectNum );

    // This one needs first to be decrypted, first we open SafeBag and
    // then we decrypt encyrpted PrivateKey

    // Decryption Starts
    // -----------------
    if ( object->Type() != KBerImplicitConstructed )
        {
        User::Leave(KErrArgument);
        }

    object = set->At( ++objectNum );

    if ( !object->IsSeqOrSet( object->Type() ) )
        {
        User::Leave(KErrArgument);
        }

    object = set->At( ++objectNum );

    if ( !object->IsSeqOrSet( object->Type() ) )
        {
        User::Leave(KErrArgument);
        }

    object = set->At( ++objectNum );

    if ( object->Type() != KBerOid)
        {
        User::Leave(KErrArgument);
        }

    // Algorithm
    HBufC* oid = object->GetOidL();   
    CleanupStack::PushL(oid);

    TInt algorithm = GetAlgorithmL( oid );
    
    CleanupStack::PopAndDestroy(oid);
    oid = NULL;

    object = set->At( ++objectNum );

    if (!object->IsSeqOrSet(object->Type()))
        {
        User::Leave(KErrArgument);
        }

    object = set->At( ++objectNum );

    if ( object->Type() != KBerOctetString )
        {
        User::Leave(KErrArgument);
        }

    // Salt
    HBufC8* salt = object->GetOctetStringL();
    CleanupStack::PushL(salt);
    ++numberOfItemsinCStack;
    
    object = set->At( ++objectNum );

    if ( object->Type() != KBerInteger)
        {
        User::Leave(KErrArgument);
        }

    // Iterations
    TInt iter = 1;
    iter = object->GetIntegerL();

    object = set->At( ++objectNum );

    if ( object->Type() != KBerOctetString )
        {
        User::Leave(KErrArgument);
        }

    // EncryptedData
    HBufC8* encryptedPrivateKeyInfo = object->GetOctetStringL();
    CleanupStack::PushL(encryptedPrivateKeyInfo);
    ++numberOfItemsinCStack;
    
    if ( errData < KErrNone )
        {
        User::Leave(KErrArgument);
        }

    // Decrypt
    if ( !DecryptPrivateKeyL( encryptedPrivateKeyInfo,
                                                 salt,
                                                 iter,
                                                 algorithm ) )
        {
        User::Leave(KErrArgument);
        }

    // Decryption Ends
    // ---------------
    
    if ( ++objectNum < set->Count() )
        {
        object = set->At( objectNum );        
        iPrivateKeyIdArray.Append(GetLocalKeyId( aSafeBag ));
        }

    CleanupStack::PopAndDestroy(numberOfItemsinCStack, set);
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::GetLocalKeyId
// Gets pkcs12-SafeBag-attribute LocalKeyId
// Parameters:     CCrData& aBagData       SafeBag
// Return Values:  TBool
// -----------------------------------------------------------------------------    
HBufC8* CCrPKCS12::GetLocalKeyId( CCrData& aBagData )
    {
    CCrBerSet* set = NULL;
    CCrBer* object;
    TInt errData = KErrNone;

    TRAP( errData,set = CCrBerSet::NewL( 1 ) );

    if ( errData < KErrNone )
        {
        return NULL;
        }

    TInt objectNum = KFirstObject;

    TRAP( errData,set->OpenL( &aBagData,KOpenAllLevels ) );

    if ( errData < KErrNone )
        {
        delete set;
        set = NULL;
        return NULL;
        }

    TInt count = set->Count();

    // If there are attributes,
    if( count > 0 )
        {
        // continue,
        object = set->At( KFirstObject );
        }
    else
        {
        // otherwise go away.
        delete set;
        set = NULL;
        return NULL;
        }

    // Attributes begin
    if ( !object->IsSeqOrSet( object->Type() ) )
        {
        delete set;
        set = NULL;
        return NULL;
        }

    // Some applications seem to put an empty set in the
    // place of LocalKeyId. If so, give up.
    if (object->ContentLen() == 0)
        {
        delete set;
        set = NULL;
        return NULL;
        }

    // If there are more ber-objects left,
    if ( count > ++objectNum )
        {
        // get next,
        object = set->At( objectNum );
        }
    else
        {
        // otherwise give up.
        delete set;
        set = NULL;
        return NULL;
        }

    // First attribute
    if ( !object->IsSeqOrSet( object->Type() ) )
        {
        delete set;
        set = NULL;
        return NULL;
        }

    // Remember, at what level we are
    TUint level = object->Level();
    TBool stop = EFalse;

    object = set->At( ++objectNum );

    for( TInt i = objectNum ; i < count && !stop; i++ )
        {
        object = set->At( i );

        if ( object->Level() < level )
            {
            stop = ETrue;
            }
               
        // What is it?
        if ( object->Type() == KBerOid )
            {
            HBufC* oid = NULL;

            TRAP( errData,oid = object->GetOidL() );

            if ( errData < KErrNone )
                {
                delete set;
                set = NULL;
                return NULL;
                }

            if ( *oid == Kpkcs9LocalKeyId )
                {
				/// BEGIN OF WORKAROUND ///
				/* There is a major error in this library which causes that in certain 
				situations the localKeyId attribute is found for certificate even though
				it is not specified in the file. This happens when for example private key buffer
				is right after CA certificate buffer in bag data. After the CA certificate is parsed then
				the module starts searching for localKeyId and it founds it from the private key buffer. 
				This buffer contains localKeyId and thus CA certificate is wrongly identified to be 
				user certificate. A proper solution would be somehow limit the searching to correct ASN.1 sets
				but there's no time for that now. This is a ugly quick fix for this problem:
				Zero the localKeyId oid so that it is used only once. */	
								
				CCrData* data = object->Data();
				TInt start = object->ContentBegin();
				TInt len = object->ContentLen();
				data->Zero(start, len);
				
				// END OF WORKAROUND ///
                i += 2;

                if ( count > i )
                    {
                    object = set->At( i );
                    HBufC8* buf = NULL;

                    // Now we can read LocalKeyId
                    TRAP( errData,buf = object->GetOctetStringL() );

                    if ( errData < KErrNone )
                        {
                        delete set;
                        set = NULL;
                        return NULL;
                    }

                    stop = ETrue;
                    delete set;
                    set = NULL;
                    delete oid;
                    oid = NULL;
                    return buf;
                    }
                }
            delete oid;
            oid = NULL;
            }
        }

    delete set;
    set = NULL;
    return NULL;
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::DecodeCertBagL
// DecodeCertBag.
// Parameters:     
// Return Values: TBool
// -----------------------------------------------------------------------------    
void CCrPKCS12::DecodeCertBagL( CCrData& aSafeBags )
    {
    TInt numberOfItemsinCStack = 0;
    CCrBerSet* set = NULL;
    
    set = CCrBerSet::NewLC( 1 );
    ++numberOfItemsinCStack;
    

    CCrBer* object;
    TInt objectNum = KFirstObject;

    set->OpenL( &aSafeBags,KOpenAllLevels );   

    object = set->At( objectNum );

    // CertificateBags are decrypted at this phase and they just need
    // to be decoded from a certificate bag and added to certificate set.
    // LocalKeyId is used to check if certificate is user's.

    // Decoding starts
    // -----------------
    if ( object->Type() != KBerImplicitConstructed )
        {
        User::Leave(KErrArgument);
        }

    object = set->At( ++objectNum );

    if ( !object->IsSeqOrSet( object->Type() ) )
        {
        User::Leave(KErrArgument);
        }

    object = set->At( ++objectNum );

    if ( object->Type() != KBerOid )
        {
        User::Leave(KErrArgument);
        }

    // Algorithm
    HBufC* oid = NULL;

    oid = object->GetOidL();
    CleanupStack::PushL(oid);
        
    if ( *oid != Kx509certificate )
        {
        User::Leave(KErrNotSupported);
        }
    
    CleanupStack::PopAndDestroy(oid);
    oid = NULL;

    object = set->At( ++objectNum );

    if ( object->Type() != KBerImplicitConstructed )
        {
        User::Leave(KErrArgument);
        }

    object = set->At( ++objectNum );

    if ( object->Type() != KBerOctetString )
        {
        User::Leave(KErrArgument);
        }

    HBufC8* x509certificate = object->GetOctetStringL();   
    CleanupStack::PushL(x509certificate);
    ++numberOfItemsinCStack;

    // For identifying certificate's type.
    HBufC8* localKeyId = NULL;
    
    if ( ++objectNum < set->Count() )
        {
        object = set->At( objectNum );
        localKeyId = GetLocalKeyId( aSafeBags );
        }
    TBool isCA = EFalse;
    CX509Certificate* cert  =  CX509Certificate::NewLC( *x509certificate );
   

    // Check certificate version
    if ( cert->Version() != KX509Version3 )
          {
          isCA = cert->IsSelfSignedL();
          }
      else
          {
          // X509 v3 certificate. Check basicConstrains
          const CX509CertExtension* certExt = cert->Extension( KBasicConstraints );
          if ( certExt )
              {
              CX509BasicConstraintsExt* basic = CX509BasicConstraintsExt::NewLC( certExt->Data() );
              isCA = basic->IsCA();
              CleanupStack::PopAndDestroy( basic ); //basic
              }
          else
              {
              isCA = cert->IsSelfSignedL();
              }
          }
     
     
    CleanupStack::PopAndDestroy( cert );
    
    if( localKeyId == NULL || isCA )
        {
        if ( localKeyId )
        	{
            delete localKeyId;
            localKeyId = NULL;
        	}
        // Add to CAcertificates;
        PutCertsIntoSetL( iCACertificates, iCACertificateBuffer, x509certificate );
        // Do not delete x509certificate in this case 
        CleanupStack::Pop(x509certificate);
        --numberOfItemsinCStack;
        }    
    else
        {        
        CleanupStack::PushL(localKeyId);
        ++numberOfItemsinCStack;
        for (TInt i = 0; i < iPrivateKeyIdArray.Count(); ++i)
            {
            if (*localKeyId == *(iPrivateKeyIdArray[i]))
                {
                PutCertsIntoSetL( iUserCertificates,iUserCertificateBuffer, x509certificate );
                CleanupStack::PopAndDestroy(localKeyId);
                --numberOfItemsinCStack;
                // Do not delete x509certificate in this case 
                CleanupStack::Pop(x509certificate);
                --numberOfItemsinCStack;
                break;
                }                
            }                       
        }
    /*else
        {
        delete x509certificate;
        x509certificate = NULL; 
        }
*/
    
    CleanupStack::PopAndDestroy(numberOfItemsinCStack, set);
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::DecryptDataL
// -----------------------------------------------------------------------------        
void CCrPKCS12::DecryptDataL( const TDesC8& aEncryptedData,
                              const TDesC8& aSalt,
                              TInt aIter,
                              TInt aAlgorithm,
                              TDes8& aDecryptedData )
    {
    HBufC8* keyBuf = NULL; 
    TBuf8<8>   iv;
    TInt keySize = 0;
    TInt numberOfItemsinCStack = 0;
            
    CCrCrypto* crypto = CCrCrypto::NewLC();
    ++numberOfItemsinCStack;
        
    switch(aAlgorithm)
        {
        case ECrKpbeWithSHAAnd128BitRC2_CBC:
            {
            keySize = K128BitRC2KeySize;
            break;
            }
        case ECrKpbeWithSHAAnd40BitRC2_CBC:
            {                      
            keySize = K40BitRC2KeySize;
            break;
            }
        case ECrpbeWithSHAAnd3_KeyTripleDES_CBC:
            {
            // We need 3 keys for triple DES.
            keySize = 3 * KDesKeySize; 
            break;
            }
        case ECrpbeWithSHAAnd2_KeyTripleDES_CBC:
            {
            keySize = 2 * KDesKeySize;             
            break;
            }
        default:
            {
            User::Leave(KErrNotSupported);
            }
        }
    keyBuf = HBufC8::NewLC(keySize);
    ++numberOfItemsinCStack;
    TPtr8 keyPtr = keyBuf->Des();
    // Get key.
    crypto->DeriveKeyPKCS12L(*iPassWord,
                             aSalt,
                             aIter,
                             ECrSHA1,
                             KId1,
                             keySize,
                             keyPtr);
    
    // Get IV
    crypto->DeriveKeyPKCS12L(*iPassWord,
                             aSalt,
                             aIter,
                             ECrSHA1,
                             KId2,
                             iv.MaxSize(),
                             iv);

           
    switch(aAlgorithm)
        {
        case ECrKpbeWithSHAAnd128BitRC2_CBC:            
            {
            crypto->InitCryptRC2L(*keyBuf, iv, EFalse, 8*K128BitRC2KeySize);    
            break;
            }
        case ECrKpbeWithSHAAnd40BitRC2_CBC:
            {            
            crypto->InitCryptRC2L(*keyBuf, iv, EFalse, 40);    
            break;
            }
        case ECrpbeWithSHAAnd3_KeyTripleDES_CBC:
            {
            crypto->InitCrypt3DESL(
                keyPtr.Left(KDesKeySize), keyPtr.Mid(KDesKeySize, KDesKeySize), 
                keyPtr.Right(KDesKeySize), iv, EFalse); 
            break;
            }
        case ECrpbeWithSHAAnd2_KeyTripleDES_CBC:
            {
            crypto->InitCrypt3DESL(
                keyPtr.Left(KDesKeySize), keyPtr.Right(KDesKeySize), 
                keyPtr.Left(KDesKeySize), iv, EFalse); 
            break;
            }
        default:
            {
            User::Leave(KErrNotSupported);
            }
        }
    crypto->ProcessL(aEncryptedData, aDecryptedData);        
    crypto->FinalCryptL(aDecryptedData);                       
    CleanupStack::PopAndDestroy(numberOfItemsinCStack, crypto);
    }
    
// -----------------------------------------------------------------------------
// CCrPKCS12::DecryptPrivateKey
// DecryptPrivateKey.
// Parameters:     
// Return Values:
// -----------------------------------------------------------------------------

TBool CCrPKCS12::DecryptPrivateKeyL( HBufC8* aEncryptedPrivateKey,
                                    HBufC8* aSalt,
                                    TInt aIter,
                                    TInt aAlgorithm )
    {    
    TBool   returnValue = ETrue;    
    HBufC8* privateKey = HBufC8::NewLC(aEncryptedPrivateKey->Length());    
    TPtr8 privateKeyPtr = privateKey->Des();
    TPtr8   paEncryptedPrivateKey = aEncryptedPrivateKey->Des();
    TPtr8   paSalt = aSalt->Des();

    DecryptDataL(*aEncryptedPrivateKey, *aSalt, aIter, aAlgorithm, privateKeyPtr);
   
    HBufC8* PKCS8PrivateKey = HBufC8::NewL(privateKey->Length());
    PKCS8PrivateKey->Des().Copy(*privateKey);
    iPKCS8PrivateKeyArray->AppendL(PKCS8PrivateKey);
                    
    CleanupStack::PopAndDestroy(privateKey);
    return returnValue;
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::VerifyMacFromEightBytePassword
// This function generates double byte pasword from given eight byte
// password and calls VerifyMac.
// Parameters:     aPassWord   password given to open function
// Return Values:  TBool       If OK, returns ETrue and if not, EFalse
// -----------------------------------------------------------------------------
TBool CCrPKCS12::VerifyMacFromEightBytePassword(const TDesC8& aPassWord)
    {
    TInt errData = KErrNone;

    // Alloc Space for password in pkcs12-format. Needed space is
    // 2 * aPassWord + 2
    if (iPassWord == NULL)
        {
        TRAP( errData,iPassWord = HBufC8::NewL( (2 * aPassWord.Length()) + 2) );
        
        if ( errData < KErrNone )
            {
            return EFalse;
            }
        }

    // Pointer to allocated pkcs12PassWord
    TPtr8 aPkcs12PassWord = iPassWord->Des();

    // Make sure password is empty.
    aPkcs12PassWord = _L8("");

    // Append 0x00 between characters
    for( TInt i = 0 ; i < aPassWord.Length() ; i++ )
        {
        aPkcs12PassWord.Append(0x00);
        aPkcs12PassWord.Append(aPassWord[i]);
        }

    // Append last 0x00 0x00
    aPkcs12PassWord.Append(0x00);
    aPkcs12PassWord.Append(0x00);
    
    TBool result = EFalse;
    TRAP(errData, result = VerifyMacL(aPkcs12PassWord));
    if ( errData < KErrNone )
        {
        return EFalse;
        }
    return result;
    }
// -----------------------------------------------------------------------------
// CCrPKCS12::VerifyMacL
// Verifies Mac. This function generates mac with password given to
// open function above, salt and iteration count, It uses crypto-
// library to do that. Generated mac is compared with iMac (mac within
// pkcs12 file) and if they are the same, password was OK and if they
// weren't the same, password was wrong, or file is corrupted.
// Parameters:     aPassWord   password given to open function
// Return Values:  TBool       If OK, returns ETrue and if not, EFalse
// -----------------------------------------------------------------------------  
TBool CCrPKCS12::VerifyMacL(const TDesC8& aPassWord)
    { 
    TPtr8 pContentInfo = iContentInfo->Des();
    
    // Key to be derivated
    iDecryptionKey = HBufC8::NewL( iMac->Length() );
    TPtr8 pDecryptionKey = iDecryptionKey->Des();

    TInt numberOfItemsInCStack = 0;
    // Mac to be calculated
    HBufC8* mac = HBufC8::NewLC( iMac->Length() );
    numberOfItemsInCStack++;        
    TPtr8 pMac = mac->Des();

    CCrCrypto* crypto = CCrCrypto::NewLC();
    numberOfItemsInCStack++;

    if ( iHMACalgorithm == ECrSHA1 )
        {
        crypto->DeriveKeyPKCS12L(aPassWord,
                                 *iSalt,
                                 iIter,
                                 ECrSHA1,
                                 KId3,
                                 iMac->Length(),
                                 pDecryptionKey);
        
        // Initialize HMAC SHA-1 algorithm
        crypto->InitDigestHMACL( pDecryptionKey,ECrSHA1 );        
        }
    else
        {
        CleanupStack::PopAndDestroy(numberOfItemsInCStack);
        return EFalse;
        }

    // Process data (filedata)
    crypto->ProcessL( pContentInfo, pMac);
    
    // Final digest algorithm
    crypto->FinalDigest( pMac );

    if ( pMac == *iMac )
        {
        CleanupStack::PopAndDestroy(numberOfItemsInCStack); 
        return ETrue;
        }
    else
        {
        CleanupStack::PopAndDestroy(numberOfItemsInCStack);
        return EFalse;
        }
    }

TBool CCrPKCS12::VerifyMacL(const TDesC16& aPassWord)
    {
    TUint8  halfCharacter;
    TUint16 character;

    // Alloc Space for password in pkcs12-format. Needed space is
    // 2 * aPassWord + 2
    if (iPassWord == NULL)
        {
        iPassWord = HBufC8::NewL( (2 * aPassWord.Length()) + 2);
        }

    // Pointer to allocated pkcs12PassWord
    TPtr8 pPkcs12PassWord = iPassWord->Des();

    // Make sure password is empty.
    pPkcs12PassWord = _L8("");

    // Divide characters into bytes and append them.
    for (TInt i = 0; i < aPassWord.Length(); i++)
        {
        character = aPassWord[i];

        halfCharacter = (TUint8) (character >> 8);
        pPkcs12PassWord.Append(halfCharacter);

        halfCharacter = (TUint8) (character & 0xFF);
        pPkcs12PassWord.Append(halfCharacter);
        }

    // Append last 0x00 0x00
    pPkcs12PassWord.Append(0x00);
    pPkcs12PassWord.Append(0x00);

    return VerifyMacL(pPkcs12PassWord);
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::JumpNextObjectAtSameLevel
// Returns integer value of how many objects shoud be jumped to get
// next object at same level. Works, when CCrBer-library works with
// definite values correctly.
// Parameters:     None.
// Return Values:  TUint   Next object's index-number at same level
// -----------------------------------------------------------------------------
TUint CCrPKCS12::JumpNextObjectAtSameLevel()
    {
    TUint levelAt = iberObject->Level();
    TUint jumps = 1;
    
    TUint index_to = iObjectNum + 1;

    iberObject = iberSet->At( index_to );
    
    while( iberObject->Level() != levelAt )
        {
        jumps++;
        index_to++;
        iberObject = iberSet->At( index_to );
        }   

    return jumps;
    }
// -----------------------------------------------------------------------------
// CCrPKCS12::GetAlgorithm
// This function identifies algorithm and returns integer about what
// algorithm was. Algorithm is identified via ObjectIdentifier.
// For example, in case if aBuf = "1.2.840.113549.1.12.1.2",
// (pbeWithSHAAnd40BitRC4) function returns 2.
// Parameters:     aBuf    buffer containing ObjectIdentifier
// Return Values:  TUint   Last number of an algorithm
// -----------------------------------------------------------------------------
TUint CCrPKCS12::GetAlgorithmL( HBufC* aBuf )
    {
    // What is the algorithm?
    TUint algorithm = 0;

    if ( *aBuf == KpbeWithSHAAnd128BitRC4 )
        {
        algorithm = ECrpbeWithSHAAnd128BitRC4;
        }
    else if ( *aBuf == KpbeWithSHAAnd40BitRC4 )
        {
        algorithm = ECrpbeWithSHAAnd40BitRC4;
        }
    else if ( *aBuf == KpbeWithSHAAnd3_KeyTripleDES_CBC )
        {
        algorithm = ECrpbeWithSHAAnd3_KeyTripleDES_CBC;
        }
    else if ( *aBuf == KpbeWithSHAAnd2_KeyTripleDES_CBC )
        {
        algorithm = ECrpbeWithSHAAnd2_KeyTripleDES_CBC;
        }
    else if ( *aBuf == KpbeWithSHAAnd128BitRC2_CBC )
        {
        algorithm = ECrKpbeWithSHAAnd128BitRC2_CBC;
        }
    else if ( *aBuf == KpbeWithSHAAnd40BitRC2_CBC )
        {
        algorithm = ECrKpbeWithSHAAnd40BitRC2_CBC;
        }
    else
        {
        User::Leave(KErrNotSupported);
        }

    return algorithm;
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::PutCertsIntoSet
// This function puts all certificates into set given at parameter.
// DecodeCertsL calls this functiuon.
// Parameters:     *set    Set, where certificates will be put into
// Return Values:  TInt    Value of error
// -----------------------------------------------------------------------------
void CCrPKCS12::PutCertsIntoSetL( CX509CertificateSet* aSet,
                                  CArrayPtrFlat<TDesC8>* aBufSet,
                                  HBufC8* aX509certificate )
    {
    // Pointer to certificate given at third parameter
    TPtr8 paX509certificate = aX509certificate->Des();

    // Add certificate to CertificateSet given at first parameter
    aSet->DecodeCertsL( paX509certificate );
    
    // Add certificate to CertificateSet given at second parameter 
    aBufSet->AppendL(aX509certificate);        
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::GetPrivateKeys
// -----------------------------------------------------------------------------
const CArrayPtrFlat<HBufC8>& CCrPKCS12::PrivateKeys() const
    {
    return *iPKCS8PrivateKeyArray;
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::GetUserCertificates
// This function returns CX509CertificateSet, set of CX509Certificates,
// that are defined in certman. These certificates are user certificates.
// All certificates from X509Certificates are tested and if certificate
// isn't CA Certificate and if KeyUsage don't mach to one given at
// parameter, it is removed from set. After that, UserCertificates
// (type CX509CertificateSet *) are returned to caller.
// Parameters:     TKeyUsage aKeyUsage.
// Return Values:  CX509CertificateSet*  Pointer to the CX509CertificateSet.
// -----------------------------------------------------------------------------
const CX509CertificateSet& CCrPKCS12::UserCertificates() const
    {
	return *iUserCertificates;
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::GetUserCertificateBuffer
// This function returns CX509CertificateSet, set of CX509Certificates,
// that are defined in certman. These certificates are user certificates.
// All certificates from X509Certificates are tested and if certificate
// isn't CA Certificate and if KeyUsage don't mach to one given at
// parameter, it is removed from set. After that, UserCertificateBuffer
// (type CArrayPtrFlat<TDes>* ) are returned to caller.
// Parameters:     TKeyUsage aKeyUsage.
// Return Values:  CArrayPtrFlat<TDesC8>*  Pointer to a buffer.
// -----------------------------------------------------------------------------
const CArrayPtrFlat<TDesC8>& CCrPKCS12::UserCertificateBuffer() const
                                        
    {
    return *iUserCertificateBuffer;
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::GetCACertificates
// This function returns CX509CertificateSet, set of CX509Certificates,
// that are defined in certman. These certificates are CA certificates.
// All certificates from X509Certificates are tested and if certificate
// isn't CA Certificate, it is removed from set. After that,
// CACertificates (type CX509CertificateSet *) are returned to caller.
// Parameters:     none.
// Return Values:  CX509CertificateSet*  Pointer to the CX509CertificateSet.
// -----------------------------------------------------------------------------
const CX509CertificateSet& CCrPKCS12::CACertificates() const
    {
    return *iCACertificates;
    }
// -----------------------------------------------------------------------------
// CCrPKCS12::GetCACertificateBuffer
// This function returns CX509CertificateSet, set of CX509Certificates,
// that are defined in certman. These certificates are CA certificates.
// All certificates from X509Certificates are tested and if certificate
// isn't CA Certificate, it is removed from set. After that,
// CACertificates (type CArrayPtrFlat<TDesC8>*) are returned to caller.
// Parameters:     none.
// Return Values:  CArrayPtrFlat<TDesC8>*  Pointer to the CX509CertificateSet.
// -----------------------------------------------------------------------------
const CArrayPtrFlat<TDesC8>& CCrPKCS12::CACertificateBuffer() const
    {
    return *iCACertificateBuffer;
    } 

// -----------------------------------------------------------------------------
// CCrPKCS12::GetIter
// This function returns the number of iterations
// Parameters: none 
// Returns:    TUint    Number of iterations.
// -----------------------------------------------------------------------------

TUint CCrPKCS12::Iter()
	{
	return iIter;
	}

// -----------------------------------------------------------------------------
// CCrPKCS12::GetMac
// This function returns the mac
// Parameters: none 
// Returns:    HBufC8*     Pointer to mac.
// -----------------------------------------------------------------------------
HBufC8* CCrPKCS12::Mac()
	{
	return iMac;
	}

// -----------------------------------------------------------------------------
// CCrPKCS12::GetSalt
// This function returns the salt
// Parameters: none 
// Returns:    HBufC8*     Pointer to salt.
// -----------------------------------------------------------------------------
HBufC8* CCrPKCS12::Salt()
    {
	return iSalt;
    }

// -----------------------------------------------------------------------------
// CCrPKCS12::GetSafeBagsCount
// This function returns number of SafeBags
// Parameters: none
// Returns:    TInt      Number of SafeBags
// -----------------------------------------------------------------------------
TUint CCrPKCS12::SafeBagsCount()
    {
	return iSafeBagsCount;
    }
// EOF
