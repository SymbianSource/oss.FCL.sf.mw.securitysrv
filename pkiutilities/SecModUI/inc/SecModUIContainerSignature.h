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
* Description:   Declaration of the CSecModUIContainerSignature class
*
*/


#ifndef SECMODUICONTAINERSIGNATURE_H
#define SECMODUICONTAINERSIGNATURE_H

// INCLUDES
#include "SecModUIContainerBase.h"
#include "SecModUIViewSignature.h"
   
// FORWARD DECLARATIONS


// CLASS DECLARATION

/**
*  CSecModUIContainerSignature  container control class.
*  
*/
class CSecModUIContainerSignature : public CSecModUIContainerBase, public MCoeControlObserver
    {
    public: // Constructors and destructor
        
        /**
        * Overrided Default constructor
        */
        CSecModUIContainerSignature(CSecModUIModel& aModel, CSecModUIViewSignature* aView );

        /**
        * Destructor.
        */
        virtual ~CSecModUIContainerSignature();
    
    public: // Function from CSecModUIContainerBase    
        void SetupListItemsL();
        
        // from MCoeControlObserver
        void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType); 

    private: // Functions from base classes        
        void ConstructListL();             
           
    private: //data
        CSecModUIViewSignature* iView; //Not owned
    };

#endif // SECMODUICONTAINERSIGNATURE_H

// End of File
