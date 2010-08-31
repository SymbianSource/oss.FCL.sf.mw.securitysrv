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
#include <e32base.h>
#include <bamdesca.h>
#include <ct/rmpointerarray.h>
#include <f32file.h> 

#include <qlist.h>
#include <qlabel.h>
#include <qvector.h>
#include <qpair.h>

class QString;
class HbListWidget;

// CONSTANTS
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
class MCTKeyStore;
class MCTToken;

class QString;

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
class CSecModUIModel : public CBase
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
		* Initializes unified keystore
		*/
		void InitializeKeyStoreL();
		
		/**
		* Retrieves the description and location of all tokens.        
		* @return Map containing the label and the appripriate location.
		*/        
		QMap<QString,QString> LoadTokenLabelsL();
		
		/**
		* Shows error note if needed.        
		* @param aError 
		* @return void
		*/
		void ShowErrorNote(TInt aError) const;
		
		/**
		* Retrieves the appropriate token based on the index.        
		* @param aTokenIndex Index of the token to be opened.
		* @return void
		*/
		void OpenTokenL(TInt aTokenIndex);
		
		/**
		* Initialize the auth objects and return the total count.
		* @param aItemArray contains localised stings 
		* @return void
		*/
		TInt CheckCodeViewStringsL();
		
		/**
		* Retrieves description of the token based on UID of the current active keystore.                
		* @return Description
		*/
		QString TokenLabelForTitle() const;
		
		/**
		 * Returns status of the selected auth object.
		 * @param index
		 * @return status
		 */
		TUint32 AuthStatus(TInt aIndex) const;
		
		/**
		* Returns status of the auth object based on its persistent properties set
		* by security module implementation.        
		* @param aIndex Index of the authentication object
		* @param aShowBlockedNote If ETrue, shows code blocked if code is blocked.
		* @return status of the pin.
		*/
		QString PINStatus( TInt aIndex, TBool aShowBlockedNote ) const;
		
		/**
		* Checks whether the Pin is still available.
		* @param aIndex Index of the authentication object.
		* @return status
		*/
		TBool PinOpen(TInt aIndex) const;
		
		/**
		* Based on the property of auth object the pin can be 
		* changed or unblocked.        
		* @param aIndex Index of the auth object
		* @return ?description
		*/
		void ChangeOrUnblockPinL(TInt aIndex);
		
		 /**
		* Changes the value of the PIN.
		* @param aIndex Index of the AO
		* @return void
		*/
		void ChangePinL(TInt aIndex);
		
		/**
		* Toggles the value of auth object if possible.        
		* @param ?arg1 ?description
		* @return ?description
		*/
		TInt ChangeCodeRequest(TInt aIndex);

		/**
		* If the auth object is open then closes it.       
		* @param aIndex Index of the AO to be closed.
		* @return void
		*/
		void CloseAuthObjL(TInt aIndex);
	     
		 /**
		* Unblocks PIN
		* @param aIndex Index of the AO
		* @return void
		*/
		void UnblockPinL(TInt aIndex);
		
		/**
		* Deletes key from the appropriate keystore.      
		* @param aTokenIndex Index of the token to be deleted.
		* @return ETrue if keystore was deleted.
		*/
		void DeleteKeysL(TInt aTokenIndex);
		 
		/**
		* Returns ETrue if PIN is changeable.        
		* @param aIndex Index of the AO
		* @return ETrue / EFalse
		*/
		TBool PinChangeable(TInt aIndex) const;
				
		/**
		* Displays security module details dialog.        
		* @param aTokenIndex Index of the token, which details are shown.
		* @return void
		*/
		QVector< QPair<QString,QString> > SecModDetailsL(TInt aTokenIndex);
		
		/**
		* Displays details dialog of the active security module .                        
		* @return void
		*/
		QVector< QPair<QString,QString> > GetSecModDetailsL();
		
		/**
		* Appends the location of the keystore.                        
		* @return void
		*/
		void AppendLocation( QVector< QPair<QString,QString> >& aSecModDetails, 
							 TUid aLocUid, 
							 QString aCertDisplayDetails );
		
		/**
		* Appends the value of a label if present otherwise appends empty descriotion.                        
		* @return void
		*/
		void AppendItem( 	QVector< QPair<QString,QString> >& aSecModDetails, 
							QString aLabel, 
							const TDesC& aValue, 
							QString aNullDescription );
		
		/**
		* Generate details of signing security module.                        
		* @return void
		*/
		void AppendPinNRs(QVector< QPair<QString,QString> >& aSecModDetails);

		QVector< QPair<QString, TUint32> > AuthDetails();
		
		/**
		* Changes PIN-NR
		* @param aIndex Index of the AO
		* @return void
		*/
		void ChangePinNrL(TInt aIndex);
		       
		/**
		* Unblocks PIN-NR
		* @param aIndex Index of the AO
		* @return void
		*/
		void UnblockPinNrL(TInt aIndex);
		
		/**
		 * Retrieves the UID of the current active keystore.
		 * @return uid
		 */
		TInt KeyStoreUID();     
		
		/**
		 * Retrieves the auth object at the speficied index.
		 * @return auth object
		 */
		const MCTAuthenticationObject& AuthObj(TInt aIndex);
		
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
		* Returns ETrue if token is deletable.
		* @param aTokenIndex Index of the token to be deleted.
		* @return ETrue / EFalse
		*/
		TBool IsTokenDeletable(TInt aTokenIndex);
		        
		
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
        
        void InitAuthObjectsL();
        
        void ListAuthObjectsL();
        
        void CreateSettingItem(const TDesC& aTitle, 
                               const TDesC& aValue,  
                               TDes& aItem) const;
        
        QString Location( TUid aUid ) const;         
        
        void AppendPinNRsL( TDes& aMessage ) const;   

    private:
        // DATA TYPES
        enum TSecModPanicCode
            {
            EPanicNullPointer,
            EPanicIndexOutOfRange
            };

    private:    // Data
        // Resource file offsets		
		TInt		iResourceFileOffset;
		TInt        iResourceFileOffset2;
		TInt        iResourceFileOffset3;
		
		CSecModUISyncWrapper* iWrapper; // owned
		RFs iRfs;
		
		/**
        * Crypto Token objects
        */
        CUnifiedKeyStore* iUnifiedKeyStore; // owned
        MCTKeyStore* iKeyStore; // not owned
        MCTAuthenticationObjectList*  iAOList; // owned, items not owned
        RMPointerArray<MCTAuthenticationObject> iAOArray; // items not owned		
		                                         
        RMPointerArray<MCTKeyStore> iAOKeyStores; // items not owned       		                                         
    };

#endif      // CSECMODUIMODEL_H   
            
// End of File
