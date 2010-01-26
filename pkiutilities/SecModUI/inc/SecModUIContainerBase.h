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
* Description:   Declaration of the CSecModUIContainerBase class
*
*/


#ifndef SECMODUICONTAINERBASE_H
#define SECMODUICONTAINERBASE_H

// INCLUDES
#include <coecntrl.h>
#include "SecModUISyncWrapper.h"   
// FORWARD DECLARATIONS
class CEikLabel;        // for example labels
//class CAknColumnListBox;
class CEikTextListBox;
class CSecModUIModel;

// CLASS DECLARATION

/**
*  CSecModUIContainerBase  container control class.
*  
*/
class CSecModUIContainerBase : public CCoeControl
    {
    public: // Constructors and destructor
    
        /**
        * Overrided Default constructor
        */
        CSecModUIContainerBase(CSecModUIModel& aModel, const TDesC& aContextName);
        
        /**
        * EPOC default constructor.
        * @param aRect Frame rectangle for container.
        */
        virtual void ConstructL(const TRect& aRect);

 
        /**
        * Destructor.
        */
        virtual ~CSecModUIContainerBase();

    public: // New functions
            
        CEikTextListBox& ListBox();
        virtual void SetupListItemsL();
        
        //CCoeControl
        void FocusChanged( TDrawNow aDrawNow );

    protected: // New functions
    	
    	virtual void CreateListL();
    	virtual void CreateResourceReaderLC(TResourceReader& aReader);	
    	virtual void ConstructListL();
    

    protected: // Functions from base classes

        /**
        * From CoeControl,SizeChanged.
        */
        virtual void SizeChanged();

        /**
        * From CoeControl,CountComponentControls.
        */
        virtual TInt CountComponentControls() const;

        /**
        * From CCoeControl,ComponentControl.
        */
        virtual CCoeControl* ComponentControl(TInt aIndex) const;
        
        /**
        * From CCoeControl, OfferKeyEventL.
        */
        virtual TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);
 
        /**
        * From CCoeControl, GetHelpContext.
        */
        void GetHelpContext(TCoeHelpContext& aContext) const;
        
        /**
        * Handles resource change
        */
		void HandleResourceChange(TInt aType);
                
    protected:        
        CEikTextListBox*    iListBox;
    	CSecModUIModel& iModel;       
    private: //data
        TCoeContextName iContextName;
    
    	
        
    };

#endif  // SECMODUICONTAINERMAIN_H

// End of File
