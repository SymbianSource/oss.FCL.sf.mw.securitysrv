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
* Description:  Represents one certificate info with trust settings.
*                This entity is called here as a "mapping".
*
*/


#ifndef WIMCERTSTOREMAPPING_H
#define WIMCERTSTOREMAPPING_H

//  INCLUDES

#include <e32base.h>
#include <s32std.h>
#include <ct.h>
#include <mctcertstore.h>

// CLASS DECLARATION

/**
* This class is used to associate an entry with the mappings class
*
*
*  @lib   WimPlugin
*  @since Series60 2.1
*/
class CWimCertStoreMapping : public CBase
    {
    public: 

        /**
        * Constructors
        */
        static CWimCertStoreMapping* NewL();

        static CWimCertStoreMapping* NewLC();

        /**
        * Destructor
        */
        ~CWimCertStoreMapping();

    public: 

        /**
        * Sets the mapping class certificate part (iEntry). 
        * If there is already an entry, it is deleted.
        * @param  aCertInfo (IN) Certificate info class.
        * @return void
        */
        void SetEntryL( CCTCertInfo* aCertInfo );

        /**
        * Returns a pointer to the mapping class certificate object (iEntry).
        * @return  A pointer to object of certificate info class.
        */
        CCTCertInfo* Entry() const;

        /**
        * Returns an array of applications the mapping supports.
        * @return  A array of of references to integers (application id's)
        */
        const RArray<TUid>& CertificateApps() const;

        /**
        * Sets an array of applications to the mapping.
        * @param  aCertificateApps (IN) TUids of applications
        * @return void
        */
        void SetCertificateAppsL( RArray<TUid>* aCertificateApps );

        /**
        * Returns a boolean value indicating whether or not
        * the certificate part of mapping supports given application.
        * @param   aApplication (IN) A TUid of application
        * @return  A boolean value true or false
        */
        TBool IsApplicable( const TUid& aApplication ) const;

        /**
        * Returns a boolean value indicating whether or not
        * the certificte part of mapping is trusted.
        * @return  A boolean value true or false
        */
        TBool Trusted() const;

        /**
        * Sets a boolean value to mapping trusted settings
        * indicating whether or not the certificate part of mapping is trusted.
        * @param   aTrusted (IN) A boolean value
        * @return ETrue, when trusted, EFalse when not trusted
        */
        void SetTrusted( TBool aTrusted );

    private:

        /**
        * Second phase constructor
        */
        void ConstructL();

        /**
        * Default constructor
        */
        CWimCertStoreMapping();

    private:

        // A pointer to the certificate part of one mapping entry. 
        // This class don't own the pointed object.
        CCTCertInfo* iEntry;

        // An array of applications the certificate part of mapping supports.
        // This class owns the pointer objects.
        RArray<TUid>* iCertificateApps;

        // Is the certificate part of this mapping trusted or not
        TBool iTrusted;
    };

#endif // WIMCERTSTOREMAPPING_H

//end of file
