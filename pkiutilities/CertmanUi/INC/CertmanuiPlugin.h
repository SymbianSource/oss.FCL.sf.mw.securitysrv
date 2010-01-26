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
* Description:   Declaration of the CCertManUIPlugin class
*
*/


#ifndef   CERTMANUIPLUGIN_H
#define   CERTMANUIPLUGIN_H

// Includes
#include <aknview.h>
#include <eikclb.h>
#include <ConeResLoader.h>

#include <gsplugininterface.h>

// Forward declarations
class CCertManUIKeeper;
class CCertManUIPluginContainer;
class CAknViewAppUi;

// Constants

/**
* Used for both the view UID and the ECOM plugin implementation UID.
*/
const TUid KCertManUIPluginUid = { 0x10008D3D };

// CLASS DECLARATION

/**
* @since Series60_3.1
*/
class CCertManUIPlugin : public CGSPluginInterface
    {

    public: // Constructors and destructor

        /**
        * Symbian OS two-phased constructor
        * @return
        */
        static CCertManUIPlugin* NewL( TAny* aInitParams );

        /**
        * Destructor.
        */
        ~CCertManUIPlugin();

    public: // From CAknView

        /**
        * See base class.
        *
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

    private: // New

        /**
        * C++ default constructor.
        */
        CCertManUIPlugin();

        /**
        * Symbian OS default constructor.
        */
        void ConstructL();

        /**
        * Used in ConstructL to get resource file
        */
        void AddResourceFileL();

    private: // Data

        // Resouce loader.
        RConeResourceLoader iResourceLoader;

        CCertManUIKeeper* iCertManUIKeeper;

        // Pointer to the container
        CCertManUIPluginContainer* iContainer;

        //Previous view (Security view)
        TVwsViewId iPrevViewId;

        // Indicates if CertManUI is initialized
        TBool iCertManUIInitialized;
    };

#endif //   CERTMANUIPLUGIN_H

// End of File
