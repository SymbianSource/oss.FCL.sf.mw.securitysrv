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
* Description:   Declaration of the CSecModUIPluginContainer class
*
*/



#ifndef CSECMODUIPLUGINCONTAINER_H
#define CSECMODUIPLUGINCONTAINER_H

// INCLUDES
#include    <coeccntx.h>
#include    <eikclb.h>

// CLASS DECLARATION

/**
*  CSecModUIPluginContainer container class
*
*  container class for Security UI Module view
*/
class CSecModUIPluginContainer : public CCoeControl
    {
    public: // Constructors and destructor

        CSecModUIPluginContainer();

        /**
        * Symbian OS constructor.
        *
        * @param aRect Listbox's rect.
        */
        void ConstructL( const TRect& aRect );

        /**
        * Destructor.
        */
        ~CSecModUIPluginContainer();

    public: // From CCoeControl

        /**
        * From CoeControl returns the amount of controls
        */
        TInt CountComponentControls() const;

        /**
        * From CCoeControl returns controls for CONE actions
        */
        CCoeControl* ComponentControl( TInt aIndex ) const;

        /**
        * From CCoeControl changes the size of the list box
        */
        void SizeChanged();

    };

#endif //CSECMODUIPLUGINCONTAINER_H
