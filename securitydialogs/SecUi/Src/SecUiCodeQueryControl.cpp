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
* Description:  Provides editor clearing method for CodeQueryDialog
*               and CodeRequestQueryDialog.
*
*
*/
#include <e32notif.h>
#include <aknQueryControl.h>
#include <aknsoundsystem.h>
#include <aknappui.h>
#include <eikenv.h>
#include <uikon/eiksrvui.h>
#include <AknEcs.h>
#include "SecUiCodeQueryControl.h"

// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CCodeQueryControl::ResetEditor()
// Empties editor
// ----------------------------------------------------------
//
void CCodeQueryControl::ResetEditorL()
	{
	if (iPinEdwin)
		{
		// reset editor
		iPinEdwin->Reset();
		// handle event editor empty
		HandleControlEventL(iPinEdwin,EEventStateChanged);
		}
		
	if (iEcsDetector)
        {//Reset ECS buffer
        iEcsDetector->Reset();
        }
	}
//
// ----------------------------------------------------------
// CCodeQueryControl::PlaySound()
// Plays a tone
// ----------------------------------------------------------
//
void CCodeQueryControl::PlaySound(TInt aSid)
	{ 
	CAknKeySoundSystem* soundSystem;
	if (iEikonEnv->AppUi())
		{
		soundSystem = static_cast<CAknAppUi*>(iEikonEnv->EikAppUi())->KeySounds();
		soundSystem->PlaySound(aSid);
		}
	}
//
// ----------------------------------------------------------
// CCodeQueryControl::IsEmergencyNumber()
// Checks if the characters in the input field are emergency numbers.
// ----------------------------------------------------------
//
TBool CCodeQueryControl::IsEmergencyNumber()
{
    TBool returnvalue = EFalse;
    if (iEcsDetector)
        {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CCodeQueryControl::IsEmergencyNumber() ecs OK"));
            #endif
        if(iEcsDetector->State()== CAknEcsDetector::ECompleteMatch)
            {
            #if defined(_DEBUG)
            RDebug::Print(_L("(SECUI)CCodeQueryControl::IsEmergencyNumber() Complete match"));
            #endif
            returnvalue = ETrue;
            }
        }
    return returnvalue;
}
//
// ----------------------------------------------------------
// CCodeQueryControl::IsEmergencyNumber()
// Checks if the characters in the input field are emergency numbers.
// ----------------------------------------------------------
//
TBool CCodeQueryControl::IsEmergencyCallSupportOn()
{
	TBool returnvalue = EFalse;
    if (iEcsDetector)
    	{
    		returnvalue = ETrue;
    	}
  return returnvalue;
}
// End of file

