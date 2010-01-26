/*
* Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Declaration of the CSecModUIModel class 
*
*/


#ifndef CSECMODUIMODEL_H
#define CSECMODUIMODEL_H

//  INCLUDES
#include "SecModUI.h"
#include <e32base.h>
#include <bamdesca.h>
#include <ct/rmpointerarray.h>

// CONSTANTS
const TInt KPinGIndex = 0; // PIN-G should be always first AO in the AO list.
const TInt KMaxSettItemSize = 200;

const TInt KPinGSettIndex = 0;
const TInt KPinNrSettIndex = 1;

const TUid KUidSecMod = { 0x101F8668 };

// Panic strings
_LIT(KPanicNullPointer, "SecModUI: invalid pointer");
_LIT(KPanicIndexOutOfRange, "SecModUI: array indexed out of range");

// FORWARD DECLARATIONS
class CUnifiedKeyStore;
class MCTAuthenticationObject;
class MCTAuthenticationObjectList;
class CSecModUISyncWrapper;
class CEikTextListBox;
class MCTKeyStore;
class MCTToken;
class CEikonEnv;


// DATA TYPES
enum TSecModPanicCode
    {
    EPanicNullPointer,
    EPanicIndexOutOfRange
    };

// FUNCTION PROTOTYPES
GLREF_C void Panic( TInt aPanic );

// CLASS DECLARATION

/**
*  class CSecModUIModel
*  Model of the application
*
*  @lib SecModUI.lib
*  @since Series 60 3.0
*/
class CSecModUIModel : public CBase, public MSecModUI
    {
    public:  // Constructors and destructLoadTokenLabelsLor
        
        /**
        * Two-phased constructor.
        */        
        static CSecModUIModel* NewL();
                
        /**
        * Destructor.
        */
        virtual ~CSecModUIModel();

    public: // New functions
        
        /**
        * Activates title to the title pane        
        * @param aTitle Title to be activated
        * @return void
        */
        void ActivateTitleL(const TDesC& aTitle) const;
        
        /**
        * Activates current tokens label to the title pane                
        * @return void
        */
        void ActivateTokenLabelToTitleL() const;
    
        /**
        * Initializes unified keystore        
        * @return Error value
        */
        TInt InitializeKeyStoreL();
        
        /**
        * Load labels of the tokens to the listbox        
        * @param aListBox Listbox
        * @return void
        */        
        void LoadTokenLabelsL(CEikTextListBox& aListBox);
        
        /**
        * Checks that does active token have also signing code(s)
        * @param aItemArray contains localised stings 
        * @return void
        */
        void CheckCodeViewStringsL(MDesCArray& aItemArray);
        
        /**
        * Opens and activates token        
        * @param aTokenIndex Index of the token to be opened.
        * @return void
        */
        void OpenTokenL(TInt aTokenIndex);
        
        /**
        * Opens authentication object view        
        * @param aIndex 0 is access code view
        *               1 is signing code view
        * @return void
        */
        void OpenAuthObjViewL(TInt aIndex);
        
        /**
        * Closes authentication object       
        * @param aIndex Index of the AO to be closed.
        * @return void
        */
        void CloseAuthObjL(TInt aIndex);
        
        /**
        * Loads PIN-NR labels to the listbox        
        * @param aListBox 
        * @param aShowBlockedNote If ETrue, shows code blocked if code is blocked.
        * @return void 
        */
        void LoadPinNRLabelsL(CEikTextListBox& aListBox, TBool aShowBlockedNote);
        
        /**
        * Loads items of the access code view list         
        * @param aListBox where items are appended.
        * @return void
        */
        void LoadPinGItemsL(CEikTextListBox& aListBox);
        
        /**
        * Writes AO status item to the descriptor        
        * @param aIndex Index of the authentication object
        * @param aItem Descriptor where status is written.
        * @param aShowBlockedNote If ETrue, shows code blocked if code is blocked.
        * @return void
        */
        void PINStatusItemL(
            TInt aIndex, TDes& aItem, TBool aShowBlockedNote) const;
        
        /**
        * Writes AO request status to the descriptor
        * @param aIndex Index of the authentication object
        * @param aItem Descriptor where status is written.
        * @return void
        */
        void PINRequestItemL(TInt aIndex, TDes& aItem) const;
        
        /**
        * Displays security module details dialog.        
        * @param aTokenIndex Index of the token, which details are shown.
        * @return void
        */
        void ViewSecModDetailsL(TInt aTokenIndex);
        
        /**
        * Displays details dialog of the active security module .                        
        * @return void
        */
        void ViewOpenedSecModDetailsL();
        
        /**
        * Deletes keystore        
        * @param aTokenIndex Index of the token to be deleted.
        * @return ETrue if keystore was deleted.
        */
        TBool DeleteKeyStoreL(TInt aTokenIndex);
        
        /**
        * Returns ETrue if token is deletable.
        * @param aTokenIndex Index of the token to be deleted.
        * @return ETrue / EFalse
        */
        TBool IsTokenDeletable(TInt aTokenIndex);
        
        /**
        * Launches change or unblock operation depending
        * of the status of the PIN-NR        
        * @param aIndex Index of the AO
        * @return ?description
        */
        void ChangeOrUnblockPinNrL(TInt aIndex);
        
        /**
        * Launches change or unblock operation depending
        * of the status of the PIN        
        * @param aIndex Index of the AO
        * @return ?description
        */
        void ChangeOrUnblockPinL(TInt aIndex);
        
        /**
        * Changes PIN-NR
        * @param aIndex Index of the AO
        * @return void
        */
        void ChangePinNrL(TInt aIndex);
        
        /**
        * Changes PIN
        * @param aIndex Index of the AO
        * @return void
        */
        void ChangePinL(TInt aIndex);
        
        /**
        * Unblocks PIN-NR
        * @param aIndex Index of the AO
        * @return void
        */
        void UnblockPinNrL(TInt aIndex);
        
        /**
        * Unblocks PIN
        * @param aIndex Index of the AO
        * @return void
        */
        void UnblockPinL(TInt aIndex);
        
        /**
        * Returns ETrue if PIN-NR is changeable.        
        * @param aIndex Index of the AO
        * @return ETrue / EFalse
        */
        TBool PinNrChangeable(TInt aIndex) const;
        
        /**
        * Returns ETrue if PIN is changeable.        
        * @param aIndex Index of the AO
        * @return ETrue / EFalse
        */
        TBool PinChangeable(TInt aIndex) const;
        
        /**
        * Returns ETrue if PIN-NR is unblockable.        
        * @param aIndex Index of the AO
        * @return ETrue / EFalse
        */
        TBool PinNrUnblockable(TInt aIndex) const;
        
        /**
        * Returns ETrue if PIN is unblockable.        
        * @param aIndex Index of the AO
        * @return ETrue / EFalse
        */
        TBool PinUnblockable(TInt aIndex) const;
        
        /**
        * Returns ETrue if PIN request can be disabled / enabled.        
        * @param aIndex Index of the AO
        * @return ETrue / EFalse
        */
        TBool PinRequestChangeable(TInt aIndex) const;
        
        /**
        * Returs ETrue if PIN is open.
        * @param aIndex Index of the AO
        * @return ?description
        */
        TBool PinOpen(TInt aIndex) const;
        
        /**
        * Disables / enables PIN depending on the status of the PIN.        
        * @param ?arg1 ?description
        * @return ?description
        */
        TInt ChangeCodeRequestL(TInt aIndex);
        
        /**
        * Shows confirmation query.        
        * @param aResourceId Id of the resource
        * @return Response of the user.
        */
        TInt ShowConfirmationQueryL(TInt aResourceId) const;
       
        /**
        * Shows confirmation note dialog.        
        * @param aResourceId Id of the resource
        * @return void
        */
        void ShowConfirmationNoteL(TInt aResourceId) const;
        
        /**
        * Shows information note dialog.        
        * @param aResourceId Id of the resource
        * @return void
        */
        void ShowInformationNoteL(TInt aResourceId, const TDesC& aString = KNullDesC) const;
        
        /**
        * Shows error note if needed.        
        * @param aError 
        * @return void
        */
        void ShowErrorNoteL(TInt aError) const;
        
        CSecModUISyncWrapper& Wrapper();            
        
        /**
         * Returns the number of security modules.
         */
        TInt SecurityModuleCount() const;
        
    private:

        /**
        * C++ default constructor.
        */        
        CSecModUIModel();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();        
        
        void ResetAndDestroyCTObjects();
        
        void ResetAndDestroyAOs();
        
        void AddResourceFileL();
        
        void InitAuthObjectsL();
        
        void ListAuthObjectsL();
        
        void CreateSettingItem(const TDesC& aTitle, 
                               const TDesC& aValue,  
                               TDes& aItem) const;
        
        void AppendItemL(TDes& aMessage,
                         TInt aResItem, 
                         const TDesC& aValue, 
                         TInt aResNotDefined) const; 
         
        void AppendResourceL(TDes& aMessage, TInt aResource) const;
        
        void AppendResourceAndEnterL(TDes& aMessage, TInt aResource) const;
                        
        void AppendValueL(TDes& aMessage, 
                          const TDesC& aValue, 
                          TInt aResNotDefined) const;
        
        void AppendLocationL(TDes& aMessage, TUid aUid, TInt aItemRes=0) const;         
        
        void AppendPinNRsL( TDes& aMessage ) const;   

    private:    // Data
        // Resource file offsets		
		TInt		iResourceFileOffset;
		TInt        iResourceFileOffset2;
		TInt        iResourceFileOffset3;
		
		CSecModUISyncWrapper* iWrapper; // owned             
		
		CEikonEnv* iEikEnv;
		
		/**
        * Crypto Token objects
        */
        CUnifiedKeyStore* iUnifiedKeyStore; // owned
        MCTKeyStore* iKeyStore; // not owned
        MCTAuthenticationObjectList*  iAOList; // owned, items not owned
        RMPointerArray<MCTAuthenticationObject> iAOArray; // items not owned		
		                                         
        RMPointerArray<MCTKeyStore> iAOKeyStores; // items not owned       		                                         
    };

#endif      // CSecModUIModel_H   
            
// End of File
