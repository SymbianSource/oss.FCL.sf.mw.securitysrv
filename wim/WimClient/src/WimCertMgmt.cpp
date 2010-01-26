/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Certificate & key -management class
*
*/


//INCLUDES
#include "WimCertMgmt.h"
#include "WimTrace.h"


// -----------------------------------------------------------------------------
// RWimCertMgmt::RWimCertMgmt()
// Default constructor
// -----------------------------------------------------------------------------
//
RWimCertMgmt::RWimCertMgmt(): iCertLst( NULL, 0, 0 ), iCertInfoLst( NULL, 0, 0 )
    {
    iPckgBufAddCertsAllocated = EFalse;
    iPckgBufCertDetailsAllocated = EFalse;
    iPckgBufRemoveCertAllocated = EFalse;
    iPckgBufGetExtrasAllocated = EFalse;
    iPckgBufKeySignAllocated = EFalse;
    iPckgBufExportPublicKeyAllocated = EFalse;
    iPckgBufOmaProvAllocated = EFalse;
    iPckgBufJavaProvAllocated = EFalse;
    iAuthObjsInfoLstPtr = NULL;
    iAuthIdLstPtr = NULL;
	  iPckgBufOmaProv = NULL;
    iPckgBufJavaProv = NULL;
    }


// -----------------------------------------------------------------------------
// RWimCertMgmt::ClientSessionL()
// Return new RWimCertMgmt object 
// -----------------------------------------------------------------------------
//
RWimCertMgmt* RWimCertMgmt::ClientSessionL()
    {
    RWimCertMgmt* self = new( ELeave ) RWimCertMgmt;
    CleanupStack::PushL( self );
    User::LeaveIfError( self->Connect() );
    CleanupStack::Pop( self ); //self
    _WIMTRACE ( _L( "RWimCertMgmt::ClientSessionL()" ) );
    return self;
    }

// -----------------------------------------------------------------------------
// RWimCertMgmt::~RWimCertMgmt() 
// Destructor, all allocated memory is released.
// -----------------------------------------------------------------------------
//
RWimCertMgmt::~RWimCertMgmt()
    {
    _WIMTRACE ( _L( "RWimCertMgmt::~RWimCertMgmt()" ) );
    iCertLst.Set( NULL, 0, 0 );
    iCertInfoLst.Set( NULL, 0, 0 );
    delete  iAuthObjsInfoLstPtr;
	  delete  iAuthIdLstPtr;
	  delete  iPckgBufOmaProv;
    delete  iPckgBufJavaProv;
    }



/* Certificate management */

// -----------------------------------------------------------------------------
// RWimCertMgmt::CertCount()
// Returns count of certificates
// -----------------------------------------------------------------------------
//
TUint8 RWimCertMgmt::CertCount( TWimEntryType aType )
    {
    _WIMTRACE ( _L( "RWimCertMgmt::CertCount()" ) );

    TPckgBuf<TUint8> pckg( 0 );
    
    TIpcArgs args;
    args.Set( 0, &pckg );
    args.Set( 1, aType );
    
    SendReceiveData( EGetCertCount, args );
    return pckg();
    }

// -----------------------------------------------------------------------------
// RWimCertMgmt::CertRefLst()
// Returns certificate list on the WIM
// -----------------------------------------------------------------------------
//
void  RWimCertMgmt::CertRefLst( TCertificateAddressList aCertAddrLst, 
                                TWimCertInfo* aCertInfoArr, 
                                TUint8 aCount, 
                                TWimEntryType aCertEntryType,
                                TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimCertMgmt::CertRefLst()" ) );
    aStatus = KRequestPending;
    TInt16 size = ( TInt16 ) ( sizeof ( TCertificateAddress ) * aCount );
    iCertLst.Set( ( TUint8* ) aCertAddrLst, size, size );   

    size = ( TInt16 )( sizeof ( TWimCertInfo ) * aCount );
    iCertInfoLst.Set( ( TUint8* ) aCertInfoArr, size, size ); 
    
    TIpcArgs args;
    args.Set( 0, &iCertLst );
    args.Set( 1, &iCertInfoLst );
    args.Set( 2, aCertEntryType );
    
    SendReceiveData( EGetWIMCertLst, args, aStatus );
    }

// -----------------------------------------------------------------------------
// RWimCertMgmt::CertRefLst()
// Returns certificate list on the WIM
// -----------------------------------------------------------------------------
//
TInt RWimCertMgmt::CertRefLst( TCertificateAddressList aCertAddrLst, 
                               TWimCertInfo* aCertInfoArr, 
                               TUint8 aCount, 
                               TWimEntryType aCertEntryType )
    {
    _WIMTRACE ( _L( "RWimCertMgmt::CertRefLst()" ) );
    TInt16 size = ( TInt16 ) ( sizeof ( TCertificateAddress ) * aCount );
    iCertLst.Set( ( TUint8* ) aCertAddrLst, size, size );   

    size = ( TInt16 )( sizeof ( TWimCertInfo ) * aCount );
    iCertInfoLst.Set( ( TUint8* ) aCertInfoArr, size, size ); 
    
    TIpcArgs args;
    args.Set( 0, &iCertLst );
    args.Set( 1, &iCertInfoLst );
    args.Set( 2, aCertEntryType );
    
    
    return SendReceiveData( EGetWIMCertLst, args );
    }

// -----------------------------------------------------------------------------
// RWimCertMgmt::CertDetailsL() 
// Returns certificate details.
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::CertDetailsL( const TCertificateAddress aCertAddr, 
                                 TWimCertDetails& aWimCertDetails,
                                 TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimCertMgmt::CertDetailsL()" ) );
    aStatus = KRequestPending;

    iPckgBufCertDetails  = new( ELeave ) 
        CWimCertPckgBuf<TWimCertDetails>( aWimCertDetails );
    iPckgBufCertDetailsAllocated = ETrue;
    
    TIpcArgs args;
    args.Set( 0, aCertAddr );
    args.Set( 1, iPckgBufCertDetails->PckgBuf() );
    args.Set( 2, aWimCertDetails.iCert );
    
    SendReceiveData( EGetWIMCertDetails, args, aStatus );
    }


// -----------------------------------------------------------------------------
// RWimCertMgmt::StoreCertificateL() 
// Stores the certificate to WIM
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::StoreCertificateL( const TDes8& aCertificate,
                                     TWimCertAddParameters& aParams,
                                     TWimCertLocation aLocation,
                                     TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimCertMgmt::StoreCertificateL()" ) );
    aStatus = KRequestPending;

    iPckgBufAddCerts = new( ELeave )  
        CWimCertPckgBuf<TWimCertAddParameters>( aParams ); 
    iPckgBufAddCertsAllocated = ETrue;
    
    TIpcArgs args;
    args.Set( 0, &aCertificate );
    args.Set( 1, iPckgBufAddCerts->PckgBuf() );
    args.Set( 2, aLocation );
    
    
    SendReceiveData( EStoreCertificate, args, aStatus );
    }

// -----------------------------------------------------------------------------
// RWimCertMgmt::GetCertExtrasL()
// Gets certificate extra data from one certificate
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::GetCertExtrasL( const TPtr8* aKeyId, 
                                 TCertExtrasInfo& aCertExtrasInfo,
                                 TUint aUsage,
                                 TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimCertMgmt::GetCertExtrasL()" ) );
    aStatus = KRequestPending;
    iPckgBufGetExtras  = new( ELeave ) 
              CWimCertPckgBuf<TCertExtrasInfo>( aCertExtrasInfo );
    iPckgBufGetExtrasAllocated = ETrue;
    
    TIpcArgs args;
    args.Set( 0, aKeyId );
    args.Set( 1, iPckgBufGetExtras->PckgBuf() );
    args.Set( 2, aUsage );
    args.Set( 3, aCertExtrasInfo.iTrustedUsage );
    
    SendReceiveData( EGetCertExtras, args, aStatus );
    }


// -----------------------------------------------------------------------------
// RWimCertMgmt::CertExtrasPckgBuf()
// Returns CWimCertPckgBuf<TCertExtrasInfo>* member variable.
// -----------------------------------------------------------------------------
//
CWimCertPckgBuf<TCertExtrasInfo>* RWimCertMgmt::CertExtrasPckgBuf()
    {
    _WIMTRACE ( _L( "RWimCertMgmt::CertExtrasPckgBuf()" ) );
    return iPckgBufGetExtras;
    }

// -----------------------------------------------------------------------------
// RWimCertMgmt::RemoveL() 
// Removes a certificate from WIM
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::RemoveL( TWimCertRemoveAddr aWimCertRemoveAddr,
                            TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimCertMgmt::RemoveL()" ) );
    aStatus = KRequestPending;
    iPckgBufRemoveCert = new( ELeave ) 
        CWimCertPckgBuf<TWimCertRemoveAddr>( aWimCertRemoveAddr );
    iPckgBufRemoveCertAllocated = ETrue;
    
    TIpcArgs args;
    args.Set( 0, iPckgBufRemoveCert->PckgBuf() );
    
    SendReceiveData( ERemoveCertificate, args, aStatus );
    }


// -----------------------------------------------------------------------------
// RWimCertMgmt::DeallocWimCertPckgBuf()
// DeAllocates memory from iPckgBufCertDetails.
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::DeallocWimCertPckgBuf()
    {
    _WIMTRACE ( _L( "RWimCertMgmt::DeallocWimCertPckgBuf()" ) );
    if ( iPckgBufCertDetailsAllocated )
        {
        delete iPckgBufCertDetails; 
        iPckgBufCertDetails = NULL;
        iPckgBufCertDetailsAllocated = EFalse;
        }
    }

// -----------------------------------------------------------------------------
// RWimCertMgmt::DeallocCertAddParametersPckgBuf()
// DeAllocates memory from iPckgBufAddCerts.
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::DeallocCertAddParametersPckgBuf()
    {
    _WIMTRACE ( _L( "RWimCertMgmt::DeallocCertAddParametersPckgBuf()" ) );
    if ( iPckgBufAddCertsAllocated )
        {
        delete iPckgBufAddCerts;
        iPckgBufAddCerts = NULL;
        iPckgBufAddCertsAllocated = EFalse;
        }
    }


// -----------------------------------------------------------------------------
// RWimCertMgmt::DeallocRemoveCertPckgBuf()
// DeAllocates memory from iPckgBufRemoveCert.
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::DeallocRemoveCertPckgBuf()
    {
    _WIMTRACE ( _L( "RWimCertMgmt::DeallocRemoveCertPckgBuf()" ) );
    if ( iPckgBufRemoveCertAllocated )
        {
        delete iPckgBufRemoveCert;
        iPckgBufRemoveCert = NULL;
        iPckgBufRemoveCertAllocated = EFalse;
        }
    }


// -----------------------------------------------------------------------------
// RWimCertMgmt::DeallocGetExtrasPckgBuf()
// DeAllocates memory from iPckgBufGetExtras.
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::DeallocGetExtrasPckgBuf()
    {
    _WIMTRACE ( _L( "RWimCertMgmt::DeallocGetExtrasPckgBuf()" ) );
    if ( iPckgBufGetExtrasAllocated )
        {
        delete iPckgBufGetExtras;
        iPckgBufGetExtras = NULL;
        iPckgBufGetExtrasAllocated = EFalse;
        }
    }




/*  Key Management */

// -----------------------------------------------------------------------------
// RWimCertMgmt::KeyList()
//                              
// Gets the KeyList from the WIM
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::KeyList( TDes8& aKeyList,
                            TDes8& aKeyCountPkg, 
                            TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimCertMgmt::KeyList()" ) );
    aStatus = KRequestPending;
    
    TIpcArgs args;
    args.Set( 0, &aKeyList );
    args.Set( 1, &aKeyCountPkg );
    
    
    SendReceiveData( EGetKeyList, args, aStatus );
    }

// -----------------------------------------------------------------------------
// RWimCertMgmt::GetKeyInfo()
//
// Gets Info for single key
// -----------------------------------------------------------------------------
//
TInt RWimCertMgmt::GetKeyInfo( const TInt32 aKeyReference,                                
                               TKeyInfo& aKeyInfo )
    {
    _WIMTRACE ( _L( "RWimCertMgmt::GetKeyInfo()" ) );
    TPckg<TKeyInfo> keyinfo ( aKeyInfo );
    
    TIpcArgs args;
    args.Set( 0, aKeyReference );
    args.Set( 1, &keyinfo );
    
    return SendReceiveData( EGetKeyDetails, args );
    }

// -----------------------------------------------------------------------------
// RWimCertMgmt::SignL()
// Sign some data
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::SignL( TKeySignParameters& aSignParameters, 
                         TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimCertMgmt::SignL" ) );
    aStatus = KRequestPending;
    iPckgBufKeySignParameters = new( ELeave ) 
              CWimCertPckgBuf<TKeySignParameters>( aSignParameters );
    iPckgBufKeySignAllocated = ETrue;
    
    TIpcArgs args;
    args.Set( 1, iPckgBufKeySignParameters->PckgBuf() );
    args.Set( 2, aSignParameters.iSigningData );
    args.Set( 3, aSignParameters.iSignature );
    
    SendReceiveData( ESignTextReq, args, aStatus );
    }
    
// -----------------------------------------------------------------------------
// RWimCertMgmt::DeallocKeySignPckgBuf()
// Deallocates member variable
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::DeallocKeySignPckgBuf()
    {
    _WIMTRACE ( _L( "RWimCertMgmt::DeallocKeySignPckgBuf" ) );
    if ( iPckgBufKeySignAllocated )
        {
        delete iPckgBufKeySignParameters;
        iPckgBufKeySignParameters = NULL;
        iPckgBufKeySignAllocated = EFalse;
        }
    }

// -----------------------------------------------------------------------------
// RWimCertMgmt::ExportPublicKeyL()
// Export public key
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::ExportPublicKeyL( TExportPublicKey& aPublicKeyParams, 
                               TRequestStatus& aStatus )
    {
    _WIMTRACE ( _L( "RWimCertMgmt::ExportPublicKeyL" ) );
    aStatus = KRequestPending;
    iPckgBufExportPublicKey = new( ELeave ) 
        CWimCertPckgBuf<TExportPublicKey>( aPublicKeyParams );
    iPckgBufExportPublicKeyAllocated = ETrue;
    
    TIpcArgs args;
    args.Set( 0, iPckgBufExportPublicKey->PckgBuf() );
    args.Set( 1, aPublicKeyParams.iPublicKey );
    
    SendReceiveData( EExportPublicKey, args, aStatus );
    }

// -----------------------------------------------------------------------------
// RWimCertMgmt::DeallocExportPublicKeyPckgBuf()
// Deallocates member variable.
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::DeallocExportPublicKeyPckgBuf()
    {
    _WIMTRACE ( _L( "RWimCertMgmt::DeallocExportPublicKeyPckgBuf" ) );
    if ( iPckgBufExportPublicKey )
        {
        delete iPckgBufExportPublicKey;
        iPckgBufExportPublicKey = NULL;
        iPckgBufExportPublicKeyAllocated = EFalse;
        }
    }



/*  OMA Provisioning */

// -----------------------------------------------------------------------------
// RWimCertMgmt::RetrieveOmaDataL()
// Retrieves OMA provisioning data.
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::RetrieveOmaDataL( TOmaProv& aOmaProvStruct, 
                                     TRequestStatus& aStatus,
                                     const TWimServRqst aOpCode )
    {
    _WIMTRACE ( _L( "RWimCertMgmt::RetrieveOmaDataL" ) );
    aStatus = KRequestPending;
    iPckgBufOmaProv = new( ELeave ) 
                CWimCertPckgBuf<TOmaProv>( aOmaProvStruct );
    iPckgBufOmaProvAllocated = ETrue;
    
    TIpcArgs args;
    args.Set( 0, iPckgBufOmaProv->PckgBuf() );
    args.Set( 1, aOmaProvStruct.iOmaData );
    
    SendReceiveData( aOpCode, args, aStatus );
    }

// -----------------------------------------------------------------------------
// RWimCertMgmt::RetrieveACIFDataL()
// Retrieves ACIF data.
// -----------------------------------------------------------------------------
//    
void RWimCertMgmt::RetrieveACIFDataL( TJavaProv& aJavaProvStruct,
                               TRequestStatus& aStatus,
                               const TWimServRqst aOpCode )
    {
	_WIMTRACE ( _L( "RWimCertMgmt::RetrieveACIFDataL" ) );
    aStatus = KRequestPending;
    iPckgBufJavaProv = new( ELeave ) 
                CWimCertPckgBuf<TJavaProv>( aJavaProvStruct );
    iPckgBufJavaProvAllocated = ETrue;
    
    TIpcArgs args;
    args.Set( 0, iPckgBufJavaProv->PckgBuf() );
    args.Set( 1, aJavaProvStruct.iJavaData );
    
    SendReceiveData( aOpCode, args, aStatus );
    }
    
// -----------------------------------------------------------------------------
// RWimCertMgmt::RetrieveACFDataL()
// Retrieves ACF data.
// -----------------------------------------------------------------------------
//    
void RWimCertMgmt::RetrieveACFDataL( TJavaProv& aJavaProvStruct,
                               TRequestStatus& aStatus,
                               const TWimServRqst aOpCode )
    {
	_WIMTRACE ( _L( "RWimCertMgmt::RetrieveACFDataL" ) );
    aStatus = KRequestPending;
    iPckgBufJavaProv = new( ELeave ) 
                CWimCertPckgBuf<TJavaProv>( aJavaProvStruct );
    iPckgBufJavaProvAllocated = ETrue;
    
    TIpcArgs args;
    args.Set( 0, iPckgBufJavaProv->PckgBuf() );
    args.Set( 1, aJavaProvStruct.iJavaData );
    args.Set( 2, aJavaProvStruct.iPath );
    
    SendReceiveData( aOpCode, args, aStatus );
    }    
       
// -----------------------------------------------------------------------------
// RWimCertMgmt::GetAuthObjsInfo()
// Get Authentication Object information
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::RetrieveAuthObjsInfoL( const RArray<TInt>& aAuthIdList,
                      RArray<TJavaPINParams>& aAuthObjsInfoList,
                      TRequestStatus& aStatus,
                      const TWimServRqst aOpCode )             
    {
    _WIMTRACE ( _L( "RWimCertMgmt::RetrieveAuthObjsInfo" ) );
      
    TInt count = aAuthIdList.Count();
    TInt authIdLength = count * sizeof( TInt );
    TInt authobjLength = count * sizeof( TJavaPINParams ); 
    
    if ( iAuthIdLstPtr == NULL )
	    {
	    iAuthIdLstPtr = new( ELeave )TPtrC8( (TText8*)&aAuthIdList[0], authIdLength );
	    }
    else
        {
        iAuthIdLstPtr->Set( (TText8*)&aAuthIdList[0], authIdLength );
        } 
    
    if ( iAuthObjsInfoLstPtr == NULL )
        {
        iAuthObjsInfoLstPtr = new( ELeave )TPtr8( (TText8*)&aAuthObjsInfoList[0], authobjLength, authobjLength );	
        }
    else
        {
        iAuthObjsInfoLstPtr->Set( (TText8*)&aAuthObjsInfoList[0], authobjLength, authobjLength );	
        }
    
    aStatus = KRequestPending;
    TIpcArgs args;
    
    args.Set( 0, iAuthIdLstPtr );
    args.Set( 1, iAuthObjsInfoLstPtr );
    args.Set( 2, count );
    
    
    SendReceiveData( aOpCode, args, aStatus );
    }
                              
                              
// -----------------------------------------------------------------------------
// RWimCertMgmt::RetrieveLabelAndPathL()
// Get Authentication Object information
// -----------------------------------------------------------------------------
//                                                      
void RWimCertMgmt::RetrieveLabelAndPathL( TDes8& aLabel,
        	                              TDes8& aPath, 
                                          TRequestStatus& aStatus,
                                          const TWimServRqst aOpCode )
    {
	aStatus = KRequestPending;
	TIpcArgs args;
	
	args.Set( 0, &aLabel );
	args.Set( 1, &aPath );
	
	SendReceiveData( aOpCode, args, aStatus );
    }
// -----------------------------------------------------------------------------
// RWimCertMgmt::TOmaProvPckgBuf()
// Returns member variable.
// -----------------------------------------------------------------------------
//
CWimCertPckgBuf<TOmaProv>* RWimCertMgmt::TOmaProvPckgBuf()
    {
    return iPckgBufOmaProv;
    }
    
// -----------------------------------------------------------------------------
// RWimCertMgmt::TJavaProvPckgBuf()
// Returns member variable.
// -----------------------------------------------------------------------------
//
CWimCertPckgBuf<TJavaProv>* RWimCertMgmt::TJavaProvPckgBuf()
    {
    return iPckgBufJavaProv;
    }    

// -----------------------------------------------------------------------------
// RWimCertMgmt::DeallocOmaDataPckgBuf()
// Deallocates member variable.
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::DeallocOmaDataPckgBuf()
    {
    if ( iPckgBufOmaProvAllocated )
        {
        delete iPckgBufOmaProv;
        iPckgBufOmaProv = NULL;
        iPckgBufOmaProvAllocated = EFalse;
        }
    }

// -----------------------------------------------------------------------------
// RWimCertMgmt::DeallocJavaDataPckgBuf()
// Deallocates member variable.
// -----------------------------------------------------------------------------
//
void RWimCertMgmt::DeallocJavaDataPckgBuf()
    {
    if ( iPckgBufJavaProvAllocated )
        {
        delete iPckgBufJavaProv;
        iPckgBufJavaProv = NULL;
        iPckgBufJavaProvAllocated = EFalse;
        }
    }
