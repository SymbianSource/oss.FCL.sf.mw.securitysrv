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
* Description:  Reads certificates bypassing crypto token framework
*
*/


#ifndef WIMCERT_H
#define WIMCERT_H


//INCLUDES
#include <e32base.h>
#include <e32std.h>
#include <ct.h>

//TYPE DEFINITIONS
typedef HBufC8* PHBufC8;
typedef TPtr8* PTPtr8;

enum TCertLocation
    {
    EUnknown_CDF,
    ECertificates_CDF,
    ETrustedCerts_CDF,
    EUsefulCerts_CDF
    };

//FORWARD DECLARATIONS
class RWimCertMgmt;
class RWimMgmt;



//CLASS DECLARATION
/**
*  Reads certificate extra information from Smart Card
*
*  @lib WimClient
*  @since Series60 2.1
*/
class CWimCert : public CActive
    {
    public: // Member functions

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CWimCert* NewL();

        /**
        * Destructor
        */
        IMPORT_C virtual ~CWimCert();
        
        /**
        * Initializes the WIM server cache and restores certificates to 
        * WimClient memory area. This is the most time consuming call. 
        * @param  aStatus  Indicates the result of this call. Values:
        *         KErrNotFound, no certificates was found
        *         KErrNone, call was successull and at least one certificate 
        *         found.
        *         KErrGeneral, any other error
        *         KErrNoMemory, no memory
        * @return void
        */
        IMPORT_C void Initialize( TRequestStatus& aStatus );

        /** 
        * Cancels outgoing Initialize operation.
        * After necessary cleanup, caller (given aStatus) is signalled with 
        * KErrCancel 
        * @return void
        */
        IMPORT_C void CancelInitialize();

        /**
        * Fetches trusted usages and location information for Key ID.
        * @param  aKeyId (IN) Key identifier (hash), which points to a 
        *         certificate.
        * @param  aTrustedUsages (OUT) An array of OID strings indicating 
        *         trusted usage. The notation is dot delimited 
        *         (for example 1.23.456.7), the length of OÌD can vary. 
        *         (OID=Object IDentifier). Caller is responsible of
        *         destroying array.
        * @param  aLocation (OUT) Indicates certificate directory location 
        *         in Smart Card.
        * @param  aStatus  Indicates the result of this call. Values:
        *         KErrNotFound, given certificate KeyId was not found from card
        *         KErrNone, no errors, given certificate was found and trusted 
        *                   usage was present and location were known
        *         KErrPathNotFound, trusted usage was not found or location 
        *                           was not known
        *         KErrGeneral, any other error
        *         KErrNoMemory, No memory
        * @return void
        */
        IMPORT_C void GetExtras( const TDesC8& aKeyId, 
                                 RCPointerArray<HBufC>& aTrustedUsages,
                                 TCertLocation& aLocation,
                                 TRequestStatus& aStatus );

        /** 
        * Cancels outgoing GetExtras operation.
        * After necessary cleanup, caller is signalled with KErrCancel 
        * @return void
        */
        IMPORT_C void CancelGetExtras();

    private:

        /**
        * Symbian constructor
        */
        void ConstructL();

        /**
        * C++ default constructor.
        */
        CWimCert();

    private: //from CActive
        
        /**
        * Different phases are handled here.
        * @return void
        */
        void RunL();

        /**
        * Cancellation function
        * Deallocates member variables and completes client status with
        * KErrCancel error code.
        * @return void
        */
        void DoCancel();

        /**
        * The active scheduler calls this function if this active 
        * object's RunL() function leaves. 
        * Handles necessary cleanup and completes request with
        * received error code.
        * @param aError -Error code which launched this event.
        * @return TInt  Error code to active scheduler, is always KErrNone.
        */ 
        TInt RunError( TInt aError );
        
    private: // functions
        
        /**
        * Tries to locate certificate by using retrieved KeyId.
        * @return boolean -If certificate is found and it has extra data, 
        * @return ETrue. Else return EFalse.
        */
        TBool CertificateLocated();

        /**
        * Fills caller's data area with requested extra data
        * @return void
        */
        void ExtraDataFoundL();

        /** 
        * Handles cleanup for an object which is not derived from CBase
        * @param aObject -object to be cleaned up
        * @return void
        */
        static void Cleanup( TAny* aObject );

        /** 
        * Handles cleanup for an object which is not derived from CBase
        * @param aObject -object to be cleaned up
        * @return void
        */
        static void CleanupRefLst( TAny* aObject );
        
        /**
        * Deallocates memory from the array.
        * @return void
        */
        void DeallocWimCertInfo();   
        
        /**
        * Sets own iStatus to KRequestPending, and signals it 
        * with User::RequestComplete() request. This gives chance 
        * to active scheduler to run other active objects. After a quick
        * visit in actives cheduler, signal returns to RunL() and starts next
        * phase of operation. 
        * @return void
        */
        void SignalOwnStatusAndComplete();
        
        /** 
        * Allocates variables when retrieving extra certificate data 
        * from WimServer.
        * @return void
        */
        void AllocMemoryForExtraDataRetrieveL();

        /* 
        * DeAllocates variables after extra data retrieving operation.
        * @return void
        */
        void DeAllocMemoryForExtraDataRetrieve();

        /* 
        * DeAllocates member variable RArrays.
        * @return void
        */
        void DeallocRArrays();

    private: // data

        /**
        * Different states of this state machine
        */
        enum TPhase
            {
            EConnectClientSession,
            ECreateNewSession,
            EInitializeWim,
            EListCertsFromWim,
            EGetCertClientSession,
            ELocateCertificate,
            EGiveExtraData
            };

        //Client status is stored here while operation
        //on the server side is done.
        TRequestStatus*                 iClientStatus;
                
        //Handle to connection with server. Owned.
        RWimCertMgmt*                   iConnectionHandle;

        // Pointer to Client Session. Owned. 
        // Valid as long as CWimSecModuleMgr is valid.
        RWimMgmt*                       iClientSession;
        
        //Used to handle different phases.
        TPhase                          iPhase;
                        
        //The count of certificates
        TUint8                          iCertCount;
        
        //Buffer, which is used when fetching trustedusage from Server. Owned
        HBufC*                          iCertHBufTrustedUsageFromServer;
    
        //Pointer, which used to point iCertHBufTrustedUsageFromServer. Owned
        TPtr*                           iCertHBufTrustedUsageFromServerPtr;

         //Array which is filled with trusted usage strings. Not owned.
        RCPointerArray<HBufC>*          iTrustedUsages;

        // Informs the certificate location to caller.
        // Used when user has called GetExtras
        TCertLocation*                  iLocation;

        //The KeyId, which is received from caller. Used when user has called
        //GetExtras
        HBufC8*                         iKeyIdToBeMatched;
        
        //This tells the index of the certificate, from which we want
        //the trusted usages taken out.
        TUint8                          iCertIndex;

        //Flag to indicate errors during WIM -initialization.
        TInt                            iWimStartErr;

        //Array for certificate type. 
        RArray<TUint>                   iCertTypes;

        //Array for trusted usage lengths
        RArray<TUint>                   iTrustedUsageLengths;

        //Arrray for usages
        RArray<TUint>                   iUsages;

        //Array for KeyIds.Owned
        RArray<HBufC8*>                 iKeyIds;

        //Used to point right keyId when retrieving extra data
        TPtr8*                          iKeyIdPointer;

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
    };

#endif  // WIMCERT_H

// End of File
