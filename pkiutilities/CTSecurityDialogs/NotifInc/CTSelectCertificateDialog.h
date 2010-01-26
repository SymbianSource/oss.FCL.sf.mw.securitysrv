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
* Description:   CT Select Certificate Dialog
*
*/

 

#ifndef __CTSELECTCERTIFICATEDIALOG_H
#define __CTSELECTCERTIFICATEDIALOG_H

//  INCLUDES
#include <aknlistquerydialog.h>

class TCTTokenObjectHandle;

// CLASS DECLARATION
/**
*  Certificate selection query class
*
*  @lib CTSecDlgNotifier.dll
*  @since Series 60 2.6
*/
NONSHARABLE_CLASS( CCTSelectCertificateDialog ): public CAknListQueryDialog
    {
    public: // Constructors and destructor
        
        /**
        * Destructor.
        */
	    ~CCTSelectCertificateDialog();

    public: // Functions from CAknQueryDialog
        
	    /**
	    * RunDlgLD Launches the Select Certificate Dialog
	    * @param	aCertificates	List of certificate labels
	    * @param	aSelectedIndex	Selected certificate index.
		* @param	aClientStatus   Client Status 		
	    * @param	iRetVal ETrue for positive response from the user, EFalse otherwise.
	    */
	    static void RunDlgLD( MDesCArray* aCertificates, 
                              RArray<TCTTokenObjectHandle>& aCertHandleList,
                              TCTTokenObjectHandle& aSelectedCert,
                              TRequestStatus& aClientStatus, 
                              TBool& aRetVal );

    private:

        /**
        * C++ default constructor.
        */        
	    CCTSelectCertificateDialog( MDesCArray* aCertificates, 
                                    RArray<TCTTokenObjectHandle>& aCertHandleList,
                                    TCTTokenObjectHandle& aSelectedCert,
                                    TRequestStatus& aClientStatus, 
                                    TBool& aRetVal );

		TBool OkToExitL( TInt aButtonId );
		
		void PostLayoutDynInitL();
		

	private:

		TRequestStatus* iClientStatus;
		TBool& iRetVal;
		TInt iSelectedIndex;
        RArray<TCTTokenObjectHandle>& iCertHandleList;
        TCTTokenObjectHandle& iSelectedCert;
    };
    
#endif // __CTSELECTCERTIFICATEDIALOG_H

// End of File