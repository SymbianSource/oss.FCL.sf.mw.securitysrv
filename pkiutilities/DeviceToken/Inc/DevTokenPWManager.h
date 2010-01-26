/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   The header file of DevTokenPWManager
*
*/



#ifndef __DEVTOKENPWMANAGER_H__
#define __DEVTOKENPWMANAGER_H__

#include <pbe.h>
#include <e32std.h>
#include <s32crypt.h>
#include <e32base.h>
#include <secdlg.h>


/**
 *  Password management class. Ask password from 
 *  user when encrypted key is imported to devcert
 *  key store.
 *
 *  @lib 
 *  @since S60 v3.2
 */
class TDevTokenPWManager
    {
    public:
	      static void ImportPassword(TPINValue& aValue, TRequestStatus& aStatus);
	      static void Cancel();
    private:
	      static void GetPassword( TPINValue& aValue, TRequestStatus& aStatus, TBool aRetry = EFalse);
	  };

#endif	//__DEVTOKENPWMANAGER_H__
//EOF

