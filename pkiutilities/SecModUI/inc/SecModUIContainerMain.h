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
* Description:   Declaration of the CSecModUIContainerMain class
*
*/


#ifndef SECMODUICONTAINERMAIN_H
#define SECMODUICONTAINERMAIN_H

// INCLUDES
#include "SecModUIContainerBase.h"
#include <coecntrl.h>
#include <eiklbo.h>
   
// FORWARD DECLARATIONS
class CEikLabel;        // for example labels
class CAknColumnListBox;
class CSecModUIModel;

// CLASS DECLARATION

/**
*  CSecModUIContainerMain  container control class.
*  
*/
class CSecModUIContainerMain : public CSecModUIContainerBase
    {
    public: // Constructors and destructor
    
        /**
        * Overrided Default constructor
        */
        CSecModUIContainerMain(CSecModUIModel& aModel);
        
        /**
        * Destructor.
        */
        virtual ~CSecModUIContainerMain();

    public: // New functions
       
    private: // Functions from base classes
           
        void SetupListItemsL();
        void ConstructListL();
        
    private: // New functions
    	 
    private: //data
      
    };

#endif  // SECMODUICONTAINERMAIN_H

// End of File
