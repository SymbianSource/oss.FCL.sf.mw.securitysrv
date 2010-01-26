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
* Description:   Declaration of the CSecModUIPlugin class
*
*/



#ifndef CSECMODUIPLUGIN_H
#define CSECMODUIPLUGIN_H

// Includes
#include <aknview.h>
#include <eikclb.h>
#include <ConeResLoader.h>

#include <gsplugininterface.h>

// Forward declarations
class CSecModUIModel;
class CSecModUIPluginContainer;
class CAknViewAppUi;

// Constants

/**
* Used for both the view UID and the ECOM plugin implementation UID.
*/
const TUid KSecModUIPluginUid = { 0x101F8668 };

// CLASS DECLARATION

/**
* @since Series60_3.1
*/
class CSecModUIPlugin : public CGSPluginInterface
    {

    public: // Constructors and destructor

        /**
        * Symbian OS two-phased constructor
        * @return
        */
        static CSecModUIPlugin* NewL( TAny* aInitParams );

        /**
        * Destructor.
        */
        ~CSecModUIPlugin();

    public: // From CAknView

        /**
        * See base class.
        *
        * This function is used also for identifying the plugin so be sure to
        * return correct UID.
        */
        TUid Id() const;

        /**
        * See base class.
        */
        void HandleClientRectChange();

        /**
        * See base class.
        */
        void DoActivateL( const TVwsViewId& aPrevViewId,
                          TUid aCustomMessageId,
                          const TDesC8& aCustomMessage );

        /**
        * See base class.
        */
        void DoDeactivate();

        /**
        * See base class.
        */
        void GetCaptionL( TDes& aCaption ) const;
        
        /**
         * See base class.
         */
        TBool Visible() const;

    private: // New

        /**
        * C++ default constructor.
        */
        CSecModUIPlugin();

        /**
        * Symbian OS default constructor.
        */
        void ConstructL();
        
        /**
         * Leaving part of Visible() member.
         */
        TBool DoIsVisibleL() const;

     private: // Data

        // Pointer to application UI
        CAknViewAppUi* iAppUi;

        // Resouce loader.
        RConeResourceLoader iResourceLoader;

        CSecModUIModel* iSecUiModel;

        // Pointer to the container
        CSecModUIPluginContainer* iContainer;

        //Previous view (Security view)
        TVwsViewId iPrevViewId;

        CEikonEnv* iEikEnv;
    };

#endif // CSECMODUIPLUGIN_H
// End of File
