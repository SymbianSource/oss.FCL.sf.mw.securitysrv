/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   DLL Entry Point
*
*/



#include "CTSecurityDialogNotifier.h"
#include <e32base.h>
#include <eiknotapi.h>

// Entry point for Notifiers
EXPORT_C CArrayPtr<MEikSrvNotifierBase2>* NotifierArray()
	{
	//The notifierArray function CAN leave, despite no trailing L
	CArrayPtrFlat<MEikSrvNotifierBase2>* subjects = new (ELeave) CArrayPtrFlat<MEikSrvNotifierBase2>( 1 );
	CleanupStack::PushL(subjects);
	CCTSecurityDialogNotifier* notifier = CCTSecurityDialogNotifier::NewL();
	CleanupStack::PushL( notifier );
	subjects->AppendL( notifier );
	CleanupStack::Pop( 2,subjects);	//notifier, subjects
	return subjects;
	}

// End of file
