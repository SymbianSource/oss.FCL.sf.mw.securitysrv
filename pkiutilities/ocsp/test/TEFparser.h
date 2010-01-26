// Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
// Test transport object.  May be used in place of the automatically-constructed
// transport object from OCSP.DLL
// Allows a file to be specified, the contents of which will be returned as the response.
// 
//

/**
 @file 
 @internalTechnology
*/

#ifndef TEFPARSER_H_
#define TEFPARSER_H_



#include <e32base.h>
#include <f32file.h>

class CTOCSPLogger;

class TEFparser
	{
public:
	
//	Checks if the input test script is a .script file
	IMPORT_C static TBool FileType(const TDesC& aBuf);
	
//	Reads the file
	IMPORT_C static TInt ReadFileLC(RFs& aFs, const TDesC& aScriptFilepath, TPtrC& aScriptFileContents);
	
//	Searches and gets the first RUN_TESTSTEP from the .script file
	IMPORT_C static TPtrC GetRunTestStep(TPtrC& aBuf, const TDesC& aTag, TInt& aPos, TInt& aError);
	
//	Gets ini file name and sectionname from the RUN_TESTSTEp
	IMPORT_C static TInt GetIniFileInfo(TDesC& aBuf, TPtrC& aIniFileName, TPtrC& aIniSectionName);
	
//	Generates the ini file path from .script file path taken from command line
	IMPORT_C static TInt GetiniPath(TDesC& aBuf, const TDesC& aScriptPath, TDes& aIniFilePath);

//	Searches the .ini file with section name to fetch the setion data
	IMPORT_C static TInt GetSectionData(TDesC& aScriptFilepath, TPtrC& aSectiontag, TDesC16 &aTocspTestFile, RFs& aFs);
	
//	Trims data of its spaces and newline
	IMPORT_C static TPtrC Trim(const TDesC& aBuf);
	
//	Gets the nth word from a data block
	IMPORT_C static TPtrC ParseNthElement(const TDesC& aBuf, TInt aWordPos);
	
	

private:
	

	};

#endif /*TEFPARSER_H_*/
