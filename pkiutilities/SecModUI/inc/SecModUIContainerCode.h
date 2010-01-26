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
* Description:   Declaration of the CSecModUIContainerCode class
*
*/


#ifndef SECMODUICONTAINERCODE_H
#define SECMODUICONTAINERCODE_H

// INCLUDES
#include "SecModUIContainerBase.h"
 
// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  CSecModUIContainerCode  container control class.
*  
*/
class CSecModUIContainerCode : public CSecModUIContainerBase
    {
    public: // Constructors and destructor
    
        /**
        * Overrided Default constructor
        */
        CSecModUIContainerCode(CSecModUIModel& aModel);

        /**
        * Destructor.
        */
        virtual ~CSecModUIContainerCode();

    public: // New functions

    private: // Functions from base classes
        
        void SetupListItemsL();
        void CreateResourceReaderLC(TResourceReader& aReader);	
                   
    private: // New functions
    	 
    private: //data
 
   };

#endif // SECMODUICONTAINERCODE_H

// End of File
