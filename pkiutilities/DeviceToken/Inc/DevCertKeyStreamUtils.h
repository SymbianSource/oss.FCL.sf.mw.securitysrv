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
* Description:   The header file of DevCertKeySteamUtils
*
*/



#ifndef __DEVCERTKEYSTREAMUTILS_H__
#define __DEVCERTKEYSTREAMUTILS_H__

#include <s32strm.h>
#include "DevTokenMarshaller.h"

class CRSAPublicKey;
class CRSAPrivateKey;
class CDSAPublicKey;
class CDSAPrivateKey;
class RInteger;

EXTERNALIZE_FUNCTION(CRSAPublicKey)

EXTERNALIZE_FUNCTION(CRSAPrivateKey)

EXTERNALIZE_FUNCTION(CDSAPublicKey)

EXTERNALIZE_FUNCTION(CDSAPrivateKey)

void ExternalizeL(const CRSAPublicKey& aData, RWriteStream& aStream);

void ExternalizeL(const CRSAPrivateKey& aData, RWriteStream& aStream);

void ExternalizeL(const CDSAPublicKey& aData, RWriteStream& aStream);

void ExternalizeL(const CDSAPrivateKey& aData, RWriteStream& aStream);

void CreateL(RReadStream& aStream, CRSAPublicKey*& aOut);

void CreateL(RReadStream& aStream, CRSAPrivateKey*& aOut);

void CreateL(RReadStream& aStream, CDSAPublicKey*& aOut);

void CreateL(RReadStream& aStream, CDSAPrivateKey*& aOut);

#endif //__DEVCERTKEYSTREAMUTILS_H__