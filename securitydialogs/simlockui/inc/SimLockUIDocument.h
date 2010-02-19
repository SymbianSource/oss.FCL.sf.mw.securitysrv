/*
* ============================================================================
*  Name         : SimLockUIDocument.h
*  Part of      : Sim Lock UI Application
*  Description  : Create session to ETel and owns SimLock UI Delegate
*  Version      : 
*  
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Build info file for Ado domain appinstall 
* ============================================================================
*/

#ifndef __SIMLOCKUI_DOCUMENT_H__
#define __SIMLOCKUI_DOCUMENT_H__

// INCLUDES
#include <AknDoc.h>
#include <rmmcustomapi.h> // RMmCustomAPI, RTelServer, RMobilePhone

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

class CSimLockUIAppUi;
class CEikApplication;
class CSimLockDataHandlingDelegate;


/**
 * CSimLockUIDocument
 * An instance of class CSimLockUIDocument is the Document part of the AVKON
 * application framework for the SimLockUI example application
 *
 * @lib avkon.lib
 * @lib eikcore.lib
 * @lib eiksrv.lib
 */
class CSimLockUIDocument : public CAknDocument
    {
public: // Public Constructors/Destructor

    /**
     * NewL
     * Construct a CSimLockUIDocument for the AVKON application aApp
     * using two phase construction, and return a pointer to the created object
     * @param aApp application creating this document
     * @return a pointer to the created instance of CSimLockUIDocument
     */
    static CSimLockUIDocument* NewL( CEikApplication& aApp );

     /**
      * NewLC
      * Construct a CSimLockUIDocument for the AVKON application aApp
      * using two phase construction, and return a pointer to the created object,
      * leaving an instance on the Cleanup Stack.
      *
      * @param aApp application creating this document
      * @return a pointer to the created instance of CSimLockUIDocument
      */
    static CSimLockUIDocument* NewLC( CEikApplication& aApp );

     /**
      * ~CSimLockUIDocument
      * Destroy the object and release all memory objects
      */
    virtual ~CSimLockUIDocument();

public: // from CAknDocument

     /**
      * CreateAppUiL
      * Create a CSimLockUIAppUi object and return a pointer to it
      *
      * @return a pointer to the created instance of the AppUi created
      */
    virtual CEikAppUi* CreateAppUiL();

private: // Private Constructors

     /**
      * ConstructL
      * Perform the second phase construction of a CSimLockUIDocument object
      */
    void ConstructL();

     /**
      * CSimLockUIDocument
      * Perform the first phase of two phase construction
      *
      * @param aApp application creating this document
      */
    CSimLockUIDocument( CEikApplication& aApp );


private: // Member Data

     /**
      * Handle to ETel Server
      */
    RTelServer   iServer;

    /**
     * Handle to ETel Mobile Phone Object
     */
    RMobilePhone iPhone;

    /**
     * Handle to ETel Custom API Object
     */
    RMmCustomAPI iCustomPhone;

    /**
     * Handle to Sim Lock Delegate
     * owns
     */
    CSimLockDataHandlingDelegate* iSimLockDelegate;
    };


#endif // __SIMLOCKUI_DOCUMENT_H__

// end of file.

