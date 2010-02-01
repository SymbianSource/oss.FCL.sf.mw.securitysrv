/*
* ============================================================================
*  Name         : SimLockUIApplication.h
*  Part of      : Sim Lock UI Application
*  Description  : SimLock UI Application header
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

#ifndef __SIMLOCKUI_APPLICATION_H__
#define __SIMLOCKUI_APPLICATION_H__

#include <aknapp.h> // CAknApplication
  
/**
 *  CSimLockUIApplication
 *  Sim Lock UI Application class.
 *
 *  An instance of CSimLockUIApplication is the application part of the AVKON
 *  application framework for the SimLockUI example application
 *
 *  @lib avkon.lib
 *  @lib eikcore.lib
 *  @lib eiksrv.lib
 */
class CSimLockUIApplication : public CAknApplication
    {
public:  // from CAknApplication

    /**
     * From CAknApplication
     * Utility function to return the application's UID.
     *
     * @return Application's UID (KUidPDApplicationApp).
     */
    TUid AppDllUid() const;

protected: // from CAknApplication

    /** 
     * CreateDocumentL
     *
     * Create a CApaDocument object and return a pointer to it
     * @return a pointer to the created document
     */
    CApaDocument* CreateDocumentL();
    };

#endif // __SIMLOCKUI_APPLICATION_H__

// end of file.
