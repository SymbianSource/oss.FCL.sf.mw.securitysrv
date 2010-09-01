 /*
* ============================================================================
* Name        : SimLockUI.pan
* Version     : 
* 
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Build info file for Ado domain appinstall 
* ============================================================================
*/

#ifndef __SIMLOCKUI_PAN__
#define __SIMLOCKUI_PAN__

/** SimLockUI application panic codes */
enum TSimLockUIPanics 
    {
    ESimLockUIBasicUi = 1,
    ESimLockUIUnableToReadSimLock
    // add further panics here
    };

inline void Panic(TSimLockUIPanics aReason)
    {    
	_LIT(applicationName,"SimLockUI"); //lint !e1534 static variable OK
    User::Panic(applicationName, aReason);
    }

#endif // __SIMLOCKUI_PAN__
