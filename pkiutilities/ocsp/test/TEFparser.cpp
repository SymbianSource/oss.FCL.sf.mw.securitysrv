//
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
//


#include <s32file.h>
#include "bautils.h" 
#include "TEFparser.h"
#include "utf.h" 

_LIT(KReturn, "\r");
_LIT(KNewline, "\n");
_LIT(KOpenBrk, "[");
_LIT(KCloseBrk, "]");
_LIT(KSpace, " ");
_LIT(KTab, "\t");

_LIT(KIniExtension, ".ini");
_LIT(KScriptFile, ".script");
_LIT(KDot, ".");

#define KIniSectionIdLength 512


EXPORT_C TBool TEFparser::FileType(const TDesC& aBuf)
	{
	
	TInt pos1 = aBuf.Find(KDot);
	
	TPtrC fileType = aBuf.Mid(pos1);
	
	if(fileType == KScriptFile)
		{
		return ETrue;
		}
	else
		{
		return EFalse;
		}
	}

EXPORT_C TInt TEFparser::ReadFileLC(RFs& aFs, 
										const TDesC& aScriptFilepath,
										TPtrC& aScriptFileContents )
	{
	
	RFile scriptFile;
	TInt err = scriptFile.Open(aFs, aScriptFilepath, EFileStream | EFileRead | EFileShareAny);
	if (err != KErrNone)
		{	
		return KErrNotFound;
		}
	// gets size of ini file
	TInt size;
	scriptFile.Size(size);
	scriptFile.Close();
			
	// reads ini into iTestInput
	HBufC* scriptFileH16 = HBufC::NewLC(size);
	HBufC8* scriptFileH = HBufC8::NewLC(size);
	
	//RBuf scriptFileR;
	TPtr8 pInput = scriptFileH->Des(); 
	TPtr pInput16 = scriptFileH16->Des(); 
	
	pInput.SetLength(size); 
	
	RFileReadStream stream;
	User::LeaveIfError(stream.Open(aFs, aScriptFilepath, EFileStream | EFileRead | EFileShareReadersOnly));
	CleanupClosePushL(stream);
	stream.ReadL(pInput,size);
	
//	stream.ReadL(pInput, size);
	
	pInput16.Copy(pInput);
	//scriptFileR.Assign(scriptFileH16);
	
	aScriptFileContents.Set(*scriptFileH16);
	
	CleanupStack::PopAndDestroy(2); // stream, testInput
	
	return KErrNone;
	}

EXPORT_C TPtrC TEFparser::GetRunTestStep(TPtrC& aBuf, 
									const TDesC& aTag,
									TInt& aPos,
									TInt& aError)
	{
	TInt endPos = 0;
	TInt startPos = 0;
	TInt tempPos = 0;
	
	TPtrC data = aBuf.Mid(aPos);
	
	tempPos = data.Find(aTag);
	
	if (tempPos != KErrNotFound)
		{
		tempPos += aTag.Length();
//			
		TPtrC temprunStepData = data.Mid(tempPos);		
//		
		endPos = temprunStepData.Find(KNewline);
		if (endPos == KErrNotFound)
			{
			endPos = temprunStepData.Find(KReturn);
			}
		if (endPos == KErrNotFound)
			{
			endPos = temprunStepData.Length();
			}
//		
		TInt len = 0;
		len = (endPos - startPos) + 1;
		TPtrC runStepData = temprunStepData.Mid(startPos,len);
		aPos += tempPos + runStepData.Length();
		aError = KErrNone;
		return Trim(runStepData);
		}
	else
		{
		aError = KErrNotFound;
		return TPtrC();
		}

	}

EXPORT_C TPtrC TEFparser::Trim(const TDesC& aBuf)
	{
	TInt startPos = 0;
	TInt endPos = 0;
	TInt i = 0, j = 0;
		
	for(i = 0; i < aBuf.Length() ; i ++)
		{
		TPtrC tmpChar = aBuf.Mid(i,1);
		if(tmpChar != KSpace && tmpChar != KTab && tmpChar != KReturn && tmpChar != KNewline )
			{
			startPos = i;
			break;
			}
		}
	for(j = aBuf.Length()-1; j >= 0 ; j --)
		{
		TPtrC tmpChar1 = aBuf.Mid(j,1);
		if(tmpChar1 != KSpace && tmpChar1 != KTab && tmpChar1 != KReturn && tmpChar1 != KNewline )
			{
			endPos = j;
			break;
			}
		}
	if(endPos < startPos)
		{
		endPos = aBuf.Length();
		}
	
//	TInt len = aBuf.Length() - (startPos + endPos);
	return aBuf.Mid(startPos, endPos - startPos + 1);
	
	}

EXPORT_C TInt TEFparser::GetIniFileInfo(TDesC& aBuf, 
											  TPtrC& aIniFileName, 
											  TPtrC& aIniSectionName)
	{
	TInt pos =0;
	TInt startPos = 0, endPos = 0;
	
	
	TPtrC temp = aBuf.Mid(pos);
	
	endPos = temp.Find(KIniExtension);
	endPos += 4;
	
	if (endPos != KErrNotFound)
		{
		TInt len = endPos - startPos;
		TPtrC iniFileName = temp.Mid(startPos, len);
		aIniFileName.Set(iniFileName);
		
		TPtrC iniSectionName = temp.Mid(iniFileName.Length());
		aIniSectionName.Set(Trim(iniSectionName));
		
		return KErrNone;
		}
	else
		{
		return KErrNotFound;
		}
	
	}

EXPORT_C TInt TEFparser::GetiniPath( TDesC& aBuf, 
									const TDesC& aScriptPath,
									TDes& aIniFilePath)
	{
	
	TInt err = KErrNone;	
	TInt endPos = aScriptPath.LocateReverse('\\');
	if (endPos == KErrNotFound)
		{
		err = KErrNotFound;
		}
	else
		{
		aIniFilePath.Copy(aBuf);
		aIniFilePath.Insert(0, aScriptPath.Left(endPos+1));
		}
	return err;
	}

EXPORT_C TInt TEFparser::GetSectionData(TDesC& aScriptFilepath, TPtrC& aSectiontag, TDesC16 &aTocspTestFile, RFs& aFs)
	{
	
	TInt err = KErrNone;	
	TInt pos = 0;
	RFile file;
	
	// open the .ini file
	if (BaflUtils::FolderExists(aFs, aScriptFilepath))
		{		
		if (BaflUtils::FileExists( aFs, aScriptFilepath ))
			{	
			file.Open(aFs, aScriptFilepath, EFileRead | EFileShareAny);
			
			TFileText aLineReader;
			TBuf<256> iLine;
			TBuf<256> tempsectID;

			// create the section name to search for
			tempsectID.Copy(KOpenBrk);
			tempsectID.Append(aSectiontag);
			tempsectID.Append(KCloseBrk);
			
			// read the ini file a line at a time until you find the the section name
			aLineReader.Set(file);		
			TInt foundTag = -1;
			while (err != KErrEof && foundTag != 0)
				{
				err = aLineReader.Read(iLine);
				if (err != KErrEof)
					foundTag =  iLine.Find(tempsectID);
				}
			
			// create the next open bracket to search for		
			TBuf<2> tempopenBrk;
			tempopenBrk.Copy(KOpenBrk);
			
			RFile testfile;	
			err = KErrNone;
			foundTag = -1;

			// while not at the end of the file and not found the next open bracket
			while (err != KErrEof && foundTag != 0)
				{

				// get the next line of the .ini file
				err = aLineReader.Read(iLine);
				if (err != KErrEof)
					{

					// if the line of the file doesn't contain an open bracket, we are still in the section body
					foundTag =  iLine.Find(tempopenBrk);
					if (BaflUtils::FolderExists(aFs, aTocspTestFile) && foundTag != 0)
						{		
						// open the test file we are going to write all our section info into
						if (BaflUtils::FileExists( aFs, aTocspTestFile ))
							{	
							testfile.Open(aFs, aTocspTestFile, EFileWrite|EFileShareAny);
							testfile.Seek(ESeekEnd, pos);
							}
						else
							{	
							User::LeaveIfError(testfile.Create(aFs, aTocspTestFile, EFileWrite|EFileShareAny));
							testfile.Open(aFs, aTocspTestFile, EFileWrite|EFileShareAny);
							}
						// append to line of the file end of line characters
						iLine.Append(_L("\r\n"));

						// write line of the code out to the test file in UNICODE format 
						TPtrC8 tmpPoint((TText8*)iLine.Ptr(),iLine.Size());
						testfile.Write(tmpPoint); 
						
						testfile.Flush();							
						}
					testfile.Close();
					}
				}
			}
		}
		return KErrNone;

	}

EXPORT_C TPtrC TEFparser::ParseNthElement(const TDesC& aBuf, TInt aWordPos)

 {
 
	 TInt startPos = KErrNotFound, endPos = KErrNotFound;
	 TInt wordCounter =0 ;
	 TBool inWord = EFalse;
	 TInt i =0;
	 
	 for(i = 0; i < aBuf.Length() ; i ++)
	  {
	  	TPtrC tmpChar = aBuf.Mid(i,1);
	  	if(tmpChar == KSpace || tmpChar == KTab || tmpChar == KReturn || tmpChar == KNewline )
	  	{
	  	if(inWord)
	  		{
	  		  if(wordCounter == aWordPos)
	  		  {
	    	   endPos =i-1;
	    	   break;
	    	   }
	  		 inWord = EFalse;
	    	}   
	   }
	  else
	   {
	   	if(inWord == EFalse)
	   		{
	   		wordCounter ++;
	   		inWord = ETrue;
	   		if(wordCounter == aWordPos)
	   			{
	   			startPos =i;
	   			}
	   		 }
	   	 }
	  } 
	 if(startPos < 0 || endPos < 0)
	 {
	  endPos = aBuf.Length();
	  return aBuf.Mid(startPos,(endPos-startPos));
	 }
	 else
	 {
	  return aBuf.Mid(startPos,(endPos-startPos+1));
	 }
}


