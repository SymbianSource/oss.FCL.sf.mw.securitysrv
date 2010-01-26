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
* Description:   Declaration of the CCertManUIViewDevice class
*
*/


#ifndef  CERTMANUIVIEWDEVICE_H
#define  CERTMANUIVIEWDEVICE_H

// INCLUDES
#include <aknview.h>
#include <e32base.h>
#include "Certmanuiviewid.h"

// FORWARD DECLERATIONS
class CCrCertificate;
class CCertificate;
class CX509Certificate;
class CCertManUIContainerDevice;

// CLASS DECLARATION

/**
*  CCertManUIViewDevice view class.
*
*  @lib certmanui.dll
*/
NONSHARABLE_CLASS( CCertManUIViewDevice ): public CAknView
    {
    public: // functions

        /**
        * Destructor
        */
        ~CCertManUIViewDevice();

        /**
        * NewL
        */
        static class CCertManUIViewDevice* NewL( const TRect& aRect, CCertManUIKeeper& aKeeper );

        /**
        * NewLC
        */
        static class CCertManUIViewDevice* NewLC( const TRect& aRect, CCertManUIKeeper& aKeeper );

        /**
        * Returns Authority view id
        */
        TUid Id() const;

        /**
        * Handles ClientRectChange
        */
        void HandleClientRectChange();

        /**
        * Returns iContainerTrustedSite to be used in CertManUIContainerTrustedSite
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
        * To get access to certificate list in authority view
        */
        CCertManUIContainerDevice*    iContainerDevice;

        /**
        * To store the position of the focus in the Authority view
        */
        TInt    iCurrentPosition;

        /**
        * Stores top item in the listbox
        */
        TInt    iTopItem;

    private: // functions

        /**
        * Constructor.
        */
        CCertManUIViewDevice( CCertManUIKeeper& aKeeper );

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
        * To get the certificate location
        */
        TInt    iCertificateLocation;
    };

#endif // CERTMANUIVIEWDEVICE_H

// End of File
