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
* Description:  Dialog used for code queries.
*
*
*/

#include <AknQueryDialog.h>
#include "secuicodequerydialog.h"
#include <eikseced.h>
#include "SecUiCodeQueryControl.h"
#include "secui.hrh"
#include <SecUi.rsg>
#include <aknsoundsystem.h>
#include <aknappui.h> 
#include <AknIncallBubbleNotify.h>

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CCodeQueryDialog::CCodeQueryDialog()
// C++ constructor
// ----------------------------------------------------------
//
EXPORT_C CCodeQueryDialog::CCodeQueryDialog(TDes& aDataText,TInt aMinLength,TInt aMaxLength,TInt aMode, TBool aIsRemoteLockQuery):
			CAknTextQueryDialog(aDataText,ENoTone), iMinLength(aMinLength), iMaxLength(aMaxLength), iMode(aMode), iIsRemoteLockQuery(aIsRemoteLockQuery)

	{		
	}
//
// ----------------------------------------------------------
// CCodeQueryDialog::~CodeQueryDialog()
// Destructor
// ----------------------------------------------------------
//
CCodeQueryDialog::~CCodeQueryDialog()
	{
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CCodeQueryDialog::~CCodeQueryDialog()"));
    #endif
    
    if ( AknLayoutUtils::PenEnabled() )
        {
        TRAP_IGNORE ( SetIncallBubbleAllowedInUsualL( ETrue ) );
        }
    
	if (iFront)
		{
		// uncapture keys  
		if (iMode != ESecUiNone)		
			{
			RWindowGroup& groupWin=iCoeEnv->RootWin();
			groupWin.CancelCaptureKeyUpAndDowns(iAppKey);
			groupWin.CancelCaptureKeyUpAndDowns(iVoiceKey2);
			groupWin.CancelCaptureKey(iVoiceKey1);
			}
			
		CAknAppUi* aknappui =static_cast<CAknAppUi*>(iEikonEnv->EikAppUi());
		
		if(!iIsRemoteLockQuery)
		    {
		    // return normal high-priority in case there are other notifiers active 
    		// and were are not going to lose foregroung right after following call     
        	if (!aknappui->IsFullScreenApp())//check that we are autolock and not a "normal" i.e. full screen application.	
    			iEikonEnv->RootWin().SetOrdinalPosition(0,ECoeWinPriorityAlwaysAtFront); 
		    }
		
		 
		iEikonEnv->BringForwards(EFalse);	
		iEikonEnv->EikAppUi()->RemoveFromStack(this);
		aknappui->KeySounds()->ReleaseContext();
		aknappui->KeySounds()->PopContext();		
		iFront = EFalse;
		}
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CCodeQueryDialog::~CCodeQueryDialog() END"));
    #endif
	}
//
// ----------------------------------------------------------
// CCodeQueryDialog::PreLayoutDynInitL()
// Called by framework before dialog is shown 
// ----------------------------------------------------------
//
void CCodeQueryDialog::PreLayoutDynInitL()
    {
    CAknTextQueryDialog::PreLayoutDynInitL();
    
    //disable in call bubble.
    if ( AknLayoutUtils::PenEnabled() )
        {
        SetIncallBubbleAllowedInUsualL( EFalse );
        }
    
	SetMaxLength(iMaxLength);
	// Add this higher than the environment filter, otherwise
	// we cannot capture keys from the EikSrvUi KeyFilter. 
	// Used because this query might be called from notifier
	if (iMode == ESecUiNone)
		{
		iEikonEnv->EikAppUi()->AddToStackL(this,ECoeStackPriorityEnvironmentFilter+100,ECoeStackFlagRefusesAllKeys);
		}
	else
		{
		iEikonEnv->EikAppUi()->AddToStackL(this,ECoeStackPriorityEnvironmentFilter+100,ECoeStackFlagRefusesFocus);
		}
	
	// capture app,voice and end keys if necessary
	if (iMode != ESecUiNone)		
		{
		
		RWindowGroup& groupWin=iCoeEnv->RootWin();
		// Capture app key
		iAppKey = groupWin.CaptureKeyUpAndDowns(EStdKeyApplication0, 0, 0);
		// capture voice key
		iVoiceKey1 = groupWin.CaptureKey(EKeySide,0,0);
		iVoiceKey2 = groupWin.CaptureKeyUpAndDowns(EStdKeyDevice6, 0, 0);
		}
    if (!iIsRemoteLockQuery)
        {
        MakeLeftSoftkeyVisible(EFalse);

        RWsSession& wsSession = iEikonEnv->WsSession();
        TInt myWgId = iEikonEnv->RootWin().Identifier();

        TInt wgPrio = wsSession.GetWindowGroupOrdinalPriority(myWgId);
        // we are already on forgeround, need to update priority differently
        /*
         if (wgPrio == ECoeWinPriorityAlwaysAtFront)
         {
         iEikonEnv->RootWin().SetOrdinalPosition(0,ECoeWinPriorityAlwaysAtFront+1); 
         }
         */

        // this must be done always to keep the reference count in synch  
        // this does not have any effect if autoforwarding has not been set true (normal application.)
        iEikonEnv->BringForwards(ETrue, ECoeWinPriorityAlwaysAtFront + 1);

        /// -- Change Window Priority for dialog and CBA 	
        DrawableWindow()->SetOrdinalPosition(0, ECoeWinPriorityAlwaysAtFront); //
        ButtonGroupContainer().ButtonGroup()->AsControl()->DrawableWindow()->SetOrdinalPosition(
                0, ECoeWinPriorityAlwaysAtFront);
        }
    	
	//key sounds
	static_cast<CAknAppUi*>(iEikonEnv->EikAppUi())->KeySounds()->PushContextL(R_AVKON_DEFAULT_SKEY_LIST);
	static_cast<CAknAppUi*>(iEikonEnv->EikAppUi())->KeySounds()->BringToForeground();
	static_cast<CAknAppUi*>(iEikonEnv->EikAppUi())->KeySounds()->LockContext();
	iFront = ETrue;

	}
//
// ---------------------------------------------------------
// CCodeQueryDialog::OfferKeyEventL
// called by framework when any key is pressed
// ---------------------------------------------------------
//
TKeyResponse CCodeQueryDialog::OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType)
	{
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CCodeQueryDialog::OfferKeyEventL"));
    TInt keycode = aKeyEvent.iCode;
    RDebug::Print(_L("(SECUI)CCodeQueryDialog::OfferKeyEventL keycode: %d"), keycode);
    TInt scancode = aKeyEvent.iScanCode;
    RDebug::Print(_L("(SECUI)CCodeQueryDialog::OfferKeyEventL scancode: %d"), scancode);
    #endif
    if(!iIsRemoteLockQuery)
        {
        // '#' key 
    	if( aKeyEvent.iScanCode == EStdKeyHash  && aType == EEventKeyUp)
    		{ 
    		TryExitL(EEikBidOk);
    		return EKeyWasConsumed;
    		}
    	
    	// '*' key
    	if (aKeyEvent.iCode == '*')
    		{
    		return EKeyWasConsumed;
    		}

        }
	
	// app key
	if (aKeyEvent.iScanCode == EStdKeyApplication0)
		{
		if (iMode == ESecUiNone)
			{
			TryExitL(EAknSoftkeyCancel);
			return EKeyWasNotConsumed;	
			}
		return EKeyWasConsumed;
		}
		
	// end key
	if (aKeyEvent.iCode == EKeyPhoneEnd)
		{
		#if defined(_DEBUG)
        RDebug::Print(_L("(SECUI)CCodeQueryDialog::OfferKeyEventL: EKeyPhoneEnd"));
        #endif
		TryExitL(EAknSoftkeyCancel);
		return EKeyWasConsumed;
		}
	if ((aKeyEvent.iScanCode == EStdKeyYes))
	    {//Since dialler listens to down/up event, 
	     //have to consume those to prevent it from opening
	    if(AknLayoutUtils::PenEnabled())
	       {
	       CCodeQueryControl* control = STATIC_CAST(CCodeQueryControl*,Control(EGeneralQuery));
	       if(!control->IsEmergencyCallSupportOn())
	         { //Only needed in dialogs where there is no emergency call support
                #if defined(_DEBUG)
                RDebug::Print(_L("(SECUI)CCodeQueryDialog::OfferKeyEventL: Send down/up event consumed."));
                #endif
                return EKeyWasConsumed;
	         }
	       }    
	    }
	if (aKeyEvent.iCode == EKeyPhoneSend)
		 {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CCodeQueryDialog::OfferKeyEventL: EKeyPhoneSend"));
            #endif
		    if(AknLayoutUtils::PenEnabled())
		        {
		        #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CCodeQueryDialog::OfferKeyEventL: pen enabled"));
            #endif
	    		 	CCodeQueryControl* control = STATIC_CAST(CCodeQueryControl*,Control(EGeneralQuery));
	    		 	if(control->IsEmergencyCallSupportOn())
	    		 	  {
			    		 	#if defined(_DEBUG)
		            RDebug::Print(_L("(SECUI)CCodeQueryDialog::OfferKeyEventL: ECS on"));
		            #endif
		    		 	if(!control->IsEmergencyNumber())
		    		 	  {
		    		 	  	#if defined(_DEBUG)
			            RDebug::Print(_L("(SECUI)CCodeQueryDialog::OfferKeyEventL: Not E number!"));
			            #endif
            			
		    		 	    ShowWarningNoteL();
		    		 	 
		    		 	    #if defined(_DEBUG)
			            RDebug::Print(_L("(SECUI)CCodeQueryDialog::OfferKeyEventL: send key consumed!"));
			            #endif
		    		 	      return EKeyWasConsumed;
		    		 	  }
	    		 	  }
	    		 	else //consume the key to prevent dialler from opening
	    		 	    {
                        #if defined(_DEBUG)
                        RDebug::Print(_L("(SECUI)CCodeQueryDialog::OfferKeyEventL: No ECS. send key consumed!"));
                        #endif
	    		 	        return EKeyWasConsumed;
	    		 	    }
			      }
		 }
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CCodeQueryDialog::OfferKeyEventL: To AVKON"));
    #endif
	return CAknTextQueryDialog::OfferKeyEventL(aKeyEvent,aType);
	}
//
// ---------------------------------------------------------
// CCodeQueryDialog::NeedToDismissQueryL()
// Handles '#' key called by CAknTextQueryDialog::OfferKeyEventL()
// ---------------------------------------------------------
//
TBool CCodeQueryDialog::NeedToDismissQueryL(const TKeyEvent& /*aKeyEvent*/)
	{
	return EFalse;
	}
//
// ---------------------------------------------------------
// CCodeQueryDialog::OkToExitL()
// called by framework when the Softkey is pressed
// ---------------------------------------------------------
//
TBool CCodeQueryDialog::OkToExitL(TInt aButtonId)
	{
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CCodeQueryDialog::OkToExitL:%d"), aButtonId);
    #endif
	CCodeQueryControl* control = STATIC_CAST(CCodeQueryControl*,Control(EGeneralQuery));
	TInt lenght = 0;
	TBool returnvalue;
	switch(aButtonId)
	    {
	    case EAknSoftkeyOk:
		    #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CCodeQueryDialog::OkToExitL:EAknSoftkeyOk"));
            #endif
            if(control)
              {
                lenght = control->GetTextLength();
              }
		    if (lenght < iMinLength)
			    {
			    #if defined(_DEBUG)
                RDebug::Print(_L("(SECUI)CCodeQueryDialog::OkToExitL:length < MIN: %d"), lenght);
                #endif	
		    	// code was too short -> play error tone & clear editor
			    if (lenght != 0)
				    {
				    #if defined(_DEBUG)
                    RDebug::Print(_L("(SECUI)CCodeQueryDialog::OkToExitL:length < MIN, Play Sound"));
                    #endif
                    if(control)
                        {
                        
			    	    control->PlaySound(EAvkonSIDErrorTone);
				        #if defined(_DEBUG)
                        RDebug::Print(_L("(SECUI)CCodeQueryDialog::OkToExitL:length < MIN, Reset Editor"));
                        #endif
				        control->ResetEditorL();
                        }
				    }
			    returnvalue = EFalse;
			    }	
		    else
			    {
			    #if defined(_DEBUG)
                RDebug::Print(_L("(SECUI)CCodeQueryDialog::OkToExitL:length >= MIN: %d"), lenght);
                #endif		
			    returnvalue = CAknTextQueryDialog::OkToExitL(aButtonId);
			    }
            break;
		
	    case EEikBidCancel: //Also includes EAknSoftkeyCancel as they have the same numerical value
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CCodeQueryDialog::OkToExitL:EEikBidCancel"));
            #endif       
            if(iMode != ESecUiNone) //PUK1 code or PIN1 in boot. Must not exit until code has been input.
                {
                #if defined(_DEBUG)
                RDebug::Print(_L("(SECUI)CCodeQueryDialog::OkToExitL:EEikBidCancel: get length."));
                #endif   
                if(control)
                  {
                    lenght = control->GetTextLength();
                  }
                returnvalue = EFalse;
                if (lenght > 0) //reset editor
                    {  
                    #if defined(_DEBUG)
                    RDebug::Print(_L("(SECUI)CCodeQueryDialog::OkToExitL:EEikBidCancel: length >0."));
                    #endif              
                     if (control)
                       {
                        #if defined(_DEBUG)
                        RDebug::Print(_L("(SECUI)CCodeQueryDialog::OkToExitL:EEikBidCancel, reset editor"));
                        #endif
                        control->ResetEditorL();
                       }
                    }
                }
            else
                returnvalue = ETrue;
            break;
        case  ESecUiEmergencyCall: //user has called emergency number from dialog; exit.
        	#if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CCodeQueryDialog::OkToExitL:Emergency Call"));
            #endif
        	returnvalue = ETrue;
        	break;
        case EAknSoftkeyEmergencyCall:
            returnvalue = ETrue;
            CAknTextQueryDialog::OkToExitL(aButtonId);
            break;
	    case ESecUiDeviceLocked: //Dialog was left open when Device lock was activated by timer and must be closed.
	    	#if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CCodeQueryDialog::OkToExitL:Phone Locked"));
            #endif
	    	if(iMode == ESecUiNone)  
                 returnvalue = ETrue;//Not a PUK1 code query.
            else
            	 returnvalue = EFalse;
	    	break;
        default:
        	#if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CCodeQueryDialog::OkToExitL:DEFAULT!"));
            #endif  
            if(iMode == ESecUiNone)
		        returnvalue = ETrue;
		    else
                returnvalue = EFalse;
            break;
	    }
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CCodeQueryDialog::OkToExitL:END: %d"), returnvalue);
    #endif
	return returnvalue;
	}
//
// ---------------------------------------------------------
// CCodeQueryDialog::HandleQueryEditorStateEventL()
// Gets called when editor sends state event
// ---------------------------------------------------------
//
TBool CCodeQueryDialog::HandleQueryEditorStateEventL(CAknQueryControl* aQueryControl, 
													TQueryControlEvent aEventType, 
													TQueryValidationStatus aStatus)
{
	
    if (aEventType == MAknQueryControlObserver::EEmergencyCallAttempted)
        {
        TryExitL(ESecUiEmergencyCall);
        return EFalse;
        }
    else
    	{
    	return CAknQueryDialog::HandleQueryEditorStateEventL(aQueryControl, aEventType, aStatus );
    	}
    

}
//
// ---------------------------------------------------------
// CCodeQueryDialog::TryCancelQueryL()
// Gets called when a client wishes to cancel the query
// ---------------------------------------------------------
//
void CCodeQueryDialog::TryCancelQueryL(TInt aReason)
{
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CCodeQueryDialog::TryCancelQuery BEGIN"));
    #endif
	TryExitL(aReason);
	#if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CCodeQueryDialog::TryCancelQuery END"));
    #endif
}

// -----------------------------------------------------------------------------
// CCodeQueryDialog::CheckIfEntryTextOk()
// -----------------------------------------------------------------------------
//
TBool CCodeQueryDialog::CheckIfEntryTextOk() const
    {
	CCodeQueryControl* control = STATIC_CAST(CCodeQueryControl*,Control(EGeneralQuery));
	if (control)
	{
		control->GetText(iDataText);
	}
	const TInt textLength = Text().Length();
	return (textLength <= iMaxLength && textLength >= iMinLength);
    }

// -----------------------------------------------------------------------------
// CCTPinQueryDialog::UpdateLeftSoftKeyL()
// -----------------------------------------------------------------------------
//
void CCodeQueryDialog::UpdateLeftSoftKeyL()
    {
	CAknTextQueryDialog::UpdateLeftSoftKeyL();
    if(iIsRemoteLockQuery)
	    MakeLeftSoftkeyVisible(CheckIfEntryTextOk());
    }
// -----------------------------------------------------------------------------
// CCodeQueryDialog::ShowWarningNoteL()
// -----------------------------------------------------------------------------
//
void CCodeQueryDialog::ShowWarningNoteL()
    {
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CCodeQueryDialog::ShowWarningNoteL BEGIN"));
    #endif
    
    CAknNoteDialog* noteDlg = new (ELeave) CAknNoteDialog();
    noteDlg->PrepareLC(R_EMERGENCY_CALLS_ONLY);
    noteDlg->SetTimeout(CAknNoteDialog::ELongTimeout);
    noteDlg->SetTone(CAknNoteDialog::EErrorTone);
    
    noteDlg->DrawableWindow()->SetOrdinalPosition(0,ECoeWinPriorityAlwaysAtFront+1); //
    noteDlg->ButtonGroupContainer().ButtonGroup()->AsControl()->DrawableWindow()->SetOrdinalPosition(0,ECoeWinPriorityAlwaysAtFront+1); 
    
    noteDlg->RunLD();
    #if defined(_DEBUG)
    RDebug::Print(_L("(SECUI)CCodeQueryDialog::ShowWarningNoteL END"));
    #endif
    }

// -----------------------------------------------------------------------------
// CCodeQueryDialog::SetIncallBubbleAllowedInUsualL()
// -----------------------------------------------------------------------------
//
void CCodeQueryDialog::SetIncallBubbleAllowedInUsualL(TBool aAllowed)
    {
    CAknIncallBubble *incallBubble =  CAknIncallBubble::NewL();
    CleanupStack::PushL(incallBubble);
    incallBubble->SetIncallBubbleAllowedInUsualL( aAllowed );
    CleanupStack::PopAndDestroy();
    }

// End of file

