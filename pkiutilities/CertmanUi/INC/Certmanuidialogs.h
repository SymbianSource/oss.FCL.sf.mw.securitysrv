/*
* Copyright (c) 2003-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Declaration of the CCertManUIWaitDialog class
*
*/

#ifndef CERTMANUIDIALOGS_H
#define CERTMANUIDIALOGS_H

#include <e32base.h>

class CAknGlobalNote;
class CAknGlobalConfirmationQuery;
class CAknWaitDialog;

/**
 *  CCertManUIWaitDialog can be used to display deleting wait note,
 *  or progress wait note.
 */
class CCertManUIWaitDialog: public CBase
    {
    public: // functions
        /**
         * Default constructor.
         */
        CCertManUIWaitDialog();

        /**
         * Destructor.
         */
        ~CCertManUIWaitDialog();

        /**
         * Display wait dialog.
         * @param Selector for progress or delete dialog, must be either
         * ECertmanUiDeleteDialog or ECertmanUiWaitDialog.
         */
        void StartWaitDialogL( TInt aDialogSelector );

        /**
         * Closes dialog.
         */
        void CloseWaitDialogL();

    private:    // Data
        CAknWaitDialog* iDialog;
        TInt iOpenCount;
    };

#endif  // CERTMANUIDIALOGS_H

