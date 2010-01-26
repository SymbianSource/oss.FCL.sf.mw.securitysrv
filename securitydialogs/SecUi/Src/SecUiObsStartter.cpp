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
* Description:  Implementation of TObsStarter, which provides api
*               for starting SecurityObserver.exe
*
*
*/


// INCLUDES
#include <eikdll.h>
#include <data_caging_path_literals.hrh> 

#include "SecUiObsStartter.h"


//  LOCAL CONSTANTS AND MACROS  
_LIT(ObserverName,"SecurityObserver.exe");

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CObsStartter::StartObserverL()
// Launches SecurityObserver.exe. StartObserverL() is called from startup.
// ----------------------------------------------------------
//
EXPORT_C void TObsStarter::StartObserverL()
	{
	RProcess process; 
    HBufC* observerPath = HBufC::NewLC(KMaxPath);
    TPtr ObserverEXE(observerPath->Des());
    ObserverEXE.Append(_L("Z:"));
    ObserverEXE.Append(KDC_PROGRAMS_DIR);
    ObserverEXE.Append(ObserverName);

		TInt err = process.Create( ObserverEXE, KNullDesC );
		process.Resume();
		if (err)
			{
			// Panic here in order to enter SW
			// reset mechanism.
			User::Panic(ObserverEXE,err);
			}
    CleanupStack::PopAndDestroy(observerPath);
	}
// End of file
