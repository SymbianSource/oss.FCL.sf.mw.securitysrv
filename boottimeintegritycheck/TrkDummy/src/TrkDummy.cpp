/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Dummy exe to prevent installing too liberal TrkApp
*
*/


#include <eikstart.h>
#include "TrkDummyApplication.h"

CApaApplication* NewApplication()
	{
	return new CTrkDummyApplication;
	}

TInt E32Main()
	{
	return EikStart::RunApplication( NewApplication );
	}


