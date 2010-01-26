/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
#ifndef DEVENCSESSIONBASE_H_
#define DEVENCSESSIONBASE_H_

#include <f32file.h>
#include <e32property.h>

// CONSTANTS

const TUid KDevEncUid2 = { 0x1000008d };
const TUid KDevEncUid3 = { 0x200025B7 };

_LIT( KDevEncCommonUtils, "\\sys\\bin\\devenccommonutils.dll" );

const TInt KMaxAutolockPeriod = 20;

// CLASS DECLARATION
/**
* A session abstract object for communication between the UI and NFE extension. This abstract base class is needed
* in order to allow runtime variation and dynamic linking outside this component.
*/
class CDevEncSessionBase: public CBase
    {
    public:

        IMPORT_C virtual TInt Connect() = 0;
        IMPORT_C virtual void Close() = 0;
        IMPORT_C virtual TInt StartDiskEncrypt() = 0;
        IMPORT_C virtual TInt StartDiskDecrypt() = 0;
        IMPORT_C virtual TInt StartDiskWipe() = 0;
        IMPORT_C virtual TInt DiskStatus( TInt& aStatus ) const = 0;
        IMPORT_C virtual TInt Progress( TInt& aProgress ) const = 0;
        IMPORT_C virtual TBool Connected() const = 0;
        IMPORT_C virtual TDriveNumber DriveNumber() const = 0;
        IMPORT_C virtual void SetDrive( TDriveNumber aNumber ) = 0;
        /**
         * Sets the access control for device encryption
         * @param aValue is of type TDevEncControl
         * @return KErrNone or system wide error
         */
        IMPORT_C virtual TInt SetDevEncControlL(TInt aValue) = 0;
        /**
         * Gets the access control value for device encryption
         * @param aValue is set when the function returns
         * @return KErrNone or system wide error
         */
        IMPORT_C virtual TInt GetDevEncControlL(TInt& aValue) = 0;
    };

#endif /*DEVENCSESSIONBASE_H_*/
