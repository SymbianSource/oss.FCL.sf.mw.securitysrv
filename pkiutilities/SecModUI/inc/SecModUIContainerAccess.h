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
* Description:   Declaration of the CSecModUIContainerAccess class
*
*/


#ifndef SECMODUICONTAINERACCESS_H
#define SECMODUICONTAINERACCESS_H

// INCLUDES
#include "SecModUIContainerBase.h"
#include "SecModUIViewAccess.h" 
// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  CSecModUIContainerAccess  container control class.
*  
*/
class CSecModUIContainerAccess : public CSecModUIContainerBase, public MCoeControlObserver
    {
    public: // Constructors and destructor
    
        /**
        * Overrided Default constructor
        */
        CSecModUIContainerAccess(CSecModUIModel& aModel, CSecModUIViewAccess* aView );

        /**
        * Destructor.
        */
        virtual ~CSecModUIContainerAccess();
         
       
        //from MCoeControlObserver
        void HandleControlEventL(CCoeControl* aControl,TCoeEvent aEventType); 

    
    private: // Functions from base classes
        void SetupListItemsL();
        void ConstructListL();
    private: 
        CSecModUIViewAccess* iView;     
   };
#endif

// End of File
