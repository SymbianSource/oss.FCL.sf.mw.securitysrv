/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description: 
*     Declares container control for application.
*
*/


#ifndef AUTOLOCKCONTAINER_H
#define AUTOLOCKCONTAINER_H

// INCLUDES
#include <coecntrl.h>
#include <eikimage.h>
#include <eiklabel.h>
#include <AknSkinnableClock.h>
   
// FORWARD DECLARATIONS

class CAknsLayeredBackgroundControlContext;

// CLASS DECLARATION

/**
*  CAutolockContainer  container control class.
*  
*/
class CAutolockContainer : public CCoeControl, MCoeControlObserver
    {
    public: // Constructors and destructor
        
        /**
        * Symbian OS default constructor.
        * @param aRect Frame rectangle for container.
        */
        void ConstructL(const TRect& aRect);

        /**
        * Destructor.
        */
        ~CAutolockContainer();

    public: // Functions from base classes
        /**
        * From CCoeControl, MopSupplyObject
        */
        TTypeUid::Ptr MopSupplyObject( TTypeUid aId );
        void GiveCoords( TRect& aRect );

    private: // Functions from base classes

       /**
        * From CoeControl,SizeChanged.
        */
        void SizeChanged();

       /**
        * From CoeControl,CountComponentControls.
        */
        TInt CountComponentControls() const;

       /**
        * From CCoeControl,ComponentControl.
        */
        CCoeControl* ComponentControl(TInt aIndex) const;

       /**
        * From CCoeControl,Draw.
        */
        void Draw(const TRect& aRect) const;

        // event handling section
        // e.g Listbox events
        void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType);
        
    private: //data
        CEikImage* iEikBitmap;
        CFbsBitmap* iBitmap;
        CFbsBitmap* iMask;
        CEikImage* iEikBitmapCall;
        CFbsBitmap* iBitmapCall;
        CFbsBitmap* iMaskCall;
        // Owned background context.
        CAknsLayeredBackgroundControlContext* iBgContext;
        
    };

#endif

// End of File
