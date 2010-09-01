/*
* Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
*
*/



#ifndef CTNOTRUSTQUERY_H
#define CTNOTRUSTQUERY_H

//  INCLUDES

#include <AknQueryDialog.h>

// FORWARD DECLARATIONS

class CCTSecurityDialogsAO;
class CAknSinglePopupMenuStyleListBox;

// CLASS DECLARATION

/**
*  Notifier class for showing SSL security dialogs
*/
NONSHARABLE_CLASS( CCTNoTrustQuery ): public CAknQueryDialog
    {
    public:     // constructors and destructor

        CCTNoTrustQuery(
            CCTSecurityDialogsAO& aNotifier,
            TBool& aRetVal,
            TRequestStatus& aClientStatus,
            HBufC* aServerName,
            TBool aShowPermAccept,
            TBool& aIsDeleted );
            
        virtual ~CCTNoTrustQuery();

    private:    // from CEikDialog
        TBool OkToExitL( TInt aButtonId );
        void PostLayoutDynInitL();

    private: // New functions
        TBool OptionsMenuL();

    private:    // data
        CCTSecurityDialogsAO& iNotifier;
        TBool& iRetVal; 		//Dialog response
        TRequestStatus* iClientStatus;
        HBufC* iServerName;
        TBool iShowPermAccept;
        TBool& iDeleted;
    };

#endif  // CTNOTRUSTQUERY_H

