/*
* Copyright (c) 2000 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  This file contains common defines for all Cradle projects.
*
*/



#ifndef CR_DEFS
#define CR_DEFS


//  INCLUDES

#include <e32std.h>

//  CONSTANTS

typedef TUint8  TCrKeyType;
typedef TUint16 TCrStatus;
typedef TUint8	TCrApduStatus;
typedef TUint8  TCrCryptoStatus;
typedef TUint8  TCrWimStatus;
typedef TInt    TCrLeaveCode;

// Buffer size
const TInt KBufSize     = 1024;

// LDAP Server Address size
const TInt KLDAPBuffer = 128;

// Message digest size
const TInt KDigestSize  = 20;

// RSA signature length
const TInt KRSASignatureSize  = 1024;

// Key length
const TInt KDESKeyLength    = 8;

// Key types.
const TCrKeyType KCrDigitalSignature = 0x80;
const TCrKeyType KCrNonRepudiation   = 0x40;

// Upmost byte is used to define context of error.
const TCrStatus KCrWim          = 0x0100;
const TCrStatus KCrWimLib       = 0x0200;
const TCrStatus KCrWimApdu      = 0x0300;
const TCrStatus KCrKey          = 0x0400;
const TCrStatus KCrCrypto       = 0x0500;
const TCrStatus KCrPkcs7        = 0x0600;
const TCrStatus KCrPkcs12       = 0x0700;
const TCrStatus KCrCertEng      = 0x0800;
const TCrStatus KCrSegMsg       = 0x0900;

// Lowest byte defines actual error.

// Common errors.
const TCrStatus KCrOK           = 0x0000;
const TCrStatus KCrNotSupported = 0x00a0;
const TCrStatus KCrNoMemory     = 0x00b0;

// WIM errors are listed in "wim_const.h"

// CrWim errors
const TCrWimStatus KCrInvalidContext    = 0x01;
const TCrWimStatus KCrInvalidParameters = 0x02;

// CrKeyApi errors


// CrCrypto errors
const TCrCryptoStatus KCrNoError              = 0x00;
const TCrCryptoStatus KCrNotSupportedAlg      = 0x01;
const TCrCryptoStatus KCrUnknownLibrary       = 0x02;
const TCrCryptoStatus KCrErrorGeneral         = 0x03;
const TCrCryptoStatus KCrDataValid            = 0x04;
const TCrCryptoStatus KCrDataInvalid          = 0x05; 
const TCrCryptoStatus KCrUnknownMode          = 0x06;
const TCrCryptoStatus KCrUndefinedLibrary     = 0x07; 
const TCrCryptoStatus KCrWrongAlgType         = 0x08;
const TCrCryptoStatus KCrNotInitialized       = 0x09;           


// CrWimApdu errors

//These errors are defined in WIM library desing spec.
const TCrApduStatus KCrApduOk                 =	0x00; //APDU_OK
const TCrApduStatus KCrApduTransmiossionError =	0x01; //APDU_TRANSMISSION_ERROR
const TCrApduStatus KCrApduCardMute           = 0x02; //APDU_CARD_MUTE		
const TCrApduStatus KCrApduCardDisconnected   = 0x03; //APDU_CARD_DISCONNECTED
const TCrApduStatus KCrApduNoCause            =	0x04; //APDU_NO_CAUSE	
const TCrApduStatus KCrApduReaderNotValid     =	0x05; //APDU_READER_NOT_VALID	
const TCrApduStatus KCrApduFormatError        =	0x06; //APDU_FORMAT_ERROR	
const TCrApduStatus KCrApduTypeNotValid       =	0x07; //APDU_TYPE_NOT_VALID	
const TCrApduStatus KCrApduReaderRemoved      =	0x08; //APDU_CARD_READER_REMOVED
const TCrApduStatus KCrApduCardRemoved        =	0x09; //APDU_CARD_REMOVED
const TCrApduStatus KCrApduCardReaderBusy     =	0x0a; //APDU_CARD_READER_BUSY	
const TCrApduStatus KCrApduCardPoweredOff     =	0x0b; //APDU_CARD_POWERED_OFF
//CrWimApdu own errors, used only in RequestList
const TCrApduStatus KCrApduNoMemory           =	0xa0;
const TCrApduStatus KCrApduScardDatabaseError = 0xa1;

//const TCrStatus KCrNotSupported = 0x00800000;

// Pkcs7 errors
const TCrStatus KCrNotValidObject       = 0x0001;
const TCrStatus KCrUnsupportedFormat    = 0x0002;
const TCrStatus KCrUnsupportedVersion   = 0x0003;
const TCrStatus KCrDigestInvalid        = 0x0004;
const TCrStatus KCrEncContentInvalid    = 0x0005;
const TCrStatus KCrContentInvalid		= 0x0006;
const TCrStatus KCrSigningFailed        = 0x0007;
const TCrStatus KCrInvalidAlgorithm     = 0x0008;
const TCrStatus KCrInvalidCertificate   = 0x0009;

// Pkcs12 errors
const TCrStatus KCrValidFile = 0x0000;
const TCrStatus KCrNotValidFile = 0x0001;
const TCrStatus KCrBerLibraryError = 0x002;
const TCrStatus KCrNotPasswordBasedEncryption = 0x003;
const TCrStatus KCrNotSupportedHMACalgorithm = 0x004;
const TCrStatus KCrWrongPassWordOrCorruptedFile = 0x005;
const TCrStatus KCrNotValidPkcs12Object = 0x006;
const TCrStatus KCrCancelled = 0x007;
const TCrStatus KCrGeneralError = 0x008;

//CrCert own Leave codes
const TCrLeaveCode KCrLocationNotCertMan  = 0x0001;
const TCrLeaveCode KCrLocationInvalid     = 0x0002;
const TCrLeaveCode KCrCertFormatInvalid   = 0x0003;
const TCrLeaveCode KCrCertNotDeletable    = 0x0004;

// certificate engine errors
const TCrStatus KCrCertificateCorrupted             = 0x0001;
const TCrStatus KCrCertificateSaveError             = 0x0003;
const TCrStatus KCrCertificateLoadError             = 0x0004;
const TCrStatus KCrLDAPCertificateServerFoundError  = 0x0005;
const TCrStatus KCrLDAPLoginFailed                  = 0x0006;
const TCrStatus KCrConnectionFailed                 = 0x0007;
const TCrStatus KCrLDAPserverIsDownOrNotFound       = 0x0008;
const TCrStatus KCrInvalidUsernameOrPassword        = 0x0009;

// certificate engine and secure email errors
const TCrStatus KCrNotEnougtMemory                  = 0x0002;

// secure email errors
const TCrStatus KCrCanNotOpenMessage                = 0x0001;
const TCrStatus KCrDeCryptionFailed                 = 0x0003;

// secure email warning code
//const TCrStatus KCrSignatureIsNotValid              = 0x0001;
//const TCrStatus KCrSignatureIsExpired               = 0x0003;
//const TCrStatus KCrSignatureIsNotTrusted            = 0x0004;
//const TCrStatus KCrSignatureIsInvalid               = 0x0005;


// certificate engine's return values
const TInt KCrCancel       = 0x0001;
const TInt KCrZero         = 0x0001;
const TInt KCrTooLong      = 0x0002;
const TInt KCrSearch       = 0x0003;
const TInt KCrSend         = 0x0004;

//  MACROS
//  DATA TYPES

enum TCrEncodeMode
    {
    ECrSendMode,
    ECrDraftMode
    };

enum TCrDecodeMode
    {
    ECrSendingMode,
    ECrReceivingMode
    };

// Certificate types

enum TCrCertValidationType
	{
    ECrValid,
    ECrInvalid,
    ECrRevoked,
    ECrExpired,
    ECrNotTrusted,
	ECrSelfSigned,
    ECrNotFound,
	ECrRemoveUser,
    ECrSignatureInvalid,
	ECrUnknown,
    ECrAddressConflict,
    ECrMultiple,
    ECrSave,
	ECrNotSupported,
    ECrAdded
	};

//Data types CrCert
enum TCrCertLocation
    {
    ECrCertLocationCertMan,
    ECrCertLocationPhoneMemory,
    ECrCertLocationPhoneMemoryURL,
    ECrCertLocationWIMCard,
    ECrCertLocationWIMURL
    };

enum TCrCertType
    {
    ECrCertTypeCA,
    ECrCertTypePersonal
    };

enum TCrCertFormat
    {
    ECrWTLSCertificate,
    ECrX509Certificate,
    ECrX968Certificate,
    ECrCertificateURL
    };

enum TCrEntryType
    {
    ECrEntryTypeAll,
    ECrEntryTypeCA,
    ECrEntryTypePersonal
    };

enum TCrKeyLocation
    {
    ECrKeyLocationWIMCard,
    ECrKeyLocationPhoneMemory,
    ECrKeyLocationNoKey
    };

enum TCrCertStoringStatus
    {
    ECrCertNotStored
    };

enum TCrBer
	{
	ECrNoBer,
	ECrSetBer
	};

//  EXTERNAL DATA STRUCTURES
//  FUNCTION PROTOTYPES
//  FORWARD DECLARATIONS
//  CLASS DEFINITIONS

#endif CR_DEFS

