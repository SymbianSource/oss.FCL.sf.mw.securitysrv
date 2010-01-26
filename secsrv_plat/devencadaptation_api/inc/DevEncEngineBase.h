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
* Description:  Enumerations used in the application UI.
 *
*/


#ifndef __DEVENCENGINEBASE_H__
#define __DEVENCENGINEBASE_H__

//  Include Files
#include <f32file.h>
#include <e32base.h>    // CBase

//  Class Definitions

class CDevEncEngineBase : public CBase
    {
public:

    virtual TInt Connect( TDriveNumber aNumber ) = 0;
    virtual void Close() = 0;
    virtual TInt StartDiskOperation( TInt aOp ) = 0;
    virtual TInt DiskStatus( TInt& aStatus ) const = 0;
    virtual TInt Progress( TInt& aProgress ) const = 0;
    virtual TBool Connected() const = 0;
    virtual TDriveNumber DriveNumber() const = 0;

// For DiskUtils
    virtual void TakeKeyInUseL( const TDesC8& aClearKey ) const = 0;
    virtual TInt RandomDataGet( TDes8& aData, const TInt aLength ) = 0;
    };

#endif  // __DEVENCENGINEBASE_H__

