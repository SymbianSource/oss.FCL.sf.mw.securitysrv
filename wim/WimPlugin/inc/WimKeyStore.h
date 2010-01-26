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
* Description:  An implementation of single key store interface
*
*/


#ifndef CWIMKEYSTORE_H
#define CWIMKEYSTORE_H

//  INCLUDES

#include <mctkeystore.h>
#include <e32base.h>

// FORWARD DECLARATION

class CWimKeyDetails;
class MCTAuthenticationObjectList;
class CWimToken;
class CWimPin;
class CWimRSASigner;


// CLASS DECLARATION

/**
*  Represents read only key store.
*
*  @lib   WimPlugin
*  @since Series60 2.1
*/

class CWimKeyStore : public CActive, public MCTKeyStore 
    {
    public: // Constructor and destructor

        /**
        * Constructor
        * @param  aToken   (IN) A reference to current token
        * @return An instance of this class
        */
        static CWimKeyStore* NewL( CWimToken& aToken );
    public: // Function from base class MCTTokenInterface
        
        /**
        * Returns a reference to current token
        * @return A Reference to current token
        */
        MCTToken& Token();

    public: // Functions from base class MKeyStore
        
        /**
        * Lists all the keys in the store that match the filter
        * The caller of this function owns all given parameters
        * @param  aKeys    (OUT) An array to which the returned keys 
        *         are appended.
        * @param  aFilter  (IN) A filter controlling which keys are returned
        * @param  aStatus  (IN/OUT) This will be completed with the final 
        *         status code. Caller should set it to value TRequestPending
        *         KErrNone, if no errors detected. Note: if no keys are found,
        *         it is also KErrNone
        *         KErrArgument, if aKeys array is NULL
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void List( RMPointerArray<CCTKeyInfo>& aKeys, 
                   const TCTKeyAttributeFilter& aFilter, 
                   TRequestStatus& aStatus );

        /**
        * Cancels an ongoing List operation
        * @return void
        */
        void CancelList();

        /**
        * Retrieves a reference to pointer of key depending on given 
        * handle (aHandle). The caller of this function owns all given 
        * parameters.
        * @param  aHandle   The handle of the required key.
        * @param  aInfo     The returned key info.
        * @param  aStatus   Async request notification.
        *         KErrNone, if no errors detected
        *         KErrArgument, if aHandle is not current token's handle
        *         KErrNotFound, if aInfo not found
        *         KErrHardwareNotAvailable, if Wim card suddenly removed
        *         Any other system wide error code (e.g. KErrNoMemory)
        * @return void
        */
        void GetKeyInfo( TCTTokenObjectHandle aHandle, 
                         CCTKeyInfo*& aInfo,
                         TRequestStatus& aStatus );

        /**
        * Cancels an ongoing GetKeyInfo operation
        * @return void
        */
        void CancelGetKeyInfo();

        /** 
        * Open an RSA key for signing
        * @param aHandle The handle of the key to be opened
        * This must be the handle of a RSA key on this store that is
        * usable for signing by this process or the operation will fail.
        * @param aSigner The returned signer object
        * @param aStatus Asynchronous request notification.
        * @return void
        */
        void Open( const TCTTokenObjectHandle& aHandle, 
                   MRSASigner*& aSigner,
                   TRequestStatus& aStatus );
        
        /** 
        * Open a DSA key for signing
        * @param aHandle -The handle of the key to be opened
        * This must be the handle of a DSA key on this store that is
        * usable by this process for signing or the operation will fail.
        * @param aSigner -The returned signer object
        * @param aStatus -Asynchronous request notification.
        * @return void
        */
        void Open( const TCTTokenObjectHandle& aHandle, 
                   MDSASigner*& aSigner, 
                   TRequestStatus& aStatus );

        /** 
        * Open a RSA key for private decryption
        * @param aHandle -The handle of the key to be opened
        * This must be the handle of a RSA key on this store that is
        * usable by this process for decryption or the operation will fail.
        * @param aDecryptor -The returned decryptor object
        * @param aStatus -Asynchronous request notification.
        * @return void
        */
        void Open( const TCTTokenObjectHandle& aHandle, 
                   MCTDecryptor*& aDecryptor,
                   TRequestStatus& aStatus );

        /** 
        * Open a DH key for key agreement
        * @param aHandle -The handle of the key to be opened
        * This must be the handle of a DH key on this store that is
        * usable by this process for decryption or the operation will fail.
        * @param aDH    -The returned agreement object
        * @param aStatus -Asynchronous request notification.
        * @return void
        */
        void Open( const TCTTokenObjectHandle& aHandle, 
                   MCTDH*& aDH, 
                   TRequestStatus& aStatus );

        /** 
        * Cancels an ongoing Open operation
        * @return void
        */
        void CancelOpen();

        /** 
        * Returns the public key in DER-encoded ASN-1
        * @param aHandle    -The handle of the key to be opened
        * @param aPublicKey -returned key
        * @param aStatus    -Asynchronous request notification.
        * @return void
        */
        void ExportPublic( const TCTTokenObjectHandle& aHandle,
                           HBufC8*& aPublicKey,
                           TRequestStatus& aStatus );
        
        /**  
        * Cancels an ongoing Export operation
        * @return void
        */
        void CancelExportPublic();

       
        /** 
        * Perform signing operation.
        * @param  aObject -The handle of signer object. 
        * @param  aDigest -The data to be signed.   
        * @param  aSignature -The returned signature. This remains owned
        *         by the signer, and is valid until the signer is released or
        *         until the next Sign.
        * @param aStatus -Async status notification
        * @return void
        */ 
        void NonRepudiableRSASign( const TCTTokenObjectHandle& aObject, 
                                   const TDesC8& aDigest, 
                                   CRSASignature*& aSignature, 
                                   TRequestStatus& aStatus );

    protected:  // From base class MCTTokenInterface
        
        virtual void DoRelease();

    private: // Constructors

        /**
        * Default constructor
        * @param  aToken   (IN) A reference to current token
        */
        CWimKeyStore( CWimToken& aToken );

        // Second phase constructor
        void ConstructL();

        // Destructor
        virtual ~CWimKeyStore();

    private:

        /**
        * Starts initialization on this class.
        * @return void
        */
        void Initialize();

        /**
        * Creates one key info for list or get key info operation
        * @param   aIndex (IN) An index value of current key info in
        *          the internal array
        * @return  A pointer to created key info
        */

        CCTKeyInfo* CreateKeyInfoL( TInt aIndex );

        /**
        * Leavable function to filter keys in list operation.
        * @return void
        */
        void DoFilterKeysL();

        /**
        * Leavable function for key creation.
        * @param   aHandle (IN) Handle of desired authentication object.
        * @return  A pointer to created authentication object
        */
        MCTAuthenticationObject* MakeAuthObjectL( TInt aHandle );

        /**
        * Returns boolean value true or false indicating if token is removed
        * @return  Boolean value: true, if token removed, false if not
        */
        TBool TokenRemoved();

        
        /**
        * Checks if token is removed or is this object active
        * @return  Boolean value: true if allowed, false if not
        */
        TBool EnteringAllowed( TRequestStatus& aStatus );


    private: // From base class CActive

        void RunL();
        
        /** 
        * Leaves in RunL are handled here.
        * @param  aError  Leaving code
        * @return An integer that should be KErrNone if leave was handled.
        */
        TInt RunError( TInt aError );
        
        void DoCancel();

    private: // Data

        // A reference to current token of this interface
        CWimToken& iToken;

        // State flag for RunL
        enum TPhase
            {
            EIdle,
            EOpenAuthObjectsInterface,
            EListAuthObjects,
            EGetKeyList,
            ECheckGetKeyList,
            EFilterKeys,
            EGetKey,
            ECreateRSASignerObject,
            EStartNonRepudiableRSASign,
            ESignCompleted,
            EStartExportPublicKey,
            EExportPublicKeyDone
            };

        // Flag for internal state machine
        TPhase iPhase;

        // Flag for internal state machine
        TPhase iPhaseOriginal;

        // The pointed object acts as a converter between
        // CWimCertStore/CWimKeyStore interface and WimClient
        // This class owns the pointed object
        CWimKeyDetails* iCWimKeyDetails;

        // Used for saving caller status in asynchronous call
        // This class don't own the pointed object
        TRequestStatus* iOriginalRequestStatus;

        // Temporal array for key info pointers
        // This array is used to append key infos in List operation
        // This class don't own the pointed objects
        RPointerArray<CCTKeyInfo>* iKeyList;

        // Temporal array for key infos. 
        // This array is used to fetch key infos from WimClient.
        RPointerArray<CCTKeyInfo> iKeyInfos;

        // Used with list operation to filter keys
        TCTKeyAttributeFilter iKeyFilter;   

        // Index for going through all keys asynchronously 
        // in list operation.
        TInt iKeyIndex;

        // Used to fetch one key info from wimclient
        // This class don't own the pointed object.
        CCTKeyInfo** iInfo;

        // Key value for key object. Used in asynchronous call of
        // getting one key info.
        TInt iHandle;
        
        // Array for key info keys fetched from wimclient
        CArrayFixFlat<TUint8>* iKeyNumbers;

        // Array of pointers to PIN-NRs.This class don't own the pointed
        // objects
        const CArrayPtrFlat<CWimPin>* iPinNRs; 

        // Is key list done or not already. Set to true value in the first
        // key list.
        TBool iListDone;

        //Signer object, which is returned to caller. Not Owned.
        CWimRSASigner*              iSigner;
              
        //Handle stored during opening new RSA signer object.
        TCTTokenObjectHandle        iOpenSigninKeyHandle;

        //Buffer, which holds received signature from server. Owned.
        HBufC8*                     iSignature;
        
        //Signature object which is filled with data received from server.
        //Not owned.
        CRSASignature**              iRSASignature; 
        
        //Buffer used to contain KeyId in Sign and public key export
        //operations. Owned.
        HBufC8*                     iKeyId;

        //Buffer to hold public key data during public key export. Not Owned.
        HBufC8*                     iPublicKey;

        // The index of exported key
        TInt                        iExportKeyIdIndex;
        
        TInt                        iRSAKeyIdIndex;
        
        const TDesC8*               iDigest;
        
    };

#endif      // CWIMKEYSTORE_H   
            
// End of File
