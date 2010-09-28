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
// This file contains classes providing functionality to read the OCSP policy repository.
// 
//

/**
 @file
 @internalTechnology
*/

#ifndef __OCSPPOLICY_H__
#define __OCSPPOLICY_H__

#include <e32base.h>

// Security settings.
#include <secsettings/secsettingsclient.h>

#include <securityerr.h>

// Following settings are read from the OCSP repository.
//   KGenerateResponseForMissingUri = True/False
//   KEnableHttpGETMethod = True/False
//

// Key for the setting - KGenerateResponseForMissingUri.
const TUint32 KGenerateResponseForMissingUri = 0x1;

// Key for the setting - KEnableHttpGetMethod.
const TUint32 KEnableHttpGetMethod = 0x2;

/*
 * COcspPolicy class - Implementation to read the OCSP central repository
 * settings.
 */
class COcspPolicy : public CBase
	{
public:
	/**
	 * This function returns the setting for KGenerateResponseForMissingUri
	 */
	IMPORT_C TBool IsGenerateResponseForMissingUriEnabled() const;

	/**
	 * This function returns the setting for KEnableHttpGETMethod
	 */
	IMPORT_C TBool IsHttpGETMethodEnabled() const;


	/**
  	 * This function is used to access the instance of this class. 
	 * TLS is used to store pointer of the instance.
  	 */
	IMPORT_C static COcspPolicy* NewL();

private:	
	COcspPolicy();


	/**
	 * The second-phase constructor. It reads the settings from
	 * the repository.
	 */
	void ConstructL();

private:
	TBool iGenerateResponseForMissingUri;
	TBool iEnableHttpGETMethod;
	};


#endif // __OCSPPOLICY_H__
