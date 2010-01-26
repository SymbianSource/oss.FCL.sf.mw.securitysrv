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
* Description:   Declaration of the CSecModUIViewSignature class
*
*/


#ifndef SECMODUIVIEWSIGNATURE_H
#define SECMODUIVIEWSIGNATURE_H

// INCLUDES
#include "SecModUIViewBase.h"

// CONSTANTS

// FORWARD DECLARATIONS
class CSecModUIContainerSignature;
class CSecModUIModel;

// CLASS DECLARATION

/**
*  CSecModUIViewSignature view class.
* 
*/
class CSecModUIViewSignature : public CSecModUIViewBase
    {
    public: // Constructors and destructor

         /**
        * Two-phased constructor.
        */
        static CSecModUIViewSignature* NewLC(CSecModUIModel& aModel);

        /**
        * Destructor.
        */
        virtual ~CSecModUIViewSignature();

    public: // Functions from base classes
     
        /**
        * From MEikMenuObserver delegate commands from the menu
        * @param aCommand a command emitted by the menu 
        * @return void
        */
        virtual void HandleCommandL(TInt aCommand);
        
        /**
        * From CAknView returns Uid of View
        * @return TUid uid of the view
        */
        TUid Id() const;
        
        void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
        
        void UpdateCbaL(TInt aIndex);
        
        void SetCbaL( TInt aCbaResourceId );
        
   protected: // Functions from base classes
   
     
        virtual void DoActivateL(const TVwsViewId& aPrevViewId, TUid aCustomMessageId,
            const TDesC8& aCustomMessage);
            
        virtual void DoDeactivate();  
        
    private: // Functions from base classes
    
        void CreateContainerL();
        
        void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);
        
        void AddNaviPaneLabelL();
        
    private: // New funtions
        
        /**
        * Constructor
        */
        CSecModUIViewSignature(CSecModUIModel& aModel);

        /**
        * EPOC default constructor.
        */
        void ConstructL();

    private: // Data  
    
    };

#endif

// End of File
