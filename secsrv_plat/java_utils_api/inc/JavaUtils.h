/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies). 
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


#ifndef JAVAUTILS_H
#define JAVAUTILS_H

//INCLUDES
#include <e32base.h>

const TInt   KPathMaxLength                = 8;
const TInt   KLabelMaxLength               = 24;
const TInt   KPINMaxLabelLength            = 64;

typedef TBuf<KPINMaxLabelLength> TPINLabel;

//FORWARD DECLARATIONS
class RWimCertMgmt;
class RWimMgmt;


struct TJavaPINParams
	{
	TPINLabel iPINLabel;  
	TInt      iFlags;
	TInt      iAuthId;
	TInt      iPinNumber;
	TInt      iMinLength;
	TInt      iMaxLength;
	TInt      iPinType;
	TInt      iStoredLength;
	TInt      iPinReference;
	TInt      iPadChar;
	};
	
/**
*  Reads Java provisioning information from Smart Card
*
*  @lib WimClient.lib
*  @since S60 3.2
*/
class CWimJavaUtils : public CActive
    {
    public:

        /**
        * Two-phased constructor.
        *
        * @param
        * @return
        */
        IMPORT_C static CWimJavaUtils* NewL();

        /**
        * Destructor
        */
        IMPORT_C virtual ~CWimJavaUtils();
        
        /**
        * Initializes the WIM Server cache. This is the most time consuming call. 
        *
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
        *
        * @param
        * @return void
        */
        IMPORT_C void CancelInitialize();

        /**
        * Fetches the size of ACIF. 
        * This information can be used as a notification of existing of provisioning 
        * information. If the size is not included in Smart Card's
        * DODF-JavaPROV file then the full length of the file containing the 
        * provisioning information is returned.
        *
        * @param  aSize    The length of ACIF
        * @param  aStatus  Indicates the result of this call.
        *         KErrNone, no errors. NOTE: size can still be 0 indicating
        *                   that no provisioning information was found.
        *                   If aSize > 0 it means that provisioning information
        *                   was found.
        *         KErrNoMemory, No memory
        *         KErrGeneral, any other error
        * @return void
        */
        IMPORT_C void GetACIFSize( TInt& aSize,
                                   TRequestStatus& aStatus );
        
        /**
        * Fetches the size of ACF
        *
        * @param aSize  The lenght of ACF
        * @param aACFPath The path of ACF on smart card
        * @param  aStatus  Indicates the result of this call.
        * @return void
        */
        IMPORT_C void GetACFSize( TInt& aSize, 
                                  const TDesC8& aACFPath,
                                  TRequestStatus& aStatus );
                                  
        /**
        * Get the Authobj information
        *
        * @param aAuthIdList List of AuthObject ID
        * @param aAuthObjsInfoList List of AuthObject Info
        * @param  aStatus  Indicates the result of this call.
        * @return void 
        */
        IMPORT_C void GetAuthObjsInfo( const RArray<TInt>& aAuthIdList, 
                                       RArray<TJavaPINParams>& aAuthObjsInfoList,
                                       TRequestStatus& aStatus );
                                  
        /** 
        * Cancels outgoing GetACIFSize operation.
        *
        * @param 
        * @return void
        */
        IMPORT_C void CancelGetACIFSize();
        
        /** 
        * Cancels outgoing GetACFSize operation.
        *
        * @param
        * @return void
        */
        IMPORT_C void CancelGetACFSize();
        
        /**
        * Cancel outgoing GetAuthObjsInfo
        *
        * @param 
        * @return void
        */
        IMPORT_C void CancelGetAuthObjsInfo();
         
        /**
        * Fetches the whole data of ACIF file. 
        *
        * @param  aACIFData  ACIF binary data from smart card
        * @param  aStatus  Indicates the result of this call.
        *         KErrNone, no errors.
        *         KErrNoMemory, No memory
        *         KErrGeneral, any other error
        * @return void
        */
        IMPORT_C void RetrieveACIFContent( TDes8& aACIFData,
                                           TRequestStatus& aStatus );
                                           
        
       /**
        * Fetches the whole data of ACF file
        *
        * @param aACFData ACF binary data from smart card
        * @param aACFPath ACF file path on smart card
        * @param  aStatus  Indicates the result of this call.
        * @return void
        */
        IMPORT_C void RetrieveACFContent( TDes8& aACFData,
                                           const TDesC8& aACFPath,
                                           TRequestStatus& aStatus );
                                           
        /**
        * Fetches the wim label and path
        *
        * @param aWimLabel 
        * @param aPath
        * @param  aStatus  Indicates the result of this call.  
        * @return void 
        */
        IMPORT_C void RetrieveWimLabelAndPath( TDes8& aWimLabel,
                                               TDes8& aPath,
                                               TRequestStatus& aStatus );                                   
                                           
        
        /** 
        * Cancels outgoing Retrieve operation.
        *
        * @return void
        */
        IMPORT_C void CancelRetrieveACIFContent();
        
        /** 
        * Cancels outgoing Retrieve operation.
        *
        * @return void
        */
        IMPORT_C void CancelRetrieveACFContent();
        
        /**
        * Cancel outgoing Retrieve operation
        *
        * @return void
        */
        IMPORT_C void CancelRetrieveWimLabelAndPath();

    private:

        /**
        * Symbian constructor
        * @return void
        */
        void ConstructL();

        /**
        * C++ default constructor.
        */
        CWimJavaUtils();

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
        * in Java Provisioning struct.
        * @param aDataLength -The length of the data to be allocated
        * @return void
        */
        void AllocMemoryForJavaProvStructL( const TInt aDataLength );
        
        /** 
        * Allocates memory for path and label
        * @return void
        */
        void AllocMemoryForPathAndLabelL();
        /** 
        * Deallocates memory from member variables
        * @return void
        */
        void DeallocMemoryFromJavaProvStruct();
        
        /** 
        * Deallocates memory from path and label
        * @return void
        */
        void DeallocMemoryFromPathAndLabel();
        
    private:

        enum TJavaUtilPhase
            {
            EInitialize,
            EConnectClientSession,
            EInitializeWim,
            EInitializeDone,
            EGetACIFSize,
            EGetACIFSizeDone,
            ERetrieveACIFContent,
            ERetrieveACIFContentDone,
            EGetACFSize,
            EGetACFSizeDone,
            ERetrieveACFContent,
            ERetrieveACFContentDone,
            EGetAuthObjsInfo,
            EGetAuthObjsInfoDone,
            ERetrieveWimLabelAndPath,
            ERetrieveWimLabelAndPathDone
            };

        //Indicator for different phases
        TJavaUtilPhase                  iPhase;
        
        //Client status is stored here while operation
        //on the server side is done.
        TRequestStatus*                 iClientStatus;
        
        //Handle to connection with server. Owned.
        RWimCertMgmt*                   iConnectionHandle;

        // Pointer to Client Session. Owned. 
        RWimMgmt*                       iClientSession;

        //Informs the size of provisioning string. Not owned.
        TInt*                           iSize;
        
        //Descriptor to hold provisioning data. Not owned.
        TDes8*                          iData;

        //Buffer to hold Java provisioning data for server. Owned.
        HBufC8*                         iJavaProvBuf;
    
        //Pointer to iJavaProvBuf. Owned.
        TPtr8*                          iJavaProvBufPtr;
        
        //Flag to indicate errors during WIM -initialization.
        TInt                            iWimStartErr;
        
        TBuf8<KPathMaxLength>           iACFPath;
        
        RArray<TInt>*                   iAuthIdList; //owed
         
        //saves pointer to user's array
		RArray<TJavaPINParams>*         iClientsAuthObjsInfoList;//not owned
		                                
		//Array to hold authOjbectInfo data
        RArray<TJavaPINParams>*         iAuthObjsInfoList; //owed
        
        TDes8*                          iLabel;
        
        TDes8*                          iPath;
        
        //Buffer to hold Label. Owned.
        HBufC8*                         iLabelBuf;
    
        //Pointer to Label. Owned.
        TPtr8*                          iLabelBufPtr;
        
        //Buffer to hold path. Owned.
        HBufC8*                         iPathBuf;
    
        //Pointer to path. Owned.
        TPtr8*                          iPathBufPtr;
        
        
    };

	
#endif  // JAVAUTILS_H

// End of File
