/*
* Copyright (c) 2003-2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Crypto Token Security Dialogs PIN Query Dialog
*
*/



#ifndef __CTPINQUERYDIALOG_H
#define __CTPINQUERYDIALOG_H

//  INCLUDES
#include <AknQueryDialog.h>

// CLASS DECLARATION

/**
*  PIN query dialog class
*
*  @lib CTSecDlgNotifier.dll
*  @since Series 60 2.6
*/
NONSHARABLE_CLASS( CCTPinQueryDialog ): public CAknTextQueryDialog
    {
    public: // Constructors and destructor

        ~CCTPinQueryDialog();

    public: // Function from CAknQueryDialog 

	    /**
	    * RunDlgLD Launches the CT PIN Query Dialog
		* @param	aClientStatus  Reference to Client Status
	    * @param	aDlgHeading Text for heading text
	    * @param	aDlgText	Text for dialog
	    * @param	aPinValue	Reference to PIN Value.
	    * @param	aMinTextLength	Minimum length for PIN Value
	    * @param	aMaxTextLength	Maximum length for PIN Value
		* @param	aRetVal	 ETrue for positive user response, EFalse for negative
        * @param	aResourceId	 Resource id
	    */
        static void RunDlgLD(
            TRequestStatus& aClientStatus, 
            const TDesC& aDlgText, 
            TDes& aPinValue, 
            const TInt aMinTextLength, 
            const TInt aMaxTextLength, 
            TBool& aRetVal, 
            TInt aResourceId, 
            CCTPinQueryDialog*& aDialog, 
            TBool& aIsDeleted );
	    

    private:
        
        /**
        * C++ default constructor.
        */
	    CCTPinQueryDialog(
            TRequestStatus& aClientStatus, 
            const TDesC& aDlgText, 
            TDes& aPinValue, 
            const TInt aMinTextLength, 
            const TInt aMaxTextLength, 
            TBool& aRetVal,
            TBool& aIsDeleted );

    private:    // Function from CAknTextQueryDialog

        /**
        *
        */
	    void PreLayoutDynInitL();

    private:    // Functions from CAknTextQueryDialog/CAknQueryDialog

        /**
        *
        */
	    void UpdateLeftSoftKeyL();

        /**
        *
        */
	    TBool CheckIfEntryTextOk() const;

        /**
        *
		* @param	aButtonId 
		* @return	TBool
        */
		TBool OkToExitL( TInt aButtonId );

        /**
        *
        */	
		void PostLayoutDynInitL();

        /**
        *
		* @param  aKeyEvent
		* @param  aType
		* @return TKeyResponse
        */
		TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

    private:    // Data
	    const TDesC& iDlgText;
	    TDes& iPinValue;
	    TInt iMinTextLength;
	    TInt iMaxTextLength;
		TRequestStatus* iClientStatus;
		TBool& iRetVal; //Dialog response
		TBool& iIsDeleted;
    };

#endif // __CTPINQUERYDIALOG_H

// End of File