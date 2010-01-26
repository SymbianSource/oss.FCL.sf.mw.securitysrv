/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Constans for SwimReader
*
*/


#ifndef SWIMREADERCONSTS_H
#define SWIMREADERCONSTS_H

//  INCLUDES
#include <e32base.h>


//  CONSTANTS

/* APDUs */
const TUint8 KSendApdu       = 79;
const TUint8 KGetATR         = 80;
const TUint8 KList           = 81;
const TUint8 KAPDUAppTypeWIM = 1;

const TUint8 KApduOK         = 0x3d;

// Error byte values
const TUint8 KApduTransmissionError = 0x3e;
const TUint8 KApduCardMute          = 0x3f;
const TUint8 KApduCardDisconnected  = 0x40;
const TUint8 KApduError             = 0x41;
const TUint8 KApduReaderNotValid    = 0x42;
const TUint8 KApduFormatError       = 0x43;
const TUint8 KApduTypeNotValid      = 0x44;

const TUint8 KSimServNoservice      = 0xFA; 
const TUint8 KSimServNotready       = 0xFB; 
const TUint8 KSimServError          = 0xFC; 

//
// Check APDU list chapter to understand these.
const TUint8 KMaxReaderAmount       = 8;
const TUint8 KListAllReaders        = 0xff;

//
// Reader status bits.
const TUint8 KIdentityMask          = 0x07;
const TUint8 KCardReaderRemovable   = 0x08;//0x10;
const TUint8 KCardReaderPresent     = 0x10;//0x08;
const TUint8 KCardReaderID1Size     = 0x20;
const TUint8 KCardPresent           = 0x40;//0x02;
const TUint8 KCardPowered           = 0x80;//0x01;

const TUint8 KMaxApduHeaderLen      = 16;

const TUint8 KSendApduHeaderLen     = 4;
const TUint8 KGetListHeaderLen      = 2;

const TUint8 KSendApduRespHeaderLen = 14;
const TUint8 KListRespHeaderLen     = 13;
const TUint8 KGetAtrRespHeaderLen   = 16;

const TUint8 KNoPreferredReader     = 254;

//APDU request bytes
const TInt KIndexServiceType = 0;
const TInt KIndexCardReader  = 1;
const TInt KIndexAppType     = 2;
const TInt KIndexPaddingByte = 3;

//APDU response bytes
const TInt KIndexRespTransActionID = 8;
const TInt KIndexRespMessageID     = 9;
const TInt KIndexRespServiceType   = 10;
const TInt KIndexRespStatus        = 11;
const TInt KIndexRespPaddingByte   = 12;
const TInt KIndexRespSDataLength   = 15;

#endif      // SWIMREADERCONSTS_H

// End of File
