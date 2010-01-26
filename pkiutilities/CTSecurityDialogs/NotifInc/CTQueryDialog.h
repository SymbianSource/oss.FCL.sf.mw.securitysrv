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
* Description:   Generic CT Query Dialog
*
*/



#ifndef __CTCONFIRMATIONQUERYDIALOG_H
#define __CTCONFIRMATIONQUERYDIALOG_H

//  INCLUDES
#include <AknQueryDialog.h>

// DATA TYPES
enum TCTQueryDialogType
    {
	ECTConfirmationDialog,
	ECTErrorDialog,
	ECTInfoDialog,
	ECTWarningDialog,
	ECTOkDialog
    };


// CLASS DECLARATION

/**
*  Query dialog class
*
*  @lib CTSecDlgNotifier.dll
*  @since Series 60 2.6
*/
NONSHARABLE_CLASS( CCTQueryDialog ): public CAknQueryDialog
    {
    
    public: // Constructors and destructor
        
	    ~CCTQueryDialog();

    public: // Functions from CAknQueryDialog
        
	    /**
	    * RunDlgLD launches the Text Query Dialog
		* @param	aStatus		Client status
		* @param	aRetVal		ETrue for positive user response, EFalse for negative
	    * @param	aQueryText	Text for query dialog
	    * @param	aDlgButtonResourceId	ID of softkey resource to be used in the dlg (Default is Yes-No).									
	    * @param	aDlgType	Dialog layout type
	    */
	    static void RunDlgLD( 
            TRequestStatus& aStatus, 
            TBool& aRetVal, 
            const TDesC& aQueryText, 
            TInt aDlgButtonResourceId = 0, 
            const TCTQueryDialogType aDlgType = ECTConfirmationDialog);

    private:

        /**
        * C++ default constructor.
        */
	    CCTQueryDialog( 
            TRequestStatus& aStatus, 
            TBool& aRetVal, 
            const TDesC& aQueryText, 
            TInt aDlgButtonResourceId, 
            const TCTQueryDialogType aDlgType);

    private: // Functions from CAknQueryDialog 

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

    private: // Data
	    TCTQueryDialogType	iDlgType;
	    TInt				iDlgButtonResourceId;
	    const TDesC&		iQueryText;
		TRequestStatus*		iClientStatus;
		TBool&				iRetVal;
    };

#endif // __CTCONFIRMATIONQUERYDIALOG_H

// End of File