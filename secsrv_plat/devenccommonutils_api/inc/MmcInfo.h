/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
*     Holds information of an MMC.
*
*/


#ifndef __TMMCINFO_H__
#define __TMMCINFO_H__

//  INCLUDES
#include <f32file.h>

const TInt KMaxMMCVolumeName = 11; // Limited by FAT

//  CLASS DEFINITION
class TMmcInfo
    {
    public:
        /**
        * @return Name of the MMC.
        */
        const TDesC& Name();

        /**
        * @return Total capacity of the MMC.
        */
        TInt64 Capacity();

        /**
        * @return Amount of free space on MMC.
        */
        TInt64 SpaceFree();

        /**
        * @return MMC ID.
        */
        TUint ID();

        /**
        * @return Boolean indicating if the MMC is formattable.
        */
        TBool IsFormattable();

        /**
        * @return Boolean indicating if the MMC is formatted.
        */
        TBool IsFormatted();

        /**
        * @return Boolean indicating if the MMC is lockable.
        */
        TBool IsLockable();

        /**
        * @return Boolean indicating if the MMC is locked.
        */
        TBool IsLocked();

        /**
        * @return Boolean indicating if the MMC is read only.
        */
        TBool IsReadOnly();

        /**
        * @return Boolean indicating if password is set in the MMC.
        */
        TBool IsPasswordSet();

        /**
        * @return Boolean indicating if MMC is inserted.
        */
        TBool IsInserted();

        /**
        * @return Boolean indicating if MMC is in use
        */
        TBool IsInUse();

        /**
        * @return Boolean indicating if a backup archive exists on the MMC.
        */
        TBool BackupExists();

    //private:
        TBufC< KMaxMMCVolumeName > iName;
        TInt64 iCapacity;
        TInt64 iSpaceFree;
        TUint iUid;
        TBool iIsFormattable;
        TBool iIsFormatted;
        TBool iIsLockable;
        TBool iIsLocked;
        TBool iIsReadOnly;
        TBool iIsPasswordSet;
        TBool iIsInserted;
        TBool iBackupExists;
        TBool iIsInUse;
    };

#endif      //  __TMMCINFO_H__

// End of File
