/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Secui Dialog notifier entry point
*
*/

#include "secuidialognotifier.h"         // CSecuiDialogNotifier
#include <ecom/implementationproxy.h>       // TImplementationProxy definition

const TInt KNumberOfImplementations = 1;


// ----------------------------------------------------------------------------
// CreateNotifiersL()
// ----------------------------------------------------------------------------
//
void CreateNotifiersL( CArrayPtr<MEikSrvNotifierBase2>& aNotifierArray )
    {
    CSecuiDialogNotifier* notifier = CSecuiDialogNotifier::NewL();
    CleanupStack::PushL( notifier );
    aNotifierArray.AppendL( notifier );
    CleanupStack::Pop( notifier );
    }

// ----------------------------------------------------------------------------
// NotifierArray()
// ----------------------------------------------------------------------------
//
CArrayPtr<MEikSrvNotifierBase2>* NotifierArray()
    {

    CArrayPtr<MEikSrvNotifierBase2>* notifierArray =
        new CArrayPtrFlat<MEikSrvNotifierBase2>( KNumberOfImplementations );
    if( notifierArray )
        {
        TRAPD( err, CreateNotifiersL( *notifierArray ) );


        if( err )
            {
            TInt count = notifierArray->Count();
            while( count-- )
                {
                (*notifierArray)[ count ]->Release();
                }
            delete notifierArray;
            notifierArray = NULL;
            }
        }
    return notifierArray;
    }

// ----------------------------------------------------------------------------
// Provides a key value pair table, that ECOM uses to identify
// the correct construction function for the requested interface.
// ----------------------------------------------------------------------------
//
const TImplementationProxy ImplementationTable[] =
    {
    // UID is the implementation uid defined in .rss file
    IMPLEMENTATION_PROXY_ENTRY( 0x102071FB, NotifierArray )
    };

// ----------------------------------------------------------------------------
// Main entry function used to return an instance of the proxy table.
// ----------------------------------------------------------------------------
//
EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
    {
    aTableCount = sizeof( ImplementationTable ) / sizeof( TImplementationProxy );
    return ImplementationTable;
    }
