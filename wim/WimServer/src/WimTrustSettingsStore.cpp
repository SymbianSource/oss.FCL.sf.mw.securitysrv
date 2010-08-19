/*
* Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Implements WIM TrustSettingsStore for WIM certificates
*
*/



// INCLUDE FILES
#include "WimTrustSettingsStore.h"
#include "WimCertInfo.h"
#include "WimCertUtil.h"
#include <x509cert.h>
#include <x509certext.h>
#include <hash.h>
#include "WimTrace.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::CWimTrustSettingsStore()
// Default constructor.
// -----------------------------------------------------------------------------
//
CWimTrustSettingsStore::CWimTrustSettingsStore() 
                      : CActive( EPriorityNormal ),
                        iCompact ( EFalse )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::ConstructL()
// Second phase constructor.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::ConstructL()
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::ConstructL()" ) );
    // Connect to DB server
    User::LeaveIfError( iDBSession.Connect() );

    // combine trust settings DB path & name

    HBufC* dbPathBuffer = HBufC::NewLC( KWimTrustSettingsStorePath().Length() +
                                        KWimTrustSettingsDBFile().Length() );
    TPtr wimTrustSettingsDB( dbPathBuffer->Des() );
    wimTrustSettingsDB = KWimTrustSettingsStorePath;
    wimTrustSettingsDB.Append( KWimTrustSettingsDBFile );

    // Open database. Note: err used because of create is done if not found
    TInt err = iDatabase.Open( iDBSession, wimTrustSettingsDB );

    if ( err != KErrNone ) //Database could not open, create new
        {
        _WIMTRACE2( _L( "CWimTrustSettingsStore::ConstructL()|DB open error %d" ), err ); 
        CreateDBL(); // Create DB
        User::LeaveIfError( iDatabase.Open( iDBSession, wimTrustSettingsDB ) );
        }

    iPhase = EIdle;
    CleanupStack::PopAndDestroy( dbPathBuffer );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimTrustSettingsStore* CWimTrustSettingsStore::NewL()
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::NewL()" ) );
    CWimTrustSettingsStore* self = new( ELeave ) CWimTrustSettingsStore;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::~CWimTrustSettingsStore()
// Destructor. If updates has been done, compact databse.
// -----------------------------------------------------------------------------

CWimTrustSettingsStore::~CWimTrustSettingsStore()
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::~CWimTrustSettingsStore()" ) );
    Cancel();

    if ( iCompact ) // Is compact needed
        {
        iDatabase.Compact(); //Compact database
        }

    iDatabase.Close();  // Close database
    iDBSession.Close(); // Close session to DB server

    if ( iWimCertUtil )
        {
        delete iWimCertUtil;
        }

    delete iEncodedCertBuf;
    delete iEncodedCertPtr;

    iCertInfos.ResetAndDestroy();
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::CloseD()
// Closes all open resources and deletes this instance of the TrustSettingsStore
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::CloseD()
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::CloseD()" ) );
    delete ( this );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::GetTrustSettings()
// Return trust settings for given certificate.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::GetTrustSettings( const CWimCertInfo& aCert,
                                               TBool& aTrusted,
                                               RArray<TUid>& aApplications, 
                                               TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::GetTrustSettings()" ) );
    TRAPD( err, DoGetTrustSettingsL( aCert, aTrusted, aApplications ) );

    TRequestStatus* status = &aStatus;
    User::RequestComplete( status, err );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::SetApplicability()
// Set applicability for certificate.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::SetApplicability( const CWimCertInfo& aCert,
                                              const RArray<TUid>& aApplications,
                                              TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::SetApplicability()" ) );
    TRAPD( err, DoSetApplicabilityL( aCert, aApplications ) );

    TRequestStatus* status = &aStatus;
    User::RequestComplete( status, err );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::SetTrust()
// Set trust flag for certificate.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::SetTrust( const CWimCertInfo& aCert,
                                       TBool aTrusted,
                                       TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::SetTrust()" ) );
    TRAPD( err, DoSetTrustL( aCert, aTrusted ) );

    TRequestStatus* status = &aStatus;
    User::RequestComplete( status, err );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::SetDefaultTrustSettings()
// Set default trust settings for certificate.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::SetDefaultTrustSettings( const CWimCertInfo& aCert,
                                                      TBool aAddApps,
                                                      TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::SetDefaultTrustSettings()" ) );

    iWimCert = &aCert;
    iAddApps = aAddApps;
    iOriginalRequestStatus = &aStatus;

    iCert = aCert.CctCert();

    //Certificate owner type
    TCertificateOwnerType ownerType;
    ownerType = iCert->CertificateOwnerType();

    //Certificate format
    TCertificateFormat certFormat;
    certFormat = iCert->CertificateFormat();
    
    // If CA/X509 certificate locates in trusted certs area
    // check if certificate extensions must be fetched
    if ( ownerType == ECACertificate &&
         certFormat == EX509Certificate &&
         iWimCert->CDFRefs() == EWimTrustedCertsCDF )
        {
        // Check if Java Midlet OID is present
        RArray<HBufC*> arr = iWimCert->TrustedUsage();
        TInt oidCount = arr.Count();
        TInt i = 0;

        for ( ; i < oidCount; i++ )
            {
            if ( !arr[i]->Compare( KIpKpJavaMidlet ) )
                {
                i = oidCount + 1;
                }
            }
        // If OID is not present, extensions must be fetched
        if ( i == oidCount )
            {
            // Fetch extensions only for CA X509 certificates         
            iPhase = ERetrieve;
            TRequestStatus* status = &iStatus;
            User::RequestComplete( status, KErrNone );
            SetActive();
            }
        else
            {
            TRAPD( err, DoSetDefaultTrustSettingsL( aCert, aAddApps ) );
            TRequestStatus* status = &aStatus;
            User::RequestComplete( status, err );
            }
        }
    else
        {
        TRAPD( err, DoSetDefaultTrustSettingsL( aCert, aAddApps ) );
        TRequestStatus* status = &aStatus;
        User::RequestComplete( status, err );
        }
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::RemoveTrustSettings()
// Remove trust settings of given certificate
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::RemoveTrustSettings( const CWimCertInfo& aCert,
                                                  TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::RemoveTrustSettings()" ) );
    TRAPD( err, DoRemoveTrustSettingsL( aCert ) );
    iCompact = ETrue;

    TRequestStatus* status = &aStatus;
    User::RequestComplete( status, err );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::DoGetTrustSettingsL ()
// Get trust settings for given certificate.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::DoGetTrustSettingsL( const CWimCertInfo& aCert,
                                                  TBool& aTrusted,
                                                  RArray<TUid>& aApplications )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::DoGetTrustSettingsL()" ) );
    TInt certID;
    GetCertificateIDL( aCert, certID ); // Get certificate ID

    if ( certID == KErrNotFound ) //Not found settings
        {
        User::Leave( KErrNotFound );
        }
    GetTrustedL( certID, aTrusted ); //Get trusted flag
    GetApplicationsL( certID, aApplications ); //Get applications
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::DoSetApplicabilityL()
// Set applicability for certificate.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::DoSetApplicabilityL( const CWimCertInfo& aCert,
                                            const RArray<TUid>& aApplications )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::DoSetApplicabilityL()" ) );    
    TInt certID;
    GetCertificateIDL( aCert, certID ); // Get certificate ID

    iDatabase.Begin(); // Begin transaction

    TRAPD( err,
    if ( certID == KErrNotFound ) //Certificate not found -> Insert new one
        {
        InsertCertificateL( aCert, EFalse ); //Insert + set flag (EFalse)
        GetCertificateIDL( aCert, certID );    //Now get certificate ID
        }
    else // Certificate found
        {
        RemoveApplicationsL( certID );   //Remove old ones before inserting
        }
    InsertApplicationsL( certID, aApplications ); // Insert new apps
    ); //TRAPD

    if ( err != KErrNone ) //Some error occured
        {
        iDatabase.Rollback(); //Do rollback = restore changes
        User::Leave ( err );
        }
    else // No errors -> commit changes
        {
        iDatabase.Commit(); // Commit changes
        }
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::DoSetTrustL()
// Set trust flag for certificate.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::DoSetTrustL( const CWimCertInfo& aCert,
                                          TBool aTrusted )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::DoSetTrustL()" ) );    
    TInt certID;
    GetCertificateIDL( aCert, certID ); // Get certificate ID

    if ( certID == KErrNotFound ) // Certificate not found -> Insert new
       {
       InsertCertificateL( aCert, aTrusted ); // Insert + set flag
       }
    else // Certificate found
       {
        SetTrustedL( certID, aTrusted ); // Set trusted flag
       }
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::DoSetDefaultTrustSettingsL()
// Set default trust settings for certificate
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::DoSetDefaultTrustSettingsL( 
                                                    const CWimCertInfo& aCert,
                                                    TBool aAddApps )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::DoSetDefaultTrustSettingsL()" ) );    
    TInt certID;
    GetCertificateIDL( aCert, certID ); // Get certificate ID

    iDatabase.Begin(); // Begin transaction

    TRAPD( err,
        if ( certID == KErrNotFound ) // Certificate not found -> Insert new one
            {
            InsertCertificateL( aCert, KDefaultTrustFlag ); // Set default
            GetCertificateIDL( aCert, certID );
            }
        else // Certificate allready exists, set default trusted flag
            {
            SetTrustedL( certID, KDefaultTrustFlag );
            }
        //
        // Applications
        //
        if ( aAddApps ) // Default applications required
            {
            RArray<TUid> applications;
            CleanupClosePushL( applications );

            DefaultApplicationsL( aCert, applications ); 
            RemoveApplicationsL( certID ); // Remove previous applications
            if ( applications.Count() > 0 ) // There is apps to insert
                {
                InsertApplicationsL( certID, applications );
                }

            CleanupStack::PopAndDestroy( &applications );
            }
    );
    _WIMTRACE2 ( _L( "CWimTrustSettingsStore::DoSetDefaultTrustSettingsL(), err=%d" ), err );

    if ( err != KErrNone ) // Some error occured
        {
        iDatabase.Rollback(); // Do rollback = restore changes
        User::Leave( err );
        }
    else // No errors
        {
        iDatabase.Commit(); // Commit changes
        }
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::DoRemoveTrustSettingsL()
// Remove trust settings of given certificate
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::DoRemoveTrustSettingsL( const CWimCertInfo& aCert )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::DoRemoveTrustSettingsL()" ) );    
    TInt certID;
    GetCertificateIDL( aCert, certID ); // Get certificate ID

    if ( certID == KErrNotFound ) // Not found
        {
        User::Leave( KErrNotFound );
        }

    iDatabase.Begin(); // Begin transaction

    TRAPD( err, 
        RemoveApplicationsL( certID );  // Remove applications
        RemoveCertificateL( certID );   // Remove certificate
    );
    _WIMTRACE2 ( _L( "CWimTrustSettingsStore::DoRemoveTrustSettingsL(), err=%d" ), err );
    
    if ( err ) // Some error occurred
        {
        iDatabase.Rollback(); // Rollback changes
        User::Leave ( err );
        }
    else
        {
        iDatabase.Commit(); // Commit changes
        }
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::GetCertificateIDL()
// Get database ID for given certificate. Return KErrNotFound, if not found
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::GetCertificateIDL( const CWimCertInfo& aCert,
                                                TInt& aCertificateID )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::GetCertificateIDL()" ) );    
    _LIT( KSQLQuery, "SELECT CertificateID,\
                             CertHash,\
                             TokenSerial,\
                             LabelHash,\
                             IsTrusted\
                      FROM Certificates" );

    TBuf8<KSHA1HashLengthBytes> hash;
    aCert.GetCertHash( hash );

    HBufC8* buf8 = HBufC8::NewLC( aCert.CctCert()->Label().Length() );
    TPtr8* ptr8 = new( ELeave ) TPtr8( buf8->Des() );
    CleanupStack::PushL( ptr8 );

    ptr8->Copy( aCert.CctCert()->Label() );

    HBufC8* labelHash = GetHashL( *ptr8 );
    CleanupStack::PushL( labelHash );

    RDbView view;
    User::LeaveIfError( view.Prepare( iDatabase, TDbQuery( KSQLQuery ) ) );
    CleanupClosePushL( view );

    TInt i = 0;
    while ( i == 0 && view.NextL() ) // Should find only one ID
        {
        view.GetL();
        // Compare given hash, token serial number, cert label hash
        if ( !view.ColDes8( 2 ).Compare( hash ) &&
             !view.ColDes( 3 ).Compare( aCert.CctCert()->
                               Token().Information( MCTToken::ESerialNo ) ) &&
             !view.ColDes8( 4 ).Compare( *labelHash ) )
            {
            aCertificateID = view.ColUint32( 1 );
            i++;
            }
        }
            
    if ( i == 0 ) // Not found any matching rows
        {
        aCertificateID = KErrNotFound;
        }

    CleanupStack::PopAndDestroy( 4, buf8 ); // view, labelHash, ptr8, buf8
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::InsertCertificateL()
// Insert certificate into the Certificates table
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::InsertCertificateL( const CWimCertInfo& aCert, 
                                                 TBool aTrusted )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::InsertCertificateL()" ) );    
    TBuf8<KSHA1HashLengthBytes> hash;
    aCert.GetCertHash( hash );
    
    HBufC8* buf8 = HBufC8::NewLC( aCert.CctCert()->Label().Length() );
    TPtr8* ptr8 = new(ELeave) TPtr8( buf8->Des() );
    CleanupStack::PushL( ptr8 );

    ptr8->Copy( aCert.CctCert()->Label() );

    HBufC8* labelHash = GetHashL( *ptr8 );
    CleanupStack::PushL( labelHash );

    RDbView view;

    _LIT( KSQLInsertCert, "SELECT CertHash,\
                                  TokenSerial,\
                                  LabelHash,\
                                  IsTrusted\
                           FROM Certificates" );

    User::LeaveIfError( view.Prepare( iDatabase, TDbQuery( KSQLInsertCert ),
        TDbWindow::EUnlimited, RDbView::EInsertOnly ) );

    CleanupClosePushL( view );
    
    view.InsertL();
    view.SetColL( 1, hash );
    view.SetColL( 2, aCert.CctCert()->
                           Token().Information( MCTToken::ESerialNo ) );
    view.SetColL( 3, *labelHash );
    view.SetColL( 4, aTrusted );
    view.PutL();

    CleanupStack::PopAndDestroy( 4, buf8 ); // view, labelHash, ptr8, buf8
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::GetHash()
// Return hash of 8 bit string
// -----------------------------------------------------------------------------
//
HBufC8* CWimTrustSettingsStore::GetHashL( TDesC8& aString )
    {
    CSHA1* sha = CSHA1::NewL();
    CleanupStack::PushL( sha );
    TPtrC8 shaFingerPrint = sha->Hash( aString );
    HBufC8* fingerPrint = HBufC8::NewL( shaFingerPrint.Length() ); //make a copy
    TPtr8 fingerPrintDes = fingerPrint->Des();
    fingerPrintDes.Copy( shaFingerPrint );
    CleanupStack::PopAndDestroy( sha );
    return fingerPrint;
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::GetTrustedL()
// Get trusted flag of given certificate
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::GetTrustedL( TInt aCertificateID,  
                                          TBool& aTrusted )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::GetTrustedL()" ) );    
    _LIT( KSQLQuery, 
         "SELECT IsTrusted FROM Certificates WHERE CertificateID = " );

    TBuf<KMaxSQLLength> SQLStatement;
    SQLStatement.Copy( KSQLQuery );

    SQLStatement.AppendNum( aCertificateID );
    TPtrC sqlStat( SQLStatement.PtrZ() );

    RDbView view;
    User::LeaveIfError( view.Prepare( iDatabase, TDbQuery( sqlStat ) ) );
    CleanupClosePushL( view );

    while ( view.NextL() ) // Should find only one certificate
        {
        view.GetL();
        aTrusted = ( TBool )view.ColUint( 1 );
        }
    
    CleanupStack::PopAndDestroy( &view );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::SetTrustedL()
// Set trusted flag for given certificate
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::SetTrustedL( TInt aCertificateID,  
                                          TBool aTrusted )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::SetTrustedL()" ) );    
    _LIT( KSQLUpdate1, "UPDATE Certificates SET IsTrusted = " );
    _LIT( KSQLUpdate2, " WHERE CertificateID = " );
    TBuf<KMaxSQLLength> SQLStatement;
    SQLStatement.Copy( KSQLUpdate1 );
    SQLStatement.AppendNum( aTrusted );
    SQLStatement.Append( KSQLUpdate2 );
    SQLStatement.AppendNum( aCertificateID );

    TPtrC sqlStat( SQLStatement.PtrZ() );

    User::LeaveIfError( iDatabase.Execute( sqlStat ) );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::RemoveCertificateL()
// Delete certificate from Certificates table
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::RemoveCertificateL( TInt aCertificateID )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::RemoveCertificateL()" ) );    
    _LIT( KSQLDelete, "DELETE FROM Certificates WHERE CertificateID = " );

    TBuf<60> SQLStatement;
    SQLStatement.Copy( KSQLDelete );
    SQLStatement.AppendNum( aCertificateID );
    TPtrC sqlStat( SQLStatement.PtrZ() );
    User::LeaveIfError( iDatabase.Execute( sqlStat ) );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::RemoveApplicationsL()
// Delete all applications of given certificate
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::RemoveApplicationsL( TInt aCertificateID )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::RemoveApplicationsL()" ) );    
    _LIT( KSQLDelete, 
         "DELETE FROM CertificateApplications WHERE CertificateID = " );

    TBuf<KMaxSQLLength> SQLStatement;
    SQLStatement.Copy( KSQLDelete );
    SQLStatement.AppendNum( aCertificateID );
    TPtrC sqlStat( SQLStatement.PtrZ() );
    User::LeaveIfError( iDatabase.Execute( sqlStat ) );
    iCompact = ETrue;
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::GetApplicationsL()
// Get applications of given certificate
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::GetApplicationsL( TInt aCertificateID, 
                                               RArray<TUid>& aApplications )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::GetApplicationsL()" ) );    
    _LIT( KSQLQuery, "SELECT AppUid FROM CertificateApplications " );
    _LIT( KSQLQuery2, "WHERE CertificateID = " );
    
    TBuf<KMaxSQLLength> SQLStatement;
    SQLStatement.Copy( KSQLQuery );
    SQLStatement.Append( KSQLQuery2 );
    SQLStatement.AppendNum( aCertificateID );
    TPtrC sqlStat( SQLStatement.PtrZ() );

    RDbView view;
    User::LeaveIfError( view.Prepare( iDatabase, TDbQuery( sqlStat ) ) );
    CleanupClosePushL( view );

    TUid uid;
    while ( view.NextL() )
        {
        view.GetL();
        uid.iUid = view.ColInt32( 1 );
        aApplications.AppendL( uid );
        }
    
    CleanupStack::PopAndDestroy( &view );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::InsertApplicationsL()
// Insert applications to certificate
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::InsertApplicationsL( TInt aCertificateID, 
                                            const RArray<TUid>& aApplications )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::InsertApplicationsL()" ) );    
    RDbView view;

    // Insert applications
    _LIT( KSQLInsert, "SELECT CertificateID, AppUid FROM\
        CertificateApplications" );

    User::LeaveIfError( view.Prepare( iDatabase, TDbQuery( KSQLInsert ),
                        TDbWindow::EUnlimited, RDbView::EInsertOnly ) );
    CleanupClosePushL( view );
    
    // Loop application list and insert each to database
    for ( TInt i = 0; i < aApplications.Count(); i++ )
        {
        view.InsertL();
        view.SetColL( 1, aCertificateID );          // CertificateID
        view.SetColL( 2, aApplications[i].iUid );   // AppUid
        view.PutL();
        }
    
    CleanupStack::PopAndDestroy( &view );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::DefaultApplicationsL()
//
// Decide default applications for certificate regarding certain rules
// Format
//    EX509Certificate    = 0x00,
//    EWTLSCertificate    = 0x01,
//    EX968Certificate    = 0x02,
//    EUnknownCertificate = 0x0f,
//    EX509CertificateUrl = 0x10,
//    EWTLSCertificateUrl = 0x11,
//    EX968CertificateUrl = 0x12
//
// Trusting applications
//    KTrustUidWapBearer     
//    KTrustUidAppController
//    KTrustUidInternet
//    KTrustUidJavaMidlet
//
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::DefaultApplicationsL( const CWimCertInfo& aCert,
                                                   RArray<TUid>& aApplications )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::DefaultApplicationsL()" ) );    
    const CCTCertInfo* cctCert = aCert.CctCert();

    //Certificate owner type
    TCertificateOwnerType ownerType;
    ownerType = cctCert->CertificateOwnerType();

    //Certificate format
    TCertificateFormat certFormat;
    certFormat = cctCert->CertificateFormat();
    TInt32 appUid;
    
    // Applications only for CA certificates
    switch ( ownerType )
        {
        case ECACertificate:
            {   
            // Applications by certificate format
            switch ( certFormat )    
                {
                case EX509Certificate: // For Url not the same handling
                    {
                    DefaultAppsByOIDsL( aCert, aApplications );
                    break;
                    }
                case EWTLSCertificate: //All WTLSs has same apps, flow through!
                case EWTLSCertificateUrl:
                    {
                    appUid = KTrustUidWapBearer; //WAP
                    TUid uid = {appUid};
                    User::LeaveIfError( aApplications.Append( uid ) );
                    break;
                    }             
                default:
                    {
                    break;
                    }
                }
            break;
            }
        case EUserCertificate: // User certificate: no defaults
            {
            break;
            }
        case EPeerCertificate: // Peer certificate: no defaults
            {
            break;
            }
        default:
            {
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::DefaultAppsByOIDsL()
// Get Default AppUid(s) from database regarding the OIDs from certificate
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::DefaultAppsByOIDsL( const CWimCertInfo& aCert,
                                                 RArray<TUid>& aApplications )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::DefaultAppsByOIDsL()" ) );    
    // Get trusted usage OID(s) of certificate
    RArray<HBufC*> certTrustedUsage = aCert.TrustedUsage();
    // Get extended key usage OID(s) of certificate
    RArray<HBufC*> certExtendedKeyUsage = aCert.ExtendedKeyUsage();
    // Get certificate location 
    TUint8 certLocation = aCert.CDFRefs();

    TInt certTrustedCount;
    TInt certExtendedUsageCount;

    certTrustedCount = certTrustedUsage.Count();
    certExtendedUsageCount = certExtendedKeyUsage.Count();
   
    // Select all rows of table
    _LIT( KSQLQuery1, 
        "SELECT TrustedUsage, ExtendedKeyUsage, Location, AppUid " );
    _LIT( KSQLQuery2, "FROM CertificateDefaultApps " );
    TBuf<KMaxSQLLength> SQLStatement;
    SQLStatement.Copy( KSQLQuery1 );
    SQLStatement.Append( KSQLQuery2 );
    
    TPtrC sqlStat( SQLStatement.PtrZ() );
    RDbView view;
    User::LeaveIfError( view.Prepare( iDatabase, TDbQuery( sqlStat ) ) );
    CleanupClosePushL( view );
    
    TInt32  appUid = 0;
    TUid    uid;
    TBool   found = EFalse;
    TBool   canContinue = EFalse;
    TPtrC   defaultTrustedUsage;
    TPtrC   defaultExtendedKeyUsage;
    TInt    defaultLocation;

    // Check first if certificate's trusted usage field matches
    // After that check extended key usage
    while ( view.NextL() )
        {
        view.GetL();

        defaultTrustedUsage.Set( view.ColDes( 1 ) );
        defaultExtendedKeyUsage.Set( view.ColDes( 2 ) );
        defaultLocation = view.ColInt( 3 ); // Not used yet
        
        if ( certTrustedCount == 0 ) // No trusted usages in the cert
            {
            if ( defaultTrustedUsage.Length() == 0 ) // Either in defaults
                {
                canContinue = ETrue; 
                }
            }
        else
            {
            for ( TInt i = 0; i < certTrustedCount; i++ )
                {
                if ( !defaultTrustedUsage.
                      Compare( certTrustedUsage[i]->Des() ) )
                    {
                    canContinue = ETrue;
                    }
                }
            }

        if ( canContinue )
            {
            canContinue = EFalse;

            if ( certExtendedUsageCount == 0 ) // No ext.key usages in the cert
                {
                if ( defaultExtendedKeyUsage.Length() == 0 ) // Either in defs
                    {
                    canContinue = ETrue;
                    }
                }
            else
                {
                for ( TInt i = 0; i < certExtendedUsageCount; i++ )
                    {
                    if ( !defaultExtendedKeyUsage.
                          Compare( certExtendedKeyUsage[i]->Des() ) )
                        {
                        canContinue = ETrue;
                        }
                    }
                }
            }

        if ( canContinue )
            {
            if ( defaultLocation == EWimUnknownCDF ) // No default location
                {
                appUid = view.ColInt32( 4 );
                }
            else
                {
                if ( defaultLocation == certLocation )
                    {
                    appUid = view.ColInt32( 4 );
                    }
                }
            }

        if ( appUid > 0 ) // Application found -> append it
            {
            //Check if app UID allready exists in array
            for ( TInt i = 0; !found && i < aApplications.Count(); i++ )
                {
                if ( aApplications[i].iUid == appUid )
                    {
                    found = ETrue;
                    }
                }
            if ( !found ) // AppUid not added to array yet
                {
                uid.iUid = appUid;
                aApplications.AppendL( uid );
                }
            found = EFalse;
            }
        appUid = 0;
        canContinue = EFalse;
        }

    CleanupStack::PopAndDestroy( &view );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::CreateDBL()
// Create database for trust settings
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::CreateDBL() const
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::CreateDBL()" ) );    
    RFs fsSession;
    User::LeaveIfError( fsSession.Connect() );
    CleanupClosePushL( fsSession );
    
    RDbNamedDatabase database;

    // combine trust settings DB path & name

    TInt pathError = fsSession.MkDirAll( KWimTrustSettingsStorePath );
    if (( pathError != KErrNone ) && ( pathError != KErrAlreadyExists ))
        {
        User::Leave( pathError ); // Dir does not exist and can't be created
        }
    HBufC* dbPathBuffer = HBufC::NewLC( KWimTrustSettingsStorePath().Length() +
                                        KWimTrustSettingsDBFile().Length() );
    TPtr wimTrustSettingsDB( dbPathBuffer->Des() );
    wimTrustSettingsDB = KWimTrustSettingsStorePath;
    wimTrustSettingsDB.Append( KWimTrustSettingsDBFile );

    User::LeaveIfError( database.Replace( fsSession, wimTrustSettingsDB ) );
    CleanupClosePushL( database );

    // Create tables

    // Certificates
    _LIT( KSQLCreateTable1, "CREATE TABLE Certificates (\
                                          CertificateID COUNTER NOT NULL,\
                                          CertHash CHAR(20) NOT NULL,\
                                          TokenSerial CHAR(20) NOT NULL,\
                                          LabelHash CHAR(20) NOT NULL,\
                                          IsTrusted BIT NOT NULL)" );
    // CertificateApplications
    _LIT( KSQLCreateTable2, "CREATE TABLE CertificateApplications (\
                                          CertificateID INTEGER NOT NULL,\
                                          AppUid INTEGER NOT NULL )" );
    // CertificateDefaultApps
    _LIT( KSQLCreateTable3, "CREATE TABLE CertificateDefaultApps (\
                                          TrustedUsage VARCHAR(200),\
                                          ExtendedKeyUsage VARCHAR(200),\
                                          Location INTEGER NOT NULL,\
                                          AppUid INTEGER NOT NULL )" );

    User::LeaveIfError( database.Execute( KSQLCreateTable1 ) );
    User::LeaveIfError( database.Execute( KSQLCreateTable2 ) );
    User::LeaveIfError( database.Execute( KSQLCreateTable3 ) );

    // Create unique index for CertHash
    _LIT( KSQLTableIndex, "CREATE UNIQUE INDEX hash_index ON\
        Certificates (CertHash, TokenSerial, LabelHash)" );
    User::LeaveIfError( database.Execute( KSQLTableIndex ) );
   
    CleanupStack::PopAndDestroy( 3, &fsSession ); // database, dbPathBuffer, fsSession

    InsertDefaultAppRulesL();

    _WIMTRACE ( _L( "CWimTrustSettingsStore::CreateDBL()|End" ) );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::InsertDefaultAppRulesL()
// Insert default application UID selection rules to the database
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::InsertDefaultAppRulesL() const
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::InsertDefaultAppRulesL()" ) );
    RDbView view;

    RFs fsSession;
    User::LeaveIfError( fsSession.Connect() );
    CleanupClosePushL( fsSession );

    RDbNamedDatabase database;

    // combine trust settings DB path & name
    HBufC* dbPathBuffer = HBufC::NewLC( KWimTrustSettingsStorePath().Length() +
                                        KWimTrustSettingsDBFile().Length() );
    TPtr wimTrustSettingsDB( dbPathBuffer->Des() );
    wimTrustSettingsDB = KWimTrustSettingsStorePath;
    wimTrustSettingsDB.Append( KWimTrustSettingsDBFile );

    User::LeaveIfError( database.Open( fsSession, wimTrustSettingsDB ) );
    CleanupClosePushL( database );

    _LIT( KSQLInsertRules, "SELECT TrustedUsage, ExtendedKeyUsage,\
        Location, AppUid FROM CertificateDefaultApps" );

    User::LeaveIfError( view.Prepare( database, TDbQuery( KSQLInsertRules ),
        TDbWindow::EUnlimited, RDbView::EInsertOnly ) );

    CleanupClosePushL( view );

    view.InsertL();
    view.SetColL( 1, _L("") );                  // TrustedUsage OID
    view.SetColL( 2, _L("") );                  // ExtendedUsage OID
    view.SetColL( 3, TInt( EWimUnknownCDF ) );  // Location
    view.SetColL( 4, KTrustUidInternet );       // Application Uid
    view.PutL();

    view.InsertL();
    view.SetColL( 1, KIpKpServerAuth );         // TrustedUsage OID
    view.SetColL( 2, KIpKpServerAuth );         // ExtendedUsage OID
    view.SetColL( 3, TInt( EWimUnknownCDF ) );  // Location
    view.SetColL( 4, KTrustUidInternet );       // Application Uid
    view.PutL();

    view.InsertL();
    view.SetColL( 1, _L("") );                  // TrustedUsage OID
    view.SetColL( 2, KIpKpServerAuth );         // ExtendedUsage OID
    view.SetColL( 3, TInt( EWimUnknownCDF ) );  // Application Uid
    view.SetColL( 4, KTrustUidInternet );       // Application Uid
    view.PutL();

    view.InsertL();
    view.SetColL( 1, KIpKpServerAuth );         // TrustedUsage OID
    view.SetColL( 2, _L("") );                  // ExtendedUsage OID
    view.SetColL( 3, TInt( EWimUnknownCDF ) );  // Location
    view.SetColL( 4, KTrustUidInternet );       // Application Uid
    view.PutL();

    view.InsertL();
    view.SetColL( 1, KIpKpJavaMidlet );         // TrustedUsage OID
    view.SetColL( 2, _L("") );                  // ExtendedUsage OID
    view.SetColL( 3, TInt( EWimTrustedCertsCDF ) ); // Location
    view.SetColL( 4, KTrustUidJavaMidlet );     // Application Uid
    view.PutL();

    view.InsertL();
    view.SetColL( 1, _L("") );                  // TrustedUsage OID
    view.SetColL( 2, KWimCodeSigningOID );      // ExtendedUsage OID
    view.SetColL( 3, TInt( EWimTrustedCertsCDF ) );  // Location
    view.SetColL( 4, KTrustUidJavaMidlet );     // Application Uid
    view.PutL();

    CleanupStack::PopAndDestroy( 4, &fsSession ); // view, database, dbPathBuffer, fsSession
    }


// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::RunL()
// Not used.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::RunL()
    {

    switch ( iPhase )
        {
        case ERetrieve:
            {
            iPhase = ECheckRestore;

            if ( !iWimCertUtil )
                {
                iWimCertUtil = CWimCertUtil::NewL( iCert->Token() );
                iCertInfos.ResetAndDestroy();
                iWimCertUtil->Restore( iCertInfos, iStatus );
                }
            else
                {
                TRequestStatus* status = &iStatus;
                User::RequestComplete( status, KErrNone );
                }
            SetActive();
            break;
            }
        case ECheckRestore:
            {
            if ( iStatus.Int() == KErrNone )
                {
                // Allocate space for certificate data
                delete iEncodedCertBuf;
                iEncodedCertBuf = 0;
                delete iEncodedCertPtr;
                iEncodedCertPtr = 0;
                iEncodedCertBuf = HBufC8::NewL( iCert->Size() );
                iEncodedCertPtr = new( ELeave ) TPtr8( iEncodedCertBuf->Des() );
                iPhase = ECheckRetrieve;

                iWimCertUtil->RetrieveCertByIndexL( iCert->Handle().iObjectId,
                                                    *iEncodedCertPtr, 
                                                    iStatus ); 
                SetActive();
                }
            else
                {
                iPhase = EIdle;
                User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
                }
            break;
            }
        case ECheckRetrieve:
            {
            if ( iStatus.Int() == KErrNone )
                {
                ExtractExtensionL();
                DoSetDefaultTrustSettingsL( *iWimCert, iAddApps );
                User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
                }
            else
                {
                iPhase = EIdle;
                User::RequestComplete( iOriginalRequestStatus, iStatus.Int() );
                }
            break;
            }
        default:
            {
            // Here we should not be
            User::RequestComplete( iOriginalRequestStatus, KErrCorrupt );
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::ExtractExtensionL()
// If the certificate extension contains extended key usage and certificate 
// locates in the trusted certificates area, add OID to extended key usages
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::ExtractExtensionL()
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::ExtractExtensionL()" ) );    
    CX509Certificate* cert = CX509Certificate::NewLC( *iEncodedCertPtr );

    const CX509CertExtension* ext = cert->Extension( KExtendedKeyUsage );

    if ( ext )
        {
        TInt dummy = 0;
        CX509ExtendedKeyUsageExt* extendedKeyUsage = 
                CX509ExtendedKeyUsageExt::NewL( ext->Data(), dummy );
        CleanupStack::PushL( extendedKeyUsage );
        const CArrayPtrFlat<HBufC>& usages = extendedKeyUsage->KeyUsages();

        TInt count = usages.Count();
        for ( TInt i = 0; i < count; i++ )
            {
            HBufC* buf = usages[i]->AllocLC();
            CWimCertInfo* tmp = const_cast<CWimCertInfo*>( iWimCert );
            User::LeaveIfError( tmp->AddExtendedKeyUsage( buf ) ); 
            CleanupStack::Pop( buf ); // Take the ownership
            }

        CleanupStack::PopAndDestroy( extendedKeyUsage );
        }

    CleanupStack::PopAndDestroy( cert );
    }

// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::CancelDoing()
// Not used.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::CancelDoing()
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::Cancel()" ) );
    Cancel();
    }
// -----------------------------------------------------------------------------
// CWimTrustSettingsStore::DoCancel()
// Not used.
// -----------------------------------------------------------------------------
//
void CWimTrustSettingsStore::DoCancel()
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::DoCancel()" ) );
    iPhase = EIdle;
    User::RequestComplete( iOriginalRequestStatus, KErrCancel );
    }

// -----------------------------------------------------------------------------
// void CWimTrustSettingsStore::RunError()
// Leave leads us here.
// -----------------------------------------------------------------------------
//
TInt CWimTrustSettingsStore::RunError( TInt aError )
    {
    _WIMTRACE ( _L( "CWimTrustSettingsStore::RunError()" ) );
    iPhase = EIdle;
    User::RequestComplete( iOriginalRequestStatus, aError );
    return KErrNone;
    }

//  End of File  
