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
* Description:  Inline functions for smart card message stack
*
*/


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardMessageStack::Count
// Returns count of messages in the stack.
// -----------------------------------------------------------------------------
//
inline const TInt CScardMessageStack::Count()
    {
    return iMessages->Count();
    }   

// -----------------------------------------------------------------------------
// CScardMessageStack::CancelOne
// Cancel one message
// -----------------------------------------------------------------------------
//
inline void CScardMessageStack::CancelOne( 
    const TInt aIndex, 
    const TInt aReason,
    const TBool aCancelTimer )
    {
    TMessageHandle& tmp = (*iMessages)[aIndex]; 

    // Do not cancel twice
    if ( !tmp.iCancelled )
        {
        tmp.iCancelled = ETrue;
        //  There are two special types of messages that don't need completing.
        //  One is a transmission from a connector and the other
        //  is ConnectToReader
        if ( ! ( (tmp.iAdditionalParameter & KConnection ) |
            ( tmp.iMessage.Function() == EScardServerConnectToReader ) ) ) 
            {
            tmp.iMessage.Complete( aReason );
            }

        if ( aCancelTimer && tmp.iTimer )
            {
            tmp.iTimer->Cancel();
            }
        }
    }

// End of File
