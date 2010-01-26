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
*     Declares document for application.
*
*/


#ifndef AUTOLOCKDOCUMENT_H
#define AUTOLOCKDOCUMENT_H

// INCLUDES
#include <AknDoc.h>
// CONSTANTS

// FORWARD DECLARATIONS
class  CEikAppUi;

// CLASS DECLARATION

/**
*  CAutolockDocument application class.
*/
class CAutolockDocument : public CAknDocument 
    {
    public: // Constructors and destructor
        /**
        * Two-phased constructor.
        */
        static CAutolockDocument* NewL(CEikApplication& aApp);

        /**
        * Destructor.
        */
        virtual ~CAutolockDocument();

    public: // From CEikDocument 
		/**
        * Updates task name --> Sets autolock hidden
        *
		* @param aWgName CApaWindowGroupName*
		*/
		void UpdateTaskNameL( CApaWindowGroupName* aWgName );

    private:

        /**
        * Symbian OS default constructor.
        */
        CAutolockDocument(CEikApplication& aApp): CAknDocument(aApp) { }
        void ConstructL();

    private:

        /**
        * From CEikDocument, create CAutolockAppUi "App UI" object.
        */
        CEikAppUi* CreateAppUiL();
    };

#endif

// End of File

