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
* Description:  Implementation of CGbaServerSession
*
*/


#include <e32svr.h>
#include "GbaSession.h"
#include "bootstrap.h"
#include "GBALogger.h"      
//Constants
const TInt KMessageIndex0 = 0;
const TInt KMessageIndex1 = 1;

// ---------------------------------------------------------------------------
// CGbaServerSession* CGbaServerSession::NewL()
// ---------------------------------------------------------------------------
//
CGbaServerSession* CGbaServerSession::NewL()
    {
    CGbaServerSession* self=new(ELeave) CGbaServerSession;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }


// ---------------------------------------------------------------------------
// CGbaServerSession::ConstructL()
// ---------------------------------------------------------------------------
//
void CGbaServerSession::ConstructL()
    {
    i3GPPBootstrap = C3GPPBootstrap::NewL( this );
    }


// ---------------------------------------------------------------------------
// CGbaServerSession::CGbaServerSession()
// ---------------------------------------------------------------------------
//
CGbaServerSession::CGbaServerSession()
    {
    // Implementation not required
    }


// ---------------------------------------------------------------------------
// CGbaServerSession::~CGbaServerSession()
// ---------------------------------------------------------------------------
//
CGbaServerSession::~CGbaServerSession()
    {
    GBA_TRACE_DEBUG(("CGbaServerSession destructor"));
    Server()->DecrementSessions();
    GBA_TRACE_DEBUG((" delete state machine"));
    delete i3GPPBootstrap;
    }


// ---------------------------------------------------------------------------
// CGbaServerSession::ServiceL()
// ---------------------------------------------------------------------------
//
void CGbaServerSession::ServiceL(const RMessage2& aMessage)
    {
    GBA_TRACE_DEBUG(("CGbaServerSession::ServiceL"));   
    switch (aMessage.Function())
        {
        case EGbaServRequestBootstrap:
            {
            GBA_TRACE_DEBUG(("Request bootstrap")); 
            RequestBootstrapL(aMessage);
            }
            break ;
        
        case EGbaServIsGBAUSupported:
            {
            GBA_TRACE_DEBUG(("Check is GBA-U supported"));    
            TBool isGBAUSupported = i3GPPBootstrap->GBAUAvailabilityL();
            
            if ( isGBAUSupported )
                {
                GBA_TRACE_DEBUG(("GBA-U is available"));    
                }
            else
                {
                GBA_TRACE_DEBUG(("GBA-U is not available"));
                }
            
            const TPckgC<TBool> pckg( isGBAUSupported );
            aMessage.Write( KMessageIndex0, pckg ); 
            aMessage.Complete( KErrNone ); 
            }
            break;
            
        case EGbaServCancelRequestBootstrap :
            {
            GBA_TRACE_DEBUG(("Cancelling bootstrapping"));
            if ( i3GPPBootstrap->GetState() != C3GPPBootstrap::EIdle )
                {
                i3GPPBootstrap->CancelBootstrap();
                // complete original bootstrapping
                iMessage.Complete(KErrCancel); 
                // complete the message for canceling
                aMessage.Complete(KErrNone);
                }
            else
                {
                aMessage.Complete(KErrNone);
                } 
            } 
            break ;

        case EGbaServWriteOption :
            {
            GBA_TRACE_DEBUG(("CGbaServerSession::ServiceL EGbaServWriteOption"));   

            //Get the dictionary store UID           
            TUid uid = TUid::Uid(aMessage.Int0());
            
            //First get the length of descriptor
            TInt length = aMessage.GetDesLength(1);
            
            // If bigger than 256,
            // we think it is an attack. 
            if ( length <= 0 || length > KMaxURLLength )
                {
                aMessage.Complete( KErrBadDescriptor );
                break;
                }
            
            //read the content
            HBufC8 *buf = HBufC8::NewLC( length );
            TPtr8 ptrbuf = buf->Des();
            aMessage.Read(KMessageIndex1,ptrbuf,0);
                       
            Server()->WriteOptionL( uid,*buf );
            aMessage.Complete( KErrNone );
            CleanupStack::PopAndDestroy( buf );
            }
            break;
                   
        default :
            {
            GBA_TRACE_DEBUG(("Unexpected request"));
            aMessage.Complete(KErrArgument);
            }
            
        }
    }


// ---------------------------------------------------------------------------
// CGbaServerSession::RequestBootstrapL()
// ---------------------------------------------------------------------------
//
void CGbaServerSession::RequestBootstrapL(const RMessage2& aMessage)
    {
    if ( i3GPPBootstrap->GetState() != C3GPPBootstrap::EIdle )
        {
        // We're already busy
        aMessage.Complete(KErrInUse);
        return;
        }
    else
        {
        // Keep a copy of message - for use later
        iMessage = aMessage ;
        
        TPtr8 inputPtr(reinterpret_cast <TUint8*>(&iGbaInputParams), sizeof(TGBABootstrapInputParams), sizeof(TGBABootstrapInputParams));
        
        //Sanity checking
        //First get the length of descriptor
        TInt inputDesLength = aMessage.GetDesLength(0);
                   
        // if the length is not eqaul to the size of gba credentails
        // return bad descriptor
       if ( inputDesLength != sizeof(TGBABootstrapInputParams))
           {
           aMessage.Complete( KErrBadDescriptor );
           return;
           }
        
        aMessage.Read(KMessageIndex0,inputPtr,0);
        
        //Initialize bootstrap process using the state machine
        i3GPPBootstrap->InitializeL();
        }
    }


// ---------------------------------------------------------------------------
// CGbaServerSession::StateMachineCallBack()
// bootstrap state machine call back function
// Copy the credentail back to client and complete the RMessage
// If something wrong, complete RMessage with error value
// ---------------------------------------------------------------------------
//
void CGbaServerSession::StateMachineCallBack( TInt aError )
    {
    if ( i3GPPBootstrap->GetState() != C3GPPBootstrap::EIdle )
        {
        if ( aError == KErrNone )
            {
            GBA_TRACE_DEBUG(("B-TID to be sent"));
            GBA_TRACE_DEBUG(iGbaOutputParams.iBTID);
            GBA_TRACE_DEBUG(("KsNAF to be sent"));
            GBA_TRACE_DEBUG_BINARY(iGbaOutputParams.iKNAF);
            // Write the obtained Gba Bootstrap Creds data back to the client
            TPtr8 gbaBuffer(NULL,0,0);
            gbaBuffer.Set(reinterpret_cast <TUint8*>(&iGbaOutputParams), sizeof(TGBABootstrapOutputParams), sizeof(TGBABootstrapOutputParams));
            TInt res = iMessage.Write(KMessageIndex1, gbaBuffer,0);
            GBA_TRACE_DEBUG_NUM(("writing message to client with return value = %d."),res);
            iMessage.Complete( res );
            } 
        else 
            {
            GBA_TRACE_DEBUG((" Didn't get bootstrap credentails, pass error back"));     
            // send error message to client     
            iMessage.Complete( aError );
            }
        
        }
    }
    


// ---------------------------------------------------------------------------
// CGbaServerSession::CreateL()
// ---------------------------------------------------------------------------
//
void CGbaServerSession::CreateL()
    {
    Server()->IncrementSessions();
    }


// ---------------------------------------------------------------------------
// CGbaServerSession::Server()
// ---------------------------------------------------------------------------
//
CGbaServer* CGbaServerSession::Server()
      {
      return static_cast<CGbaServer*>(const_cast<CServer2*>(CSession2::Server()));
      }
//EOF

