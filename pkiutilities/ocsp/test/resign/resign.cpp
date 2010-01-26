// Copyright (c) 2001-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// App for resigning OCSP responses.
// 
//

#include <e32cons.h>
#include <f32file.h>
#include <asn1dec.h>
#include <asn1enc.h>
#include <x509cert.h>
#include <signed.h>
#include <bigint.h>
#include <asymmetric.h>
#include <hash.h>
#include <bacline.h>

#include "resign.h"

// CertTool command line parameters
_LIT(KResign, "-resign");
_LIT(KResignShort, "-r");

_LIT(KCreate, "-create");
_LIT(KCreateShort, "-c");

_LIT(KExtractResponse, "-e");
_LIT(KExtractResponseShort, "-extract");

CResign* CResign::NewLC()
	{
	CResign* resign = new(ELeave) CResign();
	CleanupStack::PushL(resign);
	resign->ConstructL();
	return resign;
	}
	
CResign* CResign::NewL()
	{
	CResign* resign = CResign::NewLC();
	CleanupStack::Pop(resign);
	return resign;
	}
		
CResign::CResign()
	{}
	
void CResign::ConstructL()
	{
	User::LeaveIfError(iFs.Connect());
	}

CResign::~CResign()
	{
	delete iArguments;
	iFs.Close();
	}

void CResign::ResignL(const TPtrC8& aSignedData,
			 TPtr8& aSignature,
			 RInteger& aModulus,
			 const RInteger& /*aPublicExponent*/,
			 RInteger& aPrivateExponent)
	{
	__UHEAP_MARK;
	// Build message digest
	CSHA1* hash = CSHA1::NewL();
	CleanupStack::PushL(hash);
	TPtrC8 digest = hash->Final(aSignedData);

	// Build ASN1 encoding of digestAlgId and digest..
	CASN1EncSequence* encAll = CASN1EncSequence::NewLC();

	// Build AlgID encoder (for SHA1)
	CASN1EncSequence* encAlgId = CASN1EncSequence::NewLC();

	CASN1EncObjectIdentifier* encObjId = CASN1EncObjectIdentifier::NewLC(KSHA1);
	encAlgId->AddChildL(encObjId);
	CleanupStack::Pop(); // encObjId, now owned by endAlgId

	CASN1EncNull* encNull = CASN1EncNull::NewLC();
	encAlgId->AddChildL(encNull);
	CleanupStack::Pop(); // encNull, now owned by endAlgId

	encAll->AddChildL(encAlgId);
	CleanupStack::Pop(); // endAlgId, now owned by encAll

	CASN1EncOctetString* encDigest = CASN1EncOctetString::NewLC(digest);
	encAll->AddChildL(encDigest);
	CleanupStack::Pop(); // encDigest, now owned by encAll

	HBufC8* digestInfo = HBufC8::NewMaxLC(encAll->LengthDER());
	TUint pos = 0;
	TPtr8 digestInfoPtr = digestInfo->Des();
	encAll->WriteDERL(digestInfoPtr, pos);

	__UHEAP_MARK;

	CRSAPrivateKeyStandard* rsaPriv = CRSAPrivateKeyStandard::NewLC(aModulus, aPrivateExponent);

	CRSAPKCS1v15Signer* signer = CRSAPKCS1v15Signer::NewLC(*rsaPriv);
	
	const CRSASignature* signature = signer->SignL(digestInfoPtr);
	
	HBufC8* theSignature = signature->S().BufferLC();
	aSignature.Copy(*theSignature);
	
	CleanupStack::PopAndDestroy(4, rsaPriv);

	__UHEAP_MARKEND;

	CleanupStack::PopAndDestroy(3, hash);
	
	__UHEAP_MARKEND;
	}

void CResign::DecodeDataL(TDes8& aResponse, const TDesC8& aKey)
	{
	__UHEAP_MARK;
	TASN1DecSequence seqDec;
	TInt pos = 0;

	// Get the signed data and signature portions of the response
	CArrayPtr<TASN1DecGeneric>* ocspResponseSeq = seqDec.DecodeDERLC(aResponse, pos, 2, 2);
	TPtrC8 responseBytes = ocspResponseSeq->At(1)->GetContentDER();
	CleanupStack::PopAndDestroy(ocspResponseSeq);

	pos = 0;
	CArrayPtr<TASN1DecGeneric>* responseBytesSeq = seqDec.DecodeDERLC(responseBytes, pos, 2, 2);
	TPtrC8 basicOCSPResponse = responseBytesSeq->At(1)->GetContentDER();
	CleanupStack::PopAndDestroy(responseBytesSeq);

	pos = 0;
	CArrayPtr<TASN1DecGeneric>* basicOCSPResponseSeq = seqDec.DecodeDERLC(basicOCSPResponse, pos, 3, 4);
	TPtrC8 responseData = basicOCSPResponseSeq->At(0)->Encoding();
	TPtrC8 signatureAlgorithm = basicOCSPResponseSeq->At(1)->Encoding();
	TPtrC8 signature = basicOCSPResponseSeq->At(2)->GetContentDER();
	CleanupStack::PopAndDestroy(basicOCSPResponseSeq);

	// Check that it's RSA with SHA1
	CX509SigningAlgorithmIdentifier* algId = CX509SigningAlgorithmIdentifier::NewLC(signatureAlgorithm);
	if (algId->DigestAlgorithm().Algorithm() != ESHA1
		|| algId->AsymmetricAlgorithm().Algorithm() != ERSA)
		{
		User::Leave(KErrNotSupported);
		}
	CleanupStack::PopAndDestroy(algId);

	// Get the modulus and private key portions of the key - only RSA supported

	// ASN1 .key files have 9 parts: version, modulus, public exponent, private exponent,
	// prime 1, prime2, exponent 1, exponent 2, coefficient
	// This is defined in PKCS1 (RFC 2313), the RSAPrivateKey ASN1 data structure
	pos = 0;
	CArrayPtr<TASN1DecGeneric>* keySeq = seqDec.DecodeDERLC(aKey, pos, 9, 9);
	TASN1DecInteger decInt;

	RInteger modulus = decInt.DecodeDERLongL(*keySeq->At(1));
	CleanupStack::PushL(modulus);

	RInteger publicExponent = decInt.DecodeDERLongL(*keySeq->At(2));
	CleanupStack::PushL(publicExponent);

	RInteger privateExponent = decInt.DecodeDERLongL(*keySeq->At(3));
	CleanupStack::PushL(privateExponent);

	// Cast constness off the signature - and skip the 'number of unused bits' octet
	TPtr8 sigEdit(CONST_CAST(TUint8*, signature.Ptr() + 1), signature.Length() - 1, signature.Length() - 1);

	// Pass data on for re-signing
	ResignL(responseData, sigEdit, modulus, publicExponent, privateExponent);

	CleanupStack::PopAndDestroy(3, &modulus); // The RIntegers
	CleanupStack::PopAndDestroy(keySeq);
	__UHEAP_MARKEND;
	}

void CResign::ProcessCommandLineL()
    {
	iArguments = CCommandLineArguments::NewL();
	
	if(iArguments->Count() > 1)
		{	
		TCommand command = ENone; 
		
		TPtrC args = iArguments->Arg(1);
		if ((args.Compare(KResign)==0) || (args.Compare(KResignShort)==0))
			{
			command = EResign;
			}	
		if ((args.Compare(KCreate)==0) || (args.Compare(KCreateShort)==0))
			{
			command = ECreate;
			}
		if ((args.Compare(KExtractResponse)==0) || (args.Compare(KExtractResponseShort)==0))
			{
			command = EExtract;
			}
		
		if(command != ENone)
			{
			HandleCommandL(command);
			}
		}
    }

void CResign::HandleCommandL(TCommand aCommand)
	{
	switch(aCommand)
		{
		case EResign:
			ResignFilesL();
			break;
			
		case ECreate:
			CreateDatFileL();
			break;
		
		case EExtract:
			ExtractResponseL();
			break;
		}
	}

void CResign::ResignFilesL()
	{
	__UHEAP_MARK;
	
	TPtrC responseFile(iArguments->Arg(2));
	TPtrC keyFile(iArguments->Arg(3));
	
	// Load the key
	RFile file;
	User::LeaveIfError(file.Open(iFs, keyFile, EFileRead | EFileShareAny));
	CleanupClosePushL(file);
	
	TInt keySize;
	User::LeaveIfError(file.Size(keySize));
	HBufC8* keyBuf = HBufC8::NewLC(keySize);
	TPtr8 key = keyBuf->Des();

	User::LeaveIfError(file.Read(key));
	
	// Open the logged response file

	RFileReadStream readStream;
	User::LeaveIfError(readStream.Open(iFs, responseFile, EFileRead | EFileShareAny));
	CleanupClosePushL(readStream);
	MStreamBuf* readBuf = readStream.Source();

	RFileWriteStream writeStream;
	User::LeaveIfError(writeStream.Open(iFs, responseFile, EFileWrite | EFileShareAny));
	CleanupClosePushL(writeStream);
	MStreamBuf* writeBuf = writeStream.Sink();

	TInt totalResponses = readStream.ReadUint32L();

	for (TInt i = 0 ; i < totalResponses ; ++i)
		{
		TInt responseSize = readStream.ReadUint32L();
		TStreamPos pos = readBuf->TellL(MStreamBuf::ERead);

		HBufC8* responseBuf = HBufC8::NewLC(responseSize);

		TPtr8 response = responseBuf->Des();
		readStream.ReadL(response);

		// resigning response
		DecodeDataL(response, key);

		writeBuf->SeekL(MStreamBuf::EWrite, pos);
		writeStream.WriteL(response);

		CleanupStack::PopAndDestroy(responseBuf);
		}
	
	CleanupStack::PopAndDestroy(4,&file); // close writeStream, readStream, keyBuf and file
	
	__UHEAP_MARKEND;
	}

void CResign::CreateDatFileL()
	{
	__UHEAP_MARK;
	
	
	// this path will fail if the directories are not already present.
	_LIT(KDatFile,"c:\\system\\tocsp\\responses\\response.dat");
	RFileWriteStream writeStream;
	User::LeaveIfError(writeStream.Replace(iFs, KDatFile, EFileWrite | EFileShareAny));
	CleanupClosePushL(writeStream);
	
	TInt fileCount = iArguments->Count(); 
	
	// total no of responses in this dat file
	writeStream.WriteInt32L(fileCount-2);
	RFile file;
	for(TInt i=2;i<fileCount;i++)
		{
		TPtrC responseFile(iArguments->Arg(i));
		// Load the response file
		User::LeaveIfError(file.Open(iFs, responseFile, EFileRead | EFileShareAny));
		CleanupClosePushL(file);
		
		TInt respSize = 0;
		User::LeaveIfError(file.Size(respSize));
		HBufC8* respBuf = HBufC8::NewLC(respSize);
		TPtr8 resp = respBuf->Des();
		User::LeaveIfError(file.Read(resp));
		
		writeStream.WriteInt32L(respSize);
		writeStream.WriteL(resp);
		
		CleanupStack::PopAndDestroy(2,&file); // respBuf and file
		}
	
	CleanupStack::PopAndDestroy(&writeStream);
	
	__UHEAP_MARKEND;
	}

void CResign::ExtractResponseL()
	{
	__UHEAP_MARK;
	
	TPtrC responseFile(iArguments->Arg(2));
	
	// Load the response file
	RFileReadStream respFile;
	User::LeaveIfError(respFile.Open(iFs, responseFile, EFileRead | EFileShareAny));
	CleanupClosePushL(respFile);
	
	// reading the transaction value.
	respFile.ReadInt32L();
	TInt respSize = respFile.ReadInt32L();
	
	HBufC8* respBuf = HBufC8::NewLC(respSize);
	TPtr8 resp = respBuf->Des();

	respFile.ReadL(resp, respSize);
	
	// this path will fail if the directories are not already present.
	_LIT(KDerFile,"c:\\system\\tocsp\\responses\\response.der");
	RFileWriteStream writeStream;
	User::LeaveIfError(writeStream.Replace(iFs, KDerFile, EFileWrite | EFileShareAny));
	CleanupClosePushL(writeStream);
	writeStream.WriteL(resp);
	
	CleanupStack::PopAndDestroy(3,&respFile); // writeStream, respBuf and respFile
	
	__UHEAP_MARKEND;

	}
