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
* Description:   Declaration of the CSecModUIViewMain class
*
*/


#ifndef SECMODUIVIEWMAIN_H
#define SECMODUIVIEWMAIN_H

// INCLUDES
#include "SecModUIViewBase.h"
#include <aknview.h>


// CONSTANTS


// FORWARD DECLARATIONS
class CSecModUIContainerMain;
class CSecModUIModel;

// CLASS DECLARATION

/**
*  CSecModUIViewMain view class.
* 
*/
class CSecModUIViewMain : public CSecModUIViewBase
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CSecModUIViewMain* NewLC(CSecModUIModel& aModel);

        /**
        * Destructor.
        */
        virtual ~CSecModUIViewMain();

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
    
        void HandleListBoxEventL(
            CEikListBox* aListBox, 
            TListBoxEvent aEventType);
    
    private: // Functions from base classes
    
        
        /**
        * From CAknView activate the view
        * @param aPrevViewId 
        * @param aCustomMessageId 
        * @param aCustomMessage 
        * @return void
        */
        void DoActivateL(
            const TVwsViewId& aPrevViewId,
            TUid aCustomMessageId,
            const TDesC8& aCustomMessage);
        
        /**
        * From CSecModUIViewBase create container
        * @return void
        */    
        void CreateContainerL();
        
        /**
        * From CSecModUIViewBase set title pane
        * @return void
        */    
        void SetTitlePaneL();
        
        /**
        * Constructor
        */
        CSecModUIViewMain(CSecModUIModel& aModel);

        /**
        * EPOC default constructor.
        */
        void ConstructL();
        
        /**
        * From MEikMenuObserver
		* Updates Options list with correct items depending on 
		* whether the listbox is empty or if it has any marked items
        */
		void DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane);
		
		void UpdateCbaL();

        void SetCbaL( TInt aCbaResourceId );
        
        void HandleForegroundEventL(TBool aForeground);
        
    private: // Data
        
    };

#endif // SECMODUIVIEWMAIN_H

// End of File
