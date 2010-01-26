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
// Define OIDs we use in OCSP - comment out the ones we don't use
// 
//

/**
 @file 
 @internalComponent 
*/

#ifndef __OCSP_OIDS_H__
#define __OCSP_OIDS_H__

// OCSP OIDs

_LIT(KOCSPOidBasic,				"1.3.6.1.5.5.7.48.1.1");
_LIT(KOCSPOidNonce,				"1.3.6.1.5.5.7.48.1.2");
//_LIT(KOCSPOidCrl,				"1.3.6.1.5.5.7.48.1.3");
_LIT(KOCSPOidResponse,			"1.3.6.1.5.5.7.48.1.4");
_LIT(KOCSPOidNoCheck,			"1.3.6.1.5.5.7.48.1.5");
_LIT(KOCSPOidArchiveCutoff,		"1.3.6.1.5.5.7.48.1.6");
_LIT(KOCSPOidServiceLocator,	"1.3.6.1.5.5.7.48.1.7");
_LIT(KOCSPOidOCSPSigning,		"1.3.6.1.5.5.7.3.9");

const TUint KOCSPMaxOidEncodingLength = 20;

#endif // __OCSP_OIDS_H__



