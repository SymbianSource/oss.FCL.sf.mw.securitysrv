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
* Description:   Declaration of the CCertManUIWaitDialog class
*
*/


#ifndef     CERTMANUIDIALOGS_H
#define     CERTMANUIDIALOGS_H

// INCLUDES
#include <e32base.h>

// FORWARD DECLARATIONS

class CAknGlobalNote;
class CAknGlobalConfirmationQuery;
class CAknWaitDialog;

// CLASS DECLARATION

/**
*  CCertManUIKeeper view control class.
*
*
*/
class CCertManUIWaitDialog: public CBase
    {
    public: // functions

        /**
        * Default Constructor
        */
        CCertManUIWaitDialog();

        /**
        * Destructor
        */
        ~CCertManUIWaitDialog();

        /**
        * Display dialog
        * @param selector for progress or delete dialog
        */
        void StartWaitDialogL(TInt aDialogSelector);

        /**
        * Close dialog.
        */

        void CloseWaitDialogL();


    private:    // Functions


        static void CleanupWaitDialog(TAny* aAny);


    private:    // Data

        /**
        * For wait note
        */
        CAknWaitDialog*     iDialog;

        /**
        *  Flag whether dialof is being displayed
        */
        TBool               iDisplayed;

    };

#endif //   CERTMANUIDIALOGS_H

// End of File