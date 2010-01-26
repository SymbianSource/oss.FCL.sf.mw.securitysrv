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
* Description: Utility to perform gba bootstrapping
*
*/


#ifndef GBAUTILITY_H
#define GBAUTILITY_H

#include <e32base.h>

// FORWARD DECLARATION
class CGbaClientRequestHandler;

//CONST
const TInt KMaxBTIDLength=0xff;
const TInt KMaxKNAFLength=0xff;
const TInt KMaxURLLength=0xff;
const TInt KMaxRANDLength=0xff;
const TInt KMaxKIMPILength=0xff;
const TInt KMaxUICCLabel=0x20;
const TInt KMaxIMaterial=0xff;
const TInt KMaxKeyUsage=0x05;

_LIT8(KUSIM,"USIM");
_LIT8(KSIM,"SIM"); //Not support yet
_LIT8(KISIM,"ISIM"); //Not support yet

enum EGBABootstrapFlags
    {
    // Use the cached if not expired
    EGBADefault = 0,
    // Do not use cached credentials, instead force bootraps with BSF
    EGBAForce        
    };

// GBA_U 
enum EGBARunType
    {
    ENoType = 0,
    E2GGBA,     //2G GBA-specific functions are carried out in the ME
    E3GGBAME,   //3G GBA-specific functions are carried out in the ME
    EGBAU       //The UICC is GBA aware
    };

//Structure of Gba input
typedef struct
    {
    //Input, FQDN of NAF
    TBuf8<KMaxURLLength> iNAFName;
    //Input, EGBADefault or EGBAForces
    TUint iFlags;
    //Input, the label of UICC apps that user wants to use
    //KUSIM for example, if empty use default UICC apps
    TBuf8<KMaxUICCLabel> iUICCLabel;                            //Currently not supported.
    //Input, key usage identifier appended into NafName, for key derivation
    TBuf8<KMaxKeyUsage> iProtocolIdentifier;
    //the id of prefered access point for bootstrap, if no prefered one, set to -1.
    TInt iAPID;
    } TGBABootstrapInputParams;
    

//Structure of Gba output credentials
typedef struct
    {
    //Output B-TID
    TBuf8<KMaxBTIDLength>  iBTID;
    //Output Ks_NAF
    TBuf8<KMaxKNAFLength>  iKNAF;
    //Output lifetime
    TTime   iLifetime;               
    //Output
    TBuf8<KMaxKIMPILength> iKIMPI;         
    EGBARunType            iGbaRunType;
    //output, type of uicc application
    TBuf8<KMaxUICCLabel> iOutputUICCLabel;
    } TGBABootstrapOutputParams;
    
    
    
/**
* Observer class for gba client
*
* @lib gba2.lib
* @since S60 3.2
*
*/  
class MGbaObserver
    {
    public:
        
        /**
        * Callback function
        * When bootstrap is done and credentials are ready, this function
        * will be called.
        *
        * @since S60 3.2
        * @return error: KErrNone in case of success bootstrap request                        
        *                otherwise any of the standard Symbian error codes.                
        */
	      virtual void BootstrapComplete(TInt error) = 0;

    };


// CLASS DECLARATION
/**
 * Interface for handling gba bootstrapping
 *
 * @lib gba2.lib
 * @since S60 3.2
 *
*/  
class CGbaUtility : public CBase
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
        IMPORT_C  static CGbaUtility* NewL(MGbaObserver& aObserver);

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
        IMPORT_C static CGbaUtility* NewLC(MGbaObserver& aObserver) ;
        
        
        /**
        * NewL factory function without observer. User should create its own state 
        * machine.
        *
        * @since S60 3.2
        * @param 
        *
        * @return
        */
        IMPORT_C  static CGbaUtility* NewL();

        /**
        * NewLC factory function without observer. User should create its own state 
        * machine.
        *
        * @since S60 3.2
        * @param 
        *
        * @return
        */
        IMPORT_C static CGbaUtility* NewLC() ;

        
        /**
        * destructor
        */
        virtual ~CGbaUtility();

    public:
    
        /**
        * Bootstrap function. It is a asynchronous function with callback function.
        * Performs a bootstrap and calls the callback function when bootstrap is done.
        * This function should be used when the instance of CGbaUtility is 
        * created by factory function with observer.
        * This function requires ReadDeviceData capability
        *
        * @since S60 3.2
        * @param aInput - structure to hold input  parameters,
        *        aOutput - structure to hold output  parameters 
        *        client must ensure that ouput buffer is available when boostrapping is completed
        *
        * @return KErrNone factory function and bootstrap match
        *         KErrGeneral factory fucntion and boostrap function doesnot match
        *         KErrInUse	if there any outstanding bootstrap request in place
        */
        IMPORT_C TInt Bootstrap( const TGBABootstrapInputParams aInput, TGBABootstrapOutputParams& aOutput);
        
        /**
        * Performs asynchronous bootstrap.
        * This function should be used when the instance of CGbaUtility is 
        * created by factory function without observer.
        * This function requires ReadDeviceData capability
        *
        * @since S60 3.2
        * @param aInput - structure to hold input  parameters,
        * aOutput - structure to hold output  parameters 
        * client must ensure that ouput buffer is available when boostrapping is completed
        *        aStatus  - KErrNone - in case the bootstrap request is completed successfully
        *                 - KErrCancel - in case the bootstrap request is cancelled
        *                 - KErrInUse - if there any outstanding bootstrap request in place
        */
        IMPORT_C void Bootstrap( const TGBABootstrapInputParams aInput, TGBABootstrapOutputParams& aOutput, TRequestStatus& aStatus);
                
        /**
        * Cancel Bootstrap function
        *
        * @since S60 3.2
        * @param 
        * @return KErrNone if Cancel bootstrap request is success,otherwise return a standard symbian error.
        */
        IMPORT_C TInt CancelBootstrap();
            
        
        /**
         * Set the BSF address.
         * required WriteDeviceData capability.
         * The BSF address set by this function will overwrite
         * the one calculated from IMPI.
         * @since S60 3.2
         * @param aNewVal contain the address
         * @return KErrPermissionDenied, not enought capability
         * */
        IMPORT_C TInt SetBSFAddress( const TDesC8& aAddress );
      
    
    private:
      
      CGbaUtility();
      
      void ConstructL( MGbaObserver& aObserver );
      
      void ConstructL();
      
    private:
      CGbaClientRequestHandler* iGbaRequestHandler;
    
    };

#endif //GBAUTILITY_H
//EOF
