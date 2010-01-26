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
* Description:   Declaration of the CSecModUIViewBase class
*
*/


#ifndef SECMODUIVIEWBASE_H
#define SECMODUIVIEWBASE_H

// INCLUDES
#include <aknview.h>
#include <eiklbo.h>

// CONSTANTS
enum TCurrentMSK
     {
     EEmpty,
     EChange,
     EUnblock,
     EClose
     };
     
// FORWARD DECLARATIONS
class CSecModUIContainerBase;
class CSecModUIModel;
class CAknNavigationDecorator;

// CLASS DECLARATION

/**
*  CSecModUIViewBase view class.
* 
*/
class CSecModUIViewBase : public CAknView, public MEikListBoxObserver
    {
    public: // Constructors and destructor
                
        /**
        * Constructor
        */
        CSecModUIViewBase(CSecModUIModel& aModel);
        
        /**
        * Destructor.
        */
        virtual ~CSecModUIViewBase();

    public: // Functions from CAknView 

        /**
        * From MEikMenuObserver delegate commands from the menu
        * @param aCommand a command emitted by the menu 
        * @return void
        */
        virtual void HandleCommandL(TInt aCommand);

        /**
        * From CAknView reaction if size change
        * @return void
        */
        virtual void HandleClientRectChange();
      
    public: // Function from MEikListBoxObserver
    
        void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);
        
    protected:
    
        virtual void CreateContainerL()=0;
        
        virtual void SetTitlePaneL();
        
        virtual void AddNaviPaneLabelL();        
        
        virtual void DoAddNaviPaneL(TInt aResource = 0);
        
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

    protected:
        CSecModUIContainerBase* iContainer;
        CSecModUIModel& iModel;    
        TVwsViewId  iPrevViewId;
    
    private: // Data
        /**
        * For Navipane
        */
        CAknNavigationDecorator*		iNaviDecorator;        
 
        /**
        * To store the position of the focus in the Authority view
        */
        TInt    iCurrentPosition;

        /**
        * Stores top item in the listbox
        */
        TInt  iTopItem;     
    };

#endif // SECMODUIVIEWBASE_H

// End of File
