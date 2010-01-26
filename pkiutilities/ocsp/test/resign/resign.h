// Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Tool for generating test response files to be used for testing ocsp module.
// 
//

/**
 @file 
 @internalComponent
*/

#ifndef __RESIGN_H
#define __RESIGN_H

class CCommandLineArguments;

class CResign : public CBase
	{
	
	enum TCommand
		{
		ENone,
		EResign,
		ECreate,
		EExtract,
		};

	public:
		static CResign* NewLC();
		static CResign* NewL();
		virtual ~CResign();
		
		void ProcessCommandLineL();
		
	protected:
		CResign();
	
	private:
		void ConstructL();
		void ResignL( const TPtrC8& aSignedData,	 TPtr8& aSignature,
				 	  RInteger& aModulus, const RInteger& /*aPublicExponent*/,
				 	  RInteger& aPrivateExponent);
		void DecodeDataL(TDes8& aResponse, const TDesC8& aKey);
		void ResignFilesL();
		void HandleCommandL(TCommand aCommand);
		void CreateDatFileL();
		void ExtractResponseL();
		
	private:
		CCommandLineArguments* iArguments;
		RFs iFs;
	};
	
#endif //__RESIGN_H
