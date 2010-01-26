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
* Description:   Declaration of the CCertManUIPluginContainer class
*
*/


#ifndef   CERTMANUIPLUGINCONTAINER_H
#define   CERTMANUIPLUGINCONTAINER_H

// INCLUDES
#include    <coeccntx.h>
#include    <eikclb.h>

// CLASS DECLARATION

/**
*  CCertManUIPluginContainer container class
*
*  container class for CertMan UI view
*/
class CCertManUIPluginContainer : public CCoeControl
    {
    public: // Constructors and destructor

        CCertManUIPluginContainer();

        /**
        * Symbian OS constructor.
        *
        * @param aRect Listbox's rect.
        */
        void ConstructL( const TRect& aRect );

        /**
        * Destructor.
        */
        ~CCertManUIPluginContainer();

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

#endif // CERTMANUIPLUGINCONTAINER_H

// End of File