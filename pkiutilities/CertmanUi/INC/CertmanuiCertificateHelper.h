/*
* Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Declaration of the CCertManUICertificateHelper class
*
*/


#ifndef  CERTMANUICERTIFICATEHELPER_H
#define  CERTMANUICERTIFICATEHELPER_H

// INCLUDES
#include <pkixcertchain.h>          // for validation
#include <e32std.h>
#include "CertmanuiKeeper.h"
#include "CertmanuiCommon.h"
// FORWARD DECLARATIONS

class CCertManUIKeeper;

// CLASS DECLARATION


/**
*
*/
NONSHARABLE_CLASS( CCertManUICertificateHelper )
    {
    public:
        CCertManUICertificateHelper( CCertManUIKeeper& aKeeper );

        ~CCertManUICertificateHelper();

       /**
        * If CertLabel, Issuer and Owner length is over 80 characters, cut it down to 80
        * Returns the cut certificate field.
        */
        TPtrC CutCertificateField(TPtrC aField);

       /**
        * Check whether certificate is deletable
        * @param aIndex Index to certificate array
        * @param aIsCA Flag whether CA/User certificate is in use
        * @return ETrue if certificate is deletable
        */
        TBool IsCertificateDeletable(TInt aIndex, TInt aType)const;

       /**
        * Check whether one marked certificate is deletable
        * @param aListbox Pointer to active view's listbox
        * @param aIsCA Flag whether CA/User certificate is in use
        * @return ETrue if certificate is deletable
        */
        TBool IsOneMarkedCertificateDeletable(CEikColumnListBox* aListBox,
                                                TInt aType) const;

       /**
        * Creates the whole of certificate details view
        * Appends strings from both resources and CertManAPI to one
        * message body text and displays it.
        */
        void MessageQueryViewDetailsL( TInt aIndex, TInt aType,
                                        CEikonEnv* aEikonEnv );

       /**
        * Checks if the certificate is corrupted
        * Checks the validity period of the certificate
        */
        void ValidateCertificateL(
            TInt aIndex, CCertificate* aDetails, const CCTCertInfo& aCertInfo, TInt aType);

       /**
        * Reads line from resources
        */
        void DetailsResourceL(HBufC& aMessage, TInt aResourceOne);

       /**
        * Deletes either certificate in focus if there are no marked items,
        * or deletes the marked items
        * Shows read-only information notes if certificates are not deletable
        * Returns ETrue if at least one certificate is deleted
        */
        TBool ConfirmationQueryDeleteCertL(TInt aType,
                                       CEikColumnListBox* aListBox);

       /**
        * Handle markable list command. Does nothing
        * if listbox is not markable or empty.
        */
        void HandleMarkableListCommandL( TInt aCommand, CEikColumnListBox* aListBox);

       /**
        * Displays Delete yes/no confirmation notes
        */
        TInt DisplayConfirmationLD(HBufC* aString);

       /**
        * Moves one key from one store to another
        */
        void MoveKeyL( const TCTKeyAttributeFilter& aFilter,
                       const TUid aSourceStore,
                       const TUid aTargetStore );

       /**
        * Moves one certificate from one store to another
        */
        void MoveCertL( CCTCertInfo& aEntry, TInt aPosition,
                        CEikColumnListBox* aListBox,
                        const TUid aSourceStore,
                        const TUid aTargetStore );

    private:
       /**
        * Appends a field that has string from resources and string from CertManAPI
        * to the details view message body
        */
        void DetailsFieldDynamicL(HBufC& aMessage, TPtrC aValue,
                                  TInt aResourceOne, TInt aResourceTwo);

       /**
        * Appends a field that has two strings from resources
        * to the details view message body
        */
        void DetailsFieldResourceL(HBufC& aMessage, TInt aResourceOne, TInt aResourceTwo);

       /**
        * Devides string to four characters blocks
        * and delimits them with space.
        * @param aInput String to devide
        * @param aOutput Descriptor where result is appended
        * @return void
        */
        void DevideToBlocks( const TDesC8& aInput, TPtr aOutput );

       /**
        * Displays the warning note for corrupted/expired/not yet valid certificate
        */
        void DisplayWarningNoteLD(HBufC* aString);

       /**
        * Checks if the X509 certificate is corrupted and the validity period of
        * the certificate. Display only one warning note, in the following order:
        * 1) Expired/Not yet valid
        * 2) Corrupted
        * 3) Not trusted
        * @param aDetails
        * @return
        */
        CArrayFixFlat<TValidationError>* ValidateX509CertificateL( CX509Certificate* aDetails );

       /**
        * Reads dynamic text from CertManAPI, if the string is empty
        * put a not defined text from the resource in its place
        */
        void DetailsDynamicL(HBufC& aMessage, TPtrC aValue, TInt aResourceOne);

       /**
        * Gets the certificate name (or No label/No subject) for the Confirmation note
        */
        TInt DeleteSingleItemQueryL(CCTCertInfo& aEntry);

       /**
        * Deletes one item from CertManAPI and from listbox
        */
        void DeleteSingleItemL(CCTCertInfo& aEntry,
                               TInt aPosition,
                               CEikColumnListBox* aListBox,
                               TInt aType);

       /**
        * Gets the certificate name (or No label/No subject) for the read-only note
        */
        void ReadOnlySingleItemNoteL(CCTCertInfo& aEntry);


       /**
        * Displays read-only information notes
        */
        void DisplayNoteLD(HBufC* aString);

       /**
        * Check if Disk space goes below critical level if allocating
        * more disk space.
        * @param aShowErrorNote If ETrue, a warning note is shown.
        * @param aBytesToWrite Amount of disk space to be allocated (use an
        * estimate if exact amount is not known).
        * @return ETrue if allocation would go below critical level.
        */
        static TBool FFSSpaceBelowCriticalLevelL
            ( TBool aShowErrorNote, TInt aBytesToWrite = 0 );

       /**
        * Returns usage of key
        * @param aCertEntry Certificate in handling.
        * @param aKeyLocation Where is key is being stored
        */

        TKeyUsagePKCS15 GetKeyUsageAndLocationL(CCTCertInfo* aCertEntry, TUid* aKeyLocation);

       /**
        * Set certificate/key location info to certificate details view
        * @param aMessage Certificate details string
        * @param aCertificate flag whether handling certificate or key
        * @param aLocUid The location UID of cert/key
        */
        void SetLocationInfoL(HBufC& aMessage, TBool aCertificate, TUid* aLocUid);

        /**
         * Helper function for MessageQueryViewDetailsL() member. Fills in
         * message body text for certificate details dialog.
         * @param aIndex Certificate index, from MessageQueryViewDetailsL().
         * @param aType Certificate type, from MessageQueryViewDetailsL().
         * @param aEikonEnv Pointer to EIKON environment, from MessageQueryViewDetailsL().
         * @param aMessage Message body text to be filled in.
         */
        void CreateMessageBodyTextL(TInt aIndex, TInt aType, CEikonEnv* aEikonEnv,
                HBufC& aMessage);

    private:  //Data

        CCertManUIKeeper&   iKeeper;

    };

#endif //   CERTMANUICERTIFICATEHELPER_H

// End of File