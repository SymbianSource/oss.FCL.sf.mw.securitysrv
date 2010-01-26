/*
* Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:
*
*/


// INCLUDE FILES
#include    "WimJavaProvisioning.h"
#include    "Wimi.h"                // WIMI
#include    "WimClsv.h"
#include    "WimUtilityFuncs.h"
#include    "WimResponse.h"         // Callback response message
#include    "WimTrace.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimJavaProvisioning::CWimJavaProvisioning
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimJavaProvisioning::CWimJavaProvisioning()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimJavaProvisioning::CWimJavaProvisioning | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimJavaProvisioning::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimJavaProvisioning::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimJavaProvisioning::ConstructL | Begin"));
    iWimUtilityFuncs = CWimUtilityFuncs::NewL();
    }

// -----------------------------------------------------------------------------
// CWimJavaProvisioning::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimJavaProvisioning* CWimJavaProvisioning::NewL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimJavaProvisioning::NewL | Begin"));
    CWimJavaProvisioning* self = new( ELeave ) CWimJavaProvisioning;
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }
  
// Destructor
CWimJavaProvisioning::~CWimJavaProvisioning()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimJavaProvisioning::~CWimJavaProvisioning | Begin"));
    delete iWimUtilityFuncs;
    free_WIMI_Ref_t( iWimRef );
    }

// -----------------------------------------------------------------------------
// CWimJavaProvisioning::ACIFFileSizeL
// Get Java Provisioning file size
// -----------------------------------------------------------------------------
//
void CWimJavaProvisioning::ACIFFileSizeL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimJavaProvisioning::ACIFFileSizeL | Begin"));

    InitializePathsAndSizes();

    TJavaProv javaProvisioning;
    javaProvisioning.iSize = 0;
    TPckgBuf<TJavaProv> javaFilePckg( javaProvisioning );

    TInt ACIFfileSize = iACIFFileSize;
    
    
    _WIMTRACE2(_L("WIM | WIMServer | CWimJavaProvisioning::ACIFFileSizeL | ACIFFileSize=%d"), ACIFfileSize);

    javaFilePckg().iSize = ( TInt )ACIFfileSize;
    aMessage.WriteL( 0, javaFilePckg );
     
    aMessage.Complete( KErrNone );
    }



// -----------------------------------------------------------------------------
// CWimJavaProvisioning::ACFFileSizeL
// Get ACF file size
// -----------------------------------------------------------------------------
//
void CWimJavaProvisioning::ACFFileSizeL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimJavaProvisioning::ACFFileSizeL | Begin"));

    //InitializePathsAndSizes();

    TJavaProv javaProvisioning;
    javaProvisioning.iSize = 0;
    TPckgBuf<TJavaProv> javaFilePckg( javaProvisioning );

    aMessage.ReadL( 2, iPathACF );
    
    InitializeSize(); //iACFFileSize is updated;
    
    
    _WIMTRACE2(_L("WIM | WIMServer | CWimJavaProvisioning::ACFFileSizeL | iACFFileSize=%d"), iACFFileSize);

    javaFilePckg().iSize = ( TInt )iACFFileSize;
    aMessage.WriteL( 0, javaFilePckg );
     
    aMessage.Complete( KErrNone );
    }
    
// -----------------------------------------------------------------------------
// CWimJavaProvisioning::ACIFFileContentL
// Write content of ACIF file to client memory
// -----------------------------------------------------------------------------
//
void CWimJavaProvisioning::ACIFFileContentL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimJavaProvisioning::ACIFFileContentL | Begin"));

    TUint16 callStatus = KErrNone;
    WIMI_BinData_t pt_path;
    //TPtrC8 ACIFpathPtr;
    WIMI_BinData_t fileContent;
    
    // Do initialization
    InitializePathsAndSizes();

    if ( !iWimRef || ( iPathACIF.Length() == 0 ) )
        {
        callStatus = WIMI_ERR_KeyNotFound; // Mapped to KErrNotFound
        }

    _WIMTRACE2(_L("WIM | WIMServer | CWimJavaProvisioning::ACIFFileContentL | iACFFileSize=%d"), iACFFileSize);

    if ( callStatus == KErrNone )
        {
        pt_path.ui_buf_length = ( TUint16 )iPathACIF.Length();
        pt_path.pb_buf = ( TUint8* )iPathACIF.Ptr();

        
        fileContent.pb_buf = NULL;
        fileContent.ui_buf_length = 0;
    
       
        //Get ACIF file
        WIMI_STAT callStatus = WIMI_GetJAVAFile(iWimRef,
                                                &pt_path,
                                                &fileContent );
        }

    if ( callStatus == KErrNone )
        {
        TJavaProv javaProvisioning;
        javaProvisioning.iSize = fileContent.ui_buf_length;
        javaProvisioning.iJavaData = NULL; //The value will be reset in Client side
        TPckgBuf<TJavaProv> javaFilePckg( javaProvisioning );
       
        TPtrC8 ACIFFileContentPtr( fileContent.pb_buf, fileContent.ui_buf_length );
        TInt err = aMessage.Write( 1, ACIFFileContentPtr );
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
    _WIMTRACE(_L("WIM | WIMServer | CWimJavaProvisioning::ACIFFileContentL | End"));
    }


// -----------------------------------------------------------------------------
// CWimJavaProvisioning::ACFFileContentL
// Write content of ACIF file to client memory
// -----------------------------------------------------------------------------
//
void CWimJavaProvisioning::ACFFileContentL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimJavaProvisioning::ACFFileContentL | Begin"));

    TUint16 callStatus = KErrNone;
    WIMI_BinData_t pt_path;
    WIMI_BinData_t fileContent;
    
    aMessage.ReadL( 2, iPathACF );
    
    if ( !iWimRef || ( iPathACF.Length() == 0 ) )
        {
        callStatus = WIMI_ERR_KeyNotFound; // Mapped to KErrNotFound
        }

    if ( callStatus == KErrNone )
        {
        pt_path.ui_buf_length = ( TUint16 )iPathACF.Length();
        pt_path.pb_buf = ( TUint8* )iPathACF.Ptr();

        fileContent.pb_buf = NULL;
        fileContent.ui_buf_length = 0;
    
        //Get ACIF file
        WIMI_STAT callStatus = WIMI_GetJAVAFile(iWimRef,
                                                &pt_path,
                                                &fileContent );
        }

    if ( callStatus == KErrNone )
        {
        TJavaProv javaProvisioning;
        javaProvisioning.iSize = fileContent.ui_buf_length;
        javaProvisioning.iJavaData = NULL; //The value will be reset in Client side
        TPckgBuf<TJavaProv> javaFilePckg( javaProvisioning );
       
        TPtrC8 ACFFileContentPtr( fileContent.pb_buf, fileContent.ui_buf_length );
        TInt err = aMessage.Write( 1, ACFFileContentPtr );
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
    _WIMTRACE(_L("WIM | WIMServer | CWimJavaProvisioning::ACFFileContentL | End"));
    }


// -----------------------------------------------------------------------------
// CWimJavaProvisioning::LabelAndPath
// -----------------------------------------------------------------------------
//
void CWimJavaProvisioning::LabelAndPath( const RMessage2& aMessage )
   {
   _WIMTRACE(_L("WIM | WIMServer | CWimJavaProvisioning::LabelAndPath | Begin"));

    TUint16 callStatus = KErrNone;
    
    WIMI_BinData_t path;
    WIMI_BinData_t label;
    
    if( iWimRef )
        {
        free_WIMI_Ref_t( iWimRef );
        iWimRef = NULL;
        }
    iWimRef = WIMI_GetWIMRef( 0 );

    if ( !iWimRef )
        {
        callStatus = WIMI_ERR_KeyNotFound; // Mapped to KErrNotFound
        }

    if ( callStatus == KErrNone )
        {
        path.pb_buf = NULL;
        label.pb_buf = NULL;
        
        path.ui_buf_length = 0;
        label.ui_buf_length = 0;
    
        //Get Path and label
        callStatus = WIMI_GetLabelAndPath(iWimRef,
                                          &path,
                                          &label );
        }

    if ( callStatus == KErrNone )
        {
        TPtrC8 PathPtr( path.pb_buf, path.ui_buf_length );
        TPtrC8 LabelPtr( label.pb_buf, label.ui_buf_length );
        
        TInt err = KErrNone; 
        err = aMessage.Write( 0, LabelPtr );
        if ( err != KErrNone )
            {
            aMessage.Complete( err );
            return;	
            }
        	  
        err = aMessage.Write( 1, PathPtr );
        if ( err != KErrNone )
            {
            aMessage.Complete( err );
            return;	
            }

        aMessage.Complete( KErrNone );
        }
    else
        {
        aMessage.Complete( CWimUtilityFuncs::MapWIMError( callStatus ) );	
        }    
          
    _WIMTRACE(_L("WIM | WIMServer | CWimJavaProvisioning::OmaFileContentL | End"));	
   }
    
    
// -----------------------------------------------------------------------------
// CWimJavaProvisioning::InitializePathsAndSizes
// Initializes the path and size data members
// -----------------------------------------------------------------------------
//
void CWimJavaProvisioning::InitializePathsAndSizes()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimJavaProvisioning::InitializePathsAndSizesL | Begin"));

    // Do initialization only if initialization is not done
    if ( !iFileInfosInitialized )
        {
        TUint16 callStatus = KErrNone;

        TBuf8<KWimProvACIFFileOidLength> ACIFOid;
        
        ACIFOid.Copy( KWimProvACIFFileOID, KWimProvACIFFileOidLength );

        TBuf8<KWimProvACIFFileOidLengthTaged> ACIFOidTaged;

        ACIFOidTaged.Copy( KWimProvACIFFileOIDTaged,
                                KWimProvACIFFileOidLengthTaged );
        
        WIMI_BinData_t path;
        path.pb_buf = NULL;
        path.ui_buf_length = 0;

        WIMI_File_Info* fileInfos = NULL;

        TUint16 fileCount = 0;

        WIMI_BinData_t fileOid;
        fileOid.pb_buf = NULL;
        fileOid.ui_buf_length = 0;

        // Get WIM reference, reader is always 0
        if ( iWimRef )
            {
            free_WIMI_Ref_t( iWimRef );
            iWimRef = NULL;
            }
        iWimRef = WIMI_GetWIMRef( 0 );

        if ( !iWimRef )
            {
            callStatus = WIMI_ERR_BadReference;
            }
    
        if ( callStatus == KErrNone )
            {

            // Get Java file info from Wimlib
            callStatus = WIMI_JAVAFileInfo( iWimRef,
                                           &fileInfos,
                                           &fileCount );
            }

        if ( callStatus == KErrNone )
            {
            TBuf8<KWimJavaOidLengthTaged> oidBuf; // Buffer for OID

            // Loop all file infos and safe path to each file
            for ( TInt i = 0; i < fileCount; i++ )
                {
                // Get file OID from infos
                fileOid.pb_buf = fileInfos[i].t_oid.pb_buf;
                fileOid.ui_buf_length = fileInfos[i].t_oid.ui_buf_length;
                oidBuf.FillZ();
                oidBuf.Copy( fileOid.pb_buf, fileOid.ui_buf_length );

                // File path
                path.pb_buf = fileInfos[i].b_path.pb_buf;
                path.ui_buf_length = fileInfos[i].b_path.ui_buf_length;

                // Compare OIDs
                if ( !oidBuf.Compare( ACIFOid ) 
                    || !oidBuf.Compare( ACIFOidTaged ) ) // ACIF File
                    {
                    // Copy file path to member
                    iPathACIF.Copy( path.pb_buf, path.ui_buf_length );
                    
                    // Set file size
                    iACIFFileSize = fileInfos[i].ui_file_size;

                    // Check if path is given but size not. If path exists and
                    // size == 0 then parse size from fetched file
                    if ( iPathACIF.Length() > 0 &&
                         iACIFFileSize == 0 )
                        {
                        WIMI_BinData_t pt_path;

                        WIMI_BinData_t fileContent;
                        
                        pt_path.ui_buf_length = ( TUint16 )iPathACIF.Length();
                        pt_path.pb_buf = ( TUint8* )iPathACIF.Ptr();
        
                        fileContent.pb_buf = NULL;
                        fileContent.ui_buf_length = 0;
    
                        //Get ACIF file
                        WIMI_STAT callStatus = WIMI_GetJAVAFile(iWimRef,
                                                                 &pt_path,      
                                                                 &fileContent );
                       iACIFFileSize = fileContent.ui_buf_length;
                       }
                    }
                
                }
            }
        iFileInfosInitialized = ETrue;
        }
    }

// -----------------------------------------------------------------------------
// CWimJavaProvisioning::InitializeSizes
// Initializes the path and size data members
// -----------------------------------------------------------------------------
//
void CWimJavaProvisioning::InitializeSize()
   {
     _WIMTRACE(_L("WIM | WIMServer | CWimJavaProvisioning::InitializeSizesL | Begin"));

    // Do initialization only if initialization is not done

    if ( iPathACF.Length() > 0 )
       {
       WIMI_BinData_t pt_path;

       WIMI_BinData_t fileContent;

       // Get WIM reference, reader is always 0
       if ( iWimRef )
           {
           free_WIMI_Ref_t( iWimRef );
           iWimRef = NULL;
           }
       iWimRef = WIMI_GetWIMRef( 0 );
                        
       pt_path.ui_buf_length = ( TUint16 )iPathACF.Length();
       pt_path.pb_buf = ( TUint8* )iPathACF.Ptr();
        
       fileContent.pb_buf = NULL;
       fileContent.ui_buf_length = 0;
    
       //Get ACIF file
       WIMI_STAT callStatus = WIMI_GetJAVAFile(iWimRef,
                                               &pt_path,      
                                               &fileContent );
                                               
       iACFFileSize = fileContent.ui_buf_length;                
       }
    else
       {
       iACFFileSize = 0;	
       }         

  }
