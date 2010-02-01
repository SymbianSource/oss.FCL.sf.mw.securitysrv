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
* Description:  Implementation of CGbaUtility
*
*/

#include <GbaUtility.h>
#include "GbaClientrequesthandler.h"
#include "GBALogger.h"

// -----------------------------------------------------------------------------
// CGbaUtility::NewL()
// -----------------------------------------------------------------------------
//
EXPORT_C CGbaUtility* CGbaUtility::NewL(MGbaObserver& aObserver)
    {
    CGbaUtility* self = CGbaUtility::NewLC(aObserver);
    CleanupStack::Pop( self );
    return(self) ;
    }


// -----------------------------------------------------------------------------
// CGbaUtility::NewLC()
// -----------------------------------------------------------------------------
//
EXPORT_C CGbaUtility* CGbaUtility::NewLC(MGbaObserver& aObserver)
    {
    CGbaUtility* self = new (ELeave) CGbaUtility();
    CleanupStack::PushL(self);
    self->ConstructL( aObserver );
    return self; 
    }


// -----------------------------------------------------------------------------
// CGbaUtility::NewL()
// factory function without observer
// -----------------------------------------------------------------------------
//
EXPORT_C CGbaUtility* CGbaUtility::NewL()
    {
    CGbaUtility* self = CGbaUtility::NewLC();
    CleanupStack::Pop( self );
    return(self) ;
    }


// -----------------------------------------------------------------------------
// CGbaUtility::NewLC()
// factory function without observer
// -----------------------------------------------------------------------------
//
EXPORT_C CGbaUtility* CGbaUtility::NewLC()
    {
    CGbaUtility* self = new (ELeave) CGbaUtility();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self; 
    }

// -----------------------------------------------------------------------------
// CGbaUtility::ConstructL()
// -----------------------------------------------------------------------------
//
void CGbaUtility::ConstructL( MGbaObserver& aObserver )
    {
    iGbaRequestHandler = CGbaClientRequestHandler::NewL( aObserver );
    }



// -----------------------------------------------------------------------------
// CGbaUtility::ConstructL()
// -----------------------------------------------------------------------------
//
void CGbaUtility::ConstructL()
    {
    iGbaRequestHandler = CGbaClientRequestHandler::NewL();
    }


// -----------------------------------------------------------------------------
// CGbaUtility::CGbaUtility()
// -----------------------------------------------------------------------------
//
CGbaUtility::CGbaUtility()
    {
    
    }


// -----------------------------------------------------------------------------
// CGbaUtility::~CGbaUtility()
// -----------------------------------------------------------------------------
//
CGbaUtility::~CGbaUtility()
    {
    delete iGbaRequestHandler;
    }


// -----------------------------------------------------------------------------
// CGbaUtility::Bootstrap()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CGbaUtility::Bootstrap( const TGBABootstrapInputParams aInput, TGBABootstrapOutputParams& aOutput )
    {
    return iGbaRequestHandler->Bootstrap( aInput, aOutput );
    }


// -----------------------------------------------------------------------------
// CGbaUtility::Bootstrap()
// -----------------------------------------------------------------------------
//
EXPORT_C void CGbaUtility::Bootstrap( const TGBABootstrapInputParams aInput, TGBABootstrapOutputParams& aOutput, TRequestStatus& aStatus )
    {
    iGbaRequestHandler->Bootstrap( aInput, aOutput, aStatus );    
    }    


// -----------------------------------------------------------------------------
// CGbaUtility::CancelBootstrap()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CGbaUtility::CancelBootstrap()
    {
    return iGbaRequestHandler->CancelBootstrap();
    }



// -----------------------------------------------------------------------------
// CGbaUtility::SetBSFAddress()
// -----------------------------------------------------------------------------
//
EXPORT_C TInt CGbaUtility::SetBSFAddress( const TDesC8& aAddress )
    {
    return iGbaRequestHandler->SetBSFAddress(aAddress);
    }


//EOF
