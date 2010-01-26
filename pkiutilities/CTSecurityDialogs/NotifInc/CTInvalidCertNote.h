/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Info note shown for invalid and revoked certificates
*
*/



#ifndef CTINVALIDCERTNOTE_H
#define CTINVALIDCERTNOTE_H

#include <aknnotedialog.h>

class CCTSecurityDialogsAO;


/**
*  Notifier class for showing SSL security dialogs
*/
NONSHARABLE_CLASS( CCTInvalidCertificateNote ): public CAknNoteDialog
    {
    public:     // constructors and destructor
        CCTInvalidCertificateNote(
            CCTSecurityDialogsAO& aNotifier,
            TRequestStatus& aClientStatus );
        ~CCTInvalidCertificateNote();

    private:    // from CEikDialog
        TBool OkToExitL( TInt aButtonId );
        void PostLayoutDynInitL();
        
    private: // New functions
        void ShowDetailsL();

    private:    // data
        CCTSecurityDialogsAO& iNotifier;
        TRequestStatus* iClientStatus;
    };

#endif  // CTINVALIDCERTNOTE_H

// End of file
