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
* Description:   Dialog Requestor for RNotifier
*
*/


#ifndef __CTSECURITYDIALOGREQUESTOR_H
#define __CTSECURITYDIALOGREQUESTOR_H

//  INCLUDES
#include <e32base.h>
#include <ct.h>

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
* This class should be derived by every class
* which uses CCTSecurityDialogRequestor.
*
*  @lib CTSecDlgs.lib
*  @since Series60_2.6
*/

class MSecurityDialogResponse
    {
    public:
        /**
	    *   CWimSecurityDialogRequestor calls this function
        *   when it gets response from the notifier.
	    *	@param aResult      Result of the notifier	
	    */
        virtual void HandleResponse(TInt aResult) = 0;
    };

/**
*  Active object that sends requests to notifier. 
*
*  @lib 
*  @since 
*/

NONSHARABLE_CLASS( CCTSecurityDialogRequestor ): public CActive
    {
    private:
    	enum TState { EFillingVariableInputBuffer, EMakingRequest };

    public:
        
        /**
        * Two-phased constructor.
        */
	    static CCTSecurityDialogRequestor* NewL(MSecurityDialogResponse& aSecurityDialogs);
        
        /**
        * Destructor.
        */
	    ~CCTSecurityDialogRequestor();

    public: // New functions
        
	    /**
	    *	RequestDialog Requests a dialog through RNotifier API
	    *	@param aData	Stream to be sent through RNotifier
	    *	@param aResponse	Reference to memory where the response from Notifier is to be saved to
	    */
	    void RequestDialog(const TDesC8& aData, TDes8& aResponse);
	    
	    void RequestVariableBufferDialog( const TDesC8& aOperationData,
										const TDesC8& aVariableData,
										const RArray<TCTTokenObjectHandle>& aCertHandleList,
										TDes8& aResponse );


    private:
        
        /**
        * C++ default constructor.
        */
	    CCTSecurityDialogRequestor(MSecurityDialogResponse& aSecurityDialogs);
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();

    private:  // Functions from CActive
        /**
        *
        */
	    void DoCancel();
        /**
        *
        */
	    void RunL();
        /**
        *
        * @param aError
        */
        TInt RunError(TInt aError);

    private:    // Data
        
	    MSecurityDialogResponse&	iSecurityDialogs;
	    RNotifier	iNotifier;
	    
	    TState iState;
	    const TDesC8* iOperationDataPtr;
	    const TDesC8* iVariableDataPtr;
	    const RArray<TCTTokenObjectHandle>* iCertHandleListPtr;
	    HBufC8* iInputBuffer;
	    TDes8* iResponsePtr;
    };

#endif  // __CTSECURITYDIALOGREQUESTOR_H

// End of File