/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of TrustedSitesStoreServer
*
*/



#include "TrustedSitesServer.h"
#include "TrustedSitesConduit.h"
#include "TrustedSitesSession.h"
#include "DevTokenCliServ.h"
#include "DevTokenUtil.h"
#include "DevTokenDataTypes.h"
#include "DevTokenServer.h"
#include "DevandTruSrvCertStoreServer.h"
#include <x509cert.h>
#include <ccertattributefilter.h>
#include <e32cmn.h>
#include <badesca.h>

_LIT(KTrustedSitesStoreFilename,"TrustedSitesStore.dat");
// Maximum length of SQL query clause
const TInt KMaxSQLLength = 256;

// API policing
_LIT_SECURITY_POLICY_C1(KReadSecurityPolicy, ECapabilityReadUserData);
_LIT_SECURITY_POLICY_C1(KWriteSecurityPolicy, ECapabilityWriteDeviceData);

// ======== MEMBER FUNCTIONS ========

//CTrustedSitesServer

// ---------------------------------------------------------------------------
// CTrustedSitesServer::NewL()
// ---------------------------------------------------------------------------
//
CTrustedSitesServer* CTrustedSitesServer::NewL( CDevTokenServer* aServer )
    {
    CTrustedSitesServer* self = new (ELeave) CTrustedSitesServer( aServer );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::CTrustedSitesServer()
// ---------------------------------------------------------------------------
//
CTrustedSitesServer::CTrustedSitesServer( CDevTokenServer* aServer ) :
    iServer ( aServer )
    {
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::ConstructL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::ConstructL()
    {
    iConduit = CTrustedSitesConduit::NewL(*this);

    User::LeaveIfError( iFs.Connect() );

    TBuf<KMaxFilenameLength> filename;

    FileUtils::MakePrivateFilenameL(iFs, KTrustedSitesStoreFilename, filename);

    TInt err = iDatabase.Open( iFs, filename );

    if ( err != KErrNone ) 
        {
        CreateDBL(); // Create DB
        User::LeaveIfError( iDatabase.Open( iFs, filename ) );
        }
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::CreateDBL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::CreateDBL()
    {

    RFs fsSession;
    User::LeaveIfError( fsSession.Connect() ); // Connect to file server

    CleanupClosePushL( fsSession );

    RDbNamedDatabase database;

    TBuf<KMaxFilenameLength> filename;

    FileUtils::MakePrivateFilenameL(fsSession, KTrustedSitesStoreFilename, filename);

    FileUtils::EnsurePathL(fsSession, filename);
    CleanupClosePushL( database );

    User::LeaveIfError( database.Create( fsSession, filename ) );

    // Create tables

    // Trusted sites table
    _LIT( KSQLCreateTable1, "CREATE TABLE Certificates (\
    CertID COUNTER NOT NULL,\
    CertHash CHAR(20) NOT NULL)");

    _LIT( KSQLCreateTable2, "CREATE TABLE TrustedSites (\
    CertID INTEGER,\
    SiteName VARCHAR(100) NOT NULL,\
    Forgiven INTEGER NOT NULL,\
    OutOfDate INTEGER NOT NULL,\
    ClientUID INTEGER NOT NULL )");                                      
    
    User::LeaveIfError( database.Execute( KSQLCreateTable1 ) );
    User::LeaveIfError( database.Execute( KSQLCreateTable2 ) );

    CleanupStack::PopAndDestroy( 2 ); // database, fsSession
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::~CTrustedSitesServer()
// ---------------------------------------------------------------------------
//
CTrustedSitesServer::~CTrustedSitesServer()
    {
    delete iConduit;
    iDatabase.Close();  // Close database
    iFs.Close();
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::CreateSessionL()
// ---------------------------------------------------------------------------
//
CTrustedSitesSession* CTrustedSitesServer::CreateSessionL()
    {
    return CTrustedSitesSession::NewL(*iConduit);
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::AddL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::AddL(const TDesC8& aCertHash, const TDesC& aSiteName, const RMessage2& aMessage )
    {
    // API policing
    if (!KWriteSecurityPolicy.CheckPolicy(aMessage))
        {
        User::Leave(KErrPermissionDenied);
        }

    TInt certID;
    GetCertIDL( aCertHash, certID ); // Get certificate ID

    if ( certID == KErrNotFound ) // Not found
        {
        iDatabase.Begin(); // Begin transaction

        TRAPD( err, DoAddCertL( aCertHash ));

        if ( err ) // Some error occurred
            {
            iDatabase.Rollback(); // Rollback changes
            User::Leave ( err );
            }
        else
            {
            iDatabase.Commit(); // Commit changes
            }

        GetCertIDL( aCertHash, certID );

        if( certID == KErrNotFound )
            {
            User::Leave( KErrNotFound );
            }

        iDatabase.Begin();

        TRAP( err, DoAddSiteNameL( certID, aSiteName ));

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
    else
        {
        if(IsSiteAlreadyExistL( certID, aSiteName ))
            {
            return; 
            }
        else
            {
            iDatabase.Begin(); // Begin transaction

            TRAPD( err, DoAddSiteNameL( certID, aSiteName ));

            if ( err ) // Some error occurred
                {
                iDatabase.Rollback(); // Rollback changes
                User::Leave ( err );
                }
            else
                {
                iDatabase.Commit(); // Commit changes
                }
            return;   
            } 
        }  

    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::AddL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::AddL(const TDesC8& aCertHash, const TDesC& aSiteName )
    {
    TInt certID;
    GetCertIDL( aCertHash, certID ); // Get certificate ID

    if ( certID == KErrNotFound ) // Not found
        {
        iDatabase.Begin(); // Begin transaction

        TRAPD( err, DoAddCertL( aCertHash ));

        if ( err ) // Some error occurred
            {
            iDatabase.Rollback(); // Rollback changes
            User::Leave ( err );
            }
        else
            {
            iDatabase.Commit(); // Commit changes
            }

        GetCertIDL( aCertHash, certID );

        if( certID == KErrNotFound )
            {
            User::Leave( KErrNotFound );
            }

        iDatabase.Begin();

        TRAP( err, DoAddSiteNameL( certID, aSiteName ));

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
    else
        {
        if(IsSiteAlreadyExistL( certID, aSiteName ))
            {
            return; 
            }
        else
            {
            iDatabase.Begin(); // Begin transaction

            TRAPD( err, DoAddSiteNameL( certID, aSiteName ));

            if ( err ) // Some error occurred
                {
                iDatabase.Rollback(); // Rollback changes
                User::Leave ( err );
                }
            else
                {
                iDatabase.Commit(); // Commit changes
                }
            return;   
            } 
        }  
    }
    
    
// ---------------------------------------------------------------------------    
// CTrustedSitesServer::GetCertIDL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::GetCertIDL( const TDesC8& aCertHash, TInt& aCertID )
    {   
    _LIT( KSQLQuery, "SELECT CertID,\
    CertHash\
    FROM Certificates" );

    RDbView view;

    User::LeaveIfError( view.Prepare( iDatabase, TDbQuery( KSQLQuery ) ) );

    CleanupClosePushL( view );

    // Get data
    TInt i = 0;

    while ( i == 0 && view.NextL() ) // Should find only one ID
        {
        view.GetL();

        if ( !view.ColDes8( 2 ).Compare( aCertHash ) )
            {
            aCertID = view.ColUint32( 1 );
            i++;
            }
        }

    if ( i == 0 ) // Not found any matching rows
        {
        aCertID = KErrNotFound;
        }

    CleanupStack::PopAndDestroy( 1 ); // view
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::DoAddSiteNameL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::DoAddSiteNameL(const TInt& aCertID, const TDesC& aSiteName )   
    {
    //insert the normal trusted site record,not for forgiven site
    TBuf<KMaxSQLLength> SQL;

    _LIT( KSQLInsert1,
    "INSERT INTO TrustedSites (CertID,SiteName,Forgiven,OutOfDate,ClientUID) VALUES (" );

    SQL.Copy(KSQLInsert1);
    SQL.AppendNum(aCertID);
    SQL.Append(_L(",'"));
    
    SQL.Append( aSiteName );
    SQL.Append(_L("',"));
    //not forgiven
    SQL.AppendNum(0);
    SQL.Append(_L(","));
    //not allowed out of date
    SQL.AppendNum(0); 
    //set the UID to -1 since in the normal case we don't need the information
    SQL.Append(_L(","));
    SQL.AppendNum(-1);
    SQL.Append(_L(")"));
    
    User::LeaveIfError( iDatabase.Execute( SQL ) );
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::DoAddCertL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::DoAddCertL( const TDesC8& aCertHash )
    {   
    RDbView view;

    _LIT( KSQLInsertCert, "SELECT CertHash\
    FROM Certificates" );

    User::LeaveIfError( view.Prepare( iDatabase, TDbQuery( KSQLInsertCert ),
    TDbWindow::EUnlimited, RDbView::EInsertOnly ) );

    CleanupClosePushL( view );

    view.InsertL();
    view.SetColL( 1, aCertHash );
    view.PutL();

    CleanupStack::PopAndDestroy( 1 ); 
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::IsSiteAlreadyExistL()
// ---------------------------------------------------------------------------
//
TBool CTrustedSitesServer::IsSiteAlreadyExistL(const TInt& aCertID, const TDesC& aSiteName)
    {
    RDbView view;   
    _LIT( KSQLQuery, 
    "SELECT SiteName FROM TrustedSites WHERE certID = " );

    TBuf<KMaxSQLLength> SQLStatement;
    SQLStatement.Copy( KSQLQuery );

    SQLStatement.AppendNum( aCertID );

    TPtrC sqlStat( SQLStatement.PtrZ() );

    TInt err = view.Prepare( iDatabase, TDbQuery( sqlStat ), TDbWindow::EUnlimited );

    if ( err != KErrNone )
        {
        User::Leave( KErrArgument );
        }

    err = view.EvaluateAll();

    CleanupClosePushL( view );

    TBool alreadyExist = EFalse;
    // Get data
    while ( view.NextL() ) // Should find only one certificate
        {
        TRAPD(error, view.GetL());
        error = error;
        if(!aSiteName.Compare(view.ColDes( 1 )))
            {
            alreadyExist = ETrue;
            break;
            }
        }
    CleanupStack::PopAndDestroy( 1 ); 
    return alreadyExist;
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::IsOutOfDateAllowedForTheSiteL()
// ---------------------------------------------------------------------------
//
TBool CTrustedSitesServer::IsOutOfDateAllowedForTheSiteL( const TInt& aCertID, const TDesC& aSiteName )
    {
    RDbView view;   
    _LIT( KSQLQuery, 
    "SELECT SiteName,OutOfDate FROM TrustedSites WHERE certID = " );

    TBuf<KMaxSQLLength> SQLStatement;
    SQLStatement.Copy( KSQLQuery );

    SQLStatement.AppendNum( aCertID );

    TPtrC sqlStat( SQLStatement.PtrZ() );

    TInt err = view.Prepare( iDatabase, TDbQuery( sqlStat ), TDbWindow::EUnlimited );

    if ( err != KErrNone )
        {
        User::Leave( KErrArgument );
        }

    err = view.EvaluateAll();

    CleanupClosePushL( view );

    TBool outofdate = EFalse;
    // Get data
    while ( view.NextL() ) // Should find only one certificate
        {
        TRAPD(error, view.GetL());
        error = error;
        
        if ( (!aSiteName.Compare( view.ColDes( 1 ) ) ) && ( view.ColInt( 2 ) == 1 ) )
            {
            outofdate = ETrue;
            break;
            }
        }
        
    CleanupStack::PopAndDestroy( 1 ); //view
    return outofdate; 
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::IsTrustedSiteL()
// ---------------------------------------------------------------------------
//
TBool CTrustedSitesServer::IsTrustedSiteL( const TDesC8& aCertHash, const TDesC& aSiteName, const TDesC8& aCert, const RMessage2& aMessage )
    {
    // API policing
    if (!KReadSecurityPolicy.CheckPolicy(aMessage))
        {
        User::Leave(KErrPermissionDenied);
        }
    
    //is it a forgiven site 
    if ( IsForgivenSiteAlreadyL( aSiteName ) )
        {
        // Insert the certificate into database
        DoOverWriteCertL( aSiteName, aCertHash );
        
        //check if the certificate is already existing
        CCertAttributeFilter* filter = CCertAttributeFilter::NewL();
        CleanupStack::PushL(filter);
        filter->SetOwnerType(EPeerCertificate);
        RPointerArray<CDevTokenCertInfo> certs;
        RMessage2 message;             
        iServer->CertStoreServerL().ListL( *filter,certs,message, ETrue  );            
      
        CleanupStack::PopAndDestroy(); //filter
      
        TBool exist = EFalse;
        if ( certs.Count() )
            {
            for (TInt i= 0; i<certs.Count();i++)
                {
                HBufC8* certData = iServer->CertStoreServerL().RetrieveLC( certs[i]->CertificateId(),message,ETrue );
                TPtr8 certPtr = certData->Des();
                
                //certificate from trusted server certstore
                CX509Certificate* cert = CX509Certificate::NewLC( certPtr );
                //certificate from client
                CX509Certificate* cert2 = CX509Certificate::NewLC( aCert );
              
                if ( cert->Fingerprint() == cert2->Fingerprint() )
                    {
                    exist = ETrue;
                    CleanupStack::PopAndDestroy(3); //cert, cert2, certData
                    break;
                    }
              
                CleanupStack::PopAndDestroy(3); //cert, cert2, certData
                }
            }
          
        if ( exist )
             {
             //server certificate has saved already, no need to save again
             return ETrue;  
             }

        TDevTokenAddCertDataStruct data;
        data.iLabel.Zero();
        data.iSubjectKeyId.Zero();
        
        CX509Certificate* cert = CX509Certificate::NewL(aCert);
        data.iSubjectKeyId.Copy( cert->KeyIdentifierL() );
        delete cert;
        
        data.iIssuerKeyId.Zero();   
        data.iLabel.Copy(aSiteName);   
        data.iFormat = EX509Certificate;
        data.iCertificateOwnerType = EPeerCertificate;
        
        iServer->CertStoreServerL().AddL( data, aCert, aMessage, ETrue ) ;
         
        return ETrue;
        }
    
    //It isn't a forgiven site, query in the normal way to check whether it is trusted by the user 
    TInt certID;
    GetCertIDL( aCertHash, certID ); // Get certificate ID

    if ( certID == KErrNotFound ) // Not found
        {
        return EFalse;
        }
    else
        {
        if(IsSiteAlreadyExistL( certID, aSiteName ))
            {
            return ETrue; 
            }
        else
            {
            return EFalse;    
            } 
        }  
    }



// ---------------------------------------------------------------------------
// CTrustedSitesServer::IsOutOfDateAllowedL()
// ---------------------------------------------------------------------------
//
TBool CTrustedSitesServer::IsOutOfDateAllowedL( const TDesC8& aCertHash, const TDesC& aSiteName, const RMessage2& aMessage )
    {
    // API policing
    if (!KReadSecurityPolicy.CheckPolicy(aMessage))
        {
        User::Leave(KErrPermissionDenied);
        }

    TInt certID;
    GetCertIDL( aCertHash, certID ); // Get certificate ID

    if ( certID == KErrNotFound ) // Not found
        {
        return EFalse;
        }
    else
        {
        if(IsOutOfDateAllowedForTheSiteL( certID, aSiteName ))
            {
            return ETrue; 
            }
        else
            {
            return EFalse;    
            } 
        }  
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::GetTrustedSitesL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::GetTrustedSitesL( const TDesC8& aCertHash, 
                                           RPointerArray<HBufC>& aListOfSites,
                                           const RMessage2& aMessage)
    {
    // API policing
    if (!KReadSecurityPolicy.CheckPolicy(aMessage))
        {
        User::Leave(KErrPermissionDenied);
        }

    TInt certID;
    GetCertIDL( aCertHash, certID );

    if( certID == KErrNotFound )
        {
        User::Leave( KErrNotFound );
        }
    DoGetTrustedSitesL( certID, aListOfSites );     
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::DoGetTrustedSitesL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::DoGetTrustedSitesL( const TInt& aCertID, 
                                              RPointerArray<HBufC>& aListOfSites)
    {
    RDbView view;   
    _LIT( KSQLQuery, 
    "SELECT SiteName FROM TrustedSites WHERE certID = " );

    TBuf<KMaxSQLLength> SQLStatement;
    SQLStatement.Copy( KSQLQuery );

    SQLStatement.AppendNum( aCertID );

    TPtrC sqlStat( SQLStatement.PtrZ() );

    TInt err = view.Prepare( iDatabase, TDbQuery( sqlStat ), TDbWindow::EUnlimited );

    if ( err != KErrNone )
        {
        User::Leave( KErrArgument );
        }

    err = view.EvaluateAll();

    CleanupClosePushL( view );

    // Get data
    while ( view.NextL() ) // Should find only one certificate
        {
        TRAPD(error, view.GetL());
        error = error;
        HBufC* temp = HBufC::NewL(KMaxSiteName);
        TPtr ptrTemp = temp->Des();
        ptrTemp.Copy(view.ColDes( 1 ));      
        aListOfSites.Append(temp);
        }
    CleanupStack::PopAndDestroy( 1 ); 
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::RemoveL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::RemoveL( const TDesC8& aCertHash )
    {  
    TInt certID;
    GetCertIDL( aCertHash, certID );

    if( certID == KErrNotFound )
        {
        User::Leave( KErrNotFound );
        }
    CCertAttributeFilter* filter = CCertAttributeFilter::NewL();
    CleanupStack::PushL(filter);
    filter->SetOwnerType(EPeerCertificate);
    RPointerArray<CDevTokenCertInfo> certs;
    RMessage2 message;             
    iServer->CertStoreServerL().ListL(*filter,certs,message, ETrue  );            
    
    CleanupStack::PopAndDestroy(); //filter
    
    TBool stillexist = EFalse;
    if ( certs.Count() )
        {
        for (TInt i= 0; i<certs.Count();i++)
            {
            HBufC8* certData = iServer->CertStoreServerL().RetrieveLC( certs[i]->CertificateId(),message,ETrue );

            TPtr8 certPtr = certData->Des();
        	
            CX509Certificate* cert = CX509Certificate::NewLC( certPtr );
            
            if ( cert->Fingerprint() == aCertHash )
                 {
                 stillexist = ETrue;
            	 CleanupStack::PopAndDestroy( 2 ); //certData, cert
                 break;
                 }

            CleanupStack::PopAndDestroy( 2 ); //certData, cert
            }
        }
        
    if ( stillexist )
        {
        //there are still same server certificate saved in cert store
        //with different label, doesn't delete information then
        return;	
        }
    iDatabase.Begin(); // Begin transaction

    TRAPD( err, DoRemoveL( certID ));

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


// ---------------------------------------------------------------------------
// CTrustedSitesServer::DoRemoveL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::DoRemoveL(const TInt& aCertID )
    {  
    _LIT( KSQLDelete1, "DELETE FROM Certificates WHERE CertID = " );
    _LIT( KSQLDelete2, "DELETE FROM TrustedSites WHERE CertID = " );

    TBuf<KMaxSQLLength> SQLStatement1;
    TBuf<KMaxSQLLength> SQLStatement2;

    SQLStatement1.Copy( KSQLDelete1 );
    SQLStatement1.AppendNum( aCertID );
    TPtrC sqlStat1( SQLStatement1.PtrZ() );

    SQLStatement2.Copy( KSQLDelete2 );
    SQLStatement2.AppendNum( aCertID );
    TPtrC sqlStat2( SQLStatement2.PtrZ() );

    User::LeaveIfError( iDatabase.Execute( sqlStat1) );
    User::LeaveIfError( iDatabase.Execute( sqlStat2 ) );
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::AddForgivenSiteL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::AddForgivenSiteL( const TDesC& aSite, const TBool& aOutOfDateAllowed, const RMessage2& aMessage )
    {
    // API policing,WriteDeviceData
    if (!KWriteSecurityPolicy.CheckPolicy(aMessage))
        {
        User::Leave(KErrPermissionDenied);
        }

    if ( IsForgivenSiteAlreadyL( aSite ) )
       {
       //the hostname is in table already.
       DoOverWriteOutOfDateL( aSite, aOutOfDateAllowed );
       return;  
       }
    else
       {
       iDatabase.Begin(); // Begin transaction

       TRAPD( err, DoAddForgivenSiteL( aSite, aOutOfDateAllowed, aMessage.SecureId().iId ));

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
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::RemoveAllForgivenSitesL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::RemoveAllForgivenSitesL( const RMessage2& aMessage )
	{
    // API policing,WriteDeviceData
    if (!KWriteSecurityPolicy.CheckPolicy(aMessage))
        {
        User::Leave(KErrPermissionDenied);
        }
    
    // get certid arrays for the ClientUID
    RDbView view;   
    _LIT( KSQLQuery, 
    "SELECT SiteName,CertID FROM TrustedSites WHERE ClientUID = " );

    TBuf<KMaxSQLLength> SQLStatement;
    SQLStatement.Copy( KSQLQuery );

    SQLStatement.AppendNum( aMessage.SecureId().iId );

    TPtrC sqlStat( SQLStatement.PtrZ() );

    TInt err = view.Prepare( iDatabase, TDbQuery( sqlStat ), TDbWindow::EUnlimited );

    if ( err != KErrNone )
        {
        User::Leave( KErrArgument );
        }

    err = view.EvaluateAll();

    CleanupClosePushL( view );
    
    RArray<TInt> certIDs;
    RArray<TInt> certIDs2;
    if ( view.CountL() )
    	{
        // Get data
        while ( view.NextL() ) // Should find only one certificate
            {
            TRAPD(error, view.GetL());
            error = error;
            certIDs.Append( view.ColInt( 2 ) );      
            }
        CleanupStack::PopAndDestroy( 1 ); //view
    	}
    else
    	{
    	CleanupStack::PopAndDestroy( 1 ); //view
    	//no match found, nothing to delete 
    	return;
    	}
    
    if ( certIDs.Count() == 0 )
    	{
    	//no cert id, just return;
    	return;
    	}
    
    //the certID may contain -1, then no need to delete the certificate but go to delete the site directly
    //filter the -1 out.
    for ( TInt i = 0; i < certIDs.Count() ; i++ )
    	{
    	if ( certIDs[i] != -1 )
    		{
    		certIDs2.Append( certIDs[i] );
    		}
    	}
    
    // if certid2 is not empty, it means there are certificates associated with the sitename,
    // we should delete it.
    if ( certIDs2.Count() > 0 )
    	{
    	// get the certhash array for certid arrays
        //the desc8 array to put the certhash
        TInt num = certIDs2.Count();
        CDesC8ArrayFlat* descarray = new (ELeave) CDesC8ArrayFlat( num );
        CleanupStack::PushL( descarray );
        
        for ( TInt i = 0; i < certIDs2.Count(); i++ )
        	{
        	//empty the previous command
        	SQLStatement.Zero();
            _LIT( KSQLQuery, "SELECT CertHash FROM Certificates WHERE CertID =" );
            SQLStatement.Append( KSQLQuery );
            SQLStatement.AppendNum( certIDs2[i] );
            
           	TPtrC sqlStat1( SQLStatement.PtrZ() );
        	
            TInt err = view.Prepare( iDatabase, TDbQuery( sqlStat1 ), TDbWindow::EUnlimited );
         
            if ( err != KErrNone )
                {
                User::Leave( KErrArgument );
                }

            err = view.EvaluateAll();
            
            CleanupClosePushL( view );
           
            //put the hash into array
            while ( view.NextL() ) // Should find only one certificate
                {
                TRAPD(error, view.GetL());
                error = error;
                descarray->AppendL( view.ColDes8( 1 ) );      
                }
            CleanupStack::PopAndDestroy( 1 ); //view
        	}
        
        // List all of the certificate in trusted server certstore
        // retrive them one by one to compare the certhash.
        //if match found, remove the certificate from the trusted server certstore
        
        CCertAttributeFilter* filter = CCertAttributeFilter::NewL();
        CleanupStack::PushL(filter);
        filter->SetOwnerType(EPeerCertificate);
        RPointerArray<CDevTokenCertInfo> certs;
        RMessage2 message;             
        iServer->CertStoreServerL().ListL(*filter,certs,message, ETrue  );            
        
        CleanupStack::PopAndDestroy(); //filter
        
        if ( certs.Count() )
            {
            for ( TInt i= 0; i < certs.Count(); i++ )
                {
                HBufC8* certData = iServer->CertStoreServerL().RetrieveLC( certs[i]->CertificateId(),message,ETrue );

                TPtr8 certPtr = certData->Des();
            	
                CX509Certificate* cert = CX509Certificate::NewLC( certPtr );
                
                for ( TInt ii=0; ii < descarray->Count(); ii++ )
                	{
                	if ( (*descarray)[ii] == cert->Fingerprint() )
                		{
                           //match, then delete it 
                		iServer->CertStoreServerL().RemoveL( certs[i]->CertificateId(),message,ETrue );
                		break;
                		}
                	}
                
                CleanupStack::PopAndDestroy( 2 ); //certData, cert
                }
            }
        descarray->Reset(); //clean the buffer for certhash
        CleanupStack::PopAndDestroy(); //descarray
        
        //delete the cert record in Certificates Table
        for ( TInt i = 0; i < certIDs2.Count(); i++ )
        	{
        	_LIT( KSQLDelete1, "DELETE FROM Certificates WHERE CertID = " );
        	SQLStatement.Zero();

        	SQLStatement.Copy( KSQLDelete1 );
        	SQLStatement.AppendNum( certIDs2[i] );
        	TPtrC sqlStat2( SQLStatement.PtrZ() );
        	
        	User::LeaveIfError( iDatabase.Execute( sqlStat2 ) );
        	}
    	}
        
    //now corresponding certificates are deleted,  it is time to delete the sitenames
    // just delete all of the record where uid matched with the client uid
    
    _LIT( KSQLDelete2, "DELETE FROM TrustedSites WHERE ClientUID = " );

    SQLStatement.Zero();
    SQLStatement.Copy( KSQLDelete2 );
    SQLStatement.AppendNum( aMessage.SecureId().iId  );
    
    TPtrC sqlStat3( SQLStatement.PtrZ() );

    User::LeaveIfError( iDatabase.Execute( sqlStat3) );
	}



// ---------------------------------------------------------------------------
// CTrustedSitesServer::RemoveThisForgivenSiteL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::RemoveThisForgivenSiteL( const TDesC& aSite, const RMessage2& aMessage )
	{
    // API policing,WriteDeviceData
    if (!KWriteSecurityPolicy.CheckPolicy(aMessage))
    	{
        User::Leave(KErrPermissionDenied);
        }
    
    // get certid arrays for the ClientUID
    RDbView view;   
    _LIT( KSQLQuery, 
    "SELECT CertID FROM TrustedSites WHERE ClientUID = " );

    TBuf<KMaxSQLLength> SQLStatement;
    SQLStatement.Copy( KSQLQuery );

    SQLStatement.AppendNum( aMessage.SecureId().iId );
    SQLStatement.Append(_L(" AND SiteName = '") );
    SQLStatement.Append( aSite );
    SQLStatement.Append(_L("'"));
    
    
    TPtrC sqlStat( SQLStatement.PtrZ() );

    TInt err = view.Prepare( iDatabase, TDbQuery( sqlStat ), TDbWindow::EUnlimited );

    if ( err != KErrNone )
        {
        User::Leave( KErrArgument );
        }

    err = view.EvaluateAll();
    
    
    CleanupClosePushL( view );
    
    RArray<TInt> certIDs;
    RArray<TInt> certIDs2;
    if ( view.CountL() )
    	{
        // Get data
        while ( view.NextL() ) // Should find only one certificate
            {
            TRAPD(error, view.GetL());
            error = error;
            certIDs.Append( view.ColInt( 1 ) );      
            }
        CleanupStack::PopAndDestroy( 1 ); //view
    	}
    else
    	{
    	CleanupStack::PopAndDestroy( 1 ); //view
    	//no match found, nothing to delete 
    	return;
    	}
    
    if ( certIDs.Count() == 0 )
    	{
    	//no cert id, just return;
    	return;
    	}
    
    //the certID may contain -1, then no need to delete the certificate but go to delete the site directly
    //filter the -1 out.
    for ( TInt i = 0; i < certIDs.Count() ; i++ )
    	{
    	if ( certIDs[i] != -1 )
    		{
    		certIDs2.Append( certIDs[i] );
    		}
    	}
    // if certid2 is not empty, it means there are certificates associated with the sitename,
    // we should delete it.
    
    if ( certIDs2.Count() > 0 )
    	{
    	// get the certhash array for certid arrays
        //the desc8 array to put the certhash
        TInt num = certIDs2.Count();
        CDesC8ArrayFlat* descarray = new (ELeave) CDesC8ArrayFlat( num );
        CleanupStack::PushL( descarray );
        
        for ( TInt i = 0; i < certIDs2.Count(); i++ )
        	{
        	//empty the previous command
        	SQLStatement.Zero();
            _LIT( KSQLQuery, "SELECT CertHash FROM Certificates WHERE CertID =" );
            SQLStatement.Append( KSQLQuery );
            SQLStatement.AppendNum( certIDs2[i] );
            
           	TPtrC sqlStat1( SQLStatement.PtrZ() );
        	
            TInt err = view.Prepare( iDatabase, TDbQuery( sqlStat1 ), TDbWindow::EUnlimited );
         
            if ( err != KErrNone )
                {
                User::Leave( KErrArgument );
                }

            err = view.EvaluateAll();
            
            CleanupClosePushL( view );
           
            //put the hash into array
            while ( view.NextL() ) // Should find only one certificate
                {
                TRAPD(error, view.GetL());
                error = error;
                descarray->AppendL( view.ColDes8( 1 ) );      
                }
            CleanupStack::PopAndDestroy( 1 ); //view
        	}
        
        // List all of the certificate in trusted server certstore
        // retrive them one by one to compare the certhash.
        //if match found, remove the certificate from the trusted server certstore
        
        CCertAttributeFilter* filter = CCertAttributeFilter::NewL();
        CleanupStack::PushL(filter);
        filter->SetOwnerType(EPeerCertificate);
        RPointerArray<CDevTokenCertInfo> certs;
        RMessage2 message;             
        iServer->CertStoreServerL().ListL(*filter,certs,message, ETrue  );            
        
        CleanupStack::PopAndDestroy(); //filter
        
        if ( certs.Count() )
            {
            for ( TInt i= 0; i < certs.Count(); i++ )
                {
                HBufC8* certData = iServer->CertStoreServerL().RetrieveLC( certs[i]->CertificateId(),message,ETrue );

                TPtr8 certPtr = certData->Des();
            	
                CX509Certificate* cert = CX509Certificate::NewLC( certPtr );
                
                for ( TInt ii=0; ii < descarray->Count(); ii++ )
                	{
                	if ( (*descarray)[ii] == cert->Fingerprint() )
                		{
                           //match, then delete it 
                		iServer->CertStoreServerL().RemoveL( certs[i]->CertificateId(),message,ETrue );
                		break;
                		}
                	}
                
                CleanupStack::PopAndDestroy( 2 ); //certData, cert
                }
            }
        descarray->Reset(); //clean the buffer for certhash
        CleanupStack::PopAndDestroy(); //descarray
        
        //delete the cert record in Certificates Table
        for ( TInt i = 0; i < certIDs2.Count(); i++ )
        	{
        	_LIT( KSQLDelete1, "DELETE FROM Certificates WHERE CertID = " );
        	SQLStatement.Zero();

        	SQLStatement.Copy( KSQLDelete1 );
        	SQLStatement.AppendNum( certIDs2[i] );
        	TPtrC sqlStat2( SQLStatement.PtrZ() );
        	
        	User::LeaveIfError( iDatabase.Execute( sqlStat2 ) );
        	}
    	}
    //now corresponding certificates are deleted,  it is time to delete the sitenames
    // just delete all of the record where uid matched with the client uid
    
    _LIT( KSQLDelete2, "DELETE FROM TrustedSites WHERE ClientUID = " );

    SQLStatement.Zero();
    SQLStatement.Copy( KSQLDelete2 );
    SQLStatement.AppendNum( aMessage.SecureId().iId  );
    SQLStatement.Append(_L("AND SiteName = '"));
    SQLStatement.Append( aSite );
    SQLStatement.Append(_L("'"));
    
    TPtrC sqlStat3( SQLStatement.PtrZ() );
    
    //TInt errr = iDatabase.Execute( sqlStat3);
    User::LeaveIfError( iDatabase.Execute( sqlStat3) );
	}


// ---------------------------------------------------------------------------
// CTrustedSitesServer::DoAddForgivenSiteL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::DoAddForgivenSiteL(const TDesC& aSite, const TBool& aOutOfDateAllowed, const TUint32& aUID )
    {
    TBuf<KMaxSQLLength> SQLStatement;

    _LIT( KSQLInsert1,
    "INSERT INTO TrustedSites (CertID,SiteName,Forgiven,OutOfDate,ClientUID) VALUES (" );

    SQLStatement.Copy(KSQLInsert1);
    SQLStatement.AppendNum(-1);
    SQLStatement.Append(_L(",'"));
    
    SQLStatement.Append( aSite );
    SQLStatement.Append(_L("',"));
    SQLStatement.AppendNum(1);
    SQLStatement.Append(_L(","));
    //add record for forgiven sites, check if outofdate is allowed also
    if ( aOutOfDateAllowed )
        {
        SQLStatement.AppendNum(1);
        }
    else 
        {
        SQLStatement.AppendNum(0); 
        }
    SQLStatement.Append(_L(","));
    //append the client UID.
    SQLStatement.AppendNum( aUID );
    SQLStatement.Append(_L(")"));
    
    User::LeaveIfError( iDatabase.Execute( SQLStatement ) );
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::IsForgivenSiteAlreadyL()
// ---------------------------------------------------------------------------
//
TBool CTrustedSitesServer::IsForgivenSiteAlreadyL( const TDesC& aSite )
    {
    RDbView view;   
    _LIT( KSQLCheckForgivenSite, 
    "SELECT SiteName, Forgiven, OutOfDate FROM TrustedSites" );

    TInt err = view.Prepare( iDatabase, TDbQuery( KSQLCheckForgivenSite ) );
    
    if( err == KErrNotFound )
        {
        return EFalse; 
        }
    
    if ( err != KErrNone )
        {
        User::Leave( KErrArgument );
        }

    err = view.EvaluateAll();

    CleanupClosePushL( view );

    TBool alreadyExist = EFalse;
    
    // looking for a match
    while ( view.NextL() ) // Should find only one certificate
        {
        TRAPD(error, view.GetL());
        error = error;
        
        // sitename is matched and forgiven is set to 1
        if(  (!aSite.Compare(view.ColDes( 1 ))) && ( view.ColInt( 2 ) == 1 )   )
            {
            alreadyExist = ETrue;
            break;
            }
        }
        
    CleanupStack::PopAndDestroy( 1 ); 
    return alreadyExist;
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::DoOverWriteOutOfDateL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::DoOverWriteOutOfDateL(const TDesC& aSite, const TBool& aOutOfDateAllowed )
    {
    _LIT( KSQLOverWrite1, "UPDATE TrustedSites SET OutOfDate = ");
    
    _LIT( KSQLOverWrite2, " WHERE SiteName = '");

    TBuf<KMaxSQLLength> SQLStatement;
    SQLStatement.Copy(KSQLOverWrite1);
    SQLStatement.AppendNum(aOutOfDateAllowed);
    SQLStatement.Append(KSQLOverWrite2);
    SQLStatement.Append(aSite);
    SQLStatement.Append(_L("'"));
    
    User::LeaveIfError( iDatabase.Execute( SQLStatement ) ); 
    }


// ---------------------------------------------------------------------------
// CTrustedSitesServer::DoOverWriteCertL()
// ---------------------------------------------------------------------------
//
void CTrustedSitesServer::DoOverWriteCertL( const TDesC& aSiteName, const TDesC8& aCertHash )
    {
    TInt certID;
    GetCertIDL( aCertHash, certID );
    
    if( certID == KErrNotFound )
        {
        DoAddCertL( aCertHash );
        GetCertIDL(aCertHash, certID);
        }
    
    RDbView view;
    TBuf<KMaxSQLLength> SQLStatement;

    _LIT( KSQLQuery2, "SELECT Forgiven, OutOfDate FROM TrustedSites WHERE CertID = ");
    
    _LIT( KSQLQuery3," AND SiteName ='" );
  
    SQLStatement.Copy( KSQLQuery2 );
    SQLStatement.AppendNum(certID);
    SQLStatement.Append( KSQLQuery3);
    SQLStatement.Append( aSiteName );
    SQLStatement.Append(_L("'"));
    
    TPtrC sqlStat( SQLStatement.PtrZ() );
    TInt err = view.Prepare( iDatabase, TDbQuery( sqlStat ) );
     
    err = view.EvaluateAll();

    CleanupClosePushL( view ); 
     
    //same sitename and same cert exists
    if (view.FirstL())
        {
        //already exist, no need to add
        CleanupStack::PopAndDestroy(); //view
        return;
        }
   
    CleanupStack::PopAndDestroy(); //view
   
    // no match found
   
    _LIT( KSQLQuery, 
    "SELECT Forgiven,OutOfDate, CertID FROM TrustedSites WHERE CertId = -1 AND SiteName = '" );

    SQLStatement.Zero(); 
    SQLStatement.Copy( KSQLQuery );

    SQLStatement.Append( aSiteName );
    SQLStatement.Append(_L("'"));

    sqlStat.Set( SQLStatement.PtrZ() );
    err = view.Prepare( iDatabase, TDbQuery( sqlStat ) );
    
    err = view.EvaluateAll();

    CleanupClosePushL( view );
    
    if ( view.CountL() )
        {
        //record is there but certid is empty, add the certid into the record
        //Update the certid 
        _LIT( KSQLOverWrite1, "UPDATE TrustedSites SET CertID = ");
        _LIT( KSQLOverWrite2, " WHERE SiteName = '");
   
        SQLStatement.Zero();
        SQLStatement.Copy(KSQLOverWrite1);
        SQLStatement.AppendNum( certID );
        SQLStatement.Append(KSQLOverWrite2);
        SQLStatement.Append(aSiteName);
        SQLStatement.Append(_L("'"));
    
        User::LeaveIfError( iDatabase.Execute(SQLStatement) );
        }
    else
        {
        //retrive the forgiven and outofdate, and make a same record for the certificate
        TInt forgiven = 0;
        TInt OutOfDate = 0;
        TInt ClientUID  = 0;
        _LIT( KSQLQuery, 
         "SELECT Forgiven,OutOfDate,CertID,ClientUID FROM TrustedSites WHERE SiteName = '" );

        SQLStatement.Zero(); 
        SQLStatement.Copy( KSQLQuery );

        SQLStatement.Append( aSiteName );
        SQLStatement.Append(_L("'"));

        sqlStat.Set( SQLStatement.PtrZ() );
        err = view.Prepare( iDatabase, TDbQuery( sqlStat ) );
      
        err = view.EvaluateAll();

        CleanupClosePushL( view );
      
        // looking for a match
        if ( view.FirstL() ) // Should find only one certificate
            {
            TRAPD(error, view.GetL());
            error = error;
            //add a new record
            forgiven = view.ColInt( 1 ); 
            OutOfDate = view.ColInt( 2 );
            ClientUID = view.ColInt( 4 );

            //set the new record with new certid and same forgiven and outofdate settings
            _LIT( KSQLInsert,
            "INSERT INTO TrustedSites (CertID,SiteName,Forgiven,OutOfDate,ClientUID) VALUES(" );
            SQLStatement.Zero();
            SQLStatement.Copy(KSQLInsert);
            SQLStatement.AppendNum(certID);
            SQLStatement.Append(_L(",'"));
      
            SQLStatement.Append( aSiteName );
            SQLStatement.Append(_L("',"));
            SQLStatement.AppendNum(forgiven);
            SQLStatement.Append(_L(","));
            SQLStatement.AppendNum(OutOfDate);  
            SQLStatement.Append(_L(","));
            SQLStatement.AppendNum(ClientUID);
            SQLStatement.Append(_L(")"));
            
            User::LeaveIfError( iDatabase.Execute( SQLStatement ) );  
            } 
        CleanupStack::PopAndDestroy();    
        }
    CleanupStack::PopAndDestroy(); //view 
    }
//EOF 

