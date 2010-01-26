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
* Description:   Declaration of the CSecModUIViewAccess class
*
*/


#ifndef SECMODUIVIEWACCESS_H
#define SECMODUIVIEWACCESS_H

// INCLUDES
#include "SecModUIViewBase.h"

enum
    {
    EIndexCodeLabel = 0,
    EIndexCodeRequest,
    EIndexCodeStatus 
    };
                
// FORWARD DECLARATIONS
class CSecModUIContainerAccess;
class CSecModUIModel;

// CLASS DECLARATION

/**
*  CSecModUIViewAccess view class.
* 
*/
class CSecModUIViewAccess : public CSecModUIViewBase
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CSecModUIViewAccess* NewLC(CSecModUIModel& aModel);

        /**
        * Destructor.
        */
        virtual ~CSecModUIViewAccess();

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
        
        void UpdateCbaL();
        
        void SetCbaL( TInt aCbaResourceId );
        
        virtual void HandleForegroundEventL(TBool aForeground);
        
   protected: // Functions from base classes
   
        /**
        * From CAknView activate the view
        * @param aPrevViewId 
        * @param aCustomMessageId 
        * @param aCustomMessage 
        * @return void
        */
        virtual void DoActivateL(const TVwsViewId& aPrevViewId, TUid aCustomMessageId,
            const TDesC8& aCustomMessage);
            
        /**
        * From CAknView deactivate the view (free resources)
        * @return void
        */
        virtual void DoDeactivate();  
           
    private: // Functions from base classes
    
        
        void CreateContainerL();
        
        void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);
        
        void AddNaviPaneLabelL();
        
    private: // New functions
    
        void HandleChangeL();
        
        void UpdateListBoxItemL(TInt aIndex);
        
        void HandleEnterKeyL(TInt aIndex);
        
        /**
        * Constructor
        */
        CSecModUIViewAccess(CSecModUIModel& aModel);

        /**
        * EPOC default constructor.
        */
        void ConstructL();

    private: // Data   
       
    };

#endif // SECMODUIVIEWACCESS_H

// End of File
