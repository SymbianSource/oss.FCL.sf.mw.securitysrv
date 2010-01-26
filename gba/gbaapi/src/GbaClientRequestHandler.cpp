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
* Description:  Implementation of CGbaClientRequestHandler
*
*/

#include <e32svr.h>
#include "GbaClientrequesthandler.h"
#include "GbaServerSession.h"
#include "GBALogger.h"
#include "GbaCommon.h"


// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::NewL()
// -----------------------------------------------------------------------------
//
CGbaClientRequestHandler* CGbaClientRequestHandler::NewL(MGbaObserver& aObserver)
    {
    CGbaClientRequestHandler* self = CGbaClientRequestHandler::NewLC(aObserver);
    CleanupStack::Pop( self );
    return(self) ;
    }


// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::NewLC()
// -----------------------------------------------------------------------------
//
CGbaClientRequestHandler* CGbaClientRequestHandler::NewLC(MGbaObserver& aObserver)
    {
    CGbaClientRequestHandler* self = new (ELeave) CGbaClientRequestHandler(aObserver);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self; 
    }


// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::NewL()
// -----------------------------------------------------------------------------
//
CGbaClientRequestHandler* CGbaClientRequestHandler::NewL()
    {
    CGbaClientRequestHandler* self = NewLC();
    CleanupStack::Pop( self );
    return(self) ;
    }


// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::NewLC()
// -----------------------------------------------------------------------------
//
CGbaClientRequestHandler* CGbaClientRequestHandler::NewLC()
    {
    CGbaClientRequestHandler* self = new (ELeave) CGbaClientRequestHandler();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self; 
    }


// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::ConstructL()
// -----------------------------------------------------------------------------
//
void CGbaClientRequestHandler::ConstructL()
    {
    iSession = new(ELeave) RGbaServerSession();
    User::LeaveIfError(iSession->Connect());
    }


// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::CGbaClientRequestHandler()
// -----------------------------------------------------------------------------
//
CGbaClientRequestHandler::CGbaClientRequestHandler(MGbaObserver& aObserver)
:   CActive(EPriorityStandard),
    iObserver(&aObserver),
    iBootstrapState(EBootstrapNone)
    {
    CActiveScheduler::Add(this);
    }


// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::CGbaClientRequestHandler()
// -----------------------------------------------------------------------------
//
CGbaClientRequestHandler::CGbaClientRequestHandler()
:   CActive(EPriorityStandard),
    iBootstrapState(EBootstrapNone)
    {
    iObserver = NULL;
    CActiveScheduler::Add(this);
    }


// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::~CGbaClientRequestHandler()
// -----------------------------------------------------------------------------
//
CGbaClientRequestHandler::~CGbaClientRequestHandler()
    {
    if(IsActive())
        {
        Cancel();
        }
    if ( iSession )
        {
        iSession->Close();
        }
    delete iSession;
    }


// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::RunL()
// -----------------------------------------------------------------------------
//
void CGbaClientRequestHandler::RunL()
    {
    GBA_TRACE_DEBUG_NUM(("CGbaClientRequestHandler::RunL iStatus=%d"),iStatus.Int());
    
    // only when the observer is in use, the runl function should be called.
    // if observer in runl is NULL, that means the factory function and bootstrap function
    // that user used that doesnot match.
    if( !iObserver )
        {
        return;
        }
	if( iStatus.Int() == KErrCancel && (iBootstrapState == EBootstrapCancelling))
		  {
		  iBootstrapState = EBootstrapNone;
		  return;
		  }
    if( iStatus.Int() < KErrNone )
        {
        iObserver->BootstrapComplete( iStatus.Int() );
        return;
        }
    
    switch (iBootstrapState)
        {
        case EBootstrapping:
            {
            GBA_TRACE_DEBUG(("BTID"));
            GBA_TRACE_DEBUG_BINARY(iGbaOutputParams->iBTID);
            GBA_TRACE_DEBUG(("Ks_NAF"));
            GBA_TRACE_DEBUG_BINARY(iGbaOutputParams->iKNAF);
            iBootstrapState = EBootstrapNone;
            iObserver->BootstrapComplete( KErrNone );
            GBA_TRACE_DEBUG(("Ks_NAF 2"));
            }
            break ;

        default:
            {
            GBA_TRACE_DEBUG(("Unexpected state"));
            iBootstrapState = EBootstrapNone;
            iObserver->BootstrapComplete( KErrUnknown );
            }
            break ;
        }
    }
    
    
// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::RunError()
// -----------------------------------------------------------------------------
//
TInt CGbaClientRequestHandler::RunError(TInt aError)
    {
    //when bootstrap fails, the error value from server
    //will be passed back to client.
    GBA_TRACE_DEBUG_NUM(("CGbaClientRequestHandler::RunError =%d"),aError );    
    if (iBootstrapState == EBootstrapping)
        {
        GBA_TRACE_DEBUG(("Call HandleUnexpected") );
        iObserver->BootstrapComplete(aError);
        GBA_TRACE_DEBUG(("Call HandleUnexpected End") );
        } 
    return KErrNone;       
    }
   

// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::DoCancel()
// -----------------------------------------------------------------------------
//
void CGbaClientRequestHandler::DoCancel()
    {
    if ( iBootstrapState == EBootstrapping )
        {      
        CancelBootstrap();
        }
    }


// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::Bootstrap()
// -----------------------------------------------------------------------------
//
TInt CGbaClientRequestHandler::Bootstrap( const TGBABootstrapInputParams aInput, TGBABootstrapOutputParams& aOutput )
    {
    if ( !iObserver )
        {
        return KErrGeneral;
        }
    
    if ( iBootstrapState == EBootstrapping )
        {
        return KErrInUse;
        }
    
    iGbaInputParams.iNAFName.Copy(aInput.iNAFName);
    iGbaInputParams.iFlags = aInput.iFlags;
    iGbaInputParams.iAPID = aInput.iAPID;
    iGbaInputParams.iProtocolIdentifier.Copy(aInput.iProtocolIdentifier);
    iGbaInputParams.iUICCLabel.Copy(aInput.iUICCLabel);
        
    iGbaOutputParams = &aOutput;
    
    iBootstrapState = EBootstrapping; 
     
    if ( !IsActive() )
        {
        iSession->RequestBootstrap( iGbaInputParams, *iGbaOutputParams, iStatus);
        SetActive();
        }
    return KErrNone;    
    }


// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::Bootstrap()
// -----------------------------------------------------------------------------
//
TInt CGbaClientRequestHandler::Bootstrap( const TGBABootstrapInputParams aInput, TGBABootstrapOutputParams& aOutput, TRequestStatus& aStatus )
    {
    iGbaInputParams.iNAFName.Copy(aInput.iNAFName);
    iGbaInputParams.iFlags = aInput.iFlags;
    iGbaInputParams.iAPID = aInput.iAPID;
    iGbaInputParams.iProtocolIdentifier.Copy(aInput.iProtocolIdentifier);
    iGbaInputParams.iUICCLabel.Copy(aInput.iUICCLabel);
    
    iGbaOutputParams = &aOutput;   
        
    iSession->RequestBootstrap(iGbaInputParams, *iGbaOutputParams, aStatus);
    return KErrNone;
    }    


// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::CancelBootstrap()
// -----------------------------------------------------------------------------
//
TInt CGbaClientRequestHandler::CancelBootstrap()
    {
    GBA_TRACE_DEBUG(("CGbaClientRequestHandler::CancelBootstrap"));
    if ( iObserver )
        {
        iBootstrapState = EBootstrapCancelling;
        }
    return iSession->CancelBootstrap() ;
    }


// -----------------------------------------------------------------------------
// CGbaClientRequestHandler::SetBSFAddress()
// -----------------------------------------------------------------------------
//
TInt CGbaClientRequestHandler::SetBSFAddress(const TDesC8& aNewVal)
    {
    GBA_TRACE_DEBUG(("CGbaClientRequestHandler::SetBSFAddress"));
    TUid optionCode = KGbaBSFConfiguration;
    return iSession->WriteOption(optionCode,aNewVal);
    }

//EOF
