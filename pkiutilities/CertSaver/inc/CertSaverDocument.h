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
* Description:   Document class
*
*/


#ifndef CERTSAVERDOCUMENT_H
#define CERTSAVERDOCUMENT_H

// INCLUDES
#include "certparser.h"
#include <eikdoc.h>

// CONSTANTS

// FORWARD DECLARATIONS
class  CEikAppUi;
class CShutterAO;

// CLASS DECLARATION

/**
*  CCertSaverDocument document class.
*/
class CCertSaverDocument : public CEikDocument
    {
    public: // Constructors and destructor
        /**
        * Two-phased constructor.
        */
        static CCertSaverDocument* NewL( CEikApplication& aApp );

        /**
        * Destructor.
        */
        virtual ~CCertSaverDocument();

    public: // New functions

        /**
        * Returns reference to parser.
        */
        CCertParser& Parser() const;

    public: // Functions from base classes

        /**
        * From CEikDocument class.
        * When file-based epoc application started, Uikon framework
        * uses OpenFileL function to open the file with file name
        */
        CFileStore* OpenFileL( TBool aDoOpen, const TDesC& aFilename, RFs& aFs );

        /**
        * From CEikDocument class.
        * When file-based epoc application started, Uikon framework
        * uses OpenFileL function to open the file
        */
        void OpenFileL( CFileStore*& aFileStore, RFile& aFile );

    protected:  // New functions

    protected:  // Functions from base classes

    private:

        /**
        * EPOC default constructor.
        */
        CCertSaverDocument( CEikApplication& aApp ): CEikDocument( aApp ) { }
        void ConstructL();

    private:

        /**
        * From CEikDocument, create CCertSaverAppUi "App UI" object.
        */
        CEikAppUi* CreateAppUiL();

  private:    // Data

        CCertParser* iParser;
        CShutterAO* iAO;
    };


// CLASS DECLARATION

/**
*  CShutterAO document class.
*  Active object used to shut down application.
*/

class CShutterAO : public CActive
    {
    public:
        /**
        * Constructor.
        */
        CShutterAO();

        /**
        * Constructor.
        */
        virtual ~CShutterAO();

        /**
        * Exits the application in RunL
        */
        void Exit();

    private:
        void DoCancel();
        void RunL();
    };

#endif  // CERTSAVERDOCUMENT_H

// End of File
