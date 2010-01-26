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
* Description:  WIM Trust Settings Store handler
*
*/



#ifndef CWIMTRUSTSETTINGSHANDLER_H
#define CWIMTRUSTSETTINGSHANDLER_H

//  INCLUDES
#include <e32base.h>

// FORWARD DECLARATIONS
class CWimTrustSettingsStore;
class CWimCertInfo;

// CLASS DECLARATION

/**
*  Class for handling certificates in WIM card
*  
*  @since Series60 3.0
*/
class CWimTrustSettingsHandler : public CActive
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimTrustSettingsHandler* NewL( CWimTrustSettingsStore* aWimTrustSettingsStore );
        
        /**
        * Destructor.
        */
        virtual ~CWimTrustSettingsHandler();

    public: // New functions
        
        /**
        * Fetches trust settings for given certificate. 
        * 
        * @return void
        */
        void GetTrustSettingsL( const RMessage2& aMessage );

        /**
        * Set applicability for given certificate. New certificate entry is
        * set if one not found from database (trust flag is set to EFalse).
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void SetApplicabilityL( const RMessage2& aMessage );

        /**
        * Set trust flag for given certificate. New certificate entry is set
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void SetTrustL( const RMessage2& aMessage );

        /**
        * Set default trust settings for given certificate. If certificate not
        * found from database new entry is inserted.
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void SetDefaultTrustSettingsL( const RMessage2& aMessage );

        /**
        * Remove trust settings of given certificate. If certificate
        * is not found, message is completed with KErrNotFound
        * @param aMessage Encapsulates a client request.
        * @return void
        */
        void RemoveTrustSettingsL( const RMessage2& aMessage );

        /**
        * Cancel any issued asynchronous call
        * @return void
        */
        void CancelDoing();

    private:  // From CActive

        /**
        * From CActive. Handles completion of asynchrounous operation
        * @return void
        */
        void RunL();

        /**
        * From CActive. Cancellation function
        * @return void
        */
        void DoCancel();

        /**
        * From CActive. Handle trapped leave.
        * @param aError Leave error code
        * @return Error code
        */
        TInt RunError( TInt aError );

    private:

        /**
        * C++ default constructor.
        */
        CWimTrustSettingsHandler( CWimTrustSettingsStore* aWimTrustSettingsStore );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data

        // Pointer to Trust Settings Store instance. Owned.
        CWimTrustSettingsStore* iWimTrustSettingsStore;
        // Pointer CWimCertInfo object. Owned.
        CWimCertInfo*           iCertInfo;
        // Saved message.
        RMessage2               iMessage;
        // Flag to save trusted flag.
        TBool                   iTrusted;
        // Array to save applications.
        RArray<TUid>            iApplications;

        // Enumerator for ongoing phase
        enum TPhase
            {
            EGetTrustSettings,
            ESetDefaultTrustSettings,
            ESetApplicability,
            ESetTrust,
            ERemoveTrustSettings
            };

        // Currect phase of Trust Settings Store operation
        TPhase                  iPhase;

    };

#endif      // CWIMTRUSTSETTINGSHANDLER_H
    
// End of File
