/*
* Copyright (c) 2000, 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   This module contains the implementation of CCrTemplate class. 
*
*/



//  INCLUDE FILES
#include "crdata.h"



// -----------------------------------------------------------------------------
// CCrData
// Constructors
// These functions construct CCrData object.
// -----------------------------------------------------------------------------
CCrData::CCrData()  : iIsFile(ETrue), iPointer(0), iBuffer(NULL)
    { 
    }

CCrData::CCrData(TBool aIsFile) : iIsFile(aIsFile), iPointer(0), iBuffer(NULL), iPath(NULL)
    {
    }

// -----------------------------------------------------------------------------
// CCrData
// Destructor
// This function destructs CCrData object.
// -----------------------------------------------------------------------------
CCrData::~CCrData()
    {
    if (iBuffer != NULL)
        {
        delete iBuffer;
        }
    if (iPath != NULL)
        {
        delete iPath;
        }
    }

// -----------------------------------------------------------------------------
// CCrData::ConstructL
// This function initializes CCrData object's member objects.
// -----------------------------------------------------------------------------
TAny CCrData::ConstructL()
    {
    if (iIsFile == EFalse)
        {
        iBuffer = HBufC8::NewL(0);
        iPath = HBufC::NewL(0);
        }
    }

// -----------------------------------------------------------------------------
// CCrData::NewLC
// -----------------------------------------------------------------------------
EXPORT_C CCrData* CCrData::NewLC(TBool aIsFile) // default value ETrue
    {
    CCrData* self = new CCrData(aIsFile);
    CleanupStack::PushL(self);

    self->ConstructL();

    return self; 
    }

// -----------------------------------------------------------------------------
// CCrData::NewL
// -----------------------------------------------------------------------------
EXPORT_C CCrData* CCrData::NewL(TBool aIsFile) // default value ETrue
    {
    CCrData* self = NewLC(aIsFile);
    CleanupStack::Pop();

    return self; 
    }

// -----------------------------------------------------------------------------
// CCrData::Open
// Opens an existing file for reading or writing. If the file 
// does not already exist, an error is returned.
// Parameters:    aFs The file server session
//                aName The name of the file. Any path 
//                      components which are not specified here will be 
//                      taken from the session path.
//                aFileMode The mode in which the file is opened. 
//                          For more information see EFileShareExclusive and 
//                          other file sharing and access modes.
// Return Values:  TInt KErrNone if successful, otherwise
//                 another of the system-wide error codes.
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrData::Open(RFs& aFs,const TDesC& aName,TUint aFileMode)
    {
    if (iIsFile) // file
        {
        TInt result = iFile.Open(aFs, aName, aFileMode);
        SetPathL(aName);
        return result;
        }
    else // Buffer
        {
        return KErrNotSupported; 
        }
    
    }

EXPORT_C TInt CCrData::Open(RFs& aFs,TUint aFileMode)
    {
    TInt result = KErrNone;
    if (iIsFile) // file
        {
        if (iPath != NULL)
            {
            result = iFile.Open(aFs, *iPath, aFileMode);
            }
        else
            {
            result = KErrPathNotFound;
            }
        }
    else // Buffer
        {
        result = KErrNotSupported; 
        }
    return result;
    }


// -----------------------------------------------------------------------------
// CCrData::Replace
// Replaces a file. If there is an existing file with the same
// name, this function will overwrite it. If the file does not 
// already exist, this function creates it.
// Parameters:    aFs The file server session
//                aName The name of the file. 
//                      Any path components which are not specified 
//                      here will be taken from the session path.
//                 aFileMode The mode in which the file  is opened. 
//                           For more information see EFileShareExclusive and 
//                           other file sharing and access modes. Note that the 
//                           access mode is set to 
//                           EFileWrite whatever the caller specifies.
// Return Values:  KErrNone if successful, otherwise
//                 another of the system-wide error codes.
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrData::Replace(RFs& aFs, const TDesC& aName, TUint aFileMode)
    {
    if (iIsFile) // File
        {
        TInt result = iFile.Replace(aFs, aName, aFileMode);
        return result;
        }
    else // Buffer
        {
        return KErrNotSupported; 
        }  
    }

// -----------------------------------------------------------------------------
// CCrData::Close
// Closes the file. Any open files are closed when the file server 
// session is closed. Close() is guaranteed to return, and provides 
// no indication of whether it completed successfully or not. When 
// closing a file that you have written to, you should ensure that
// data is committed to the file by invoking RFile::Flush() before 
// closing. If Flush() completes successfully, Close() is essentially 
// a no-operation.
// -----------------------------------------------------------------------------
EXPORT_C void CCrData::Close()
    {
    if (iIsFile) // File
        {
        iFile.Close();
        return; 
        }
    else // Buffer
        {
        return; 
        }      
    }

// -----------------------------------------------------------------------------
// CCrData::Read
// Reading transfers data from a file (or a buffer) to a descriptor.
// If CCrData object is created to be a file function only call 
// RFile::Read(.), but if CCrData is buffer, function makes same things
// to buffer as RFile::Read make to file. (See EPOC documentation)
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrData::Read(TDes8& aDes)
    {
    TInt err = KErrNone;

    if (!iIsFile)                            // CCrData is buf 
        {       
        if (aDes.MaxLength() < iBuffer->Length())
            {
            err = KErrOverflow;
            }
        else 
            {
            aDes.Copy(*iBuffer);            // Copy iBuffer to aDes
            iPointer = aDes.Length();       // Set pointer to end of buf
            }           
        }
    else                                    // else it is file
        {  
        err = iFile.Read(aDes); 
        }
    return err;                             // error code
    }


EXPORT_C TInt CCrData::Read(TDes8& aDes,TInt aLength)
    {
    TInt err = KErrNone;
    TInt i;

    if (!iIsFile)                           // CCrData is buffer
        {       
        if ((aLength < NULL) || (aLength > aDes.MaxLength())) 
            {
            err = KErrGeneral;
            }
        else 
            {
            aDes.Zero();
            for (i = iPointer; (i < (aLength + iPointer)) &&
                (i < iBuffer->Length()); i++)
                {
                aDes.Append((*iBuffer)[i]);        // append data to aDes
                }
            iPointer = i;                   // set pointer to next byte
            }
        }
    else                                    // else CCrData is file
        {
        err = iFile.Read(aDes, aLength);
        }

    return err;
    }


EXPORT_C TInt CCrData::Read(TInt aPos,TDes8& aDes,TInt aLength)
    {
    TInt i;
    TInt err = KErrNone;

    if (!iIsFile)                           // CCrDAta is buffer 
        {       
        if (aLength < NULL)                 // aLength must be positive 
            {   
            err = KErrArgument;
            }
        else if (aLength > aDes.MaxLength()) // if buffer is too small 
            {
            err = KErrOverflow;            
            }
        else 
            {
            aDes.Delete(0, aDes.Length());  // delete old string
            TPtr8 ptr = iBuffer->Des();         
        
            for (i = aPos; (i < (aLength + aPos)) &&
                (i < iBuffer->Length()); i++)
                {
                aDes.Append(ptr[i]);
                }
            iPointer  = i;                  // set pointer to end of read       
            }
        }
    else                                    // CCrData is file
        { 
        err = iFile.Read(aPos, aDes, aLength);
        }

    return err;
    }

// -----------------------------------------------------------------------------
// CCrData::Write
// Writing transfers data from a descriptor to a buffer or file.
// If CCrData object is created to be a file function only call 
// RFile::Write(.), but if CCrData is buffer, function makes same things
// to buffer as RFile::Write make to file. After Write(..) iPointer is moved
// to end of the buffer. 
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrData::Write(const TDesC8& aDes)
    { 
    TInt err = KErrNone;
    if (!iIsFile)                           // buf
        {
        err = Write(aDes, aDes.Length()); 
        }

     else                                    // file
        {
        err = iFile.Write(aDes);
        }

    return err;
    }

EXPORT_C TInt CCrData::Write(const TDesC8& aDes, TInt aLength)
    { 
    TInt err = KErrNone;
    if (aLength < NULL)                     // aLen must be positive 
        {                                   
        err = KErrArgument;           
        }
     
    if (!iIsFile)                           // buf
        { 
        TInt bufferLength = iBuffer->Length();
        TInt neededBufferLength = iPointer + aLength;
        if (neededBufferLength > bufferLength) // Reallocation is needed
            {
            TRAP(err, iBuffer = iBuffer->ReAllocL(neededBufferLength)); 
            if (err != KErrNone)
                    {
                    return KErrNoMemory;
                    }
            }
            
        TPtr8 ptr = iBuffer->Des();             
        // append new data, ONLY the amount of aLength
        if (aLength == aDes.Length())
            {
            ptr.Append(aDes); 
            }
        else // aLength < a.Des.Length()
            {
            ptr.Append(aDes.Left(aLength));
            }
        // iPointer is moved to the end of the buffer.        
        iPointer = iBuffer->Length();
        }
    else                                    // file
        {
        err = iFile.Write(aDes);
        }

    return err;
    }

// -----------------------------------------------------------------------------
// CCrData::Zero
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrData::Zero(const TInt aPos, const TInt aLength)
    {
	TPtr8 ptr = iBuffer->Des();
	if (ptr.Length() < aPos + aLength)
	    {
		// Out of bounds
		return KErrArgument;
	    }
	for (TInt i = 0; i < aLength; i++)
	    {
		ptr[aPos+i] = 0;
	    }
	return KErrNone;
    }

// -----------------------------------------------------------------------------
// CCrData::Seek
// If CCrData object is created to be a file function only call 
// RFile::Seek(.), but if CCrData is buffer, function makes same things
// to buffer as RFile::Seek make to file. (See EPOC documentation)
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrData::Seek(TSeek aMode,TInt& aPos)
    {
    TInt err = KErrNone;

    if (aPos < NULL)                        // aPos must be positive
        {
        err = KErrArgument;
        }

    else if (!iIsFile)                      // CCrData is buffer
        {
        switch (aMode)
            {
            case (ESeekCurrent):            // this place
                {
                iPointer += aPos;              
                break;
                }
            case (ESeekStart):              // from start of buf
                {                
                iPointer = aPos;   
                break;
                }
            case (ESeekEnd):                // from end of buf 
                {
                iPointer = iBuffer->Length() - aPos;
                break;
                }
            default:
                {
                break;                      // ESeekAddress
                }
            }
    
            if (iPointer < NULL)            // iPointer must be positive
                {
                iPointer = NULL;            // set pointer to start of buf
                }

            else if (iPointer > iBuffer->Length())
                {
                iPointer = iBuffer->Length();   // set pointer to end of buf
                }
          
            aPos = iPointer;
            
        }

    else                                    // CCrData is file
        {       
        err = iFile.Seek(aMode, aPos);
        }
    
    return err;
    }

// -----------------------------------------------------------------------------
// CCrData::Size
// Reading the size of data.
// If CCrData is a buffer, returns length of the buffer. Otherwise 
// calls RFile::Size( TInt ).
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrData::Size(TInt& aSize)
    {
    TInt err = KErrNone;

    if (!iIsFile)                            // CCrData is buf 
        {       
        aSize = iBuffer->Length();
        }
    else                                    // else it is file
        {  
        err = iFile.Size(aSize); 
        }
    return err;                             // error code
    }


// -----------------------------------------------------------------------------
// CCrData::LineL
// LineL sets data from the current place, where iPointer is, till next new
// line character to aLine. 
// iPointer moves to the following item from the found new line character.
// If no new line character is found, aLine is the rest of the data and 
// iPointer is moved to the end of the iBuffer or the iFile.
// Return Values:  KErrNone if everything succeeds
//                 KErrNoMemory if aLine is not length enough
//                 KErrEof if iPointer is at the end of file.
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrData::LineL(TDes8& aLine)
    {
    TBufC8<2> newLine(KNewLine);  // New line

    // Area to find the new line
    TInt size = 0;
    Size(size);
    TInt currentPlace = iPointer;
    TInt neededLengthToFind = size - currentPlace;

    if(neededLengthToFind == 0) // If the area is empty
        {
        return KErrEof;
        }
    TInt newLinePlace = 0;

    if (!iIsFile) // CCrData is a buffer.
        {
        TPtrC8 temp = iBuffer->Mid(currentPlace, neededLengthToFind);

        // Find next new line.
        newLinePlace = temp.Find(newLine);
        // If not found, return the whole area.
        if (newLinePlace == KErrNotFound)
            {
            newLinePlace = neededLengthToFind;
            }
        else
            {
            newLinePlace = newLinePlace + newLine.Length();
            }

        
        if (aLine.MaxLength() < newLinePlace)
            {
            return KErrNoMemory;
            }

        // Put the line to aLine
        aLine = temp.Left(newLinePlace);
        }

    else // CCrData is a file.
        {

        HBufC8* tempBuf = HBufC8::NewL(neededLengthToFind);
        TDes8 tempDes = tempBuf->Des();

        // Read data to a buffer from iPointer to end of file
        Read(currentPlace, tempDes, neededLengthToFind);

        // Find the next new line character
        newLinePlace = tempDes.Find(newLine);
        // If not found, return the whole area.
        if (newLinePlace == KErrNotFound)
            {
            newLinePlace = neededLengthToFind;
            }
        else
            {
            newLinePlace = newLinePlace + newLine.Length();
            }

        // Is there enough memory
        if (aLine.MaxLength() < newLinePlace)
            {
            delete tempBuf;
            tempBuf = NULL;
            return KErrNoMemory;
            }

        // Put data to aLine
        aLine = tempBuf->Left(newLinePlace);
        
        delete tempBuf;
        tempBuf = NULL;
        }

    // Move iPointer to next place from newLine.
    iPointer = currentPlace + newLinePlace;

    return KErrNone;
            
    }

// -----------------------------------------------------------------------------
// CCrData::Path
// -----------------------------------------------------------------------------
EXPORT_C HBufC* CCrData::Path()
    {
    return iPath;
    }

// -----------------------------------------------------------------------------
// CCrData::SetPathL
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrData::SetPathL(const TDesC& aPath )
    {
    delete iPath;
    iPath = NULL;
    iPath = HBufC::NewL( aPath.Length() );
    TPtr pointer = iPath->Des();
    pointer.Copy( aPath );
    }

// -----------------------------------------------------------------------------
// CCrData::IsFile
// -----------------------------------------------------------------------------
EXPORT_C TBool CCrData::IsFile()
    {
    return iIsFile;
    }

// -----------------------------------------------------------------------------
// CCrData::Delete
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrData::Delete( TInt aPos, TInt aLength )
    {
    if (!iIsFile)                            // CCrData is buf 
        {       
        if(aPos >= 0)
            {
            TPtr8 pointer = iBuffer->Des();
            pointer.Delete(aPos, aLength);
            }
        else
            {
            return KErrArgument;
            }
        }
    else                                    // else it is file
        {  
        return KErrNotSupported;
        }
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CCrDataSet
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// CCrDataSet
// Constructor.
// This function constructs CCrDataSet object.
// -----------------------------------------------------------------------------
CCrDataSet::CCrDataSet(TInt aGranularity)
    : CArrayPtrSeg<CCrData>(aGranularity)
    {
    }

// -----------------------------------------------------------------------------
// CCrDataSet
// Destructor.
// This function destructs CCrDataSet object.
// -----------------------------------------------------------------------------
CCrDataSet::~CCrDataSet()
    {
    ResetAndDestroy();
    }

// -----------------------------------------------------------------------------
// CCrDataSet::ConstructL
// This function initializes CCrDataSet object's member objects.
// -----------------------------------------------------------------------------
TAny CCrDataSet::ConstructL()
    {
    }

// -----------------------------------------------------------------------------
// CCrDataSet::NewLC
// -----------------------------------------------------------------------------
EXPORT_C CCrDataSet* CCrDataSet::NewLC(TInt aGranularity)
    {
    CCrDataSet* self = new CCrDataSet(aGranularity);
    CleanupStack::PushL(self);

    self->ConstructL();

    return self; 
    }

// -----------------------------------------------------------------------------
// CCrDataSet::NewL
// -----------------------------------------------------------------------------
EXPORT_C CCrDataSet* CCrDataSet::NewL(TInt aGranularity)
    {
    CCrDataSet* self = NewLC(aGranularity);
    CleanupStack::Pop();

    return self; 
    }

// End of file

