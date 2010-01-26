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
* Description:   Declaration of class CCertSaverContainer
*
*/


#ifndef CERTSAVERCONTAINER_H
#define CERTSAVERCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <AknsBasicBackgroundControlContext.h>

// FORWARD DECLARATIONS

// CONSTANTS

// CLASS DECLARATION

class CCertSaverContainer : public CCoeControl
  {
public:

        /**
        * Second-phase constructor.
        */
        void ConstructL( const TRect& aRect );

        /**
        * Destructor.
        */
        ~CCertSaverContainer();


public:
        /**
        * From CCoeControl : Return count of controls contained in a compound control.
        * @return Count of controls contained in a compound control
        */
        TInt CountComponentControls() const;

        /**
        * From CCoeControl : Get the components of a compound control
        * @param aIndex : index of control
        * @return Control of argument aIndex
        */
        CCoeControl* ComponentControl( TInt aIndex ) const;

        /**
        * From CCoeControl : Notifier for changing language
        * @param aType : Type of resource change
        */
        void HandleResourceChange( TInt aType );


private:
        /**
        * From CCoeControl : Clear whole rectangle.
        * @param aRect : rectangle of control
        */
        void Draw( const TRect& aRect ) const;

        /**
        * From CCoeControl : Control size is set.
        */
        void SizeChanged();

        /**
        * From CCoeControl.
        * Pass skin information if need.
        */
        TTypeUid::Ptr MopSupplyObject( TTypeUid aId );

private:
        CAknsBasicBackgroundControlContext* iBgContext;
  };

#endif

// End of File
