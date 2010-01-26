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
* Description:  View for Device & SIM security plug-in.
*
*/


#ifndef GSSIMSECPLUGIN_H
#define GSSIMSECPLUGIN_H

// INCLUDES
#include "GSSimSecPluginContainer.h"
#include "GSSimSecPluginModel.h"
#include "MGSSettingPSObserver.h"
#include "GSPubSubsListener.h"

#include <aknsettingpage.h>
#include <ConeResLoader.h>
#include <gsplugininterface.h>
#include <gsfwviewuids.h>
#include <gsbaseview.h>
#include <secuisecuritysettings.h>
#include <RSSSettings.h>

// CONSTANTS
const TInt KMaxStringLength = 80;
const TUid KGSSimSecPluginUid = { 0x10207438 };

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class CAknViewAppUi;
class CGSSimSecPluginContainer;


// CLASS DECLARATION

/**
*  CGSSimSecPlugin view class
*
*  View class for Sim Security sub-folder
*/
class CGSSimSecPlugin : public CGSBaseView,
                        private MGSSettingPSObserver
    {
    public: // Constructors and destructor
        
        /**
        * Symbian OS two-phased constructor
        * @return GS sim & device security view.
        */
        static CGSSimSecPlugin* NewL( TAny* aInitParams );
        
        /**
        * C++ default constructor.
        */
        CGSSimSecPlugin();

        /**
        * Symbian OS default constructor.
        */
        void ConstructL();

        /**
        * Destructor.
        */
        ~CGSSimSecPlugin();

    public: // Functions from base classes
        
       /**
        * Returns view id.
        * @return TUid.
        */
        TUid Id() const;

        /**
        * Handles commands.
        * @param aCommand Command to be handled.
        * 
        */
        void HandleCommandL( TInt aCommand );

    public: //new

        /**
        * Updates listbox's item's value.
        * @param aItemId An item which is updated.
        * 
        */
        void UpdateListBoxL( TInt aItemId );
        
        //From CGSBaseView
        void HandleResourceChangeL( TInt aType );

    public: // From CGSPluginInterface

        /**
        * @see CGSPluginInterface header file.
        */
        void GetCaptionL( TDes& aCaption ) const;
        
        /**
        * @see CGSPluginInterface header file.
        */
        TInt PluginProviderCategory() const;

        /**
        * @see CGSPluginInterface header file.
        */
        TBool Visible() const;
        
    protected: // From MEikMenuObserver

        void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

    private: // from CAknView
        /**
        * Activates the view.
        * @param aPrevViewId ID of previous view
        * @param aCustomMessageId customized message ID
        * @param aCustomMessage sutomized message payload
        */
        void DoActivateL( const TVwsViewId& aPrevViewId,
                          TUid aCustomMessageId,
                          const TDesC8& aCustomMessage );

    private: // from CGSBaseView
       
        void NewContainerL();
       
        /**
        * From CGSBaseView, handles list box selections.
        * 
        */
        void HandleListBoxSelectionL();
 
    private:
        /**
        * Callback from MGSSettingPSObserver
        */
        void HandleNotifyPSL( const TUid aUid, const TInt& aKey, 
                                      const TRequestStatus& aStatus );
      
    private: //new
    	
        /**
        * Sets Autolock period
        * 
        */
        void SetAutolockTimeL( TInt aPeriod );
       
        /**
        * Set sat operations value (on/off).
        * 
        */
       
        void SetSatOperationsL();
       
        /**
        * Set sat operations value with setting page(on/off).
        * 
        */
        void SetSatWithSettingPageL();
       
        /**
        * Shows closed user group index query
        * @param aCugIndex Current Cug index
        * @param aCugDefault Default CUG index
        * @return TInt (query result)
        */
        TInt CugIndexQueryL( TInt& aCugIndex, TInt& aCugDefault );

        /**
        * Set closed user group setting
        * 
        */
        void CugModeL();
       
        /**
        * Get CGSSimSecPlugin's ccontainer.
        */
        CGSSimSecPluginContainer* Container();
        
        
        TInt SetRemoteLockStatusL( TInt aAutoLockPeriod );
        
    private: // Data
        //for CUG settings
        RSSSettings        iCugSettings;
        //to launch SecUI
        CSecuritySettings* iSecurity;
        //plugin model.
        CGSSimSecPluginModel* iModel;
        //resource loader
        RConeResourceLoader iResourceLoader;
        //PubSub object for BT SAP state
        CGSPubSubsListener* iBtSapListener;

    };

#endif //GSSIMSECPLUGIN_H

// End of File
