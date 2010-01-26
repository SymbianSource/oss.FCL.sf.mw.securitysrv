/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Implementation of CGbaServer
*
*/


#include <e32svr.h>
#include <e32math.h>
#include <s32file.h>
#include "GbaServer.h"
#include "GbaSession.h"
#include "GBALogger.h"

_LIT(KGBAStoreStandardDrive, "C:");
_LIT(KGbaIniFileName, "GbaConf.ini");

const TInt KPrivateFilepathLength = 20; 

// -----------------------------------------------------------------------------
// CGbaServer::CGbaServer()
// -----------------------------------------------------------------------------
//
CGbaServer::CGbaServer():CPolicyServer( EPriorityStandard, GBAPolicy, ESharableSessions )
    {
    // Implementation not required
    }


// -----------------------------------------------------------------------------
// CGbaServer::~CGbaServer()
// -----------------------------------------------------------------------------
//
CGbaServer::~CGbaServer()
    {
    GBA_TRACE_DEBUG(("CGbaServer::~CGbaServer"));
    }


// -----------------------------------------------------------------------------
// CGbaServer::NewL()
// -----------------------------------------------------------------------------
//
CGbaServer* CGbaServer::NewL()
    {
    CGbaServer* self = CGbaServer::NewLC();
    CleanupStack::Pop( self ) ;
    return self;
    }


// -----------------------------------------------------------------------------
// CGbaServer::NewLC()
// -----------------------------------------------------------------------------
//
CGbaServer* CGbaServer::NewLC()
    {
    CGbaServer* self = new (ELeave) CGbaServer();
    CleanupStack::PushL( self ) ;
    self->ConstructL() ;
    return self;
    }


// -----------------------------------------------------------------------------
// CGbaServer::ConstructL()
// -----------------------------------------------------------------------------
//
void CGbaServer::ConstructL()
    { 
    StartL( KGbaServerName ) ;
    iShutdown.ConstructL();
    if(!iShutdown.IsActive())
        {
        iShutdown.Start();
        }
    }


// -----------------------------------------------------------------------------
// CGbaServer::ConstructL()
// -----------------------------------------------------------------------------
//
CSession2* CGbaServer::NewSessionL(const TVersion& aVersion, const RMessage2& /*aMessage*/) const
    {
    GBA_TRACE_DEBUG(("Creating NewSessionL"));
    // check we're the right version
    if (!User::QueryVersionSupported(TVersion(KGbaServMajorVersionNumber,
                                              KGbaServMinorVersionNumber,
                                              KGbaServBuildVersionNumber),
                                              aVersion))
        {
        GBA_TRACE_DEBUG(("Version isn't supported"));
        User::Leave(KErrNotSupported);
        }
    CGbaServerSession* session = CGbaServerSession::NewL();
    return session;
    
    }


// -----------------------------------------------------------------------------
// CGbaServer::IncrementSessions()
// -----------------------------------------------------------------------------
//
void CGbaServer::IncrementSessions()
    {
    GBA_TRACE_DEBUG(("CGbaServer::IncrementSessions"));
    iSessionCount++;
    iShutdown.Cancel();
    }


// -----------------------------------------------------------------------------
// CGbaServer::DecrementSessions()
// -----------------------------------------------------------------------------
//
void CGbaServer::DecrementSessions()
    {
    GBA_TRACE_DEBUG(("CGbaServer::DecrementSessions"));
    if ( --iSessionCount == 0 )
        {
        if(!iShutdown.IsActive())
            {
            iShutdown.Start();
            }
        }
    GBA_TRACE_DEBUG(("CGbaServer::DecrementSessions END"));
    }


// -----------------------------------------------------------------------------
// CGbaServer::ReadOptionL()
// -----------------------------------------------------------------------------
//
TBool CGbaServer::ReadOptionL(const TUid& aOptionID, TDes8& aValue) const
    {
    GBA_TRACE_DEBUG(("ReadOptionL"));
    TInt pushCount = 0;
    TInt result = EFalse;
    RFs fs;
       
    User::LeaveIfError( fs.Connect() );
    CleanupClosePushL( fs );
    pushCount++; 
    TFindFile folder( fs );
  
    TFileName fullPath;
    MakePrivateFilenameL(fs, KGbaIniFileName, fullPath);
  
    TInt err = folder.FindByDir( fullPath, KNullDesC);
  
    if (  err != KErrNone )
        {
        CleanupStack::PopAndDestroy( pushCount );
        return result;
        }
    else
       {
       CDictionaryFileStore* pStore = CDictionaryFileStore::OpenLC( fs, fullPath , KGbaIniUid );   
       pushCount++;

       if ( pStore->IsPresentL( aOptionID ) )
          {          
          RDictionaryReadStream drs;
          CleanupClosePushL( drs );
          drs.OpenL(*pStore,aOptionID); 

          TInt length = drs.ReadInt32L();
          drs.ReadL(aValue,length); 
          CleanupStack::PopAndDestroy( &drs );
          result = ETrue;
          }
       }

     CleanupStack::PopAndDestroy( pushCount );
     return result;
    }



// -----------------------------------------------------------------------------
// CGbaServer::WriteOptionL()
// -----------------------------------------------------------------------------
//
void CGbaServer::WriteOptionL(const TUid& aOptionID, const TDesC8& aValue) const
    {
    GBA_TRACE_DEBUG(("WriteOptionL"));
    TInt pushCount = 0;
    RFs fs;
    User::LeaveIfError(fs.Connect());
    CleanupClosePushL( fs );
    pushCount++;
    TFindFile folder( fs );
     
    TFileName fullPath;
    MakePrivateFilenameL(fs, KGbaIniFileName, fullPath);
    EnsurePathL(fs, fullPath );
    
    GBA_TRACE_DEBUG(fullPath); 
     
    TInt err = folder.FindByDir( fullPath, KNullDesC);
  
    if (  err == KErrNotFound || err == KErrNone )  
       {
       CDictionaryFileStore* pStore = CDictionaryFileStore::OpenLC( fs, fullPath, KGbaIniUid );   
       pushCount++;       

       RDictionaryWriteStream wrs;
       CleanupClosePushL( wrs );
       wrs.AssignL(*pStore,aOptionID);
       
       wrs.WriteInt32L(aValue.Length());
       wrs.WriteL(aValue);
       wrs.CommitL();
       
       pStore->CommitL();  
       CleanupStack::PopAndDestroy( &wrs );
       CleanupStack::PopAndDestroy( pushCount );     
       GBA_TRACE_DEBUG(aValue);
       }
    else
        {
        CleanupStack::PopAndDestroy( pushCount );    
        User::LeaveIfError( err );
        }
    }



// ---------------------------------------------------------------------------
// CGbaServer::MakePrivateFilenameL()
// ---------------------------------------------------------------------------
//
void CGbaServer::MakePrivateFilenameL(RFs& aFs, const TDesC& aLeafName, 
                                     TDes& aNameOut) const
    {
    aNameOut.Copy(KGBAStoreStandardDrive);
    // Get private path
    TBuf<KPrivateFilepathLength> privatePath;
    aFs.PrivatePath(privatePath);
    aNameOut.Append(privatePath);
    aNameOut.Append(aLeafName);
    }
    
      
// ---------------------------------------------------------------------------
// CGbaServer::EnsurePathL()
// ---------------------------------------------------------------------------
//      
void CGbaServer::EnsurePathL( RFs& aFs, const TDesC& aFile ) const
    {
    TInt err = aFs.MkDirAll(aFile);
    if (err != KErrNone && err != KErrAlreadyExists)
        {
        User::Leave(err);
        }
    } 
    
    
// -----------------------------------------------------------------------------
// CShutdown::CShutdown
// -----------------------------------------------------------------------------
//
inline CShutdown::CShutdown()
  :CTimer(-1)
    {
    CActiveScheduler::Add(this);
    }


// -----------------------------------------------------------------------------
// CShutdown::ConstructL
// -----------------------------------------------------------------------------  
//
inline void CShutdown::ConstructL()
    {
    CTimer::ConstructL();
    }


// -----------------------------------------------------------------------------
// CShutdown::Start
// -----------------------------------------------------------------------------  
//
inline void CShutdown::Start()
    {
    GBA_TRACE_DEBUG(("GbaServer: starting shutdown timeout"));
    After( EGbaServerShutdownDelay );
    }


// -----------------------------------------------------------------------------
// CShutdown::RunL
// -----------------------------------------------------------------------------  
//
void CShutdown::RunL()
    {
    GBA_TRACE_DEBUG(("GbaServer: server timeout ... closing"));
    CActiveScheduler::Stop();
    }

// ======== LOCAL FUNCTIONS ========

// --------------------------------------------
// PanicClient()
// -------------------------------------------- 
//
void PanicClient( const RMessage2& aMessage, TInt aPanic )
    {
    aMessage.Panic( KGbaServer, aPanic );
    }


// --------------------------------------------
// PanicServer()
// -------------------------------------------- 
//
void PanicServer( TInt aPanic )
    {
    User::Panic( KGbaServer, aPanic );
    }


// --------------------------------------------
// RunServerL()
// -------------------------------------------- 
//
static void RunServerL()
    {
    // Construct active scheduler
    CActiveScheduler* activeScheduler = new (ELeave) CActiveScheduler;
    CleanupStack::PushL(activeScheduler) ;
    CActiveScheduler::Install(activeScheduler);
    // Construct our server
    CGbaServer* server = CGbaServer::NewLC();    
    RProcess::Rendezvous(KErrNone);
    // Start handling requests
    CActiveScheduler::Start();
    CleanupStack::PopAndDestroy( server );
    CleanupStack::PopAndDestroy( activeScheduler );
    }


// --------------------------------------------
// E32Main:()
// -------------------------------------------- 
//
TInt E32Main()
    {
 __UHEAP_MARK;
    CTrapCleanup* cleanup = CTrapCleanup::New();
    
    TInt err = KErrNoMemory;
    
    if ( cleanup )
        { 
        TRAP(err, RunServerL());
        delete cleanup;
        }
__UHEAP_MARKEND;
    return err;
    }
    
//EOF

