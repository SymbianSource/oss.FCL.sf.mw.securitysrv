/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Header file of the multiline PIN query dialog
*
*/


#ifndef CCTPINPINQUERYDIALOG_H
#define CCTPINPINQUERYDIALOG_H

//  INCLUDES
#include <AknQueryDialog.h>

// CLASS DECLARATION

/**
*  Multiline PIN query dialog class
*
*  @lib CTSecDlgNotifier.dll
*  @since Series 60 2.8
*/
NONSHARABLE_CLASS( CCTPinPinQueryDialog ): public CAknMultiLineDataQueryDialog
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CCTPinPinQueryDialog* NewL(
            const TDesC& aDlgText1,
            const TDesC& aDlgText2,
            TDes&  aPin1,
            TDes&  aPin2,
            const TInt aMinTextLength, 
            const TInt aMaxTextLength, 
            TBool& aRetVal);
        
        /**
        * Destructor.
        */
        virtual ~CCTPinPinQueryDialog();

    public: // New functions

        /**
        * RunDlgLD Launches the multiline PIN Query Dialog
        * @param aClientStatus  Reference to Client Status
        * @param aResourceId  Resource ID
        */
        void RunDlgLD(TRequestStatus& aClientStatus, TInt aResourceId);

    private: // Functions from base classes

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
	    void PreLayoutDynInitL();

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


    private:

        /**
        * C++ default constructor.
        */
        CCTPinPinQueryDialog(
            const TDesC& aDlgText1,
            const TDesC& aDlgText2,
            TDes&  aPin1,
            TDes&  aPin2,
            const TInt aMinTextLength, 
            const TInt aMaxTextLength, 
            TBool& aRetVal);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data
	    const TDesC& iDlgText1;
        const TDesC& iDlgText2;
	    TDes& iPinValue1;
        TDes& iPinValue2;
	    TInt iMinTextLength;
	    TInt iMaxTextLength;
		TRequestStatus* iClientStatus;
		TBool& iRetVal; //Dialog response

        TBool iFirstOk;
        TBool iSecondOk;
    };

#endif      // CCTPINPINQUERYDIALOG_H   
            
// End of File
