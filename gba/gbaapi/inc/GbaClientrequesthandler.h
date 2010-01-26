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
* Description:  Interface for handling gba bootstrapping
*
*/


#ifndef GBACLIENTREQUESTHANDLER_H
#define GBACLIENTREQUESTHANDLER_H

#include <e32base.h>
#include "gbautility.h"

// CLASS DECLARATION
class RGbaServerSession;

/**
 * Interface for handling gba bootstrapping
 *
 * @lib gba2.lib
 * @since S60 3.2
 *
*/  
class CGbaClientRequestHandler : public CActive
    {
    public:
        
        /**
        * NewL factory function with an observer. It is used with callback function
        * which should be implemented by the client. The interface is defined in class
        * MGbaObserver
        *
        * @since S60 3.2
        * @param aObserver Client implemented observer that will be
        *                  called when the bootstrap is done
        * @return
        */
        static CGbaClientRequestHandler* NewL(MGbaObserver& aObserver);

        /**
        * NewLC factory function with an observer. It is used with callback function
        * which should be implemented by the client. The interface is defined in class
        * MGbaObserver
        *
        * @since S60 3.2
        * @param aObserver Client implemented observer that will be
        *                  called when the bootstrap is done
        * @return
        */
        static CGbaClientRequestHandler* NewLC(MGbaObserver& aObserver) ;
        
        
        /**
        * NewL factory function without observer. User should create its own state 
        * machine.
        *
        * @since S60 3.2
        * @param 
        *
        * @return
        */
        static CGbaClientRequestHandler* NewL();

        /**
        * NewLC factory function without observer. User should create its own state 
        * machine.
        *
        * @since S60 3.2
        * @param 
        *
        * @return
        */
        static CGbaClientRequestHandler* NewLC() ;

        
        /**
        * destructor
        */
        virtual ~CGbaClientRequestHandler();

    public:
    
        /**
        * Bootstrap function. It is a asynchronous function with callback function.
        * Performs a bootstrap and calls the callback function when bootstrap is done.
        * This function should be used when the instance of CGbaClientRequestHandler is 
        * created by factory function with observer.
        * This function requires ReadDeviceData capability
        *
        * @since S60 3.2
        * @param aInput/aOutput structures to hold input and output parameters, 
        *        client must ensure that buffer is available when boostrapping is completed
        *
        * @return KErrNone factory function and bootstrap match
        *         KErrGeneral factory fucntion and boostrap function doesnot match
        */
        TInt Bootstrap( const TGBABootstrapInputParams aInput, TGBABootstrapOutputParams& aOutput);
                
        /**
        * Asynchronous bootstrap function. Performs asynchronous bootstrap.
        * This function should be used when the instance of CGbaClientRequestHandler is 
        * created by factory function without observer.
        * This function requires ReadDeviceData capability
        *
        * @since S60 3.2
        * @param aInput/aOutput structures to hold input and output parameters, 
        *                 client must ensure that buffer is available when
        *                 boostrapping is completed
        *        aStatus 
        *
        * @return KErrNone factory function and bootstrap match
        *         KErrGeneral factory fucntion and boostrap function doesnot match
        */
        //IMPORT_C TInt Bootstrap(TGBABootstrapCredentials& aCredentials, TRequestStatus& aStatus );
        TInt Bootstrap( const TGBABootstrapInputParams aInput, TGBABootstrapOutputParams& aOutput, TRequestStatus& aStatus);
                
        /**
        * Cancel Bootstrap function
        *
        * @since S60 3.2
        * @param 
        * @return 
        */
       TInt CancelBootstrap();
          
      /**
        * Set the BSF address. 
        * required WriteDeviceData capability. 
        * The BSF address set by this function will overwrite
        * the one calculated from IMPI.
        *
        * @since S60 3.2
        * @param aNewVal contain the address
        *            
        * @return KErrPermissionDenied, not enought capability
        */
        TInt SetBSFAddress( const TDesC8& aAddress );
      
    protected: // from CActive
        
        void RunL();

        void DoCancel();

       TInt RunError(TInt aError);

    private:

        void ConstructL();

        CGbaClientRequestHandler(MGbaObserver& aObserver);
      
        CGbaClientRequestHandler();
    
    private: 

    enum TGbaState
        {
        EBootstrapNone = 0,
        EBootstrapping,
        EBootstrapCancelling
        };
        

    private:
  
        // the Gba server session 
        RGbaServerSession*  iSession;
        
        TGBABootstrapOutputParams* iGbaOutputParams;
        
        TGBABootstrapInputParams iGbaInputParams;
        
        //the observer of this class that is informed of Gba updated
        MGbaObserver* iObserver;
        
        //bootstrap state
        TGbaState iBootstrapState;
  
    };

#endif //GBACLIENTREQUESTHANDLER_H  


//EOF
