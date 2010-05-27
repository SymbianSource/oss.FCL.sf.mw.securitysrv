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
* Description:  LockApp Application UI class
*
*/


#include "lockappappui.h"
#include "lockappstatecontrol.h"
#include "lockappserver.h"
#include "lockapptrace.h"
#include <lockappclientserver.h>
#include <aknglobalpopupprioritycontroller.h>
#include <apgcli.h>

#ifdef __SAP_TERMINAL_CONTROL_FW
#include <SCPClient.h>
#endif // __SAP_TERMINAL_CONTROL_FW

// ---------------------------------------------------------------------------
// Second phase constructor
// ---------------------------------------------------------------------------
void CLockAppAppUi::ConstructL( )
    {
    //	RDebug::Printf( "%s %s (%u) 1 value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0x8 );
    	    
    TInt use_old_autolock=1;
    if(use_old_autolock)
    	{
			// start autolock instead of lockapp . This is a backup solution to use in case that not all SysAp and Avkon changes are implemented
	 	  RDebug::Printf( "%s %s (%u) 1=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0x1 );
	    TApaTaskList taskList( iCoeEnv->WsSession() );
	    TApaTask task( taskList.FindApp( _L("autolocksrv.exe" )) );
	    if ( !task.Exists() )
	        {
	        RApaLsSession ls;                   
	        User::LeaveIfError(ls.Connect());   
	        CleanupClosePushL(ls);         
	        
	        CApaCommandLine* commandLine = CApaCommandLine::NewLC();
	        commandLine->SetExecutableNameL( _L("autolocksrv.exe" ) );     
	        commandLine->SetCommandL( EApaCommandRun );
	        
		    // Try to launch the application.        
	        User::LeaveIfError(ls.StartApp(*commandLine));
	    	  RDebug::Printf( "%s %s (%u) Start: autolocksrv.exe created=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0x7 );
	        
	        CleanupStack::PopAndDestroy(2); // commandLine, ls
			}
	  }

    INFO( "CLockAppAppUi::ConstructL started" );
   // default appui constructor has to be called
    BaseConstructL( );  
    if(use_old_autolock)
    	{  	    
	    Exit();
	  }
    	    

    // start the server with the specified name
	iLockServer = CLockAppServer::NewL( KLockAppServerName );
	
    // status pane is should not be visible
    StatusPane()->MakeVisible( EFalse );

    // we need high priority even if lockapp is not in the foreground
    iEikonEnv->WsSession().ComputeMode( RWsSession::EPriorityControlDisabled );
    RThread().SetProcessPriority( EPriorityHigh );

    /*
     * LockApp is set as system application (Symbian terminology).
     * This means it does not get closed when system is asked to close applications.
     */
    iEikonEnv->SetSystem( ETrue );

#ifdef _GLOBAL_PRIORITY_SUPPORTED
    // Enable global popup notes
    AknGlobalPopupPriorityController::EnablePriorityControlL();
    AknGlobalPopupPriorityController::AllowGlobalPopups( ETrue );
#endif

    // main control storing and controling phone keylock/devicelock status
    iStateControl = CLockAppStateControl::NewL( );

    // the main control is given high stack priority
    // ECoeStackPriorityEnvironmentFilter-1 used to allow hw keys for keyfiler even if keypad is locked.
    AddToStackL( iStateControl, ECoeStackPriorityEnvironmentFilter-1, ECoeStackFlagStandard );
    INFO( "CLockAppAppUi::ConstructL finished" );
    }

// ---------------------------------------------------------------------------
// Default Constructor
// ---------------------------------------------------------------------------
CLockAppAppUi::CLockAppAppUi( )
    {
    // no implementation required
    }

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
CLockAppAppUi::~CLockAppAppUi( )
    {
	if ( iLockServer )
		{
		delete iLockServer;
		iLockServer = NULL;
		}
    if ( iStateControl )
        {
        // remove main control from stack
        RemoveFromStack( iStateControl );
        delete iStateControl;
        iStateControl = NULL;
        }
    }

// ---------------------------------------------------------------------------
// Returns interface to LockAppServer sessions for changing lock states
// ---------------------------------------------------------------------------
MLockAppStateControl* CLockAppAppUi::StateControl( )
    {
    return iStateControl;
    }

#ifdef __SAP_TERMINAL_CONTROL_FW
// ---------------------------------------------------------------
// Handles the TARM command to unlock the phone from WindowServer.
// ---------------------------------------------------------------
MCoeMessageObserver::TMessageResponse CLockAppAppUi::HandleMessageL(
        TUint32 aClientHandleOfTargetWindowGroup,
        TUid aMessageUid,
        const TDesC8& aMessageParameters )
    {
    MCoeMessageObserver::TMessageResponse messageResponse( EMessageHandled);
    if ( aMessageUid.iUid == SCP_CMDUID_UNLOCK)
        {
        // For security reasons we must check from the SCP server
        // did this command originate from it.
        RSCPClient scpClient;
        if ( scpClient.Connect() == KErrNone )
            {
            CleanupClosePushL( scpClient );
            if ( scpClient.QueryAdminCmd( ESCPCommandUnlockPhone ) )
                {
                INFO( "CLockAppAppUi::HandleMessageL(): Admin command received, unlocking" );
                iStateControl->DisableDevicelockL();
                }
            else
                {
                INFO( "CLockAppAppUi::HandleMessageL(): Unauthorized attempt to unlock" );
                }
            CleanupStack::PopAndDestroy(); // calls Close() on scpClient
            }
        else
            {
            INFO( "CLockAppAppUi::HandleMessageL(): Failed to connect to SCP, ignoring unlock-message." );
            }
        }
    else // aMessageUid.iUid != SCP_CMDUID_UNLOCK
        {
        messageResponse = CAknAppUi::HandleMessageL(
                aClientHandleOfTargetWindowGroup,
                aMessageUid,
                aMessageParameters);
        }
    return messageResponse;
    }
#endif // __SAP_TERMINAL_CONTROL_FW

// ----------------------------------------------------------
// Handle window-server events
// ----------------------------------------------------------
void CLockAppAppUi::HandleWsEventL( const TWsEvent& aEvent, CCoeControl* aDestination )
    {
		RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );
    if ( aEvent.Type() == TRawEvent::EKeyDown )
        {
        RDebug::Printf("CLockAppAppUi::HandleWsEventL() - aEvent.Key()->iCode: %d", aEvent.Key()->iCode );
        }

    // call super-class
    CAknAppUi::HandleWsEventL( aEvent, aDestination );
    // propagate call
    iStateControl->HandleWsEventL( aEvent, aDestination);
    }

// ---------------------------------------------------------------------------
// From @c CAknAppUiBase. Overriden the parent method, because otherwise
// the main lock state control would not receive the call, because is is not
// window-owning control (=without window->not visible). The call is needed
// because the main state control owns window-owning child controls.
// ---------------------------------------------------------------------------
void CLockAppAppUi::HandleResourceChangeL( TInt aType )
    {
    // call super-class
    CAknAppUi::HandleResourceChangeL( aType );
    // propagate call - since does not own window
    iStateControl->HandleResourceChange( aType );
    }
