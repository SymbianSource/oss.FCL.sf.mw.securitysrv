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


#ifndef DEVENC_SESSION_H
#define DEVENC_SESSION_H

#include <f32file.h>
#include <e32property.h>

#include "DevEncSessionBase.h"
class CDevEncEngineBase;
// CONSTANTS



// CLASS DECLARATION
/**
* A session object for communication between the UI and NFE extension.
*/
class CDevEncSession: public CDevEncSessionBase
    {
    public:
        IMPORT_C explicit CDevEncSession();
        IMPORT_C explicit CDevEncSession( TDriveNumber aNumber );
        ~CDevEncSession();

        IMPORT_C TInt Connect();
        IMPORT_C void Close();
        IMPORT_C TInt StartDiskEncrypt();
        IMPORT_C TInt StartDiskDecrypt();
        IMPORT_C TInt StartDiskWipe();
        IMPORT_C TInt DiskStatus( TInt& aStatus ) const;
        IMPORT_C TInt Progress( TInt& aProgress ) const;
        IMPORT_C TBool Connected() const;
        IMPORT_C TDriveNumber DriveNumber() const;
        IMPORT_C void SetDrive( TDriveNumber aNumber );   
        /**
         * Sets the access control for device encryption
         * @param aValue is of type TDevEncControl
         * @return KErrNone or system wide error
         */
        IMPORT_C TInt SetDevEncControlL(TInt aValue);

        /**
         * Gets the access control value for device encryption
         * @param aValue is set when the function returns
         * @return KErrNone or system wide error
         */
        IMPORT_C TInt GetDevEncControlL(TInt& aValue);
        
    private:
        TInt StartDiskOperation( TInt aOp );
        void LoadDevEncEngineL();
        void UnloadDevEncEngine();

    private:
        TBool iConnected;
        TDriveNumber iDriveNumber;
        mutable RProperty iToNfe;
        mutable RProperty iFromNfe;
        mutable RProperty iProgress;
        mutable RProperty iNfeStatusToUi;
        CDevEncEngineBase* iDevEncEngine;
        RLibrary iLibrary;
    };

#endif // DEVENC_SESSION_H

// End of file
