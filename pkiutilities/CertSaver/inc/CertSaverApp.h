/*
* Copyright (c) 2003-2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Application class
*
*/


#ifndef CERTSAVERAPP_H
#define CERTSAVERAPP_H

#include <aknapp.h>

// CONSTANTS
// UID of the application
const TUid KUidCertSaver = { 0x100059D2 };

// CLASS DECLARATION

/**
* CCertSaverApp application class.
* Provides factory to create concrete document object.
*
*/
class CCertSaverApp : public CAknApplication
    {

    private:

        /**
        * From CApaApplication, creates CCertSaverDocument document object.
        * @return A pointer to the created document object.
        */
        CApaDocument* CreateDocumentL();

        /**
        * From CApaApplication, returns application's UID (KUidCertSaver).
        * @return The value of KUidCertSaver.
        */
        TUid AppDllUid() const;
    };

#endif  // CERTSAVERAPP_H

// End of File
