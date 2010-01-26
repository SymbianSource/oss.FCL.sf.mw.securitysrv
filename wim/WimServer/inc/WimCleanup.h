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
* Description:  Cleanup macros and definitions
*
*/

#ifndef WIMCLEANUP_H
#define WIMCLEANUP_H

#include "wimi.h"


//
// CleanupStack functions for WIMI data
//

inline void CleanupPushWimBufL( WIMI_BinData_t& aWimData )
    {
    CleanupStack::PushL( TCleanupItem( WSL_OS_Free, aWimData.pb_buf ) );
    }

inline void CleanupPushWimRefL( WIMI_Ref_t* aWimRef )
    {
    CleanupStack::PushL( TCleanupItem( free_WIMI_Ref_t, aWimRef ) );
    }

void FreeRefList( void* aRefList );

inline void CleanupPushWimRefListL( WIMI_RefList_t aWimRefList )
    {
    CleanupStack::PushL( TCleanupItem( FreeRefList, aWimRefList ) );
    }


//
// CleanupResetAndDestroyPushL() function
//

template <class T>
class CleanupResetAndDestroy
    {
public:
    inline static void PushL( T& aRef );
private:
    static void ResetAndDestroy( TAny *aPtr );
    };

template <class T>
inline void CleanupResetAndDestroyPushL( T& aRef );

template <class T>
inline void CleanupResetAndDestroy<T>::PushL( T& aRef )
    {
    CleanupStack::PushL( TCleanupItem( &ResetAndDestroy, &aRef ) );
    }

template <class T>
void CleanupResetAndDestroy<T>::ResetAndDestroy( TAny *aPtr )
    {
    if( aPtr )
        {
        static_cast<T*>( aPtr )->ResetAndDestroy();
        }
    }

template <class T>
inline void CleanupResetAndDestroyPushL( T& aRef )
    {
    CleanupResetAndDestroy<T>::PushL( aRef );
    }


#endif // WIMCLEANUP_H

