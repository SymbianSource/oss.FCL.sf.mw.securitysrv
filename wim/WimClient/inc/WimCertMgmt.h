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
* Description:  Certificate & key -management class.
*
*/


#ifndef WIMCERTMGMT_H
#define WIMCERTMGMT_H

//  INCLUDES
#include "WimClient.h"
#include "WimCertTClassWrappers.h"
#include "JavaUtils.h"


// CLASS DECLARATION

/**
*  RWimCertMgmt 
*  This class handle certificate handling and KeyInformation handling
*  related traffic between client and server.
*  @lib WimClient
*  @since Series 60 2.1
*/
class RWimCertMgmt: public RWimClient
    {
    public:

        /**
        * A Static method to get the client Session
        * @Return RWimCertMgmt*
        */  
        static RWimCertMgmt* ClientSessionL();

        /**
        * Destructor.
        * Allocated memory is released.
        */  
        virtual ~RWimCertMgmt();
        
        /**
        * Returns the count of certificates
        * @param aType  -The type of certificates to be counted.
        * @return TUint8 -The count of certificates
        */
        TUint8 CertCount( TWimEntryType aType );

        /**
        * Returns certificate list on the WIM.
        * @param aCertAddrLst - to hold references to certificates
        * @param aCertInfoArr -address of the certlist address
        * @param aCount       -the count of certificates
        * @param aCertEntryType -What type of certificates is wanted:
        *                ECrEntryTypeAll, ECrEntryTypeCA or ECrEntryTypePersonal  
        * @param aStatus -Caller's status 
        * @return void
        */  
        void  CertRefLst( TCertificateAddressList aCertAddrLst,
                          TWimCertInfo* aCertInfoArr, 
                          TUint8 aCount, 
                          TWimEntryType aCertEntryType, 
                          TRequestStatus& aStatus );
      
        /**
        * Returns certificate list on the WIM.
        * @param aCertAddrLst - to hold references to certificates
        * @param aCertInfoArr -address of the certlist address
        * @param aCount       -the count of certificates
        * @param aCertEntryType -What type of certificates is wanted:
        *                ECrEntryTypeAll, ECrEntryTypeCA or ECrEntryTypePersonal  
        * @return TInt -Errorcode from server.
        */          
        TInt CertRefLst( TCertificateAddressList aCertAddrLst, 
                         TWimCertInfo* aCertInfoArr, 
                         TUint8 aCount, 
                         TWimEntryType aCertEntryType );
        /**
        * Stores the certificate to WIM.
        * @param aCertificate- certificate
        * @param aParams - TWimCertAddParameters params to the certificate
        * @param aLocation  -Location where we want to store. In this case:
                            EWimCertLocationWIMCard
        * @return void
        */
        void StoreCertificateL( const TDes8& aCertificate, 
                               TWimCertAddParameters& aParams, 
                               TWimCertLocation aLocation,
                               TRequestStatus& aStatus );
        
        /**
        * Removes a certificate from WIM.
        * @param aWimCertRemoveAddr - reference to the certificate
        * @param aStatus    -Status from caller
        * @return void
        */  
        void RemoveL( TWimCertRemoveAddr aWimCertRemoveAddr,
                      TRequestStatus& aStatus );

        /**
        * Returns certificate details.
        * @param aCertAddr - reference to the certificate
        * @param aWimCertDetails -Preallocated structure to hold 
        *                        the certificate info
        * @param aStatus    -Caller's status
        * @return void
        */  
        void CertDetailsL( const TCertificateAddress aCertAddr, 
                           TWimCertDetails& aWimCertDetails,
                           TRequestStatus& aStatus );

        /**
        * Gets certificate extra information from one certificate
        * @param aKeyId - identifies needed certificate
        * @param aCertExtrasInfo -Preallocated structure to hold 
        *                        the certificate extra info
        * @param aUsage     -Identifies the usage of certificate, User or CA
        * @param aStatus    -Caller's status
        * @return void
        */  
        void GetCertExtrasL(  const TPtr8* aKeyId, 
                              TCertExtrasInfo& aCertExtrasInfo,
                              TUint aUsage,
                              TRequestStatus& aStatus );

        /** 
        * Returns KeyList from the WIM.
        * @param aKeyList   -KeyReferences are listed to this
        * @param aKeyCount  -The number of keys is put to this
        * @param aStatus    -Caller's status
        */
        void KeyList( TDes8& aKeyList,
                      TDes8& aKeyCount, 
                      TRequestStatus& aStatus );

        /** 
        * Gets keyInfo for single key.
        * @param aKeyReference  -Unique identifier for key.
        * @param aKeyInfo   -Struct for other key information.
        */
        TInt GetKeyInfo( const TInt32 aKeyReference,
                         TKeyInfo& aKeyInfo );

        /** 
        * Signs some data
        * @param aSignParameters -holds data for signing operation
        * @param aStatus         -Caller's status
        */
        void SignL( TKeySignParameters& aSignParameters, 
                   TRequestStatus& aStatus );

        /** 
        * Export public key
        * @param aPublicKeyParams -struct which holds publickeyPtr and Key 
        *                          identifier.
        *                          pointer.
        * @param aStatus        -Caller's Status.
        */
        void ExportPublicKeyL( TExportPublicKey& aPublicKeyParams, 
                              TRequestStatus& aStatus );
 

        /**
        * Retrieves data for OMA Provisioning support. Data can be data size,
        * type or the actual binary data.
        * @param aOmaProvStruct -Struct to hold OMA Provisioning data.
        * @param aStatus        -Caller's status
        * @param aOpCode        -Code of operation ( EGetOMAFileSize or 
        *                                            EGetOMAFile
        *
        */
        void RetrieveOmaDataL( TOmaProv& aOmaProvStruct,
                               TRequestStatus& aStatus,
                               const TWimServRqst aOpCode );
       
       /**
        * Retrieves ACIF data.
        *
        * @since S60 3.2
        * @param aJavaProvStruct Struct to hold Java data.
        * @param aStatus Caller's status
        * @param aOpCode Code of operation 
        * @return                                            
        *
        */
        void RetrieveACIFDataL( TJavaProv& aJavaProvStruct,
                               TRequestStatus& aStatus,
                               const TWimServRqst aOpCode );
        
        /**
        * Retrieves ACF data.
        *
        * @since S60 3.2
        * @param aJavaProvStruct Struct to hold Java data.
        * @param aStatus Caller's status
        * @param aOpCode Code of operation 
        * @return                                           
        *
        */                        
        void RetrieveACFDataL( TJavaProv& aJavaProvStruct,
                               TRequestStatus& aStatus,
                               const TWimServRqst aOpCode );   
        
        /**
        * Retrieves Authtication object data.
        *
        * @since S60 3.2
        * @param aAuthIdList 
        * @param aAuthObjsInfoList 
        * @param aStatus Caller's status
        * @param aOpCode Code of operation 
        * @return                                           
        *
        */                       
        void RetrieveAuthObjsInfoL(const RArray<TInt>& aAuthIdList,
                              RArray<TJavaPINParams>& aAuthObjsInfoList,
                              TRequestStatus& aStatus,
                              const TWimServRqst aOpCode );  
        
        
        /**
        * Retrieves Label and Path.
        *
        * @since S60 3.2
        * @param aLabel
        * @param aPath 
        * @param aStatus Caller's status
        * @param aOpCode Code of operation 
        * @return                                           
        *
        */                         
        void RetrieveLabelAndPathL( TDes8& aLabel,
        	                        TDes8& aPath, 
                                    TRequestStatus& aStatus,
                                    const TWimServRqst aOpCode );                      
                                                                                    
        /** 
        * Deallocates memory from iPckgCertDetails.
        * @return void
        */
        void DeallocWimCertPckgBuf();

        /** 
        * Deallocates memory from iPckgAddCerts.
        * @return void
        */
        void DeallocCertAddParametersPckgBuf();

        /** 
        * Deallocates memory from iPckgRemoveCert.
        * @return void
        */
        void DeallocRemoveCertPckgBuf();

        /**
        * Deallocates memory from iPckgGetExtrasAllocated.
        * @return void
        */
        void DeallocGetExtrasPckgBuf();

        /**
        * Deallocates memory from iPckgKeySignParameters.
        * @return void
        */
        void DeallocKeySignPckgBuf();

        /**
        * Deallocates memory from iPckgBufExportPublicKey.
        * @return void
        */
        void DeallocExportPublicKeyPckgBuf();

        /** 
        * Deallocates memory from iPckgBufOmaProv
        * @return void
        */
        void DeallocOmaDataPckgBuf();
        
        /** 
        * Deallocates memory from iPckgBufJavaProv
        * @return void
        */
        void DeallocJavaDataPckgBuf();

        /** 
        * Return CWimCertPckgBuf<TCertExtrasInfo> pointer to caller
        * @return CWimCertPckgBuf<TCertExtrasInfo>* 
        */
        CWimCertPckgBuf<TCertExtrasInfo>* CertExtrasPckgBuf();

        /** 
        * Return CWimCertPckgBuf<TOmaProv> pointer to caller
        * @return CWimCertPckgBuf<TOmaProv>* 
        */
        CWimCertPckgBuf<TOmaProv>* TOmaProvPckgBuf();
        
        /** 
        * Return CWimCertPckgBuf<TJavaProv> pointer to caller
        * @return CWimCertPckgBuf<TJavaProv>* 
        */
        CWimCertPckgBuf<TJavaProv>* TJavaProvPckgBuf();
        
    private:
        
        /**
        * Constructor 
        */
        RWimCertMgmt();

        /** 
        * Copy constructor
        * @param aSource -Reference to class object.
        */
        RWimCertMgmt( const RWimCertMgmt& aSource );

        /** 
        * Assignment operator
        * @param aParam -Reference to class object.
        */
        RWimCertMgmt& operator = ( const RWimCertMgmt& aParam );

    private:
      
        //Used to point data when listing every certificate from WIM.
        TPtr8                                   iCertLst;

        //Used to point data when listing every certificate from WIM.
        TPtr8                                   iCertInfoLst;

        //PckgBuffer wrapper which is used when retrieving certificate.Owned.
        CWimCertPckgBuf<TWimCertDetails>*       iPckgBufCertDetails;

        //PckgBuffer wrapper which is used when adding a certificate.Owned.
        CWimCertPckgBuf<TWimCertAddParameters>* iPckgBufAddCerts;

        //PckgBuffer wrapper which is used when removing a certificate.Owned.
        CWimCertPckgBuf<TWimCertRemoveAddr>*    iPckgBufRemoveCert;

        //PckgBuffer wrapper which is used when getting certificate extra 
        //information from server.Owned.
        CWimCertPckgBuf<TCertExtrasInfo>*       iPckgBufGetExtras;

        //Pckgbuffer wrapper which is used when signing data.Owned.
        CWimCertPckgBuf<TKeySignParameters>*    iPckgBufKeySignParameters;

        //Pckgbuffer wrapper which is used when exporting public key.Owned.
        CWimCertPckgBuf<TExportPublicKey>*      iPckgBufExportPublicKey;

        //Pckgbuffer wrapper which is used when getting data for 
        //OMA Provisioning.Owned.
        CWimCertPckgBuf<TOmaProv>*              iPckgBufOmaProv;
        
        //Pckgbuffer wrapper which is used when getting data for 
        //Java Provisioning.Owned.
        CWimCertPckgBuf<TJavaProv>*             iPckgBufJavaProv;
        
        //Flag to indicate whether certificate details package is allocated or 
        //not.
        TBool                                   iPckgBufCertDetailsAllocated;
        
        //Flag to indicate whether add certificate package is allocated or not.
        TBool                                   iPckgBufAddCertsAllocated;

        //Flag to indicate whether remove certificate package is allocated or 
        //not.
        TBool                                   iPckgBufRemoveCertAllocated;

        //Flag to indicate whether extra information package is allocated or 
        //not
        TBool                                   iPckgBufGetExtrasAllocated;

        //Flag to indicate whether signing package is allocated or not.
        TBool                                   iPckgBufKeySignAllocated;
    
        //Flag to indicate whether export public key package is allocated or 
        //not
        TBool                                   iPckgBufExportPublicKeyAllocated;

        //Flag to indicate whether OMA Provisioning package is allocated or 
        //not
        TBool                                   iPckgBufOmaProvAllocated;
        
        //Flag to indicate whether Java Provisioning package is allocated or 
        //not
        TBool                                   iPckgBufJavaProvAllocated;
        
        TPtr8*                                  iAuthObjsInfoLstPtr;
	    
	    TPtrC8*                                 iAuthIdLstPtr;

    };

#endif //WIMCERTMGMT_H










































