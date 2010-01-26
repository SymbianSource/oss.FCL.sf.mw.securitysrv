// Copyright (c) 2008-2009 Nokia Corporation and/or its subsidiary(-ies).
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
//

#include "ocsppolicy.h"

#include <f32file.h>
#include <s32file.h>


// OCSP policy repository UID
static const TUid KUidOCSPRepository = { 0x2002B28B };

// Default values for the settings
const TBool KDefaultGenerateResponseForMissingUri 	= ETrue;
const TBool KDefaultEnableHttpGETMethod 			= EFalse;

EXPORT_C COcspPolicy* COcspPolicy::NewL()
	{
	COcspPolicy* self = new(ELeave)COcspPolicy();
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

EXPORT_C TBool COcspPolicy::IsGenerateResponseForMissingUriEnabled() const
	{
	return iGenerateResponseForMissingUri;
	}

EXPORT_C TBool COcspPolicy::IsHttpGETMethodEnabled() const
	{
	return iEnableHttpGETMethod;
	}

COcspPolicy::COcspPolicy()
	: iGenerateResponseForMissingUri(KDefaultGenerateResponseForMissingUri),
	iEnableHttpGETMethod(KDefaultEnableHttpGETMethod)
	{
	}

/*
 * Leaves with KErrNotFound if the OCSP repository is not found and 
 * default values for the settings are retained if the settings are missing 
 * in the repository.
 * Otherwise, the settings are set to the read values from the repository.
 */ 
void COcspPolicy::ConstructL()
	{
    // Session to access OCSP Central Repository Server.
	SecuritySettingsServer::RSecSettingsSession SecSettingsSession;

    // Connect to the Central Repository server.
	User::LeaveIfError(SecSettingsSession.Connect());

	CleanupClosePushL(SecSettingsSession);
    
	// Read-in the values of the settings - GenerateResponseForMissingUri and 
	// EnableHttpGETMethod from the OCSP Central Repository. 
	// These will retain the default values if any error occurs.

	TInt value = 1;

	TRAPD(err, (value = SecSettingsSession.SettingValueL(KUidOCSPRepository, KGenerateResponseForMissingUri)));
	
	if( err == KErrNone )
	{
		iGenerateResponseForMissingUri = value;
	}
	else if( err != KErrSettingNotFound )
	{
		User::Leave(err);
	}

	TRAP(err, (value = SecSettingsSession.SettingValueL(KUidOCSPRepository, KEnableHttpGetMethod)));

	if ( err == KErrNone )
	{
		iEnableHttpGETMethod = value;
	}
	else if( err != KErrSettingNotFound )
	{
		User::Leave(err);
	}	

	CleanupStack::PopAndDestroy(&SecSettingsSession);
	}

