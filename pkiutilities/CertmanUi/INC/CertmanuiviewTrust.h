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
* Description:   Declaration of the CCertManUIViewTrust class.
*                Updates Options list depending on the status of the
*                trusted clients (unavailable, located in WIM).
*                Handles Softkey and Options list commands.
*                Calls ChangeTrust method in CertManUIContainerTrust.
*
*/


#ifndef  CERTMANUIVIEWTRUST_H
#define  CERTMANUIVIEWTRUST_H

// INCLUDES
#include <aknview.h>

// FORWARD DECLERATIONS
class CCertManUIContainerTrust;

// CLASS DECLARATION

/**
*  CCertManUIViewTrust view class.
*
*  @lib certmanui.dll
*/
NONSHARABLE_CLASS( CCertManUIViewTrust ): public CAknView
    {
    public: // functions

        /**
        * Destructor
        */
        ~CCertManUIViewTrust();

        /**
        * NewL
        */
        static class CCertManUIViewTrust* NewL( const TRect& aRect, CCertManUIKeeper& aKeeper );

        /**
        * NewLC
        */
        static class CCertManUIViewTrust* NewLC( const TRect& aRect, CCertManUIKeeper& aKeeper );

        /**
        * Returns Trust view id
        */
        TUid Id() const;

        /**
        * Handles ClientRectChange
        */
        void HandleClientRectChange();

        /**
        * Returns iContainerTrust to be used in CertManUIContainerTrust
        */
        CCoeControl* Container();

        /**
        * Handles Softkey and Options list commands
        */
        void HandleCommandL( TInt aCommand );

        /**
        * Calls Change trust method UpdateTrustSettingsViewL in certmanuicontainertrust
        * for not supported certificate formats ECrX968Certificate and ECrCertificateURL
        * and for not supported certificate locations  ECrCertLocationWIMCard, ECrCertLocationWIMURL,
        * ECrCertLocationPhoneMemory, ECrCertLocationPhoneMemoryURL UpdateTrustSettingsViewL is not called
        * also UpdateTrustSettingsViewL is not called if there are no clients in Trust view
        */
        void ChangeTrustL();

        /**
        * Shows Trust Change Setting page
        */        
        void PopupTrustChangeSettingPageL();

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
        * Updates navi pane
        */
        void UpdateNaviPaneL();
        
        /**
        * Removes navi pane
        */
        void RemoveNaviPane();        
        
  public: // data

        /**
        * To get hold of CertManUIKeeper
        */
        CCertManUIKeeper&   iKeeper;

        /**
        * To get access to Trust container
        * Changed from private to public so that Change Trust's CAknRadioButtonSettingPage can access it
        */
        CCertManUIContainerTrust*   iContainerTrust;

    private: // functions

        /**
        * Constructor.
        */
        CCertManUIViewTrust( CCertManUIKeeper& aKeeper );

        /**
        * EPOC default constructor.
        */
        void ConstructL();


        /**
        * Closes the view
        */
        void DoDeactivate();

        /**
        * Updates Options list with correct items depending on
        * whether the listbox is empty or if the trust can be changed
        * Change trust is not shown if certificate location is not supported
        * unsupported certificate locations are ECrCertLocationWIMCard,
        *                   ECrCertLocationWIMURL,
        *                   ECrCertLocationPhoneMemory,
        *                   ECrCertLocationPhoneMemoryURL
        * Change trust is not shown if certificate format is not supported
        * unsupported certificate formats are ECrX968Certificate
        *                   ECrCertificateURL
        */
        void DynInitMenuPaneL( TInt aResourceId,CEikMenuPane* aMenuPane );

    private: // Data

        /**
        * For Navipane
        */
        CAknNavigationDecorator*		iTrustSettingsNaviPane;

        /**
        * For Navi container
        */
        CAknNavigationControlContainer* iNaviPane;

    };

#endif // CERTMANUIVIEWTRUST_H

// End of File
