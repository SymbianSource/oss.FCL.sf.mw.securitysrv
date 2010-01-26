/*
* Copyright (c) 2003-2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of class CCertManUIPluginContainer
*
*/


// INCLUDE FILES
#include    "CertmanuiPluginContainer.h"

// CONSTANTS

// ========================= MEMBER FUNCTIONS ================================

CCertManUIPluginContainer::CCertManUIPluginContainer()
    {

    }

// ---------------------------------------------------------------------------
// CCertManUIPluginContainer::ConstructL(const TRect& aRect)
// Symbian OS two phased constructor
//
// ---------------------------------------------------------------------------
//
void CCertManUIPluginContainer::ConstructL( const TRect& aRect )
    {
    CreateWindowL();
    SetRect( aRect );
    ActivateL();
    }

// ---------------------------------------------------------------------------
// CCertManUIPluginContainer::~CCertManUIPluginContainer()
// Destructor
//
// ---------------------------------------------------------------------------
//
CCertManUIPluginContainer::~CCertManUIPluginContainer()
    {
    }

// ---------------------------------------------------------------------------
// CCertManUIPluginContainer::SizeChanged
//
// ---------------------------------------------------------------------------
//
void CCertManUIPluginContainer::SizeChanged()
    {
    }

// ---------------------------------------------------------------------------
// CCertManUIPluginContainer::CountComponentControls
//
// ---------------------------------------------------------------------------
//
TInt CCertManUIPluginContainer::CountComponentControls() const
    {
    return 0;
    }

// ---------------------------------------------------------------------------
// CCertManUIPluginContainer::ComponentControl
//
// ---------------------------------------------------------------------------
//
CCoeControl* CCertManUIPluginContainer::ComponentControl( TInt /*aIndex*/ ) const
    {
    return NULL;
    }

// End of File
