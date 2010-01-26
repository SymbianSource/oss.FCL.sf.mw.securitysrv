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
* Description:   This file contains the header of CCrData class.
*
*/



#ifndef CR_DATA
#define CR_DATA


//  INCLUDES

#include <e32base.h>    // CArrayFixFlat
#include <f32file.h>    // RFile

#include "crdefs.h"     // Defines

//  CONSTANTS         

 _LIT8(KNewLine,"\r\n"); // New line for LineL method 

// CLASS DECLARATION

/**
*  CCrData
*  CCrData class handles file and stream I/O.
*
*  @lib crber.lib
*  @since Series 60 3.0
*/
NONSHARABLE_CLASS( CCrData ): public CBase
    {
    /////////////////////////////////////////////
    // Constructors and destructors.
    public:
        IMPORT_C CCrData();
        // aIsFile set CCrData object either file (True) or buffer (False)
        IMPORT_C CCrData(TBool aIsFile);            // EFalse for a buffer
        IMPORT_C ~CCrData();
        
        TAny ConstructL();

        IMPORT_C static CCrData* NewLC(TBool aIsFile = ETrue); 
        IMPORT_C static CCrData* NewL(TBool aIsfile = ETrue);
        
    public: // Functions.

        // Opens an existing file for reading or writing. If the file 
        // does not already exist, an error is returned.
        IMPORT_C TInt Open(RFs& aFs,const TDesC& aName,TUint aFileMode);
        IMPORT_C TInt Open(RFs& aFs,TUint aFileMode);

        // Replaces a file. If there is an existing file with the same
        // name, this function will overwrite it. If the file does not 
        // already exist, this function creates it.
        IMPORT_C TInt Replace(RFs& aFs,const TDesC& aName,TUint aFileMode);

        // Closes the file. Any open files are closed when the file server 
        // session is closed. Close() is guaranteed to return, and provides 
        // no indication of whether it completed successfully or not. When 
        // closing a file that you have written to, you should ensure that
        // data is committed to the file by invoking RFile::Flush() before 
        // closing. If Flush() completes successfully, Close() is essentially 
        // a no-operation.
        IMPORT_C void Close();


        // Reading transfers data from a file (or a buffer) to a descriptor.
        // Íf CCrData object is created to be a file function only call 
        // RFile::Read(.), but if CCrData is buffer, function makes same
        // things to buffer as RFile::Read make to file.
        // See EPOC documentation to get more information.
        IMPORT_C TInt Read(TDes8& aDes);
        IMPORT_C TInt Read(TDes8& aDes,TInt aLength);
        IMPORT_C TInt Read(TInt aPos,TDes8& aDes,TInt aLength);

        // Writing transfers data from a descriptor to a file or to a buffer.
        // If CCrData object is created to be a file function only call 
        // RFile::Write(.), but if CCrData is buffer, function makes same
        // things to buffer as RFile::Write make to file.
        // aLength must be more than zero or error is returned.
        // See EPOC documentation to get more information.
        IMPORT_C TInt Write(const TDesC8& aDes);
        IMPORT_C TInt Write(const TDesC8& aDes,TInt aLength);

        // Use this function to change the current buffer (or file) position.
        // It can also be used to retrieve the current buffer (or file)
        // position without changing it (ESeekCurrent, 0).
        // The buffer (or file) position is the position at which 
        // buffer (file) reads and writes take place.
        // The start of the buffer(file) is position zero.
        // If CCrData object is created to be a file function only call 
        // RFile::Seek(.), but if CCrData is buffer, function makes same
        // things to buffer as RFile::Seek make to file.
        // aPos must be positive or error is returned. 
        // See EPOC documentation to get more information (about Seek).
        IMPORT_C TInt Seek(TSeek aMode,TInt& aPos);

        IMPORT_C TInt Size(TInt& aSize);

        // LineL sets data from the current place, where iPointer is, till
        // next new line character to aLine. 
        // iPointer moves to the following item from the found new line 
        // character. If no new line character is found, aLine is the rest
        // of the data and iPointer is moved to the end of the iBuffer or 
        // the iFile.

        IMPORT_C TInt LineL(TDes8& aLine);
        IMPORT_C HBufC* Path();
        IMPORT_C TAny SetPathL(const TDesC& aPath);
        IMPORT_C TBool IsFile();
        IMPORT_C TInt Delete(TInt aPos,TInt aLength);

		IMPORT_C TInt Zero(const TInt aPos, const TInt aLength);


    private: // Data
        // Variable set CCrData object either file(1) or buffer(0)
        TBool iIsFile;
        // Index to iBuffer
        TInt  iPointer;
        // Buffer for data
        HBufC8* iBuffer;
        // File for data
        RFile iFile;
        // path of file
        HBufC* iPath;
    };

// CLASS DECLARATION

/**
*  CCrDataSet
*  CCrDataSet class is used to handle multiple CCrData objects.
*
*  @lib crber.lib
*  @since Series 60 3.0
*/
class CCrDataSet : public CArrayPtrSeg<CCrData>
    {
    /////////////////////////////////////////////
    // Constructors and destructors.
    private:
        CCrDataSet(TInt aGranularity);
        
        TAny ConstructL();
        
    public:
        IMPORT_C static CCrDataSet* NewLC(TInt aGranularity); 
        IMPORT_C static CCrDataSet* NewL(TInt aGranularity);

        ~CCrDataSet();

    };

#endif CR_DATA
