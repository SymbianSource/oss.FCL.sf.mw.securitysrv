/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Sleeping note with timeout and a reference flag that informs
 *                the parent when the note is visible
 *
*/


#include "lockappsleepingnote.h"

#ifndef __LOCKAPPLOCKEDNOTE_H__
#define __LOCKAPPLOCKEDNOTE_H__

const TInt KNoteCmdFocusLost = 3100;

/**
 *  CLockAppLockedNote class is used as the basic dismissable note in Lockapp.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Joona Petrell
 *  @author Tamas Koteles
 */
class CLockAppLockedNote : public CLockAppSleepingNote
    {
    public:

        /**
         * C++ default constructor.
         *
         * @param aCommandObserver pointer to parent implementing observer interface
         */
        CLockAppLockedNote( MEikCommandObserver* aCommandObserver = NULL );

        /**
         * Destructor.
         */
        ~CLockAppLockedNote( );

        /**
         * Locked note can be canceled.
         */
        void CancelNote( );

        /**
         * From @c CCoeControl. Used to inform parent that
         * dialog is no longer in focus.
         */
        void FocusChanged( TDrawNow aDrawNow );

    };

#endif // __LOCKAPPLOCKEDNOTE_H__
