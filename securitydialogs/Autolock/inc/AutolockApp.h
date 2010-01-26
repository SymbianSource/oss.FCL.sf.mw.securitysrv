/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description: 
*     Declares main application class.
*
*/


#ifndef AUTOLOCKAPP_H
#define AUTOLOCKAPP_H

// INCLUDES
#include <aknapp.h>

// CONSTANTS
// UID of the application
const TUid KUidAutolock = { 0x100059B5 };

// CLASS DECLARATION

/**
* CAutolockApp application class.
* Provides factory to create concrete document object.
* 
*/
class CAutolockApp : public CAknApplication
    {
    
    private:

        /**
        * From CApaApplication, creates CAutolockDocument document object.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();
        
        /**
        * From CApaApplication, returns application's UID (KUidAutolock).
        * @return The value of KUidAutolock.
        */
        TUid AppDllUid() const;
    };

#endif

// End of File

