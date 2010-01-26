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
* Description:   Header file of the CDigSigningNote
*
*/


#ifndef CDIGSIGNINGNOTE_H
#define CDIGSIGNINGNOTE_H

//  INCLUDES
#include "SecurityDialogBase.h"
#include <DigSigningNote.h>  // MDigSigningNote
#include <e32std.h>

// CLASS DECLARATION

/**
*  This class implements MDigSigningNote interface.
*  
*
*  @lib CTSecDlgs.lib
*  @since Series60_2.6
*/
class CDigSigningNote : public CSecurityDialogBase, public MDigSigningNote
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CDigSigningNote* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CDigSigningNote();
    
    public: // Functions from MDigSigningNote

        /**
        * Informs the user during digital signing.
        * @param aNoteType Identifies the note type.
        * @param aStatus OUT: KErrNone or KErrNotSupported
        * @return void
        */
        void ShowNote(MDigSigningNote::TNoteType aNoteType, TRequestStatus& aStatus);
        
        /**
        * Saves receipt to Notepad if receipt saving is on in .ini-file and
        * informs user about this.
        * @since ?Series60_version
        * @param aSignedText Signed text in readable form.
		* @param aStatus OUT: KErrNone or KErrNotSupported
        * @return void
        */
        void SaveSignedText(const TDesC& aSignedText, TRequestStatus& aStatus);
        
        /**
        * Shows instructions how to unblock PIN.
        * @since ?Series60_version
        * @param aLabel Label of the PIN.
        * @param aStatus KErrNone or KErrNotSupported
        * @return void
        */
        void ShowPINBlockedInfo(const TPINLabel& aLabel, TRequestStatus& aStatus);

        /**
    	*	Release is called to free resources of the MSecurityDialog class
	    */
	    void Release();

    private:

        /**
        * C++ default constructor.
        */
        CDigSigningNote();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data
        
	    CBufFlat*	iSenderBuffer;	// Stream buffer for outgoing streams.
    	HBufC8*     iBufferData;    // iSenderBuffer help buffer
    };

#endif      // CDIGSIGNINGNOTE_H  
            
// End of File
