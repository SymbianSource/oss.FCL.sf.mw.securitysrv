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
*
*/



#ifndef __SECNOT_CERTIFICATE_QUERY_H__
#define __SECNOT_CERTIFICATE_QUERY_H__

//  INCLUDES

#include <aknmessagequerydialog.h>

// FORWARD DECLARATIONS

class CCTSecurityDialogsAO;

// CLASS DECLARATION

/**
*  Query class for showing certificate details
*/
NONSHARABLE_CLASS( CCTCertificateQuery ) : public CAknMessageQueryDialog
    {
    public:     // constructors and destructor
        static CCTCertificateQuery* NewL(CCTSecurityDialogsAO& aAO);
        virtual ~CCTCertificateQuery();

    private:    // from CEikDialog
        TBool OkToExitL( TInt aButtonId );

    private:    // new functions
        CCTCertificateQuery(CCTSecurityDialogsAO& aAO);
        void ConstructL();

    private:    // data

        CCTSecurityDialogsAO& iNotifier;        
        HBufC* iHeader;
        HBufC* iMessage;
    };

#endif  // __SECNOT_CERTIFICATE_QUERY_H__

// End of file
