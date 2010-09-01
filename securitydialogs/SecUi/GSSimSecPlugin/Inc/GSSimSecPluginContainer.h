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
* Description:  Container for the Display sub-folder
*
*/



#ifndef GSSIMSECCONTAINER_H
#define GSSIMSECCONTAINER_H

// INCLUDES
#include <secuisecuritysettings.h>
#include <e32property.h>

#include <gsbasecontainer.h>


// FORWARD DECLARATION
class CGSListBoxItemTextArray;
class CGSSimSecPluginModel;

// CLASS DECLARATION

/**
*  CGSSimSecPluginContainer container class
*
*  Container class for SIM security view
*  @lib GSSimSecPlugin.lib
*  @since Series 60_3.1
*/
class CGSSimSecPluginContainer : public CGSBaseContainer
    {
    public: // Constructors and destructor
        
        /**
        * Symbian OS constructor.
        * @param aRect Listbox's rect.
        * 
        */
        void ConstructL( const TRect& aRect );

        /**
        * Destructor.
        */
        ~CGSSimSecPluginContainer();
        
        /**
        * Constructor
        */
        CGSSimSecPluginContainer( CGSSimSecPluginModel* aModel );

   public: // new
        /**
        * Updates the listbox items
        * @param aFeatureId: selected listbox item ID
        */
        void UpdateListBoxL( TInt aFeatureId );
		
        /**
        * Retrieves the feature id for the selected item in the listbox
        * @return listbox item array's current feature.
        */
        TInt CurrentFeatureId() const;
        
        void HandleResourceChangeL( TInt aType );

   protected: // from CGSBaseContainer
       /**
        * Constructs listbox.
        * @param aResLbxId Resource id for listbox.
        * 
        */
       void ConstructListBoxL( TInt aResLbxId );

   private: //new
        /**
        * Creates list box items
        */   
        void CreateListBoxItemsL();
        /**
        * Creates Code in use item
        */   
        void MakeCodeInUseItemL();
        /**
        * Creates upin code request list box item
        */  
        void MakeUpinRequestItemL();
        /**
        * Creates pin code request list box item
        */  
        void MakePinRequestItemL();
        /**
        * Creates code list box item
        *
        * @ param aItemType TInt (code type pin/pin2/security)
        */  
        void MakeCodeItemL( const TInt aItemType );
        /**
        * Creates autolock list box item
        */  
        void MakeAutolockItemL();
        /**
        * Creates sim change security list box item
        */  
        void MakeSimChangeItemL();
        /**
        * Creates closed user group list box item
        */  
        void MakeClosedUserGroupItemL();
        
        /**
        * Creates sat operations list box item
        */  
        void MakeSatOperationsItemL();

        /**
        * Creates Trusted provisioning server list box item
        */  
        void MakeTPServerItemL();
        /**
        * Recreates different pin code related list box items as needed
        * after switching PIN codes from PIN to UPIN or vice versa.
        */  
        void RecreatePinItemsL();
        
        void MakeRemoteLockItemL();
    private:
        /**
        * Required for help.
        * 
        */
        void GetHelpContext( TCoeHelpContext& aContext ) const;
   
	private: //data
        //Code in use item's text
        CDesCArrayFlat* iCodeItems;
        //UPIN item's text
        CDesCArrayFlat* iUpinItems;
        //autolock item's text
        CDesCArrayFlat* iAutoLockItems;
        // RemoteLock item's text
        CDesCArrayFlat* iRemoteLockItems;
        //PIN item's text
        CDesCArrayFlat* iPinItems;
        //security item's text
        CDesCArrayFlat* iSecurityItems;
        //SAT item's text
        CDesCArrayFlat* iSatItems;
        //items from resource 
        CDesCArray* iItemArray;
        //for SecUI 
        CSecuritySettings* iSecurity;
        //check if UPIN code is supported
        TBool iUpinCodeSupported;
        //check if UPIN code is active
        TBool iUpinCodeActive;
        //check if UPIN code is permanently rejected
        TBool iUpinCodeRejected;
        //GS listbox model
		CGSListBoxItemTextArray* iListboxItemArray;
        //Model pointer. Does not own it, so do not delete this pointer.
        CGSSimSecPluginModel* iModel;

    };

#endif //GSSIMSECCONTAINER_H

// End of File
