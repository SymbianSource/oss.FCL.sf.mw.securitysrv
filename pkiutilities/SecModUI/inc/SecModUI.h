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
* Description:   Interface of the Security Module UI DLL
*
*/


#ifndef SECMODUI_H
#define SECMODUI_H

//  INCLUDES
#include <e32base.h>

// CONSTANTS
const TUid KSecModUIViewMainId = {22};
const TUid KSecModUIViewCodeId = {23};
const TUid KSecModUIViewAccessId = {45};
const TUid KSecModUIViewSignatureId = {46};


// CLASS DECLARATION

/**
*  Interface of the Security Module UI.
*  Creates and deletes instance of the Security Module UI.
*
*  @lib SecModUI.lib
*  @since Series 60 3.0
*/
class MSecModUI
    {
    public:  // Constructors and destructor
        
        /**
        * Creates an instance of a subclass of MSecModUI.
        *
        * @return	An object that implements MSecModUI destructor.
        */
        IMPORT_C static MSecModUI* CreateL();
        
        /**
        * Destructor.
        */
        virtual ~MSecModUI() {};

    };
    
#endif      // SECMODUI_H   
            
// End of File
