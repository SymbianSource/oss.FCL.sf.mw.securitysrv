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
* Description:   Declaration of the CCertManUIViewPersonal class.
*
*/


#ifndef  CERTMANUIVIEWPERSONAL_H
#define  CERTMANUIVIEWPERSONAL_H

// INCLUDES
#include <aknview.h>
#include "Certmanuiviewid.h"

// CLASS DECLARATION
class CCertManUIContainerPersonal;

/**
*  CCertManUIViewPersonal view class.
*
*  @lib certmanui.dll
*/
NONSHARABLE_CLASS( CCertManUIViewPersonal ): public CAknView
    {
    public: // functions

        /**
        * Destructor
        */
        ~CCertManUIViewPersonal();

        /**
        * NewL
        */
        static class CCertManUIViewPersonal* NewL( const TRect& aRect, CCertManUIKeeper& aKeeper );

        /**
        * NewLC
        */
        static class CCertManUIViewPersonal* NewLC( const TRect& aRect, CCertManUIKeeper& aKeeper );

        /**
        * Returns Personal view id
        */
        TUid Id() const;

        /**
        * Handles ClientRectChange
        */
        void HandleClientRectChange();

        /**
        * Returns iContainerPersonal to be used in CertManUIContainerPersonal
        */
        CCoeControl* Container();

        /**
        * Handles Softkey and Options list commands
        */
        void HandleCommandL( TInt aCommand );

        /**
        * Updates the view when opening it
        */
        void DoActivateL( const TVwsViewId& aPrevViewId,TUid aCustomMessageId,
              const TDesC8& aCustomMessage );

        /**
        * Updates the menu bar
        */
        void UpdateMenuBar();

        /**
        * Updates cba
        */
        void UpdateCba(TInt aCbaResourceId);

  public: // data

        /**
        * To get hold of CertManUIKeeper
        */
        CCertManUIKeeper&   iKeeper;

       /**
        * Stores top item in the listbox
        */
        TInt    iTopItem;

       /**
        * To store the position of the focus in the Personal view
        */
        TInt    iCurrentPosition;

    private: // functions

        /**
        * Constructor.
        */
        CCertManUIViewPersonal( CCertManUIKeeper& aKeeper );

        /**
        * EPOC default constructor.
        */
        void ConstructL();

        /**
        * Saves focus position when closing view
        */
        void DoDeactivate();

        /**
        * Updates Options list with correct items depending on
        * whether the listbox is empty or if it has any marked items
        */
        void DynInitMenuPaneL( TInt aResourceId,CEikMenuPane* aMenuPane );


    private: // Data

       /**
        * To get access to certificate list in Personal view
        */
        CCertManUIContainerPersonal*  iContainerPersonal;

    };

#endif // CERTMANUIVIEWPERSONAL_H

// End of File
