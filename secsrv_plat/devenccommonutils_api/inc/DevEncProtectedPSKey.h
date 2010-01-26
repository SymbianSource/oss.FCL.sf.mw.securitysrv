/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Ps key definition
 *
*/



#ifndef DEVENCPROTECTEDPSKEY_H_
#define DEVENCPROTECTEDPSKEY_H_

enum TDevEncOperation
	{
	EOpIdle,
	EOpEncrypting,
	EOpDecrypting
	};

//same Uid as DevEncStarter which is the component defining and deleting it
const TUid  KDevEncProtectedUid = {0x2000259B};          

/**
 * Store the last memory status changed
 * 0: Idle. It can be Encrypted or Decrypted
 * 1: Encrypting
 * 2: Decrypting
 **/
const TUint KDevEncOperationKey = 0x01;

#endif /* DEVENCPROTECTEDPSKEY_H_ */
