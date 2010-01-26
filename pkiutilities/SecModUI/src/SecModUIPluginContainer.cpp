/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of the CSecModUIPluginContainer class
*
*/



// INCLUDE FILES
#include    "SecModUIPluginContainer.h"

// CONSTANTS

// ========================= MEMBER FUNCTIONS ================================

CSecModUIPluginContainer::CSecModUIPluginContainer()
    {

    }

// ---------------------------------------------------------------------------
// CApSettingsPluginContainer::ConstructL(const TRect& aRect)
// Symbian OS two phased constructor
//
// ---------------------------------------------------------------------------
//
void CSecModUIPluginContainer::ConstructL( const TRect& aRect )
    {

    CreateWindowL();
    SetRect( aRect );

    }

// ---------------------------------------------------------------------------
// CSecModUIPluginContainer::~CSecModUIPluginContainer()
// Destructor
//
// ---------------------------------------------------------------------------
//
CSecModUIPluginContainer::~CSecModUIPluginContainer()
    {

    }

// ---------------------------------------------------------------------------
// CSecModUIPluginContainer::SizeChanged
//  
// ---------------------------------------------------------------------------
//
void CSecModUIPluginContainer::SizeChanged()
    {

    }

// ---------------------------------------------------------------------------
// CSecModUIPluginContainer::CountComponentControls
//  
// ---------------------------------------------------------------------------
//
TInt CSecModUIPluginContainer::CountComponentControls() const
    {
    return 0;
    }

// ---------------------------------------------------------------------------
// CSecModUIPluginContainer::ComponentControl
//  
// ---------------------------------------------------------------------------
//
CCoeControl* CSecModUIPluginContainer::ComponentControl( TInt /*aIndex*/ ) const
    {
    return NULL;
    }

// End of File
