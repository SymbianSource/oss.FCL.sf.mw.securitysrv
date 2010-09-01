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
* Description:   Implementation of Application UI class
*
*/


// INCLUDE FILES
#include <sysutil.h>            // For FFSSpaceBelowCriticalLevelL(..)
#include <aknnotewrappers.h>    // Note dialogs
#include <mpkcs12.h>
#include <cryptostrength.h>
#include <CertSaver.rsg>
#include "CertSaverAppUi.h"
#include "CertSaverContainer.h"
#include "CertSaverDocument.h"
#include "CertSaverModel.h"


// ================= MEMBER FUNCTIONS =======================
//
// ----------------------------------------------------------
// CCertSaverAppUi::ConstructL()
// Epoc second phase constructor.
// ----------------------------------------------------------
//
void CCertSaverAppUi::ConstructL()
    {
    BaseConstructL( EAknEnableSkin | EAknEnableMSK );

    iContainer = new( ELeave ) CCertSaverContainer();
    iContainer->ConstructL( ClientRect() );
    iContainer->SetMopParent( this );
    AddToStackL( iContainer );
    }

// ----------------------------------------------------
// CCertSaverAppUi::~CCertSaverAppUi()
// Destructor
// Frees reserved resources
// ----------------------------------------------------
//
CCertSaverAppUi::~CCertSaverAppUi()
    {
    delete iModel;

    if( iContainer )
       {
       RemoveFromStack( iContainer );
       delete iContainer;
       }
    }

// ------------------------------------------------------------------------------
// CCertSaverAppUi::::DynInitMenuPaneL(TInt aResourceId,CEikMenuPane* aMenuPane)
// This function is called by the EIKON framework just before it displays
// a menu pane. Its default implementation is empty, and by overriding it,
// the application can set the state of menu items dynamically according
// to the state of application data.
// ------------------------------------------------------------------------------
//
void CCertSaverAppUi::DynInitMenuPaneL(
    TInt /*aResourceId*/,CEikMenuPane* /*aMenuPane*/ )
    {
    }

// ----------------------------------------------------
// CCertSaverAppUi::HandleKeyEventL(
//     const TKeyEvent& aKeyEvent,TEventCode /*aType*/)
// No key events are handled by the application.
// ----------------------------------------------------
//
TKeyResponse CCertSaverAppUi::HandleKeyEventL(
    const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/ )
    {
    return EKeyWasNotConsumed;
    }

// ----------------------------------------------------
// CCertSaverAppUi::HandleCommandL(TInt aCommand)
// In principle, this function does nothing, because all
// input is handled by dialogs.
// ----------------------------------------------------
//
void CCertSaverAppUi::HandleCommandL( TInt aCommand )
    {
    switch ( aCommand )
        {
        case EEikCmdExit:
            {
            doExit();
            break;
            }
        default:
            break;
        }
    }

// ----------------------------------------------------
// CCertSaverAppUi::RunFileL(const TDesC& aFileName)
// Create CCertSaver dialog class and activates the
// dialog.
// ----------------------------------------------------
//
void CCertSaverAppUi::RunFileL(RFile& aFile)
    {
    TCrypto::TStrength cryptoStrength = TCrypto::Strength();
    if ( TCrypto::EWeak == cryptoStrength )
        {
        User::Panic( KCertSaverPanic, KErrWrongCryptoLib );
        }
    ((CCertSaverDocument*)iDocument)->Parser().SetContentL( aFile );

    iModel = new (ELeave) CCertSaverModel(
        this, ((CCertSaverDocument*)iDocument)->Parser());

    switch ( Document()->Parser().CertType() )
        {
        case CCertParser::ETypeX509CA:
            {
            iModel->SaveCertificateL( (Document()->Parser().Certificate()),
                ECACertificate, EX509Certificate );
            break;
            }
        case CCertParser::ETypeX509Peer:
            {
            iModel->SaveCertificateL( (Document()->Parser().Certificate()),
                EPeerCertificate, EX509Certificate );
            break;
            }
        case CCertParser::ETypePKCS12:
            {
            iModel->SavePKCS12L();
            break;
            }
        case CCertParser::ETypeCorrupt:
            {
            // Do nothing
            break;
            }
        default:
            {
            break;
            }
        }
    }

// ----------------------------------------------------
// CCertSaverAppUi::CoeEnv()
// Returns pointer to the control environment.
// ----------------------------------------------------
//
const CCoeEnv* CCertSaverAppUi::CoeEnv() const
    {
    return iCoeEnv;
    }

// ----------------------------------------------------
// CCertSaverAppUi::doExit()
// Exits the application and notifies iDoorObserver about it.
// ----------------------------------------------------
//
void CCertSaverAppUi::doExit()
    {
    if ( iDoorObserver )
        {
        iDoorObserver->NotifyExit( MApaEmbeddedDocObserver::EEmpty );
        }
    Exit();
    }

// ----------------------------------------------------
// CCertSaverAppUi::HandleError(TInt aError,
// const SExtendedError& aExtErr, TDes& aErrorText,
// TDes& aContextText)
// When a leave that is not trapped by application occurs,
// this function is called.
// ----------------------------------------------------
//
TErrorHandlerResponse CCertSaverAppUi::HandleError(
    TInt aError,
    const SExtendedError& aExtErr,
    TDes& aErrorText,
    TDes& aContextText )
    {
    iErrorOccured = ETrue;
    return CAknAppUi::HandleError( aError, aExtErr, aErrorText, aContextText );
    }

// ----------------------------------------------------
// CCertSaverAppUi::HandleForegroundEventL(TBool aForeground)
// Exits the application if leave occurs. Leave is noticed by
// member varible iErrorOccured. It is set to ETrue in
// CCertSaverAppUi::HandleError. After error note has been
// shown (by HandleError), this function is called by
// framework.
// ----------------------------------------------------
//
void CCertSaverAppUi::HandleForegroundEventL( TBool aForeground )
    {
    if ( iErrorOccured )
        {
        doExit();
        }
    CAknAppUi::HandleForegroundEventL( aForeground );
    }
// ----------------------------------------------------
// CCertSaverAppUi::ProcessCommandParametersL()
// ----------------------------------------------------
//
TBool CCertSaverAppUi::ProcessCommandParametersL(
    TApaCommand /*aCommand*/,
    TFileName& /*aDocumentName*/,
    const TDesC8& /*aTail*/ )
    {
    return ETrue;
    }

// End of File
