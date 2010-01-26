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
* Description:  For describing required Smart Card reader connections
*
*/



// INCLUDE FILES
#include    "ScardConnectionRequirement.h"
#include    "ScardServerBase.h"
#include    "ScardBase.h"
#include    "WimTrace.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// TScardConnectionRequirement::TScardConnectionRequirement
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
EXPORT_C TScardConnectionRequirement::TScardConnectionRequirement()
    : iExplicitReader( NULL ),
      iExcludedReader( NULL ),
      iATRBytes( NULL ),
      iAIDBytes( NULL ),
      iParameterCount( 0 ), 
      iNewCards( EFalse ),
      iNewReaders( EFalse )
    {
    _WIMTRACE(_L("WIM|Scard|TScardConnectionRequirement::TScardConnectionRequirement|Begin"));
    }

// -----------------------------------------------------------------------------
// TScardConnectionRequirement::SetExplicitL
// Set explicit reader
// -----------------------------------------------------------------------------
//
EXPORT_C void TScardConnectionRequirement::SetExplicitL(
    TScardReaderName& aExplicitName )
    {
    _WIMTRACE(_L("WIM|Scard|TScardConnectionRequirement::SetExplicitL|Begin"));
    __ASSERT_ALWAYS( iParameterCount < 2, User::Leave( KScErrFull ) );
    __ASSERT_ALWAYS( !iExcludedReader, User::Leave( KErrNotSupported ) );
    iExplicitReader = &aExplicitName;
    iParameterCount++;
    }

// -----------------------------------------------------------------------------
// TScardConnectionRequirement::SetExcludedL
// Set exluded reader
// -----------------------------------------------------------------------------
//
EXPORT_C void TScardConnectionRequirement::SetExcludedL(
    TScardReaderName& aExcludedName )
    {
    _WIMTRACE(_L("WIM|Scard|TScardConnectionRequirement::SetExcludedL|Begin"));
    __ASSERT_ALWAYS( iParameterCount < 2, User::Leave( KScErrFull ) );
    __ASSERT_ALWAYS( !iExplicitReader, User::Leave( KErrNotSupported ) );
    iExcludedReader = &aExcludedName;
    iParameterCount++;
    }

// -----------------------------------------------------------------------------
// TScardConnectionRequirement::SetATRL
// Set ATR bytes
// -----------------------------------------------------------------------------
//
EXPORT_C void TScardConnectionRequirement::SetATRL( TScardATR& aATR )
    {
    _WIMTRACE(_L("WIM|Scard|TScardConnectionRequirement::SetATRL|Begin"));
    __ASSERT_ALWAYS( iParameterCount < 2, User::Leave( KScErrFull ) );
    __ASSERT_ALWAYS( !(iAIDBytes), User::Leave( KErrNotSupported ) );
    iATRBytes = &aATR;
    iParameterCount++;
    }

// -----------------------------------------------------------------------------
// TScardConnectionRequirement::SetAIDBytesL
// Set AID bytes
// -----------------------------------------------------------------------------
//
EXPORT_C void TScardConnectionRequirement::SetAIDBytesL( TDesC8& aAID )
    {
    _WIMTRACE(_L("WIM|Scard|TScardConnectionRequirement::SetAIDBytesL|Begin"));
    __ASSERT_ALWAYS( iParameterCount < 2, User::Leave( KScErrFull ) );
    __ASSERT_ALWAYS( !(iATRBytes), User::Leave( KErrNotSupported ) );
    iAIDBytes = &aAID;
    iParameterCount++;
    }

// -----------------------------------------------------------------------------
// TScardConnectionRequirement::ClearExplicitL
// Clear explicit requirement
// -----------------------------------------------------------------------------
//
EXPORT_C void TScardConnectionRequirement::ClearExplicitL()
    {
    _WIMTRACE(_L("WIM|Scard|TScardConnectionRequirement::ClearExplicitL|Begin"));
    if ( !iExplicitReader )
        {
        User::Leave( KScErrEmpty );
        }
    if ( iParameterCount && iExplicitReader )
        {
        iParameterCount--;
        }
    iExplicitReader = NULL;
    }

// -----------------------------------------------------------------------------
// TScardConnectionRequirement::ClearExcludedL
// Clear exluded requirement
// -----------------------------------------------------------------------------
//
EXPORT_C void TScardConnectionRequirement::ClearExcludedL()
    {
    _WIMTRACE(_L("WIM|Scard|TScardConnectionRequirement::ClearExcludedL|Begin"));
    if ( !iExcludedReader )
        {
        User::Leave( KScErrEmpty );
        }

    if ( iParameterCount && iExcludedReader )
        {
        iParameterCount--;
        }

    iExcludedReader = NULL;
    }

// -----------------------------------------------------------------------------
// TScardConnectionRequirement::ClearATRL
// Clear ATR
// -----------------------------------------------------------------------------
//
EXPORT_C void TScardConnectionRequirement::ClearATRL()
    {
    _WIMTRACE(_L("WIM|Scard|TScardConnectionRequirement::ClearATRL|Begin"));
    if ( !iATRBytes )
        {
        User::Leave( KScErrEmpty );
        }
    if ( iParameterCount && iATRBytes )
        {
        iParameterCount--;
        }
    iATRBytes = NULL;
    }

// -----------------------------------------------------------------------------
// TScardConnectionRequirement::ClearAIDBytesL
// Clear AID bytes
// -----------------------------------------------------------------------------
//
EXPORT_C void TScardConnectionRequirement::ClearAIDBytesL()
    {
    _WIMTRACE(_L("WIM|Scard|TScardConnectionRequirement::ClearAIDBytesL|Begin"));
    if ( !iAIDBytes )
        {
        User::Leave( KScErrEmpty );
        }
    if ( iParameterCount && iAIDBytes )
        {
        iParameterCount--;
        }
    iAIDBytes = NULL;
    }

// -----------------------------------------------------------------------------
// TScardConnectionRequirement::NewCardsOnly
// Set New cards only flag
// -----------------------------------------------------------------------------
//
EXPORT_C void TScardConnectionRequirement::NewCardsOnly( const TBool aNewOnly )
    {
    _WIMTRACE(_L("WIM|Scard|TScardConnectionRequirement::NewCardsOnly|Begin"));
    iNewCards = aNewOnly;
    }

// -----------------------------------------------------------------------------
// TScardConnectionRequirement::NewReadersOnly
// Set new readers only flag
// -----------------------------------------------------------------------------
//
EXPORT_C void TScardConnectionRequirement::NewReadersOnly( const TBool aNewOnly )
    {
    _WIMTRACE(_L("WIM|Scard|TScardConnectionRequirement::NewReadersOnly|Begin"));
    iNewReaders = aNewOnly;
    }

//  End of File  
