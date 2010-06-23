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
* Description:   Implementation of CCertParser class
*
*/


// INCLUDE FILES
#include <e32std.h>
#include <apmrec.h>
#include <x509cert.h>
#include <x509certext.h>
#include <hash.h>
#include <X509CertNameParser.h>
#include <featmgr.h>
#include <data_caging_path_literals.hrh>
#include <CertSaver.rsg>
#include <StringLoader.h>
#include <securityerr.h>
#include <aknnotewrappers.h>              // Note dialogs text, TODO: could be removed after localization is ready
#include <apmrec.h>
#include "certparser.h"

#include "SecQueryUi.h"                   // needed for password dialog

#include <hbdevicemessageboxsymbian.h>    // needed for Note dialogs    

// CONSTANTS
_LIT( KPKCS12DllName, "crpkcs12.dll" );   // PKCS12 DLL name
const TInt KX509Version3 = 3;
const TInt KPwMaxLength = 32;

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// ==================== LOCAL FUNCTIONS ====================

// ================= MEMBER FUNCTIONS =======================

// C++ default constructor can NOT contain any code, that
// might leave.
//
CCertParser::CCertParser()
    {
    }

// EPOC default constructor can leave.
void CCertParser::ConstructL()
    {
    }

// Two-phased constructor.
CCertParser* CCertParser::NewL()
    {
    CCertParser* self = new (ELeave) CCertParser;

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }


// Destructor
CCertParser::~CCertParser()
    {
    delete iCert;
    if ( iPKCS12 )
        {
        iPKCS12->Release();
        }
    iLibrary.Close();
    }

// ----------------------------------------------------------
// CCertParser::SetContentL()
// Checks if given buffer is a WTLS or a X.509 certificate and
// sets member data accordingly.
// ----------------------------------------------------------
//
void CCertParser::SetContentL( RFile& aFile )
    {

    delete iCert;
    iCert = NULL;
    TInt fileSize = 0;
    CleanupClosePushL( aFile );
    User::LeaveIfError( aFile.Size( fileSize ) );

    HBufC8* buffer = HBufC8::NewL( fileSize );
    CleanupStack::PushL( buffer );

    TPtr8 ptr8(buffer->Des());
    User::LeaveIfError( aFile.Read( ptr8 ) );

    if ( !CheckIfX509CertificateL( *buffer ) )
        {
        TFileName fileName;
        User::LeaveIfError( aFile.Name( fileName ) );
        if ( !CheckIfPKCS12L( *buffer, fileName ) )
            {
            iCertType = ETypeCorrupt;
            }
        }
    CleanupStack::PopAndDestroy( buffer );
    CleanupStack::PopAndDestroy( &aFile );
    }

// ----------------------------------------------------------
// CCertParser::CreatePKCS12L()
// Creates PKCS#12 DLL
// ----------------------------------------------------------
//
void CCertParser::CreatePKCS12L()
    {

        // Load PKCS#12 dll
#ifdef __WINS__
    User::LeaveIfError ( iLibrary.Load( KPKCS12DllName ) );
#else
    User::LeaveIfError ( iLibrary.Load( KPKCS12DllName, KDC_SHARED_LIB_DIR ) );
#endif

    // Define the function that returns pointer to MPKCS12
    typedef MPKCS12* ( *TLibraryFunc )();
    // Find CreateL() method from pkcs12.dll that implements the function
    TLibraryFunc createL =
        reinterpret_cast<TLibraryFunc>( iLibrary.Lookup( 1 ) );
    // Create new instance of MPKCS12
    iPKCS12 = createL();
    }

// ----------------------------------------------------------
// CCertParser::CheckIfPKCS12L()
// Checks if PKCS#12 file
// ----------------------------------------------------------
//
TBool CCertParser::CheckIfPKCS12L(
    const TDesC8& aPKCS12,
    const TDesC& aFileName )
    {

    CreatePKCS12L();

    if ( !iPKCS12->IsPKCS12Data( aPKCS12 ) )
        {
        ShowErrorNoteL( R_CERTSAVER_PKCS12_FILE_CORRUPTED );
        return EFalse;
        }

    TBool done = EFalse;
    TBuf<KPwMaxLength> password;
    HBufC* buffer = NULL;

    if ( !iEikEnv )
        {
        iEikEnv = CEikonEnv::Static();
        }

    while ( !done )
        {
        if ( !GetPasswordL( password, aFileName ) )
            {
            buffer = iEikEnv->AllocReadResourceLC( R_CERTSAVER_PKCS12_DISCARDED );
            CHbDeviceMessageBoxSymbian::InformationL(buffer->Des()); 
            CleanupStack::PopAndDestroy( buffer );
            User::Leave( KErrExitApp );
            }
        TRAPD( err, iPKCS12->ParseL( aPKCS12, password ) );
        if ( err != KErrBadPassphrase )
            {
            switch ( err )
                {
                case KErrNone:
                    {
                    iCertType = ETypePKCS12;
                    break;
                    }
                case KErrNoMemory:
                    {
                    User::Leave( KErrNoMemory );
                    }
                case KErrNotSupported:
                    {
                    ShowErrorNoteL( R_QTN_CM_PKCS12_FORMAT_NOT_SUPPORTED );
                    break;
                    }
                default:
                    {
                    ShowErrorNoteL( R_CERTSAVER_PKCS12_FILE_CORRUPTED );
                    break;
                    }
                }
            if ( err != KErrNone )
                {
                return EFalse;
                }
            done = ETrue;
            }
        else
            {
            password.Zero();
            // Show error note
            ShowErrorNoteL( R_QTN_CM_INCORRECT_PASSWORD );
            }
        }
    return ETrue;
    }

// ----------------------------------------------------------
// CCertParser::GetPasswordL(...)
//
// ----------------------------------------------------------
//
TInt CCertParser::GetPasswordL( TDes& aPassword, const TDesC& aFileName )
    {
    CSecQueryUi* SecQueryUi = CSecQueryUi::NewL();                                       
    HBufC* prompt =StringLoader::LoadLC( R_QTN_CM_TITLE_P12_PASSWORD, aFileName );        
    TInt queryAccepted = SecQueryUi->SecQueryDialog(prompt->Des(), aPassword,                
                                                1,KPwMaxLength,                               
                                                ESecUiAlphaSupported |                        
                                                ESecUiCancelSupported |                       
                                                ESecUiSecretSupported |                
                                                ESecUiEmergencyNotSupported); 
    CleanupStack::PopAndDestroy( prompt );                                                
    delete SecQueryUi;                                                                   
    SecQueryUi=NULL;
    return (queryAccepted==KErrNone);
  }

// ----------------------------------------------------------
// CCertParser::CheckIfX509CertificateL()
// Returns the type of the certificate.
// ----------------------------------------------------------
//
TBool CCertParser::CheckIfX509CertificateL(const TDesC8& aCert)
    {

    TRAPD( err,
           iCert = CX509Certificate::NewL(aCert);
         );

    if ( err != KErrNone )
        {
        iCertType = ETypeCorrupt;
        if ( err == KErrNoMemory )
            {
            User::Leave( err );
            }
        else
            {
            return EFalse;
            }
        }

  if ( iCert )
      {
      // Check certificate version
      if ( iCert->Version() != KX509Version3 )
          {
          // X509 v1 or v2 certificate. Certificate type is CA
          iCertType = ETypeX509CA;
          }
      else
          {
          // X509 v3 certificate. Check basicConstrains
          const CX509CertExtension* certExt = iCert->Extension( KBasicConstraints );
          if ( certExt )
              {
              CX509BasicConstraintsExt* basic = CX509BasicConstraintsExt::NewLC( certExt->Data() );
              if ( basic->IsCA() )
                  {
                  iCertType = ETypeX509CA;
                  }
              else
                  {
                  iCertType = ETypeX509Peer;
                  }
              CleanupStack::PopAndDestroy( basic ); //basic
              }
           else
              {
              // No basicConstrains extension
              iCertType = ETypeX509Peer;
              }
          }
      }

    return ETrue;
    }

// ----------------------------------------------------------
// CCertParser::CACertificates()
// ----------------------------------------------------------
//
const CArrayPtr<CX509Certificate>&  CCertParser::CACertificates() const
    {

    __ASSERT_ALWAYS( iPKCS12, User::Panic( KCertSaverPanic, KPanicNullPointer ) );
    return iPKCS12->CACertificates();
    }

// ----------------------------------------------------------
// CCertParser::UserCertificates()
// ----------------------------------------------------------
//
const CArrayPtr<CX509Certificate>&  CCertParser::UserCertificates() const
    {

    __ASSERT_ALWAYS( iPKCS12, User::Panic( KCertSaverPanic, KPanicNullPointer ) );
    return iPKCS12->UserCertificates();
    }

// ----------------------------------------------------------
// CCertParser::Keys()
// ----------------------------------------------------------
//
const CArrayPtr<HBufC8>& CCertParser::Keys() const
    {

    __ASSERT_ALWAYS( iPKCS12, User::Panic( KCertSaverPanic, KPanicNullPointer ) );
    return iPKCS12->PrivateKeys();
    }

// ----------------------------------------------------------
// CCertParser::CertType()
// ----------------------------------------------------------
//
 CCertParser::TCertType CCertParser::CertType() const
    {

    return iCertType;
    }

// ----------------------------------------------------------
// CCertParser::CertificateBuf()
// Returns the content of the certificate.
// ----------------------------------------------------------
//
const TPtrC8 CCertParser::CertificateBuf() const
    {

    __ASSERT_ALWAYS( iCert, User::Panic( KCertSaverPanic, KPanicNullPointer ) );
    return iCert->Encoding();
    }

// ----------------------------------------------------------
// CCertParser::Certificate()
// Returns the X.509 certificate object.
// ----------------------------------------------------------
//
const CX509Certificate& CCertParser::Certificate() const
    {

    __ASSERT_ALWAYS( iCert, User::Panic( KCertSaverPanic, KPanicNullPointer ) );
    return *((CX509Certificate*)iCert);
    }

// ----------------------------------------------------------
// CCertParser::ShowErrorNoteL() const
// Creates and shows an error note.
// ----------------------------------------------------------
//
void CCertParser::ShowErrorNoteL( TInt aResourceID )
    {
     if ( !iEikEnv )
        {
        iEikEnv = CEikonEnv::Static();
        }
    HBufC* buffer = iEikEnv->AllocReadResourceLC( aResourceID );
    CHbDeviceMessageBoxSymbian::WarningL(buffer->Des()); 
    CleanupStack::PopAndDestroy( buffer );
    }

//  End of File

