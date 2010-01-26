/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  The header file of KeyEncryptor
*
*/


#ifndef __KEYENCRYPTOR_H__
#define __KEYENCRYPTOR_H__

#include <e32des8.h>

#define KEY_ENCRYPTOR 0x101FB670

const TUid KKeyEncryptorInterfaceUID = { KEY_ENCRYPTOR };

/**
 * Key Encryptor Interface class
 *
 * This class defines the interface for key encryptor which is used
 * to encrypt the key when it is stored to the keystore and decrypt it back
 * when the key is used or exported.
 * 
 *  @lib 
 *  @since S60 v3.2
 */
class MKeyEncryptor
    {
    public:

       /**
        * Encrypt the data in the given buffer
        *
        * @param aBuf data to be encrypted
        * @return encrypted data
        */
        virtual HBufC8* EncryptL( TDesC8& aBuf )=0;

       /**
        * Decrypt the data in the given buffer
        *
        * @param aBuf data to be decrypted
        * @return decrypted data
        */
        virtual HBufC8* DecryptL( TDesC8& aBuf )=0;
        
       /**
        * release the encryptor when not needed
        *
        * @param 
        * @return 
        */
        virtual void Release()=0;
    };

#endif //KEYENCRYPTOR_H  

//EOF

