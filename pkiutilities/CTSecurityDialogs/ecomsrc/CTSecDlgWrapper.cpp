/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   CTSecDlgWrapper.cpp
*
*/


// INCLUDE FILES
#include <ecom/implementationproxy.h>
#include <AknNotifierWrapper.h> // link against aknnotifierwrapper.lib
#include <secdlgimpldefs.h>
#include "CTSecurityDialogDefs.h"

// CONSTANTS
const TInt KMyPriority =  MEikSrvNotifierBase2::ENotifierPriorityAbsolute;
 
// ---------------------------------------------------------
// CleanupArray()
// ---------------------------------------------------------
//
void CleanupArray( TAny* aArray )
    {
    CArrayPtrFlat<MEikSrvNotifierBase2>* subjects = 
                static_cast<CArrayPtrFlat<MEikSrvNotifierBase2>*>( aArray );
        
    TInt lastInd = subjects->Count()-1;
    
    for ( TInt i = lastInd; i >= 0; i-- )
        {            
        subjects->At( i )->Release();	    	    
        }
        
    delete subjects;
    }
    
// ---------------------------------------------------------
// DoCreateNotifierArrayL()
// ---------------------------------------------------------
//
CArrayPtr<MEikSrvNotifierBase2>* DoCreateNotifierArrayL()
    {
    CArrayPtrFlat<MEikSrvNotifierBase2>* subjects =
        new ( ELeave )CArrayPtrFlat<MEikSrvNotifierBase2>( 1 );
    
    CleanupStack::PushL( TCleanupItem( CleanupArray, subjects ) );

    // Create Wrappers

    // Session owning notifier(if default implementation is enough)
    CAknCommonNotifierWrapper* master = 
        CAknCommonNotifierWrapper::NewL( KUidSecurityDialogNotifier,
                                   KUidSecurityDialogNotifier,
                                   KMyPriority,
                                   _L("CTSecDialogImpl.dll"),
                                   1 ); // we don't use synch reply                               
	  	
    CleanupStack::PushL( master );   
    subjects->AppendL( master );
    CleanupStack::Pop( master );
    
    CleanupStack::Pop();	// array cleanup
    return( subjects );
    }

// ---------------------------------------------------------
// NotifierArray()
// ---------------------------------------------------------
//
CArrayPtr<MEikSrvNotifierBase2>* NotifierArray()
    // old Lib main entry point
    {
    CArrayPtr<MEikSrvNotifierBase2>* array = 0;
    TRAP_IGNORE( array = DoCreateNotifierArrayL() ); 	
    return array;
    }

// ---------------------------------------------------------
// ImplementationTable
// ---------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] =
	{
	{{0x101F883C},(TProxyNewLPtr)NotifierArray}
	};
// ---------------------------------------------------------
// ImplementationGroupProxy
// entry point
// ---------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
	{
	aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy) ;
	return ImplementationTable;
	}

// End of File
