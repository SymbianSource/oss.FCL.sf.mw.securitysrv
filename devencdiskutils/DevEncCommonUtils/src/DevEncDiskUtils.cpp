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


#include <e32base.h>
#include <f32file.h> // for TDriveNumber
#include <centralrepository.h>
#include <e32property.h>

#include <UiklafInternalCRKeys.h> // for disc space

#include "DevEncCommonUtils.hrh"
#include "DevEncDiskUtils.h"
#include "DevEncLog.h"

/*static const TPtrC KDriveLetter[] = {
    _L("A"), _L("B"), _L("C"), _L("D"), _L("E"), _L("F"), _L("G"), _L("H"),
    _L("I"), _L("J"), _L("K"), _L("L"), _L("M"), _L("N"), _L("O"), _L("P"),
    _L("Q"), _L("R"), _L("S"), _L("T"), _L("U"), _L("V"), _L("W"), _L("X"),
    _L("Y"), _L("Z") };*/

_LIT( KA, "A"); _LIT( KB, "B");_LIT( KC, "C"); _LIT( KD, "D");
_LIT( KE, "E"); _LIT( KF, "F"); _LIT( KG, "G"); _LIT( KH, "H");
_LIT( KI, "I"); _LIT( KJ, "J");_LIT( KK, "K"); _LIT( KL, "L");
_LIT( KM, "M"); _LIT( KN, "N"); _LIT( KO, "O"); _LIT( KP, "P");
_LIT( KQ, "Q"); _LIT( KR, "R"); _LIT( KS, "S"); _LIT( KT, "T");
_LIT( KU, "U"); _LIT( KV, "V"); _LIT( KW, "W"); _LIT( KX, "X");
_LIT( KY, "Y"); _LIT( KZ, "Z");

_LIT( KContentTestFile, "c:\\content_test.txt" );
static const TUint32 KTestBlockSizeBt = 1024 * sizeof( TUint32 );
static const TUint32 KTestBlocks      = 320;
static const TInt32  KMaxInt          = 2147483647;       

// --------------------------------------------------------------------------
// CDevEncDiskUtils::DriveLetter()
// 
// --------------------------------------------------------------------------
EXPORT_C TInt CDevEncDiskUtils::DriveLetter( TInt aNumber,
                                             TDes& aResult )
    {
    const TInt KNumOfDriveLetters( 25 );
    const TPtrC KDriveLetter[] = {
        KA(), KB(), KC(), KD(), KE(), KF(), KG(), KH(),
        KI(), KJ(), KK(), KL(), KM(), KN(), KO(), KP(),
        KQ(), KR(), KS(), KT(), KU(), KV(), KW(), KX(),
        KY(), KZ() };
    if ( aNumber > KNumOfDriveLetters )
        {
        return KErrOverflow;
        }
    aResult.Append( KDriveLetter[ aNumber ] );
    return KErrNone;
    }

// --------------------------------------------------------------------------
// CDevEncDiskUtils::DrivePath()
// 
// --------------------------------------------------------------------------
EXPORT_C void CDevEncDiskUtils::DrivePath( TDes& aDes, TDriveNumber aNumber )
    {
    DriveLetter( aNumber, aDes );
    aDes.Append( _L(":\\") );
    }

// --------------------------------------------------------------------------
// CDevEncDiskUtils::DoWriteTestFileL()
// 
// --------------------------------------------------------------------------
void CDevEncDiskUtils::DoWriteTestFileL()
    {
    RFile file;
    RFs fs;

    User::LeaveIfError( fs.Connect() );
    CleanupClosePushL( fs );
    User::LeaveIfError( file.Replace( fs, KContentTestFile, EFileWrite ) );
    CleanupClosePushL( file );

    HBufC8* buf = HBufC8::NewL( KTestBlockSizeBt );
    CleanupStack::PushL( buf );

    TPtr8 pbuf = buf->Des();
    pbuf.SetLength( KTestBlockSizeBt );

    for( TUint32 i=0; i < KTestBlocks; i++ )
        {
        for( TUint32 k=0; k < KTestBlockSizeBt; k++ )
            pbuf[k] = 0xff;

        User::LeaveIfError( file.Write( pbuf ) );
        }

    CleanupStack::PopAndDestroy(3);
    }

// --------------------------------------------------------------------------
// CDevEncDiskUtils::WriteTestFile()
// 
// --------------------------------------------------------------------------
EXPORT_C void CDevEncDiskUtils::WriteTestFile()
    {
    if ( ! RProcess().HasCapability( ECapabilityDiskAdmin ) )
        {
        DFLOG( "Process does not have DiskAdmin capability" );
        return;
        }

    DFLOG("Writing test file");
    TRAPD( err, DoWriteTestFileL() );
    if( err )
        {
        DFLOG2("...failed %d", err);
        }
    else
        {
        DFLOG( "...ok" );
        }
    }

// --------------------------------------------------------------------------
// CDevEncDiskUtils::DoVerifyTestFileL()
// 
// --------------------------------------------------------------------------
void CDevEncDiskUtils::DoVerifyTestFileL()
    {
    if ( ! RProcess().HasCapability( ECapabilityDiskAdmin ) )
        {
        DFLOG( "Process does not have DiskAdmin capability" );
        return;
        }

    RFile file;
    RFs fs;

    User::LeaveIfError( fs.Connect() );
    CleanupClosePushL( fs );
    User::LeaveIfError( file.Open( fs, KContentTestFile, EFileRead ) );
    CleanupClosePushL( file );

    HBufC8* buf = HBufC8::NewL(KTestBlockSizeBt);
    CleanupStack::PushL( buf );

    TPtr8 pbuf = buf->Des();
    pbuf.SetLength( KTestBlockSizeBt );

    for( TUint32 i=0; i < KTestBlocks; i++ )
        {
        User::LeaveIfError( file.Read( pbuf ) );

        for( TUint32 k=0; k < KTestBlockSizeBt; k++ )
            {
            if( pbuf[k] != 0xff )
                DFLOG3("pos=%d val=%d", (i * KTestBlockSizeBt + k), pbuf[k]);
            }
        }

    CleanupStack::PopAndDestroy(3);
    }

// --------------------------------------------------------------------------
// CDevEncDiskUtils::VerifyTestFile()
// 
// --------------------------------------------------------------------------
EXPORT_C void CDevEncDiskUtils::VerifyTestFile()
    {
    DFLOG("Verifying test file");
    TRAPD( err, DoVerifyTestFileL() );
    if( err )
        {
        DFLOG2("...failed %d", err);
        }
    else
        {
        DFLOG( "..ok" );
        }
    }

// --------------------------------------------------------------------------
// CDevEncDiskUtils::DeleteTestFile()
// 
// --------------------------------------------------------------------------
EXPORT_C void CDevEncDiskUtils::DeleteTestFile()
    {
    DFLOG("Deleting test file");
    RFs fs;

    TInt err = fs.Connect();

    if( !err )
        {
        err = fs.Delete( KContentTestFile );

        if( !err || err == KErrNotFound )
            {
            DFLOG("...ok");
            }
        else
            {
            DFLOG2( "...failed %d", err );
            }
        }
    else
        {
        DFLOG2( "fs.Connect() failed %d", err );
        }
    }

// --------------------------------------------------------------------------
// CDevEncDiskUtils::DiskFinalize()
// 
// --------------------------------------------------------------------------
EXPORT_C TInt CDevEncDiskUtils::DiskFinalize( TDriveNumber aNumber )
    {
    if ( ! RProcess().HasCapability( ECapabilityDiskAdmin ) )
        {
        DFLOG( "Process does not have DiskAdmin capability" );
        return KErrAccessDenied;
        }
    
    DFLOG("Finalizing");

    RArray <TFileName> temps;
    RArray <RFile> files;
    RFs fs;

    TInt err = fs.Connect();
    if( err )
        DFLOG2( "..fs.Connect fail %d", err);

    TBool freeSpace = ETrue;

    TVolumeInfo volumeInfo;
    err = fs.Volume( volumeInfo, aNumber );
    if( err )
        DFLOG2( "..fs.Volume fail %d", err );

    TInt64 size = 0;
    TInt64 memoryCriticalSpace(0);
    TRAP(err, memoryCriticalSpace = FindCriticalLevelTresholdL());
    if (!err)
        return err;
    TInt64 memoryWarningSpace(0);
    TRAP(err, memoryWarningSpace = FindWarningLevelTresholdL( aNumber ));
    if (!err)
        return err;
    
    DFLOG2( "CDevEncDiskUtils::DiskFinalize => memoryCriticalSpace = %d", (TInt) memoryCriticalSpace );
    DFLOG2( "CDevEncDiskUtils::DiskFinalize => memoryWarningSpace = %d", (TInt) memoryWarningSpace );
    
    //create the number of files needed to fill the mmc or phone memory free space
    while( freeSpace )
        {
        TInt64 newMemoryWarningSpace(0);
        if( aNumber == EDriveC )
        	{
            // The real warning space is a percentage of the free space. This applies only on drive C. 
            newMemoryWarningSpace = ( TInt64 ) ( volumeInfo.iSize*( 100 - memoryWarningSpace ) ) / 100;
        	}
        
        DFLOG2( "CDevEncDiskUtils::DiskFinalize => newMemoryWarningSpace = %d", (TInt) newMemoryWarningSpace );

        if( TInt64( volumeInfo.iFree ) > TInt64( KMaxInt ) )
        	{
            size = KMaxInt;
            }
        else
        	{
            if( aNumber == EDriveC )
                {
        	    size = Max( 0LL, TInt64( volumeInfo.iFree ) - memoryCriticalSpace - newMemoryWarningSpace );
                }
            else
                {
                size = Max( 0LL, TInt64( volumeInfo.iFree ) - memoryCriticalSpace - memoryWarningSpace );
                }
        	}

        TFileName temp;
        RFile file;

        if( !err )
            {
            TBuf<8> path;
            DrivePath( path, aNumber );

            err = file.Temp( fs, path, temp, EFileWrite );
            if( err )
                DFLOG2( "..file.Temp fail %d", err );
            }

        err = file.SetSize( ( TInt ) size );
        if( err )
            DFLOG2( "..file.SetSize fail %d", err );

        files.Append( file );
        temps.Append( temp );

        err = fs.Volume( volumeInfo, aNumber );
        if( err )
            DFLOG2( "..fs.Volume fail %d", err );    

        if( aNumber == EDriveC )
        	{
            if( TInt64( volumeInfo.iFree ) <= memoryCriticalSpace + newMemoryWarningSpace )
        	    {
                freeSpace = EFalse;
                }
            }
        else
            {
            if( TInt64( volumeInfo.iFree ) <= memoryCriticalSpace + memoryWarningSpace )
                {
                freeSpace = EFalse;
                }
            }
        }

    //delete the created files
    for( TInt i=0;i < files.Count();i++ )
    	{
        files[i].Close();
        fs.Delete( temps[i] ); // Error value ignored
        }

    files.Close();
    temps.Close();
    fs.Close();
    return err;
    }

// --------------------------------------------------------------------------
// CDevEncDiskUtils::FindValueL()
// 
// --------------------------------------------------------------------------
TInt CDevEncDiskUtils::FindValueL( const TUid aRepositoryUid, const TUint aCRKey )
	{
	TInt val( 0 );
    CRepository* repository = CRepository::NewLC( aRepositoryUid );
    User::LeaveIfError( repository->Get( aCRKey, val ) );
    CleanupStack::PopAndDestroy( repository );

    return val;
	}

// --------------------------------------------------------------------------
// CDevEncDiskUtils::FindCriticalLevelTresholdL()
// 
// --------------------------------------------------------------------------
TInt64 CDevEncDiskUtils::FindCriticalLevelTresholdL()
	{
    return (TInt64) FindValueL( KCRUidUiklaf, KUikOODDiskCriticalThreshold );
	}

// --------------------------------------------------------------------------
// CDevEncDiskUtils::FindWarningLevelTresholdL()
// 
// --------------------------------------------------------------------------
TInt64 CDevEncDiskUtils::FindWarningLevelTresholdL( const TDriveNumber aNumber )
	{
      if( aNumber == EDriveC )
          {
	      return (TInt64) FindValueL( KCRUidUiklaf, KUikOODDiskFreeSpaceWarningNoteLevel );
          }
      else
          {
          return (TInt64) FindValueL( KCRUidUiklaf, KUikOODDiskFreeSpaceWarningNoteLevelMassMemory );
          }
	}

// End of file
