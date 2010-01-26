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
* Description:  Provides api for starting SecurityObserver
*
*
*/


#ifndef     __SECUIOBSSTARTTER_H__
#define     __SECUIOBSSTARTTER_H__

class TObsStarter 
	{
	public:
		/**
		* Launches SecurityObserver.exe. 
		* StartObserver() is called from startup app.
		*/
		IMPORT_C static void StartObserverL();
	};

#endif

// End of file
