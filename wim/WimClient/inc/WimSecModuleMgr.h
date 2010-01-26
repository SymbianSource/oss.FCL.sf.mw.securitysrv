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
* Description:  API for listing all WIM Security Modules
*
*/


#ifndef WIMSECMODULEMGR_H
#define WIMSECMODULEMGR_H

//  INCLUDES
#include "WimClsv.h"
#include "WimSig.h"
#include <e32base.h>
#include <ct.h> 

// FORWARD DECLARATIONS
class CWimSecModule;
class RWimMgmt;


// CLASS DECLARATION
/**
*  API for list all WIM Security Modules
*  This API provides function to list all WIM Security Modules
*  which your device finds. Listed CWimSecModule objects provides 
*  methods to manage key stores. 
*
*  @lib WimClient
*  @since Series 60 2.1
*/
class CWimSecModuleMgr: public CActive
    {
    public:  
        
        /**
        * Two-phased constructor.
        */
        IMPORT_C static CWimSecModuleMgr* NewL(); 
         
        /**
        * Destructor.
        */
        IMPORT_C virtual ~CWimSecModuleMgr();

    public: // New functions
        
        /**
        * Returns all WIM Security Modules in array. 
        * @param  aTokens     Array containing all WIMs.(OUT)
        * @param  aStatus   Status from caller.         
        * @return void
        */
        IMPORT_C void GetEntries( RCPointerArray<HBufC>& aTokens, 
            TRequestStatus& aStatus );
        

        /**
        * Cancels listing operation. 
        * @return void
        */        
        IMPORT_C void CancelList();

        /**
        * Returns the number of WIM cards
        * If WIM is not initialized and listing operation completed
        * before calling WimCount(), this will return KErrNotFound.
        * @return TInt     aNumber of wims.
        */        
        IMPORT_C TInt WimCount();

        /**
        * Returns CWimSecModule according to received index.
        * If WIM is not initialized and listing operation completed
        * before calling this function, User::Leave( KErrNotFound )
        * is called.        
        * @param aIndex -needed element from array.
        * @return CWimSecModule*  -the element from array.
        */
        IMPORT_C CWimSecModule* GetWimSecModuleByIndexL( 
            const TInt aIndex );

        /** 
        * Returns WIM label according to received index.
        * If WIM is not initialized and listing operation completed
        * before calling this function, User::Leave( KErrNotFound )
        * is called.                
        * @param aIndex -wanted index from array.
        * @return TPtrC -pointer to label.
        */
        IMPORT_C TPtrC WimLabelByIndexL( const TInt aIndex );

        /** 
        * Returns the number of "slot" where current token is attached.
        * If WIM is not initialized and listing operation completed
        * before calling this function, User::Leave( KErrNotFound )
        * is called.                
        * @param aIndex -needed index from array.
        * @return TInt -the place where token is attached.
        */
        IMPORT_C TInt WimTokenNumberByIndexL( const TInt aIndex );

        
    private:
        
        /**
        * C++ default constructor.
        */
        CWimSecModuleMgr();

    
        /**
        * By default Symbian 2nd phase constructor is private
        */
        void ConstructL();

        /**
        * Frees memory & preferences that have been allocated.
        * @return void
        */
        void DeAllocateWimReferences();

    private: 
        

        /**
        * Sets own iStatus to KRequestPending, and signals it 
        * with User::RequestComplete() request. This gives chance 
        * to active scheduler to run other active objects. After a quick
        * visit in active scheduler, signal returns to RunL() and starts next
        * phase of listing operation. This makes possible the cancelling 
        * function to get through.
        * @return void
        */
        void SignalOwnStatusAndComplete();
        
        /**
        * Appends labels of available Tokens (wims) to an array.
        * @return void
        */
        void AppendTokenLabelsToArrayL();
        
        /**
        * Reads the count of the WIM.
        * @return void
        */  
        void ReadWimCount();
        
        /**
        * Five different phases, which are used to initialize
        * WIM and list security modules.
        */
        enum TPhase
            {
            EConnectClientSession,
            ECreateNewSession,
            EInitializeWim,
            EGetWimRefs
            };
    private: //From CActive

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
        * Handles neccessary cleanup and completes request with
        * received error code.
        * @param aError -Error code which caused this event
        * @return TInt  -Error code to active scheduler, is always KErrNone.
        */ 
        TInt RunError( TInt aError );

    private: 

        /**
        * Allocate pointer to WIM and create CWimSecModules
        * @return TInt -Error code
        */
        TInt WIMModulesL();


    private:

        //Used to handle initialization of WIM and listing tokentypes
        TPhase                          iPhase;
                
        //Client status is stored here while WIM is started and
        //listing operation is made. After that User::RequestComplete()
        //is signalled with iClientStatus.
        TRequestStatus*                 iClientStatus;
        
        //Array containing WIM labels. Not owned, caller is 
        //responsible to destroy received array.
        RCPointerArray<HBufC>*          iTokenLabels;
        
        //Flag to indicate errors during WIM initialization.
        TInt                            iWimStartErr;
        
        // Array of pointers to WIM Security Modules. Owned. 
        // Valid as long as CWimSecModuleMgr is valid.
        CArrayPtrFlat<CWimSecModule>*   iWims; 
        
        // Pointer to Client Session. Owned. 
        // Valid as long as CWimSecModuleMgr is valid.
        RWimMgmt*                       iClientSession;
        
        // WIM reference list. Owned. Valid as long as iClientSession
        // is valid.
        TWimAddressList                 iWimAddrLst;
        
        // WIM count, defines the amount of WIM cards.
        TUint                           iWimCount;
        
        TPhase                          iCurrentPhase;

    };

#endif      // WIMSECMODULEMGR_H   
            
// End of File
