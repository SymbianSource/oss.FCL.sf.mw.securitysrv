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
* Description:   Implementation of DevTokenNullStream
*
*/



#include "DevTokenNullStream.h"

// ======== MEMBER FUNCTIONS ========

// TNullBuf 

// ---------------------------------------------------------------------------
// TNullBuf::TNullBuf()
// ---------------------------------------------------------------------------
// 
TNullBuf::TNullBuf() :
    iBytesWritten(0)
    {
    }


// ---------------------------------------------------------------------------
// TNullBuf::DoWriteL(const TAny* /*aPtr*/,TInt aLength)
// ---------------------------------------------------------------------------
//
void TNullBuf::DoWriteL(const TAny* /*aPtr*/,TInt aLength)
    {
    iBytesWritten += aLength;
    }


// ---------------------------------------------------------------------------
// TNullBuf::BytesWritten()
// ---------------------------------------------------------------------------
//
TUint TNullBuf::BytesWritten()
    {
    return iBytesWritten;
    }


// RNullWriteStream 

// ---------------------------------------------------------------------------
// RNullWriteStream::RNullWriteStream()
// ---------------------------------------------------------------------------
//
RNullWriteStream::RNullWriteStream()
    {
    Attach(&iSink);
    }


// ---------------------------------------------------------------------------
// RNullWriteStream::BytesWritten()
// ---------------------------------------------------------------------------
//
TUint RNullWriteStream::BytesWritten()
    {
    return iSink.BytesWritten();
    }

//EOF
