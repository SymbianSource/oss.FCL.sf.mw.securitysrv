/*
* Copyright (c) 2003-2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Application UI class
*
*/


#ifndef CERTSAVERAPPUI_H
#define CERTSAVERAPPUI_H

// INCLUDES
#include <eikapp.h>
#include <eikdoc.h>
#include <e32std.h>
#include <coeccntx.h>
#include <aknappui.h>
#include <eikdialg.h>

// FORWARD DECLARATIONS
class CCertSaverDialog;
class CCertSaverDocument;
class CCertSaverContainer;


// CONSTANTS

// CLASS DECLARATION
class CCertSaverModel;

/**
* Application UI class.
* Provides support for the following features:
* - detects if an unhandled leave occurs
*
*/
class CCertSaverAppUi : public CAknAppUi
    {
    public: // // Constructors and destructor

        /**
        * EPOC default constructor.
        */
        void ConstructL();

        /**
        * Destructor.
        */
        ~CCertSaverAppUi();

    public: // New functions

        /**
        * Starts certificate saving.
        * @param void
        */
        void RunFileL();

        void RunFileL( RFile& aFile );

        /**
        * Returns pointer to control environment.
        * @param void
        */
        const CCoeEnv* CoeEnv() const;

        /**
        * Notifies iDoorObserver about exit, and then exits.
            * In this case iDoorObserver is Document Handler.
        * @param void
        */
        void doExit();

    public: // Functions from base classes

        //From CEikAppUi
        TErrorHandlerResponse HandleError (
              TInt aError,
              const SExtendedError& aExtErr,
              TDes& aErrorText,
              TDes& aContextText );

    protected: // Functions from base classes
        // From CEikAppUi
        TBool ProcessCommandParametersL(
              TApaCommand aCommand ,
              TFileName& aDocumentName,
              const TDesC8& Tail );

        //From CCoeAppUi
        void HandleForegroundEventL( TBool aForeground );

    private:
        // From MEikMenuObserver
        void DynInitMenuPaneL( TInt aResourceId,CEikMenuPane* aMenuPane );

        /**
        * From CEikAppUi, takes care of command handling.
        * @param aCommand command to be handled
        */
        void HandleCommandL( TInt aCommand );

    private:


        CCertSaverDocument* Document() const
            {
            return( (CCertSaverDocument*)iDocument );
            }

        void ConstructPKCS12QueryMsgL( HBufC* aMessage ) const;

        /**
        * From CEikAppUi, handles key events.
        * @param aKeyEvent Event to handled.
        * @param aType Type of the key event.
        * @return Response code (EKeyWasConsumed, EKeyWasNotConsumed).
        */
        virtual TKeyResponse HandleKeyEventL(
            const TKeyEvent& aKeyEvent,TEventCode aType );

    private: //Data
        CCertSaverModel*      iModel;
        TBool                 iErrorOccured;
        CCertSaverContainer*  iContainer;
    };

#endif  // CERTSAVERAPPUI_H

// End of File
