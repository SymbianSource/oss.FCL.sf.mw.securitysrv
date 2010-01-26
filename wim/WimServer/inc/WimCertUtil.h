/*
* Copyright (c) 2004-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Interface which handles certificate related operations
*
*/



#ifndef WIMCERTUTIL_H
#define WIMCERTUTIL_H


//INCLUDES
#include "WimClsv.h"
#include "Wimi.h"
#include <ct.h> 


// FORWARD DECLARATIONS
class CWimCertInfo;
class CCTCertInfo;
class CWimUtilityFuncs;

// CONSTANTS
typedef HBufC8* PHBufC8;
typedef TPtr8* PTPtr8;


// CLASS DECLARATION
/**
*  Utility class that handles certificate related operations.

*  @since Series60 3.0
*/
class CWimCertUtil : public CActive
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @param aToken -Reference to current token
        */
        static CWimCertUtil* NewL( MCTToken& aToken );

        /**
        * Destructor
        */
        virtual ~CWimCertUtil();
        
    public: // New functions

        /**
        * Restores certificates from WIM cache. If Restore cannot find
        * any certificate, it will return KErrNotFound to caller.
        * @param  aArray    -Array where new certificates are 
        *                   inserted. Caller is responsible 
        *                   to deallocate CWimCertInfo -objects
        *                   by calling ResetAndDestroy.(IN/OUT)
        * @param  aStatus   -Status from caller.         
        */
        void Restore( RPointerArray<CWimCertInfo>& aArray, 
                      TRequestStatus& aStatus );

        
        /** 
        * Cancels outgoing Restore operation. Sets an internal flag to true. 
        * After necessary cleanup, caller is signalled with KErrCancel 
        * -error code.
        */
        void CancelRestore();
        
        /**
        * Retrieves the actual certificate. In case of other cert than
        * Url cert the binary encoded certificate is written into the 
        * aEncodedCert parameter. In case of Url cert the data is 
        * the actual url.
        * @param  aIndex. Used to point iCertRefLst and iCertInfoArr 
        *         -arrays for correct certificate.
        * @param  aEncodedCert. A buffer to put the certificate in. 
        * @param  aStatus. A request status that is completed when the 
        *         operation has finished.(IN/OUT)
        */
        void RetrieveCertByIndexL( const TInt aIndex,
                                   TDes8& aEncodedCert, 
                                   TRequestStatus& aStatus );

    private:

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * C++ default constructor.
        * @param aToken -Reference to current token
        */
        CWimCertUtil( MCTToken& aToken );

        /**
        * Get count of certificates in WIM
        * @param aRef       WIM card reference pointer.
        * @param aCertCount Count of certificates in WIM card pointed by aRef
        * @param aUsage     Certificate usage.
        * @return Status returned by WIMI. See in WimClsv.h.
        */
        WIMI_STAT GetCertificateCountByWIM( WIMI_Ref_t* aRef, 
                                            TUint8& aCertCount, 
                                            TUint8 aUsage ) const;
        /**
        * Get certificate extra data
        * @param aWimRef         WIM card reference pointer.
        * @param aUsage          Certificate usage.
        * @param aKeyHash        Key hash of certificate
        * @param aCertExtrasInfo Returned extra data
        * @return Status returned by WIMI.
        */
        WIMI_STAT GetExtrasFromWimRefL( 
                                    WIMI_Ref_t* aWimRef,
                                    TInt8 aUsage,
                                    TDesC8& aKeyHash,
                                    TCertExtrasInfo& aCertExtrasInfo ) const;

        /**
        * Allocates memory for the array which is filled by server.
        * @param  aWimCertInfoArr    -Array to be initialized 
        * @param  aCount             -Count of elements in the array         
        */
        void AllocWimCertInfoL( TWimCertInfo* aWimCertInfoArr, TInt aCount );
        
        /**
        * Deallocates memory from the array.
        */
        void DeallocWimCertInfo();   
        
        /**
        * Allocates memory for a struct which is filled by server.
        */
        void AllocMemoryForCertDetailsL();

        /**
        * Creates new certificate objects which can be returned to the caller.
        */
        void CreateNewCertObjectsL();
        
        /**
        * Creates new wim certificate objects.
        */
        void CreateNewWimCertObjectL();

        /**
        * Gets trusted usages.
        */
        void GetTrustedUsagesL();

        /**
        * Completion of trusted usages.
        */
        void TrustedUsagesDoneL();

        /**
        * Sets own iStatus to KRequestPending, and signals it 
        * with User::RequestComplete() -request. This gives chance 
        * to activescheduler to run other active objects. After a quick
        * visit in activescheduler, signal returns to RunL() and starts next
        * phase of operation. 
        */
        void SignalOwnStatusAndComplete();
        
        /**
        * Copies data to caller's buffer by using pointer.
        */
        void CopyRetrievedCertData();

        /**
        * DeAllocates memory from member variables, which are used
        * when communicating with WIM.
        */
        void DeallocCertHBufs();

        /**
        * If user has cancelled initialization process, dealloc references 
        * from loaded certs. 
        */
        void DeallocReferences();

        /**
        * Returns certificate format according to received index.
        * @param aIndex -index of the certificate to be returned.
        * @return TCertificateFormat -format of the certificate
        */
        TCertificateFormat GetCertFormatByIndex( TInt aIndex );

        /**
        * Copy certificate extra data
        * @param aCert           Reference to certificate
        * @param aCertExtrasInfo Returned extra data
        */
        void CopyCertExtrasInfoL( WIMI_Ref_t* aCert,
                                  TCertExtrasInfo& aCertExtrasInfo ) const;

        /**
        * Return count of certificates in WIM
        * @param aType           Certificate type
        * @return Count of certificates
        */
        TUint8 CertCount( TWimEntryType aType );

        /**
        * Get certificate reference list
        * @param aCertAddrLst
        * @param aCertInfoArr
        * @param aCertEntryType
        */
        void CertRefLstL( TCertificateAddressList aCertAddrLst, 
                          TWimCertInfo* aCertInfoArr, 
                          TWimEntryType aCertEntryType );

        /**
        * Get certificate extra data
        * @param aKeyId
        * @param aCertExtrasInfo
        * @param aUsage
        * @param aStatus
        */
        void GetCertExtrasL( const TPtr8* aKeyId, 
                             TCertExtrasInfo& aCertExtrasInfo,
                             TUint aUsage,
                             TRequestStatus& aStatus );

        /**
        * Get certificate details
        * @param aCertAddr
        * @param aWimCertDetails
        */
        void CertDetailsL( const TCertificateAddress aCertAddr, 
                           TWimCertDetails& aWimCertDetails );

        /**
        *
        * @param aTmpWimRef
        * @param aUsage
        * @param aCertNum
        * @param aCertRefLst
        * @param aCertInfoLst
        * @return Error code returned by WIMI
        */
        WIMI_STAT GetCertificateFromWimRefL( WIMI_Ref_t* aTmpWimRef,
                                             TInt8 aUsage,
                                             TUint8& aCertNum,
                                             TUint32* aCertRefLst,
                                             TWimCertInfo* aCertInfoLst );

        /**
        * Copy certificate info
        * @param aCertInfo
        * @param aCert
        */
        void CopyCertificateInfo( TWimCertInfo& aCertInfo,
                                  WIMI_Ref_t* aCert );

    private: //from CActive
        
        /**
        * Different phases are handled here.
        */
        void RunL();

        /**
        * Cancellation function
        * Deallocates member variables and completes client status with
        * KErrCancel error code.
        */
        void DoCancel();

        /**
        * The active scheduler calls this function if this active 
        * object's RunL() function leaves. 
        * Handles necessary cleanup and completes request with
        * received error code.
        * @param aError -The error code which caused this function call.
        * @return TInt  -Error code to activescheduler, is always KErrNone.
        */ 
        TInt RunError( TInt aError );

    private:

        /**
        * Seven different phases, which are used to 
        * Read certificates from WIM cache, create new certificate objects and
        * retrieve certificate
        */
        enum TPhase
            {
            EListCertsFromWim,
            ECreateNewCertObjects,
            EGetTrustedUsages,
            ECertObjectsDone,
            ETrustedUsagesDone,
            ERetrieveCertificate,
            ERetrievingCompleted
            };

        //Client status is stored here while operation
        //on the server side is done.
        TRequestStatus*                 iClientStatus;
        //Array which is used to contain new certificate objects.
        //Not owned. Caller is responsible to destroy array.
        RPointerArray<CWimCertInfo>*    iArray;
        //Details of the certificate. Owned.
        TWimCertDetails                 iWimCertDetails;
        //Reference to current token. Needed in CCTCertInfo creation.
        MCTToken&                       iToken;
        //Typedefs for the reference to certificate. Owned.
        TCertificateAddressList         iCertRefLst; 
        // Certificate information structure. Owned.
        TWimCertInfo*                   iCertInfoArr;
        //Used to handle different phases.
        TPhase                          iPhase;
        //Index to point which certificate we want to be retrieved
        TInt                            iCertRetrieveIndex;
        //Used to inform the size of the array, needed in array construction/
        //destruction.
        TInt                            iArraySize;
        //The count of certificates
        TUint8                          iCertCount;
        //Pointer to HBufC8*. This is used as an array when listing every 
        //certificate from WIM to an array during startup. Owned.
        PHBufC8*                        iLabel;
        //Pointer to HBufC8*. This is used as an array when listing every 
        //certificate from WIM to an array during startup. Owned.
        PHBufC8*                        iKeyId;
        //Pointer to HBufC8*. This is used as an array when listing every 
        //certificate from WIM to an array during startup. Owned.
        PHBufC8*                        iCAId;
        //Pointer to HBufC8*. This is used as an array when listing every 
        //certificate from WIM to an array during startup. Owned.
        PHBufC8*                        iIssuerHash;
        //Pointer to iLabel. Also used as an array. Owned.        
        PTPtr8*                         iLabelPtr;
        //Pointer to iKeyId. Also used as an array. Owned.        
        PTPtr8*                         iKeyIdPtr;
        //Pointer to iCAId. Also used as an array. Owned.        
        PTPtr8*                         iCAIdPtr;
        //Pointer to iIssuerHash. Also used as an array. Owned.
        PTPtr8*                         iIssuerHashPtr;
        //Modifiable 8-bit descriptor which points
        //to caller's descriptor. We copy retrieved certificate
        //data to this descriptor. Owned.
        TDes8*                          iEncodedCert;
        //Buffer which is used to contain certificate data. Owned.
        HBufC8*                         iCertHBufOne;
        //Pointer to iCertHBufOne. Owned.
        TPtr8*                          iCertHBufOnePtr;
        //Buffer which is used to contain certificate data. Owned.        
        HBufC8*                         iCertHBufTwo;
        //Pointer to iCertHBufTwo. Owned.
        TPtr8*                          iCertHBufTwoPtr;
        //Buffer which is used to contain certificate data. Owned.        
        HBufC8*                         iCertHBufThree;
        //Pointer to iCertHBufThree. Owned.
        TPtr8*                          iCertHBufThreePtr;
        //Buffer which is used to contain certificate data. Owned.        
        HBufC8*                         iCertHBufFour;
        //Pointer to iCertHBufFour. Owned.
        TPtr8*                          iCertHBufFourPtr;
        //ActiveSchedulerWaiter for certificate extra information
        //fetching.
        CActiveSchedulerWait            iActiveSchedulerWait;
        //Pointer for trusted usage buffer
        HBufC*                          iTrustedUsages;
        //Pointer to trusted usage buffer pointer
        TPtr*                           iTrustedUsagesPtr;
        //Index pointing to certificate elements
        TUint8                          iIndex;
        //Pointer to one certificate info, owned
        CCTCertInfo*                    iCert;
        //Pointer to one WIM specific certificate info, owned
        CWimCertInfo*                   iCertInfo;
        //An array of trusted usage oids
        RArray<HBufC*>*                 iOids;
        //Package structure for certificate extra info
        TCertExtrasInfo                 iCertExtrasInfo;
        //Pointer for key identifier buffer
        HBufC8*                         iKeyIdBuf;
        //Used to point right keyId when retrieving extra data
        TPtr8*                          iKeyIdPointer;
        //Pointer to utility functions. Owned.
        CWimUtilityFuncs*               iWimUtilFuncs;
    };

#endif  //WIMCERTUTIL_H

// End of File
