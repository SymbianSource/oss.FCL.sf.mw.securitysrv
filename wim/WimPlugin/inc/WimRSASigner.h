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
* Description:  Interface which handles RSA signing operations.
*
*/
  


#ifndef WIMRSASIGNER_H
#define WIMRSASIGNER_H


//INCLUDES

#include <e32base.h>
#include <e32std.h>


//FORWARD DECLARATIONS

class CMessageDigest;
class CWimKeyStore;


/**
*  RSA signer object. Handles RSA signing.
*
*  @lib   WimPlugin
*  @since Series60 2.6
*/
class CWimRSASigner: public CBase, public MRSASigner
    {
 
    public:
    
        /** 
        * Creates new Instance of CWimRSASigner
        * @param aClient -Reference to keystore
        * @return An instance of this class
        */
        static CWimRSASigner* NewL( CWimKeyStore& aClient );
    
        /** 
        * Destructor
        */
        ~CWimRSASigner();

        /** 
        * Returns the object's human-readable label
        * @return TDesC&
        */
        const TDesC& Label() const;
    
        /** 
        * Returns a reference to the associated token
        * @return MCTToken&
        */
        MCTToken& Token() const;
    
        /** 
        * Returns a UID representing the type of the token object. The
        * meanings of possible UIDs should be documented in the
        * documentation for the interface that returns them.
        * @return TUid
        */
        TUid Type() const;

        /** 
        * Returns a handle for the object. The primary purpose of the
        * handle is to allow token objects to be 'passed' between
        * processes. See TCTTokenObjectHandle for more details.
        * @return   TCTTokenObjectHandle
        */
        TCTTokenObjectHandle Handle() const;

        /** 
        * Sign some data. 
        * The data is hashed before the signature is created using the SHA-1
        * algorithm.
        * @param    aPlaintext The string to be signed.
        * @param    aSignature The returned signature. This remains owned
        *           by the signer, and is valid until the signer is released or
        *           until the next Sign.
        * @param    aStatus Async status notification
        * @return void
        */
        void SignMessage( const TDesC8& aPlaintext, 
                          CRSASignature*& aSignature, 
                          TRequestStatus& aStatus );
             
        /** 
        * Perform a raw signing operation.
        * @param  aData The data to be signed - this should be some form of hash 
        *         of the actual message to be signed.  If the data is too long, 
        *         this method will return KErrOverflow through aStatus. 
        * @param  aSignature The returned signature. This remains owned
        *         by the signer, and is valid until the signer is released or
        *         until the next Sign.
        * @param aStatus Async status notification
        * @return void
        */
        void Sign( const TDesC8& aData, 
                   CRSASignature*& aSignature, 
                   TRequestStatus& aStatus );
    
        /** 
        * Cancel an ongoing sign
        * @return void
        */
        void CancelSign();

        /** 
        * Release object
        * @return void
        */
        void Release();

        /** 
        * Sets label for signin key object.
        * @param aLabel -label to be set.
        * @return void
        */
        void SetLabelL( const TDesC& aLabel );

        /** 
        * Sets objectId for this object.
        * @param aObjectID -points to a certain key
        * @return void
        */
        void SetObjectId( const TInt aObjectId );

    protected:

        /** 
        * Releases the object once the base-class framework work has
        * been done. The default implementation simply does a 'delete
        * this', but derived classes can substitute their own behaviour,
        * for instance to implement reference counting of the token
        * objects themselves.
        * @return void
        */
        void DoRelease();

    private:

        /**
        * Default constructor
        * @param    aClient pointer to caller.
        */
        CWimRSASigner( CWimKeyStore& aClient );

    private:
    
        //Handles hashing. Owned
        CMessageDigest*         iDigest;

        //Objects handle
        TCTTokenObjectHandle    iHandle;

        //Reference to keystore
        CWimKeyStore&           iClient;
    
        //holds key label. Owned.
        HBufC*                  iLabel;
        
        //buffer for hash result
        HBufC8*                 iDigestBuf;

    };

#endif // WIMRSASIGNER_H

// end of file 
