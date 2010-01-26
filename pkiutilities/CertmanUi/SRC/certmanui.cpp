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
* Description:   Implementation of class CCertManUI. Methods for reading
*                the resources
*
*/


// INCLUDE FILES
#include <eikenv.h>
#include <bautils.h>
#include <data_caging_path_literals.hrh>
#include "Certmanui.h"

// CONSTANTS

_LIT( KResourceFile, "z:CertManUi.rsc" );

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------
// CCertManUi::InitializeLibL()
// Initializes CCertManUi resource file
// ----------------------------------------------------------
//
void CCertManUi::InitializeLibL()
  {
  if ( Dll::Tls() != NULL )
    {
    return;
    }
  CCertManUi* self = new ( ELeave ) CCertManUi();
  CleanupStack::PushL( self );
  self->ConstructL();
  Dll::SetTls( self );
  CleanupStack::Pop(); // self
  }

// ----------------------------------------------------------
// CCertManUi::UnInitializeLib()
// Uninitializes CCertManUi resource file
// ----------------------------------------------------------
//
void CCertManUi::UnInitializeLib()
  {
  CCertManUi* instance = STATIC_CAST( CCertManUi*, Dll::Tls() );
  delete instance;
  Dll::SetTls( NULL );
  }

// ----------------------------------------------------------
// CCertManUi::CCertManUi()
// C++ default constructor
// ----------------------------------------------------------
//
CCertManUi::CCertManUi() : iResourceLoader( *iCoeEnv )
  {
  }

// ----------------------------------------------------------
// CCertManUi::~CCertManUi()
// Destructor
// ----------------------------------------------------------
//
CCertManUi::~CCertManUi()
  {
  iResourceLoader.Close();
  }

// ----------------------------------------------------------
// CCertManUi::ConstructL()
// EPOC default constructor
// ----------------------------------------------------------
//
void CCertManUi::ConstructL()
  {
  AddResourceFileL();
  }

// ----------------------------------------------------------
// CCertManUi::AddResourceFileL()
// Used in ConstructL to get resource file
// ----------------------------------------------------------
//
void CCertManUi::AddResourceFileL()
  {
  TFileName fileName;

  TParse parse;
  parse.Set( KResourceFile, &KDC_RESOURCE_FILES_DIR, NULL );

  fileName = parse.FullName();
  iResourceLoader.OpenL( fileName );
  }

// End of File


