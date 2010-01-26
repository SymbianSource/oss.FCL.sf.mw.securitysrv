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
* Description:  This class returns all supported readers and reader groups.
*
*/



// INCLUDE FILES
#include    "ScardBase.h"
#include    "ScardReaderQuery.h"
#include    "WimTrace.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CScardReaderQuery::CScardReaderQuery
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CScardReaderQuery::CScardReaderQuery()
    {
    }

// -----------------------------------------------------------------------------
// CScardReaderQuery::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CScardReaderQuery::ConstructL( )
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderQuery::ConstructL|Begin"));
    }
    
// -----------------------------------------------------------------------------
// CScardReaderQuery::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
EXPORT_C CScardReaderQuery* CScardReaderQuery::NewL()
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderQuery::NewL|Begin"));
    CScardReaderQuery* self = new( ELeave ) CScardReaderQuery;
    
    CleanupStack::PushL( self );
    //  ReaderQuery does not need writing access
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
EXPORT_C CScardReaderQuery::~CScardReaderQuery()
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderQuery::~CScardReaderQuery|Begin"));
    }

// -----------------------------------------------------------------------------
// CScardReaderQuery::ListReadersL
// List card readers belonging to given group
// -----------------------------------------------------------------------------
//
EXPORT_C void CScardReaderQuery::ListReadersL(
    CArrayFixFlat<TScardReaderName>* aReaderList,
    const TScardReaderName& aGroupName )
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderQuery::ListReadersL|Begin"));
    TScardReaderName groupName( KGroupName );

    if ( !aGroupName.Compare( groupName ) ) //Check if group name is as given
        {
        TScardReaderName readerName( KReaderName );
        aReaderList->AppendL( readerName );
        }
    else // Group name is not what we expected 
        {
        User::Leave( KScErrNotSupported );
        }
    }
// -----------------------------------------------------------------------------
// CScardReaderQuery::ListGroupsL
// List reader groups supported by Scard
// -----------------------------------------------------------------------------
//
EXPORT_C void CScardReaderQuery::ListGroupsL(
   CArrayFixFlat<TScardReaderName>* aGroupList )
    {
    _WIMTRACE(_L("WIM|Scard|CScardReaderQuery::ListGroupsL|Begin"));
    TScardReaderName groupName( KGroupName );
    aGroupList->AppendL( groupName );
    }

//  End of File  
