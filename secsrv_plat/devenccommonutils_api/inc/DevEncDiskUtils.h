/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:
*
*/

#ifndef DEVENCDISKUTILS_H
#define DEVENCDISKUTILS_H

#include <f32file.h>

// Constants

// Forward declarations
//enum TDriveNumber; // Defined in f32file.h

class CDevEncDiskUtils : public CBase
    {
    public:
        //IMPORT_C static const TDesC& DriveLetter( TDriveNumber aNumber );
        IMPORT_C static TInt DriveLetter( const TInt aNumber,
                                          TDes& aResult );
        IMPORT_C static void DrivePath( TDes& aDes,
                                        const TDriveNumber aNumber );
        IMPORT_C static void WriteTestFile();
        IMPORT_C static void VerifyTestFile();
        IMPORT_C static void DeleteTestFile();
        IMPORT_C static TInt DiskFinalize( const TDriveNumber aNumber );

    private:
        static void DoWriteTestFileL();
        static void DoVerifyTestFileL();
        static TInt64 FindCriticalLevelTresholdL();
        static TInt FindValueL( const TUid aRepositoryUid, const TUint aCRKey );
        static TInt64 FindWarningLevelTresholdL();
    };

#endif // DEVENCDISKUTILS_H

// End of file
