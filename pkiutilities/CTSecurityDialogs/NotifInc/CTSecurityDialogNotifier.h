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
* Description:   Crypto Token Security Dialog Notifier
*
*/



#ifndef __CTSECURITYDIALOGNOTIFIER_H
#define __CTSECURITYDIALOGNOTIFIER_H

//  INCLUDES
#include "CTSecurityDialogDefs.h"
#include "CTSecurityDialogsLogger.h"
#include <secdlgimpldefs.h>
#include <eiknotapi.h>

// FORWARD DECLARATIONS
class RReadStream;
class CEikonEnv;
class CUnifiedCertStore;
class CCTCertInfo;
class CCertAttributeFilter;
class CCTSecurityDialogsAO;

// CLASS DECLARATION

/**
* CCTSecurityDialogNotifier is the Server-side class for Notifier plugin.
* @lib 
* @since
*/
NONSHARABLE_CLASS( CCTSecurityDialogNotifier ): public CBase, public MEikSrvNotifierBase2
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
	    static CCTSecurityDialogNotifier* NewL();
        
        /**
        * Destructor.
        */
	    ~CCTSecurityDialogNotifier();
    
    public: // new functions

        // Loads the resource strings etc for TDialogType aType
	    
	    HBufC* LoadResourceStringLC( const TInt aType, const TDesC& aDynamicText = KNullDesC, const TDesC& aDynamicText2= KNullDesC );
        
        // Finds the corresponding resource items for aType
	    TDialogTypeItem GetDialogTypeItem( const TInt aType );

    private: // from MEikSrvNotifierBase2

        /**
        * Deletes this
        */
	    void Release();

        /**
        * 
        * @return TNotifierInfo
        */
	    TNotifierInfo RegisterL();
        
        /**
        *
        * @return TNotifierInfo
        */
	    TNotifierInfo Info() const;
	    
	    /**
	    * StartL is the Notifier request handler.
	    * @param aBuffer Data that is passed from the client-side. 
	    * @param aReplySlot Identifies which message argument to use for the reply.
	    * @param aMessage Encapsulates a client request.
	    */
	    void StartL( const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage );
	    
        /**
        * Does nothing.
        * @param aBuffer
        * @return KNullDesC
        */
	    TPtrC8 StartL( const TDesC8& aBuffer );

        /**
        * Does nothing
        */
	    void Cancel();

        /**
        * Does nothing
        * @param aBuffer
        * @return KNullDesC
        */
	    TPtrC8 UpdateL( const TDesC8& aBuffer );

    private:

        /**
        * C++ default constructor.
        */
	    CCTSecurityDialogNotifier();
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
	    void ConstructL();
	    
	    void DoStartL( const TDesC8& aBuffer, TInt aReplySlot, const RMessagePtr2& aMessage );
		
    private:    // Data
        
	    CEikonEnv* iEikEnv;
	    TInt iResourceFileOffset;
	    TInt iResourceFileOffset2;
		TBool iSaveReceipt; //text to save to notepad
		TInt32 iDialogType;
		CCTSecurityDialogsAO* iSecurityDialogAO;
		TBool iSecurityDialogAODeleted;
    };

#endif // __CTSECURITYDIALOGNOTIFIER_H

