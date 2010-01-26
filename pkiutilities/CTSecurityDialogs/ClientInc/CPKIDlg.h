/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Header file of the CPKIDialog
*
*/


#ifndef CPKIDLG_H
#define CPKIDLG_H

//  INCLUDES
#include "SecurityDialogBase.h"
#include <PKIDlg.h>  // MPKIDialog
#include <e32std.h>
#include <secdlgimpldefs.h>

// FORWARD DECLARATIONS
class CCTSecurityDialogRequestor;

// CLASS DECLARATION

/**
*  This class implements MPKIDialog interface.
*  
*
*  @lib CTSecDlgs.lib
*  @since Series60_3:0
*/
NONSHARABLE_CLASS( CPKIDialog ): 
    public CBase, public MSecurityDialogResponse, public MPKIDialog
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CPKIDialog* NewL();
        
    
    public: // Functions from MPKIDialog
    
       /**
    	* User authentication dialogs
    	* @param aCertHandleList	A selection of certificates to display to the user.
    	*							All certificates are displayed if this is empty.
    	* @param aCertHandle		On return, an identifier for the certificate the user selected.
    	* 							aStatus - this will be set to KErrNotFound if no certificates
    	*							could be presented to the user.
    	* @param aStatus			This will be set to KErrNotFound if no certificates could
    	* 							be presented to the user.
    	*/
    	void UserAuthentication(
	            const RArray<TCTTokenObjectHandle>& aCertHandleList, 
	            TCTTokenObjectHandle& aCertHandle,
	            TRequestStatus& aStatus );
        
        /**
    	* User authentication dialogs with text
    	* 
    	* @param aTextToSign		The text to be signed.
    	* @param aCertHandleList	A selection of certificates to display to the user.
    	*							All certificates are displayed if this is empty.
    	* @param aCertHandle		On return, an identifier for the certificate the user selected.
    	* 							aStatus - this will be set to KErrNotFound if no certificates
    	*							could be presented to the user.
    	* @param aStatus			This will be set to KErrNotFound if no certificates could
    	* 							be presented to the user.
    	*/
    	void UserAuthenticationText(
                const TDesC& aTextToSign,
	            const RArray<TCTTokenObjectHandle>& aCertHandleList, 
	            TCTTokenObjectHandle& aCertHandle,
	            TRequestStatus& aStatus );
    						   
    	/**
    	* Certificate signing request dialog
    	* 
    	* @param aSubject		    Subject of the certificate
    	* @param aKeyHandle      	An identifier of the private used in the CSR.                           
    	* @param aStatus			This will be set to KErrNotFound if no certificates could
    	* 							be presented to the user.
    	*/
    	void CreateCSR( 
    	        const TDesC& aSubject,
                const TCTTokenObjectHandle& aKeyHandle,
                TRequestStatus& aStatus );					  

            
        /**
        * Shows details of the certificate
        * @param aCertHandle	    An identifier of the certificate
        * @param aStatus            KErrNone or KErrNotSupported
        *                           KErrNotFound if no certificates could
    	* 							be presented to the user.  
        * @return void
        */
        void CertificateDetails( 
                const TCTTokenObjectHandle& aCertHandle,
                TRequestStatus& aStatus  );
        /**
        * Asks user's approval for certificate saving.
        * @param aFormat		   The format of the certificate.
        * @param aCertificateOwnerType	The owner type.
        * @param aCert					The certificate to be added.
        * @param aStatus            KErrNone, KErrCancel or KErrNotSupported,
        *                           KErrArgument if error in certificate parsing
        * @return void
        */
        void SaveCertificate( 
                TCertificateFormat aFormat,
                TCertificateOwnerType aCertificateOwnerType, 
                const TDesC8& aCert, 
                TRequestStatus& aStatus);
        
        /**
        * Asks user's approval for certificate deleting.
        * @param aCertHandle	    An identifier of the certificate
        * @param aStatus            KErrNone, KErrCancel or KErrNotSupported,
        *                           KErrNotFound if no certificates could
    	* 							be presented to the user.    
        * @return void
        */
        void DeleteCertificate( 
                const TCTTokenObjectHandle& aCertHandle,
                TRequestStatus& aStatus  );
        
        /**
        * Informs the user during digital signing.
        * @param aNoteType Identifies the note type.
        * @param aStatus KErrNone or KErrNotSupported
        * @return void
        */
        void Note( MPKIDialog::TNoteType aNoteType,
                   TRequestStatus& aStatus );
        
        /**
        * Saves receipt to Notepad if receipt saving is on.
        * @param aSignedText Signed text in readable form.
        * @param aStatus KErrNone or KErrNotSupported
        * @return void
        */
        void SaveSignedText( const TDesC& aSignedText,
                             TRequestStatus& aStatus );
        
        /**
        * Shows instructions how to unblock PIN.
        * @param aLabel Label of the PIN.
        * @param aStatus KErrNone or KErrNotSupported
        * @return void
        */
        void PINBlockedInfo( const TPINLabel& aLabel,
                             TRequestStatus& aStatus );       
        /**
        * Release is called to free resources of the MDigSigningNote class
        */
        void Release();
        
    public: // Functions from MSecurityDialogResponse
    
        void HandleResponse(TInt aResult);
        

    private:

        /**
        * C++ default constructor.
        */
        CPKIDialog();
        
        /**
        * Destructor.
        */
        ~CPKIDialog();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
        
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
           
        void RequestWithTokenHandleL( 
            const TDesC& aText,
            const TCTTokenObjectHandle& aTokenHandle,
            TRequestStatus& aStatus );   
            
        void DoSaveCertificateL( 
            TCertificateFormat aFormat,
            TCertificateOwnerType aCertificateOwnerType, 
            const TDesC8& aCert, 
            TRequestStatus& aStatus);
                       
    private:    // Data
        CCTSecurityDialogRequestor* iRequester;	  // Owned
        TRequestStatus* iClientStatus;  // Pointer to client's request status
									    // We must signal it when dialogrequestor
									    // RunL is called to signal the completion of a dialog!
        
	    CBufFlat*	iSenderBuffer;	// Stream buffer for outgoing streams.
    	HBufC8*     iBufferData;    // iSenderBuffer help buffer
    	
	    TDialogType iCurrentOperation;
	
	    TSignInputBuf iSignInputBuf;
	    TPtrC8 iVariableDataPtr;
	    TCTTokenObjectHandle* iClientCertInfoHandlePtr;
	    TCTTokenObjectHandleBuf iCertInfoHandleBuf;
	    TBuf8<10> iDlgResponseBuf;
    };

#endif      // CPKIDLG_H
            
// End of File
