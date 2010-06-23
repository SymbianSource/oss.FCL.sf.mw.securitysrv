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
* Description:   Document class
*
*/


// INCLUDE FILES
#include "CertSaverDocument.h"
#include "CertSaverAppUi.h"

// Class CShutterAO
// ================= MEMBER FUNCTIONS =======================
// Constructor
CShutterAO::CShutterAO(): CActive( EPriorityStandard )
    {
    CActiveScheduler::Add( this );
    }

// Destructor
CShutterAO::~CShutterAO()
    {
    Cancel();
    }

//------------------------------------------------------
// CShutterAO::Exit()
//
//------------------------------------------------------
//
void CShutterAO::Exit()
    {
    SetActive();
    // Complete the AO immediately so that buffer processing can occur in RunL().
    TRequestStatus* statusPtr = &iStatus;
    User::RequestComplete( statusPtr, KErrNone );
    }

//------------------------------------------------------
// CShutterAO::DoCancel()
//
//------------------------------------------------------
//
void CShutterAO::DoCancel()
    {
    }

//------------------------------------------------------
// CShutterAO::RunL()
//
//------------------------------------------------------
//
void CShutterAO::RunL()
    {
    iAvkonAppUi->Exit();
    }

// class CCertSaverDocument
// ================= MEMBER FUNCTIONS =======================
// destructor
CCertSaverDocument::~CCertSaverDocument()
    {
    delete iParser;
    delete iAO;
    }

// EPOC default constructor can leave.
void CCertSaverDocument::ConstructL()
    {
    iParser = CCertParser::NewL();
    }

// Two-phased constructor.
CCertSaverDocument* CCertSaverDocument::NewL(
        CEikApplication& aApp )     // CCertSaverApp reference
    {
    CCertSaverDocument* self =
        new (ELeave) CCertSaverDocument( aApp );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(self);

    return self;
    }

// -----------------------------------------------------------------------------
// CCertSaverDocument::OpenFileL
// Called by the framework to open a certificate file with file name.
// -----------------------------------------------------------------------------
//

CFileStore* CCertSaverDocument::OpenFileL(TBool /*aDoOpen*/,
    const TDesC& aFilename, RFs& aFs )
    {
    CFileStore* docStore=NULL;

    RFile file;
    TInt err = file.Open( aFs, aFilename, EFileRead );
    User::LeaveIfError( err );
    CleanupClosePushL<RFile>( file );

    OpenFileL( docStore, file );

    CleanupStack::PopAndDestroy(&file); // Close file

    return docStore;
    }

// ----------------------------------------------------
// CCertSaverDocument::OpenFileL
// Since Uikon framework always starts application from
// this function, Certificate Saver uses this function
// to handle the certificate file. Doc Handler sets
// aDoOpen to ETrue.
// ----------------------------------------------------
//
void CCertSaverDocument::OpenFileL(CFileStore*& /*aFileStore*/, RFile& aFile)
    {
    TRAPD( err, STATIC_CAST(CCertSaverAppUi*,iAppUi)->RunFileL( aFile ) );

    if ( err != KErrNone && err != KErrExitApp && err != KErrCancel )
        {
        User::Leave( err );
        }

    iAO = new (ELeave) CShutterAO;
    iAO->Exit();
    #ifdef _DEBUG
        
    #endif
    }

//------------------------------------------------------
// CCertSaverDocument::Parser()
//
//------------------------------------------------------
//
CCertParser& CCertSaverDocument::Parser() const
    {
    return *iParser;
    }

// ----------------------------------------------------
// CCertSaverDocument::CreateAppUiL()
// constructs CCertSaverAppUi
// ----------------------------------------------------
//
CEikAppUi* CCertSaverDocument::CreateAppUiL()
    {
    return new (ELeave) CCertSaverAppUi;
    }

// End of File
