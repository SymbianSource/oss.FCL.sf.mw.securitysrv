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
*		Dialog used for code queries. Inherits AknQueryDialog
*
*
*/


#ifndef __SECUICODEQUERYDIALOG__
#define __SECUICODEQUERYDIALOG__

#include <aknquerydialog.h>
#include <aknnotedialog.h>
#include <secui.hrh>

class CCodeQueryDialog : public CAknTextQueryDialog
	{
	public://construction and destruction
		/**
        * C++ Constructor.
        * @param aDataText TDes& (code which is entered in query)
		* @param aMinLength TInt (code min length)
		* @param aMaxLength TInt (code max length)
		* @param aMode TInt (mode ESecUiCodeEtelReqest\ESecUiNone)
		*/
		IMPORT_C CCodeQueryDialog(TDes& aDataText, TInt aMinLength,TInt aMaxLength,TInt aMode, TBool aIsRemotelockQuery = EFalse);
		/**
        * Destructor.
        */
		~CCodeQueryDialog();
	public:
		/**
		* Allows dismissing of code queries. Only mandatory requirement is that PIN
		* queries are dismissed by the # 
		*
		* @param aKeyEvent TKeyEvent&
		* @return ETrue query is dismissed
		*		  EFalse not dismissed
		*/
		TBool NeedToDismissQueryL(const TKeyEvent& aKeyEvent);
		/**
		* Calls the dialog's TryExitL and passes the reason given as a parameter.
		*
		* @param TInt aReason (The reason for the cancel request)
		*/
		void TryCancelQueryL(TInt aReason);

	protected://from CAknTextQueryDialog
		/**
		* From CAknTextQueryDialog This function is called by the UIKON dialog framework 
		* just before the dialog is activated, after it has called
		* PreLayoutDynInitL() and the dialog has been sized.
		*/
		void PreLayoutDynInitL();
		/**
		* From CAknTextQueryDialog This function is called by the UIKON framework 
		* if the user activates a button in the button panel. 
		* It is not called if the Cancel button is activated, 
		* unless the EEikDialogFlagNotifyEsc flag is set.
		* @param aButtonId  The ID of the button that was activated
		* @return           Should return ETrue if the dialog should exit, and EFalse if it should not.
		*/
		TBool OkToExitL(TInt aButtonId);
		/**
		* From CAknTextQueryDialog This function is called by the UIKON dialog framework 
        * just after a key is pressed
		* @param aKeyEvent TKeyEvent& 
		* @param aType TEventCode 
        */
		TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);
			/**
   		* From MAknQueryControlObeserver; overrides the default implementation in CAknQueryDialog. 
   		* Gets called by framework when editor sends state event
    	*/	
		TBool HandleQueryEditorStateEventL(CAknQueryControl* aQueryControl, TQueryControlEvent aEventType, TQueryValidationStatus aStatus);
		
		TBool CheckIfEntryTextOk() const;
		
		void UpdateLeftSoftKeyL();
	private:	
		void ShowWarningNoteL();
		
	    /*
	    * SetIncallBubbleAllowedInUsualL
	    */
	    void SetIncallBubbleAllowedInUsualL(TBool aAllowed);

	private: // DATA	
		TInt	iMinLength;
		TInt	iMaxLength;
		TInt	iMode;
		TBool	iFront;
		TInt	iAppKey;
		TInt	iVoiceKey1;
		TInt	iVoiceKey2;
		TInt	iEndKey;
		TBool   iIsRemoteLockQuery;
	};
#endif

// End of file
