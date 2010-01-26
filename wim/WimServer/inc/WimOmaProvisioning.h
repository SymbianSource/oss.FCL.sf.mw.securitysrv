/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  WIM OMA Provisioning handler. 
*
*/



#ifndef CWIMOMAPROVISIONING_H
#define CWIMOMAPROVISIONING_H

//  INCLUDES
#include "Wimi.h"    // WIMI functions and definitions
#include "ProvSC.h"  // OMA Provisioning Smart Card API types

// CONSTANTS
const TUint8 KWimProvBootstrapFileOID[]      = {0x67, 0x2B, 0x05, 0x01};
const TUint8 KWimProvConfig1FileOID[]        = {0x67, 0x2B, 0x05, 0x02};
const TUint8 KWimProvConfig2FileOID[]        = {0x67, 0x2B, 0x05, 0x03};
const TInt   KWimProvFileOidLength           = 4;

const TUint8 KWimProvBootstrapFileOIDTaged[] =
    {0x06, 0x04, 0x67, 0x2B, 0x05, 0x01};
const TUint8 KWimProvConfig1FileOIDTaged[]   =
    {0x06, 0x04, 0x67, 0x2B, 0x05, 0x02};
const TUint8 KWimProvConfig2FileOIDTaged[]   = 
    {0x06, 0x04, 0x67, 0x2B, 0x05, 0x03};
const TInt   KWimProvFileOidLengthTaged      = 6;

const TInt   KWimOidLengthTaged              = 14;

const TInt KFilePathMaxLength                = 8;

// FORWARD DECLARATION
class CWimUtilityFuncs;

// CLASS DECLARATION

/**
*  Class for handling OMA Provisioning files.
*  
*  @since Series60 2.6
*/
class CWimOmaProvisioning : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimOmaProvisioning* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CWimOmaProvisioning();

    public: // New functions
        
        /**
        * Write size of OMA provisioning file to client memory
        * @param aMessage Client message
        * @return void
        */
        void OmaFileSizeL( const RMessage2& aMessage );

        /**
        * Write content of OMA provisioning file to client memory
        * @param aMessage Client message
        * @return void
        */
        void OmaFileContentL( const RMessage2& aMessage );
        
    private:

        /**
        * C++ default constructor.
        */
        CWimOmaProvisioning();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Initialize path and size member variables with values got from WIMI.
        * @return void
        */
        void InitializePathsAndSizes();

        /**
        * Parse size of OMA Provisioning file if not given in DODF-PROV file
        * @param aOmaType Type of file
        * @return void
        */
        void ParseFileSize( TOMAType aOmaType );

       
    private:    // Data
        // Reference to WIM. Not owned.
        WIMI_Ref_t*               iWimRef;
        
        // Path of bootstrap file. Length is 0 if not exists.
        TBuf8<KFilePathMaxLength> iPathBootstrap;
        
        // Path of config1 file. Length is 0 if not exists.
        TBuf8<KFilePathMaxLength> iPathConfig1;
        
        // Path of config2 file. Length is 0 if not exists.
        TBuf8<KFilePathMaxLength> iPathConfig2;
        
        // Size of Bootstrap file
        TInt                      iBootstrapFileSize;
        
        // Size of Config1 file
        TInt                      iConfig1FileSize;
        
        // Size of Config2 file
        TInt                      iConfig2FileSize;
        
        // Flag to tell if provisiong file infos are initialized
        TBool                     iFileInfosInitialized;
        
        // Pointer to CWimUtilityFuncs. Owned.
        CWimUtilityFuncs*         iWimUtilityFuncs;
    };

#endif      // CWIMOMAPROVISIONING_H
    
// End of File