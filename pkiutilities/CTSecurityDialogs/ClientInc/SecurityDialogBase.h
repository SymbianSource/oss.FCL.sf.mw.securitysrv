/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Class CSecurityDialogBase that implements common functionality 
*                for CTSecurityDialogs.
*
*/


#ifndef CSECURITYDIALOGBASE
#define CSECURITYDIALOGBASE

//  INCLUDES
#include "CTSecurityDialogRequestor.h"
#include "CTSecurityDialogDefs.h"

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  Base class of the CTSecurityDialogs  
*
*  @lib CTSecDlgs.lib
*  @since Series60_2.6
*/
NONSHARABLE_CLASS( CSecurityDialogBase ): 
    public CBase, public MSecurityDialogResponse
    {
    public:  // Constructors and destructor
        
        /**
        * C++ default constructor.
        */
        CSecurityDialogBase();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
        
        /**
        * Destructor.
        */
        virtual ~CSecurityDialogBase();

    public: // For Dialog Requester responses.

        /**
        *
        * @param aResult
        */
	    virtual void HandleResponse(TInt aResult);
        
    protected:  // New functions
        
        /**
        *
        * @param aStatus
        */
        void InitClientStatus(TRequestStatus& aStatus);

        /**
	    * ShowNoteL shows a dialog/note using Notifier API.
	    *
	    * @param aStatus	Signaled value for async operation. KErrNone if successful, otherwise system error.
	    * @param aDlgType	Dialog type
	    * @param aReturnValue	Return value of a dialog. If aStatus == KErrNone, then the user's response
	    *						is located here. ETrue means positive response (OK, continue etc.),
	    *						EFalse means negative response.
	    * @param aDynamic	Dynamic text for dialog. Effective only if %U is in the localisation resource.
	    *
	    * Leaves on: System error before showing the actual dialog
	    *
	    * Asserts on: If aDlgType is a PIN/PUK code query.
	    */
	    void ShowNoteL(TRequestStatus& aStatus, 
                       TInt aNoteTypeaDlgType, 
                       const TDesC& aDynamic = KNullDesC);
        
        /**
	    * Sends dialog request to CCTSecurityDialogRequestor
	    *
	    * @param aData	Data needed to display dialog.
	    */
        void RequestDialog(const TDesC8& aData);

        /**
	    * Checks error and if aError != KErrNone completes iClientStatus
        * with aError.
        *
        * @param aError Error to check
	    *
	    * @param aData	Data needed to display dialog.
	    */
        void CheckError(TInt aError);
       
        CCTSecurityDialogRequestor& Requester();


    private:    // Data
        TRequestStatus* iClientStatus;  // Pointer to client's request status
									    // We must signal it when dialogrequestor
									    // RunL is called to signal the completion of a dialog!

        CBufFlat*	iSenderBuffer;	// Stream buffer for outgoing streams.
    	HBufC8*     iBufferData;    // iSenderBuffer help buffer

        CCTSecurityDialogRequestor* iRequester;	  // Owned

        TBuf8<10> iDlgResponseBuf;

        TAny* iReserved;
    };

#endif      // CSECURITYDIALOGBASE   
            
// End of File
