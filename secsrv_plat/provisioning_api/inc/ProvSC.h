/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Reads provisioning information from Smart Card 
*
*/


#ifndef PROVSC_H
#define PROVSC_H

//INCLUDES
#include <e32base.h>


//FORWARD DECLARATIONS
class RWimCertMgmt;
class RWimMgmt;


//TYPE DEFINITIONS
enum TOMAType
    {
    EBootStrap,
    EConfig1,
    EConfig2
    };


/**
*  Reads OMA Provisioning information from Smart Card
*
*  @lib WimClient
*  @since Series60 2.6
*/
class CWimOMAProv : public CActive
    {
    public:

        /**
        * Two-phased constructor.
        */
        IMPORT_C static CWimOMAProv* NewL();

        /**
        * Destructor
        */
        IMPORT_C virtual ~CWimOMAProv();
        
        /**
        * Initializes the WIM Server cache. This is the most time consuming call. 
        * @param  aStatus  Indicates the result of this call. Values:
        *         KErrNone, call was successull
        *         KErrNotFound, no WIM was found
        *         KErrNoMemory, no memory
        *         KErrGeneral, any other error
        * @return void
        */
        IMPORT_C void Initialize( TRequestStatus& aStatus );

        /** 
        * Cancels outgoing Initialize operation. 
        * @return void
        */
        IMPORT_C void CancelInitialize();

        /**
        * Fetches the size of provisioning information string. 
        * This information can be used as a notification, does provisioning 
        * information exist. If the size is not included in Smart Card's
        * DODF-PROV file then the full length of the file containing the 
        * provisioning information is returned.
        * @param  aSize (OUT) The length of provisioning string
        * @param  aOMAType (IN) OMA provisioning file type 
        *         Pseudo values:
        *         -Bootstrap
        *         -Config 1
        *         -Config 2
        * @param  aStatus  Indicates the result of this call.
        *         KErrNone, no errors. NOTE: size can still be 0 indicating
        *                   that no provisioning information was found.
        *                   If aSize > 0 it means that provisioning information
        *                   was found.
        *         KErrNoMemory, No memory
        *         KErrGeneral, any other error
        * @return void
        */
        IMPORT_C void GetSize( TInt& aSize,
                               const TOMAType& aOMAType,
                               TRequestStatus& aStatus );

        /** 
        * Cancels outgoing GetSize operation.
        * @return void
        */
        IMPORT_C void CancelGetSize();

        /**
        * Fetches the whole data of provisioning information string. 
        * @param  aOMAType (IN) The type of provisioning string
        * @param  aOMAData (OUT) provisioning  binary data
        * @param  aStatus  Indicates the result of this call.
        *         KErrNone, no errors.
        *         KErrNoMemory, No memory
        *         KErrGeneral, any other error
        * @return void
        */
        IMPORT_C void Retrieve( const TOMAType& aOMAType, 
                                TDes8& aOMAData,
                                TRequestStatus& aStatus );

        /** 
        * Cancels outgoing Retrieve operation.
        * @return void
        */
        IMPORT_C void CancelRetrieve();

    private:

        /**
        * Symbian constructor
        * @return void
        */
        void ConstructL();

        /**
        * C++ default constructor.
        */
        CWimOMAProv();

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
        * @param aError -The error code which caused this function call.
        * @return TInt  -Error code to active scheduler, is always KErrNone.
        */ 
        TInt RunError( TInt aError );

    private:

        /** 
        * Sets own iStatus to KRequestPending, and signals it 
        * with User::RequestComplete() -request. This gives chance 
        * active scheduler to run other active objects. After a quick
        * visit in active scheduler, signal returns to RunL() and starts next
        * phase of operation. 
        * @return void
        */
        void SignalOwnStatusAndComplete();

        /** 
        * Allocates memory for member variables, which are needed
        * in OMA Provisioning struct.
        * @param aDataLength -The length of the data to be allocated
        * @return void
        */
        void AllocMemoryForOmaProvStructL( const TInt aDataLength );

        /** 
        * Deallocates memory from member variables
        * @return void
        */
        void DeallocMemoryFromOmaProvStruct();

    private:

        enum TOMAPhase
            {
            EInitialize,
            EConnectClientSession,
            EInitializeWim,
            EInitializeDone,
            EGetSize,
            EGetSizeDone,
            ERetrieve,
            ERetrieveDone
            };

        //Indicator for different phases
        TOMAPhase                       iPhase;
        
        //Client status is stored here while operation
        //on the server side is done.
        TRequestStatus*                 iClientStatus;
        
        //Handle to connection with server. Owned.
        RWimCertMgmt*                   iConnectionHandle;

        // Pointer to Client Session. Owned. 
        RWimMgmt*                       iClientSession;
        
        //Informs the type:EBootStrap, EConfig1, EConfig2
        TOMAType                        iOMAType;

        //Informs the size of provisioning string. Not owned.
        TInt*                           iSize;
        
        //Descriptor to hold provisioning data. Not owned.
        TDes8*                          iData;

        //Buffer to hold provisioning data for server. Owned.
        HBufC8*                         iOmaProvBuf;
    
        //Pointer to iOmaProvBuf. Owned.
        TPtr8*                          iOmaProvBufPtr;
        
        //Flag to indicate errors during WIM -initialization.
        TInt                            iWimStartErr;
    };

#endif  // PROVSC_H

// End of File
