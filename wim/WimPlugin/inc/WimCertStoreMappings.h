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
* Description:  Implements a class managing mapping entries which contain
*                information of certificates with trusted settings
*
*/


#ifndef WIMCERTSTOREMAPPINGS_H
#define WIMCERTSTOREMAPPINGS_H

// INCLUDES 

#include "WimCertStoreMapping.h"
#include <s32file.h>

// CLASS DECLARATION

/**
* This class manages the entries in an internal array
* where the data of the certificate is stored.
*
*
*  @lib   WimPlugin
*  @since Series60 2.1
*/
class CWimCertStoreMappings : public CBase
    {
    public: 

        /**
        * Constructor
        */
        static CWimCertStoreMappings* NewL();

        /**
        * Destructor
        */
        ~CWimCertStoreMappings();

    public: 

        /**
        * Returns the count of mapping entries in the array
        * @return A integer value
        */
        TInt Count() const;

        /**
        * Adds a new mapping entry in the array
        * @param  aEntry (IN) One mapping entry
        * @return void
        */
        void AddL( const CWimCertStoreMapping* aEntry );

        /**
        * Removes an mapping entry from the array
        * @param   aEntry The entry to delete
        * @return  KErrNotFound if aEntry is not found in the array 
        *          KErrNone is the entry was successfully deleted.
        */
        //TInt Remove( const CCTCertInfo& aEntry );

        /**
        * Updates a new entry in the array
        * @param  aEntry (IN) One certificate info
        * @return void
        */
        void UpdateL( const CCTCertInfo& aEntry );

        /**
        * Returns the index in the array. The function goes through
        * the array entries and returns the index of the first entry whose 
        * label matches the label of aEntry.
        * @param   aEntry The certificate info in the mapping entry
        *          we want the index of.
        * @return  The index of mapping entry in the array.
        */
        TInt Index( const CCTCertInfo& aEntry );

        /**
        * Returns one mapping entry from the array. 
        * @param   aIndex The index of the mapping
        * @return  The mapping from the array.
        */
        CWimCertStoreMapping* Mapping( TInt aIndex );
    
        /**
        * Returns certificate info from the array in the given index. 
        * @param   aIndex The index of the certificate info
        * @return  The certificate from the array.
        */
        const CCTCertInfo& Entry( TInt aIndex );

        /**
        * Returns certificate info from the array using given handle. 
        * @param   aHandle The handle of the mapping
        * @return  The certificate from the array.
        */
        const CCTCertInfo& EntryByHandleL( TInt aHandle ) const;

        /**
        * Returns next available handle that can be used for 
        * adding a new mapping in to the array
        * @return  A integer value of new handle.
        */
        //TInt NextHandle() const;

    private:

        /**
        *   Second phase constructor
        */
        void ConstructL();

        /**
        *   Default constructor
        */
        CWimCertStoreMappings();

        // An internal array of mapping entries
        RPointerArray<CWimCertStoreMapping> iMappings;
    };

#endif // WIMCERTSTOREMAPPINGS_H

// End of file
