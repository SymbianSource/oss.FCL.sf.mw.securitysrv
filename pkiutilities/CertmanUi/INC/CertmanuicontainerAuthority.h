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
* Description:   Declaration of the CCertManUIContainerAuthority class
*
*/


#ifndef  CERTMANUICONTAINERAUTHORITY_H
#define  CERTMANUICONTAINERAUTHORITY_H

// INCLUDES
#include "CertmanuiviewAuthority.h"

// CLASS DECLARATION

class CAknIconArray;

/**
*  CCertManUIContainerAuthority container control class.
*
*  @lib certmanui.lib
*/
NONSHARABLE_CLASS( CCertManUIContainerAuthority ):
    public CCoeControl, public MCoeControlObserver, public MEikListBoxObserver
    {

    /**
    * Container authority needs to get access to CertManUIViewAuthority's DoActivateL
    */
    friend void CCertManUIViewAuthority::DoActivateL( const TVwsViewId&,
                                                      TUid, const TDesC8& );

    public: // functions

        /**
        * Overrided Default constructor
        */
        CCertManUIContainerAuthority( CCertManUIViewAuthority& aParent,
                                      CCertManUIKeeper& aKeeper );

        /**
        * Destructor.
        */
        ~CCertManUIContainerAuthority();

        /**
        * Change focus
        */
        void FocusChanged( TDrawNow aDrawNow );

        /**
        * Draws listbox, fetches graphic icons for markable list
        */
        void DrawListBoxL( TInt aCurrentPosition, TInt aTopItem );

        /**
        * Gets list of certificates from CertManAPI, displays them,
        * sets No label/No subject text to them if their names are empty
        */
        void UpdateListBoxL( TInt& aCurrentPosition, TInt& aTopItem );

        /**
        * Handles listbox events
        */
        void HandleListBoxEventL( CEikListBox* aListBox, TListBoxEvent aEventType );

        /**
        * Handles resource change
        */
        void HandleResourceChange( TInt aType );

    public: // data

        /**
        * To handle listbox
        * Type column for making the list markable
        * Cannot be private because CertManUIViewTrust uses it
        */
        CEikColumnListBox*    iListBox;

        /**
        * To get hold of ViewAuthority
        */
        CCertManUIViewAuthority&    iParent;

        /**
        * To get hold of CertManUIKeeper
        */
        CCertManUIKeeper&   iKeeper;

        /**
        * To store the list item count
        */
        TInt    iListItemCount;

    private: // functions

        /**
        * Default constructor
        */
        CCertManUIContainerAuthority();

        /**
        * EPOC default constructor.
        * @param aRect Frame rectangle for container.
        */
        void ConstructL( const TRect& aRect, TInt& aCurrentPosition, TInt& aTopItem );

        /**
        * Creates listbox, sets empty listbox text
        */
        void CreateListBoxL();

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
        CCoeControl* ComponentControl( TInt aIndex ) const;

        /**
        * From CCoeControl,Draw.
        */
        void Draw( const TRect& aRect ) const;

        /**
        * Handles control events
        * Cannot be changed to non-leaving function.
        * L-function is required by the class definition, even if empty.
        */
        void HandleControlEventL( CCoeControl* aControl,TCoeEvent aEventType );

        /**
        * Handles the key events OK button, arrow keys, delete key.
        * OK button functionality is different depending on the number of marked items
        * in the list; zero marked -> show details view, one or more marked -> show
        * OKOptions list, list empty -> OK inactive.
        * Delete active only if list not empty.
        */
        TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

        /**
        * Create the iconarray. Derived classes please don't forget that the
        * 0th icon must be the markable listbox's mark icon.
        * @return The created iconarray. Owner is the caller.
        */
        CAknIconArray* CreateIconArrayL() const;

        /**
        * Get resource id for the iconarray.
        * @return The resource id for the iconarray.
        */
        TInt IconArrayResourceId() const;


        /**
        * This function is called when Help application is launched.
        * (other items were commented in a header).
        */
        void GetHelpContext( TCoeHelpContext& aContext ) const;

    protected: //data

       /**
        * If ETrue, OK-Options menu is shown instead of normal Options menu.
        */
        TBool   iShowOkOptions;

    private: //data

        /**
        * Icon handler (this creates icons and calculates icon indexes).
        */
        const CCertManUIContainerAuthority*   iIconHandler;
 };

#endif // CERTMANUICONTAINERAUTHORITY_H

// End of File


