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
* Description:  Wrapper class for security dialog management
*
*/



#ifndef CWIMSECURITYDLGHANDLER_H
#define CWIMSECURITYDLGHANDLER_H

//  INCLUDES
#include    <e32base.h>
#include    <secdlg.h>  // Symbian security dialog interface
#include    <data_caging_path_literals.hrh>
//#include    <PKIDlg.h>

// CONSTANTS
_LIT( KWimSecDlgsDllName, "CTSecDlgs.dll" );        // Security dialogs DLL

// CLASS DECLARATION

/**
*  Wrapper for security dialog management
*
*  @since Series60 2.6
*/
class CWimSecurityDlgHandler : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimSecurityDlgHandler* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CWimSecurityDlgHandler();

    public: // New functions
        
        /**
        * Shows a security dialog to enable/disable PIN query.
        * @param aEnable TRUE = enable query, FALSE = disable query.
        * @param aRetry is it first time or retry.
        * @param aPINParams PIN parameters.
        * @param aPINValue returned PIN value.
        * @param aStatus Status of request.
        * @return   void
        */
        void EnablePIN( TBool aEnable,
                        TBool aRetry,
                        const TPINParams& aPINParams, 
                        TPINValue& aPINValue,
                        TRequestStatus& aStatus );


        /**
        * Shows a security dialog for changing PIN value.
        * @param aRetry Is it first time or retry?
        * @param aPINParams PIN parameters.
        * @param aOldPINValue old PIN to be chanced.
        * @param aNewPINValue New PIN to set up.
        * @param aStatus Status of request.
        * @return   void
        */
        void ChangePINValue( TBool aRetry,
                             const TPINParams& aPINParams,
                             TPINValue& aOldPINValue,
                             TPINValue& aNewPINValue,
                             TRequestStatus& aStatus );

        /**
        * Shows the note about blocked PIN to the user.
        * @param aPINParams PIN parameters.
        * @param aStatus Status of request.
        * @return   void
        */
        void ShowPINBlocked( const TPINParams& aPINParams, 
                             TRequestStatus& aStatus );
        
        /**
        * Shows the note about total blocked PIN to the user.
        * @param aPINParams PIN parameters.
        * @param aStatus Status of request.
        * @return   void
        */
        void ShowPINTotalBlocked( const TPINParams& aPINParams, 
                                  TRequestStatus& aStatus );
        /**
        * Shows the note about card removing to the user.
        * @param aStatus Status of request.
        * @return   void
        */                     
        void ShowCardRemoved( TRequestStatus& aStatus );

        /**
        * Shows a security dialog for fetching PIN and PUK 
        * values for unblocking blocked PIN.
        * @param aRetry is it first time or retry.
        * @param aBlockedPINParams Blocked PIN parameters.
        * @param aUnblockingPINParams Unblocking PIN parameters.
        * @param aUnblockingPINValue Unblocking PIN value.
        * @param aNewPINValue New PIN to set up.
        * @param aStatus Status of request.
        * @return   void
        */
        void UnblockPIN( TBool aRetry,
                         const TPINParams& aBlockedPINParams,
                         const TPINParams& aUnblockingPINParams,
                         TPINValue& aUnblockingPINValue, 
                         TPINValue& aNewPINValue,
                         TRequestStatus& aStatus );

        /**
        * Shows a security dialog for asking user to enter PIN code.
        * @param aRetry is it first time or retry.
        * @param aPINParams PIN parameters.
        * @param aPINValue PIN value to be query.
        * @param aStatus Status of request.
        * @return   void
        */
        void EnterPIN( TBool aRetry, 
                       const TPINParams& aPINParams, 
                       TPINValue& aPINValue,
                       TRequestStatus& aStatus );
                       
        /**
        * Close the ongoing dialog 
        * @param
        * @return void 
        */
        void Cancel();               

    private:

        /**
        * C++ default constructor.
        */
        CWimSecurityDlgHandler();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();


        /**
        * Load and get new instance of security dialog. Completes status
        * if call to DoLoadSecurityDialogL() leaves (Dll load fails).
        * @return Error code (KErrNone means dialog loaded succesfully)
        * If something else than KErrNone status is completed with same error.
        */
        TInt LoadSecurityDialog( TRequestStatus& aStatus );
        
        /**
        * Load and get new instance of PKI dialog. Completes status
        * if call to DoLoadPKIDialogL() leaves (Dll load fails).
        * @return Error code (KErrNone means dialog loaded succesfully)
        * If something else than KErrNone status is completed with same error.
        */
        TInt LoadPKIDialog( TRequestStatus& aStatus );
        
        /**
        * Load and get new instance of security dialog. Leaves if load fails.
        * Leave is trapped in LoadSecurityDialog().
        * @return void
        */
        void DoLoadSecurityDialogL();
        
        /**
        * Load and get new instance of PKI dialog. Leaves if load fails.
        * Leave is trapped in LoadPKIDialog().
        * @return void
        */
        void DoLoadPKIDialogL();
        
    private:    // Data
        // Pointer to security dialog class. Owned.
        MSecurityDialog* iSecurityDlg;
        
        //Pointer to PKI dialog class Owned
        //MPKIDialog*      iPKIDlg;
        
        // Pointer to RLibrary object. Owned.
        RLibrary         iLibrary;
         
    };

#endif      // CWIMSECURITYDLGHANDLER_H   
            
// End of File
