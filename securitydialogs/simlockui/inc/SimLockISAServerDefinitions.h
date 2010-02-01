/*
* ============================================================================
*  Name         : SimLockISAServerDefinitions.h
*  Part of      : Sim Lock UI Application
*  Description  : Definitions for Sim Lock ISA Server
*  Version      : 
*  
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Build info file for Ado domain appinstall 
* ============================================================================
*/

#ifndef SimLockISAServerDefinitions_H
#define SimLockISAServerDefinitions_H

// INCLUDES
#include <e32base.h>

// =============================================    
// Convert to little endian
// =============================================    

// Macro to convert big endian to native type
// EPOC ARM is little endian
#define BE2INT16( x )     ( ( ( ( x ) & 0xFF00 ) >> 8 ) |( ( ( x ) & 0x00FF ) << 8 ) )


// =============================================    
// From Simlock_lock.h in cellmo: ISA server
// =============================================

// MACROS

#define SIMLOCK_PROFILE_SIZE 8
#define SIMLOCK_CONFIG_SIZE  8      

/* Block status */
typedef TUint8 SL_BLOCK_STATUS;
#define SL_BLOCK_NOT_DISCARD     0x00
#define SL_BLOCK_DISCARD         0x01
#define SL_BLOCK_PERMANENT       0x02
#define SL_BLOCK_MANDATORY       0x03 

/* Requirement status */
typedef TUint8 SL_LOCKTYPE;
#define SL_LOCKTYPE_AUTO   0x01
#define SL_LOCKTYPE_CLOSED 0x02
#define SL_LOCKTYPE_OPEN   0x03 


/* Criteria */
typedef TUint8 SL_OPERATOR;
#define SL_OP_BYTEWISE_DISTINCT_EQ    0x01
#define SL_OP_BYTEWISE_DISTINCT_GTE   0x02
#define SL_OP_BYTEWISE_DISTINCT_LTE   0x03
#define SL_OP_BYTEWISE_DISTINCT_NEQ   0x04
#define SL_OP_NIBBLEWISE_DISTINCT_EQ  0x05
#define SL_OP_NIBBLEWISE_DISTINCT_GTE 0x06
#define SL_OP_NIBBLEWISE_DISTINCT_LTE 0x07
#define SL_OP_NIBBLEWISE_DISTINCT_NEQ 0x08
#define SL_OP_BITWISE_DISTINCT_NEQ    0x09
#define SL_OP_BYTEWISE_UNIFIED_GTE    0x0A
#define SL_OP_BYTEWISE_UNIFIED_LTE    0x0B
#define SL_OP_BYTEWISE_UNIFIED_NEQ    0x0C   
#define SL_OP_NIBBLEWISE_UNIFIED_GTE  0x0D
#define SL_OP_NIBBLEWISE_UNIFIED_LTE  0x0E
#define SL_OP_NIBBLEWISE_UNIFIED_NEQ  0x0F
#define SL_OP_BITWISE_DISTINCT_EQ     0x10
#define CRITERIA_MAXIMUM 0x10 


/* No mask( all data counts ) */
#define SIMLOCK_MASK_NOT_DEF 0x0000

// DATA TYPES

// Sim Lock data structures 
struct TSimLockConfigStr
    {
    TUint8 Byte[SIMLOCK_CONFIG_SIZE];
    };

struct TSimLockProfileStr
    {
    TUint8 Byte[SIMLOCK_PROFILE_SIZE];
    };

/* An aboslute path on SIM card */
struct TSimPathStr
    {
    TUint8 ah;
    TUint8 al;
    TUint8 bh;
    TUint8 bl;
    TUint8 ch;
    TUint8 cl;
    TUint8 dh;
    TUint8 dl;
    }; /* sizeof( SIM_PATH_STR ) = 8 */
    
#define SIM_PATH_SIZE sizeof( TSimPathStr )

/* A match requirement */
struct TSimLockMRStr
    {
    TUint32      ReadSpec;
    TSimPathStr FileId;
    TUint16      Mask; 
    TUint16      Offset; 
    TUint8        DataSize; 
    TUint8        UnitLength;
    SL_OPERATOR Criterion;
    SL_LOCKTYPE LockType;
    }; /* sizeof( SL_MR_STR ) = 20  */
    
#define SL_MR_STR_SIZE sizeof( TSimLockMRStr )

/* A match block */
struct TSimLockMBStr
    {
    TUint16 Offset; 
    TUint8  AccessClass;
    SL_BLOCK_STATUS mbStatus;
    TUint8  mrCount;
    TUint8 Reserved1;
    TUint8 Reserved2;
    TUint8 Reserved3;
    }; /* sizeof( SL_MB_STR ) = 8            */
    
#define SL_MB_STR_SIZE sizeof( TSimLockMBStr )

/* Sim Lock header */
struct TSimLockHeadStr
    {
    // Use BB5 Head Str
    TSimLockProfileStr Profile;             //  !!!! 8 TUint8s
    TSimLockConfigStr ConfigData;           //  !!!! 8 TUint8s
 
    TUint16 Offset;  
    TUint8 mbCount;
    TUint8 UnlockCount;
    TUint8 Reserved1;
    TUint8 Reserved2;
    
    TUint8  Reserved3;
    TUint8  Reserved4; 
    }; /* sizeof( SL_HEAD_STR ) = 8 or 24 */
    
#define SL_HEAD_STR_SIZE  ( ( TInt )sizeof( TSimLockHeadStr ) )

// CONSTANTS

extern TInt KSimLockUnlockAttemptsAvailable;

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS


#endif // SimLockISAServerDefinitions_H

// End of File
