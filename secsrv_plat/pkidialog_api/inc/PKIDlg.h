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
* Description:  Interface for PKI dialog
*
*/


#ifndef MPKIDIALOG_H
#define MPKIDIALOG_H

//  INCLUDES
#include <e32base.h>
#include <ct.h>

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  PKI Dialog interface
*  
*  @since Series60 3,0
*/
class MPKIDialog
    {
    public:
        
        /**
        * TNoteType defines different possible note types.
        */
        enum TNoteType
            {
            ESignatureRequested = 0xFF00,   // Digital signature requested
            ESignatureDone,                 // Digital signature done.
            ESigningCancelled,              // Digital signing cancelled   
            ENoMatchCert,                   // No matchig certificate found
            ENoSecurityModule,              // Security Module unvailable
            EInternalError                  // Error note in any other case
            };
    public: // New functions
    
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
    	virtual void UserAuthentication(
                        const RArray<TCTTokenObjectHandle>& aCertHandleList, 
    			        TCTTokenObjectHandle& aCertHandle,
    				    TRequestStatus& aStatus ) = 0;
        
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
    	virtual void UserAuthenticationText( 
    	                const TDesC& aTextToSign,
                        const RArray<TCTTokenObjectHandle>& aCertHandleList, 
                        TCTTokenObjectHandle& aCertHandle,
                        TRequestStatus& aStatus ) = 0;
    						   
    	/**
    	* Certificate signing request dialog
    	* 
    	* @param aSubject		    Subject of the certificate
    	* @param aKeyHandle      	An identifier of the private used in the CSR.                           
    	* @param aStatus			This will be set to KErrNotFound if no certificates could
    	* 							be presented to the user.
    	*/
    	virtual void CreateCSR( 
                        const TDesC& aSubject,
                        const TCTTokenObjectHandle& aKeyHandle,
                        TRequestStatus& aStatus ) = 0;					  

            
        /**
        * Shows details of the certificate
        * @param aCertHandle	    An identifier of the certificate
        * @param aStatus            KErrNone or KErrNotSupported
        *                           KErrNotFound if no certificates could
    	* 							be presented to the user.  
        * @return void
        */
        virtual void CertificateDetails( 
                        const TCTTokenObjectHandle& aCertHandle,
                        TRequestStatus& aStatus  ) = 0;
        /**
        * Asks user's approval for certificate saving.
        * @param aFormat		   The format of the certificate.
        * @param aCertificateOwnerType	The owner type.
        * @param aCert					The certificate to be added.
        * @param aStatus            KErrNone, KErrCancel or KErrNotSupported,
        *                           KErrArgument if error in certificate parsing
        * @return void
        */
        virtual void SaveCertificate( 
                        TCertificateFormat aFormat,
                        TCertificateOwnerType aCertificateOwnerType, 
                        const TDesC8& aCert, TRequestStatus& aStatus) = 0;
        
        /**
        * Asks user's approval for certificate deleting.
        * @param aCertHandle	    An identifier of the certificate
        * @param aStatus            KErrNone, KErrCancel or KErrNotSupported,
        *                           KErrNotFound if no certificates could
    	* 							be presented to the user.    
        * @return void
        */
        virtual void DeleteCertificate( 
                        const TCTTokenObjectHandle& aCertHandle,
                        TRequestStatus& aStatus  ) = 0;
        
        /**
        * Informs the user during digital signing.
        * @param aNoteType Identifies the note type.
        * @param aStatus KErrNone or KErrNotSupported
        * @return void
        */
        virtual void Note( MPKIDialog::TNoteType aNoteType,
                           TRequestStatus& aStatus ) = 0;
        
        /**
        * Saves receipt to Notepad if receipt saving is on.
        * @param aSignedText Signed text in readable form.
        * @param aStatus KErrNone or KErrNotSupported
        * @return void
        */
        virtual void SaveSignedText( const TDesC& aSignedText,
                                     TRequestStatus& aStatus ) = 0;
        
        /**
        * Shows instructions how to unblock PIN.
        * @param aLabel Label of the PIN.
        * @param aStatus KErrNone or KErrNotSupported
        * @return void
        */
        virtual void PINBlockedInfo( const TPINLabel& aLabel,
                                     TRequestStatus& aStatus ) = 0;       
        /**
        * Release is called to free resources of the MDigSigningNote class
        */
        virtual void Release() = 0;

    protected: 
        
        // Destructor
        inline virtual ~MPKIDialog() = 0;

    };

inline MPKIDialog::~MPKIDialog(){}

/**
* Factory for creating the relevant concrete subclass
* of the PKI Dialog.
*/
class PKIDialogFactory
    {
    public:
    
    /**
     * Creates an instance of a subclass of MPKIDialog. 
     * Implement to create the appropriate security dialog.
     * 
     * @param returns an object that implements MPKIDialog methods
     */
    IMPORT_C static MPKIDialog* CreateNoteL(); 
    
    };

#endif      // MPKIDIALOG_H   
            
// End of File
