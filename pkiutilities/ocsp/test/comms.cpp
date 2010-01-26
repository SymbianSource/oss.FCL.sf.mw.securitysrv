// Copyright (c) 2001-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
// Contains utilities such as a global comms init function for starting c32
// and loading device drivers for test code
// 
//

#include "comms.h"

#include <e32base.h>
#include <c32comm.h>

#if defined (__WINS__)
#define PDD_NAME		_L("ECDRV")
#else
#define PDD_NAME		_L("EUART1")
#define PDD2_NAME		_L("EUART2")
#define PDD3_NAME		_L("EUART3")
#define PDD4_NAME		_L("EUART4")
#endif

#define LDD_NAME		_L("ECOMM")

void InitCommsL(void)
	{
	TInt ret = User::LoadPhysicalDevice(PDD_NAME);
	User::LeaveIfError(ret == KErrAlreadyExists ? KErrNone : ret);

#ifndef __WINS__
	User::LoadPhysicalDevice(PDD2_NAME);
	User::LoadPhysicalDevice(PDD3_NAME);
	User::LoadPhysicalDevice(PDD4_NAME);
#endif

	ret = User::LoadLogicalDevice(LDD_NAME);
	User::LeaveIfError(ret == KErrAlreadyExists ? KErrNone : ret);

	ret = StartC32();
	User::LeaveIfError(ret == KErrAlreadyExists ? KErrNone : ret);
	}

