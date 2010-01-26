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
* Description:   CT Read and Sign Dialog for Digital Signature
*
*/



#ifndef __CTSIGNTEXTDIALOG_H
#define __CTSIGNTEXTDIALOG_H

//  INCLUDES
#include "CTSecurityDialogDefs.h"
#include <aknmessagequerydialog.h>


// CLASS DECLARATION

/**
*
*
*  @lib 
*  @since 
*/
NONSHARABLE_CLASS( CCTSignTextDialog ): public CAknMessageQueryDialog
    {
    public:  // Constructors and destructor

        /**
        * Destructor.
        */
	    ~CCTSignTextDialog();

    public:

	    /**
	    * RunDlgLD launches the Read And Sign Dialog
	    * @param	aReceiptToSign	Text to be signed
		* @param	aStatus			Client status				
	    * @param	aRetVal			ETrue for positive user response
	    */
	    static void RunDlgLD(TInt aResourceId, const TDesC& aReceiptToSign, 
	                         TRequestStatus& aStatus, TBool& aRetVal);        
    
    protected:

        /**
        * C++ default constructor.
        */
	    CCTSignTextDialog(const TDesC& aReceiptToSign, TRequestStatus& aStatus, TBool& aRetVal, TBool aSignText);

    protected:  // Functions from CAknMessageQueryDialog  
        
        /**
        *
		* @param  aKeyEvent
		* @param  aType
		* @return TKeyResponse
        */
		TKeyResponse OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType );

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
        */        
        TBool OkToExitL( TInt aButtonId );

        


    protected:  //Data
        
	    const TDesC& iReceiptToSign;
		TRequestStatus* iClientStatus;
	
	private:
		TBool& iRetVal;
        TBool iSignText;
    };

#endif // __CTSIGNTEXTDIALOG_H

// End of File