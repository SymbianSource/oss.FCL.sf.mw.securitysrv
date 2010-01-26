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
* Description:   Application UI class
*
*/


#ifndef     CERTMANUI_H
#define     CERTMANUI_H

//  INCLUDES
#include <aknview.h>
#include <ConeResLoader.h>

// CLASS DECLARATION

/**
*  CertManUI application UI class.
*
*  @lib certmanui.lib
*/
NONSHARABLE_CLASS( CCertManUi ): public CCoeControl
  {
  public: // functions

        /**
        * Initializes CCertManUi resource file
        */
    static void InitializeLibL();

        /**
        * Uninitializes CCertManUi resource file
        */
        static void UnInitializeLib();

  private: // functions

        /**
        * C++ default constructor
        */
    CCertManUi();

        /**
        * Destructor
        */
    ~CCertManUi();

        /**
        * EPOC default constructor
        */
    void ConstructL();

        /**
        * Used in ConstructL to get resource file
        */
    void AddResourceFileL();

  private: // data

        /**
        * To get hold of iResourceLoader
        */
        RConeResourceLoader       iResourceLoader;
  };

#endif //   CERTMANUI_H

// End of File
