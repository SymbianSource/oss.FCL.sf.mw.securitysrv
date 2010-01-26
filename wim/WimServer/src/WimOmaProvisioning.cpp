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
* Description:  WIM OMA Provisioning functionality. Reads OMA Provisioning
*               documents from Smart Card.
*
*/


// INCLUDE FILES
#include    "WimOmaProvisioning.h"
#include    "Wimi.h"                // WIMI
#include    "WimClsv.h"
#include    "WimUtilityFuncs.h"
#include    "WimResponse.h"         // Callback response message
#include    "WimTrace.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimOmaProvisioning::CWimOmaProvisioning
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimOmaProvisioning::CWimOmaProvisioning()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimOmaProvisioning::CWimOmaProvisioning | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimOmaProvisioning::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimOmaProvisioning::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimOmaProvisioning::ConstructL | Begin"));
    iWimUtilityFuncs = CWimUtilityFuncs::NewL();
    }

// -----------------------------------------------------------------------------
// CWimOmaProvisioning::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimOmaProvisioning* CWimOmaProvisioning::NewL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimOmaProvisioning::NewL | Begin"));
    CWimOmaProvisioning* self = new( ELeave ) CWimOmaProvisioning;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }
  
// Destructor
CWimOmaProvisioning::~CWimOmaProvisioning()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimOmaProvisioning::~CWimOmaProvisioning | Begin"));
    delete iWimUtilityFuncs;
    free_WIMI_Ref_t( iWimRef );
    }

// -----------------------------------------------------------------------------
// CWimOmaProvisioning::OmaFileSizeL
// Get OMA Provisioning file size
// -----------------------------------------------------------------------------
//
void CWimOmaProvisioning::OmaFileSizeL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimOmaProvisioning::OmaFileSizeL | Begin"));

    InitializePathsAndSizes();

    TOmaProv omaProvisioning;
    omaProvisioning.iSize = 0;
    TPckgBuf<TOmaProv> omaFilePckg( omaProvisioning );
    aMessage.ReadL( 0, omaFilePckg );

    TInt fileSize = 0;
    TInt err = KErrNone;

    switch ( omaFilePckg().iOmaType )
        {
        case EBootStrap:
            {
            fileSize = iBootstrapFileSize;
            break;
            }
        
        case EConfig1:
            {
            fileSize = iConfig1FileSize;
            break;
            }
        
        case EConfig2:
            {
            fileSize = iConfig2FileSize;
            break;
            }
        
        default:
            {
            err = KErrArgument;
            break;
            }
        }

    _WIMTRACE3(_L("WIM | WIMServer | CWimOmaProvisioning::OmaFileSizeL | FileType=%d, FileSize=%d"), omaFilePckg().iOmaType, fileSize);

    if ( err == KErrNone )
        {
        omaFilePckg().iSize = ( TInt )fileSize;
        aMessage.WriteL( 0, omaFilePckg );
        }
    aMessage.Complete( err );
    }

// -----------------------------------------------------------------------------
// CWimOmaProvisioning::OmaFileContentL
// Write content of OMA provisioning file to client memory
// -----------------------------------------------------------------------------
//
void CWimOmaProvisioning::OmaFileContentL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimOmaProvisioning::OmaFileContentL | Begin"));

    TUint16 callStatus = KErrNone;
    WIMI_BinData_t pt_path;
    WIMI_BinData_t fileContent;
    TUint16 fileSize = 0;
    TPtrC8 path;

    // Do initialization
    InitializePathsAndSizes();

    TOmaProv omaProvisioning;
    omaProvisioning.iSize = 0;
    TPckgBuf<TOmaProv> omaFilePckg( omaProvisioning );
    aMessage.ReadL( 0, omaFilePckg );

    if ( !iWimRef )
        {
        callStatus = WIMI_ERR_KeyNotFound; // Mapped to KErrNotFound
        }

    if ( callStatus == KErrNone )
        {
        switch ( omaFilePckg().iOmaType )
            {
            case EBootStrap:
                {
                path.Set( iPathBootstrap.Ptr() );
                fileSize = ( TUint16 )iBootstrapFileSize;
                break;
                }
        
            case EConfig1:
                {
                path.Set( iPathConfig1.Ptr() );
                fileSize = ( TUint16 )iConfig1FileSize;
                break;
                }
        
            case EConfig2:
                {
                path.Set( iPathConfig2.Ptr() );
                fileSize = ( TUint16 )iConfig2FileSize;
                break;
                }
        
            default:
                {
                callStatus = WIMI_ERR_BadParameters;
                break;
                }
            }
        }

    if ( callStatus == KErrNone )
        {
        if ( path.Length() == 0 )
            {
            callStatus = WIMI_ERR_KeyNotFound; // Mapped to KErrNotFound
            }
        }

    _WIMTRACE3(_L("WIM | WIMServer | CWimOmaProvisioning::OmaFileContentL | FileType=%d, FileSize=%d"), omaFilePckg().iOmaType, fileSize);

    if ( callStatus == KErrNone )
        {
        pt_path.ui_buf_length = ( TUint16 )path.Length();
        pt_path.pb_buf = ( TUint8* )path.Ptr();

        WIMI_TransactId_t trId = ( void* )EOMAProvContentSync;

        fileContent.pb_buf = NULL;
        fileContent.ui_buf_length = 0;
        // Get OMA Provisioning file from card
        callStatus = WIMI_GetOMAFileSync( trId,
                                          iWimRef,
                                          &pt_path,
                                          0,
                                          &fileContent );
                                          
        if ( callStatus == KErrNone )
            {
            //Use filesize here, should be equal or smaller than fileContent.ui_length
            TPtrC8 omaFilePtr( fileContent.pb_buf, fileSize );
            TInt err = aMessage.Write( 1, omaFilePtr );
            if( err )
                {
                WSL_OS_Free( fileContent.pb_buf );
                User::Leave( err );
                }
            aMessage.Complete( KErrNone );
            }
        else
            {
            aMessage.Complete( CWimUtilityFuncs::MapWIMError( callStatus ) );	
            }    
        
        WSL_OS_Free( fileContent.pb_buf ); // Deallocate file content buffer                                     
        }
      
    _WIMTRACE(_L("WIM | WIMServer | CWimOmaProvisioning::OmaFileContentL | End"));
    }

// -----------------------------------------------------------------------------
// CWimOmaProvisioning::InitializePathsAndSizes
// Initializes the path and size data members
// -----------------------------------------------------------------------------
//
void CWimOmaProvisioning::InitializePathsAndSizes()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimOmaProvisioning::InitializePathsAndSizesL | Begin"));

    // Do initialization only if initialization is not done
    if ( !iFileInfosInitialized )
        {
        TUint16 callStatus = KErrNone;

        TBuf8<KWimProvFileOidLength> bootstrapOid;
        TBuf8<KWimProvFileOidLength> config1Oid;
        TBuf8<KWimProvFileOidLength> config2Oid;

        bootstrapOid.Copy( KWimProvBootstrapFileOID, KWimProvFileOidLength );
        config1Oid.Copy( KWimProvConfig1FileOID, KWimProvFileOidLength );
        config2Oid.Copy( KWimProvConfig2FileOID, KWimProvFileOidLength );

        TBuf8<KWimProvFileOidLengthTaged> bootstrapOidTaged;
        TBuf8<KWimProvFileOidLengthTaged> config1OidTaged;
        TBuf8<KWimProvFileOidLengthTaged> config2OidTaged;

        bootstrapOidTaged.Copy( KWimProvBootstrapFileOIDTaged,
                                KWimProvFileOidLengthTaged );
        config1OidTaged.Copy( KWimProvConfig1FileOIDTaged,
                              KWimProvFileOidLengthTaged );
        config2OidTaged.Copy( KWimProvConfig2FileOIDTaged,
                              KWimProvFileOidLengthTaged );
        
        WIMI_BinData_t path;
        path.pb_buf = NULL;
        path.ui_buf_length = 0;

        WIMI_File_Info* fileInfos = NULL;

        TUint16 fileCount = 0;

        WIMI_BinData_t fileOid;
        fileOid.pb_buf = NULL;
        fileOid.ui_buf_length = 0;

        // Get WIM reference, reader is always 0
        iWimRef = WIMI_GetWIMRef( 0 );

        if ( !iWimRef )
            {
            callStatus = WIMI_ERR_BadReference;
            }
    
        if ( callStatus == KErrNone )
            {

            // Get OMA file info from Wimlib
            callStatus = WIMI_OMAFileInfo( iWimRef,
                                           &fileInfos,
                                           &fileCount );
            }

        if ( callStatus == KErrNone )
            {
            TBuf8<KWimOidLengthTaged> oidBuf; // Buffer for OID

            // Loop all file infos and safe path to each file
            for ( TInt i = 0; i < fileCount; i++ )
                {
                // Get file OID from infos
                fileOid.pb_buf = fileInfos[i].t_oid.pb_buf;
                fileOid.ui_buf_length = fileInfos[i].t_oid.ui_buf_length;
                oidBuf.Copy( fileOid.pb_buf, fileOid.ui_buf_length );

                // File path
                path.pb_buf = fileInfos[i].b_path.pb_buf;
                path.ui_buf_length = fileInfos[i].b_path.ui_buf_length;

                // Compare OIDs
                if ( !oidBuf.Compare( bootstrapOid ) 
                    || !oidBuf.Compare( bootstrapOidTaged ) ) // Bootstrap File
                    {
                    // Copy file path to member
                    iPathBootstrap.Copy( path.pb_buf, path.ui_buf_length );
                    // Set file size
                    iBootstrapFileSize = fileInfos[i].ui_file_size;

                    // Check if path is given but size not. If path exists and
                    // size == 0 then parse size from fetched file
                    if ( iPathBootstrap.Length() > 0 &&
                         iBootstrapFileSize == 0 )
                        {
                        ParseFileSize( EBootStrap );
                        }
                    }
                else if ( !oidBuf.Compare( config1Oid ) 
                    || !oidBuf.Compare( config1OidTaged ) ) // Config1 File
                    {
                    // Copy file path to member
                    iPathConfig1.Copy( path.pb_buf, path.ui_buf_length );
                    // Set file size
                    iConfig1FileSize = fileInfos[i].ui_file_size;

                    // Check if path is given but size not. If path exists and
                    // size == 0 then parse size from fetched file
                    if ( iPathConfig1.Length() > 0 && iConfig1FileSize == 0 )
                        {
                        ParseFileSize( EConfig1 );
                        }
                    }
                else if ( !oidBuf.Compare( config2Oid )
                    || !oidBuf.Compare( config2OidTaged ) ) // Config2 File
                    {
                    // Copy file path to member
                    iPathConfig2.Copy( path.pb_buf, path.ui_buf_length );
                    // Set file size
                    iConfig2FileSize = fileInfos[i].ui_file_size;

                    // Check if path is given but size not. If path exists and
                    // size == 0 then parse size from fetched file
                    if ( iPathConfig2.Length() > 0 && iConfig2FileSize == 0 )
                        {
                        ParseFileSize( EConfig2 );
                        }
                    }
                }
            }
        iFileInfosInitialized = ETrue;
        }
    }

// -----------------------------------------------------------------------------
// CWimOmaProvisioning::ParseFileSize
// Get content of file and parse size of it. For WIM size is mandatory in 
// DODF-PROV file but for SIM implementation it is not mandatory.
// -----------------------------------------------------------------------------
//
void CWimOmaProvisioning::ParseFileSize( TOMAType aOmaType )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimOmaProvisioning::ParseFileSize | Begin"));
    WIMI_BinData_t path;
    WIMI_BinData_t fileContent;
    TInt fileSize;
    TPtrC8 pathPtr;

    // Set path to file
    switch ( aOmaType )
        {
        case EBootStrap:
            {
            pathPtr.Set( iPathBootstrap.Ptr() );
            break;
            }
        
        case EConfig1:
            {
            pathPtr.Set( iPathConfig1.Ptr() );
            break;
            }
        
        case EConfig2:
            {
            pathPtr.Set( iPathConfig2.Ptr() );
            break;
            }
        
        default:
            {
            return;
            }
        }

    path.ui_buf_length = ( TUint16 )pathPtr.Length();
    path.pb_buf = ( TUint8* )pathPtr.Ptr();

    
    WIMI_TransactId_t trId = ( void* )EOMAProvContentSync;

    TPtr8 contentPtr = TPtr8( NULL, 0, 0 );

    fileContent.pb_buf = NULL;
    fileContent.ui_buf_length = 0;
    // Get OMA Provisioning file from card
    WIMI_STAT callStatus = WIMI_GetOMAFileSync( trId,
                                                iWimRef,
                                                &path,
                                                0,
                                                &fileContent );

    if ( callStatus != KErrNone )
        {
        fileSize = 0;
        }
    else
        {
        contentPtr.Set( fileContent.pb_buf,
                        fileContent.ui_buf_length, 
                        fileContent.ui_buf_length );

        fileSize = fileContent.ui_buf_length;
        }

    // Set file size
    switch ( aOmaType )
        {
        case EBootStrap:
            {
            iBootstrapFileSize = fileSize;
            _WIMTRACE2( _L("WIM | WIMServer | CWimOmaProvisioning::ParseFileSize | Bootstrap size=%d"),
                        fileSize );
            break;
            }
        
        case EConfig1:
            {
            iConfig1FileSize = fileSize;
            _WIMTRACE2( _L("WIM | WIMServer | CWimOmaProvisioning::ParseFileSize | Config1 size=%d"),
                        fileSize );
            break;
            }
        
        case EConfig2:
            {
            iConfig2FileSize = fileSize;
            _WIMTRACE2( _L("WIM | WIMServer | CWimOmaProvisioning::ParseFileSize | Config2 size=%d"),
                        fileSize );
            break;
            }
        
        default:
            {
            break;
            }
        }

    WSL_OS_Free( fileContent.pb_buf ); // Deallocate file content buffer
    }

// End of File
