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



// INCLUDE FILES

#include "CTCertificateQuery.h"
#include "CTSecurityDialogsAO.h"

#include <CTSecDlgs.rsg>
#include <certmanui.rsg>
#include <aknmessagequerydialog.h>
#include <aknPopupHeadingPane.h>

// ================= MEMBER FUNCTIONS ==========================================

CCTCertificateQuery::CCTCertificateQuery(CCTSecurityDialogsAO& aAO):  
    CAknMessageQueryDialog( CAknQueryDialog::ENoTone ), 
    iNotifier(aAO)    
    {
    }

CCTCertificateQuery* CCTCertificateQuery::NewL(CCTSecurityDialogsAO& aAO)
    {
    CCTCertificateQuery* query = 
    new( ELeave ) CCTCertificateQuery(aAO);

    CleanupStack::PushL( query );
    query->ConstructL();
    CleanupStack::Pop();
    
    return query;
    }

void CCTCertificateQuery::ConstructL()
    {    
    iHeader = CEikonEnv::Static()->AllocReadResourceL( R_TEXT_RESOURCE_DETAILS_VIEW_HEADER );    
    SetHeaderTextL( *iHeader );    
    iMessage = iNotifier.CreateMessageL();    
    SetMessageTextL( *iMessage );          
    }

CCTCertificateQuery::~CCTCertificateQuery()
    {
    delete iHeader;
    delete iMessage;
    }

// -----------------------------------------------------------------------------
// CCTCertificateQuery::OkToExitL()
// -----------------------------------------------------------------------------

TBool CCTCertificateQuery::OkToExitL( TInt /*aButtonId*/ )
    {
    return ETrue;
    }

// end of file
