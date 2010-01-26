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

#ifndef CWIMJAVAPROVISIONING_H
#define CWIMJAVAPROVISIONING_H

//  INCLUDES
#include "Wimi.h"    //WIMI functions and definitions
#include "JavaUtils.h"  //Java Provisioning Smart Card API types

// CONSTANTS
const TUint8 KWimProvACIFFileOID[]      = {0x01, 0x03, 0x06, 0x01, 0x04, 0x01, 0x2A, 0x02, 0x6E, 0x03, 0x01, 0x01, 0x01 };
const TInt   KWimProvACIFFileOidLength           = 13;

const TUint8 KWimProvACIFFileOIDTaged[] = {0x06, 0x0C, 0x2B, 0x06, 0x01, 0x04, 0x01, 0x2A, 0x02, 0x6E, 0x03, 0x01, 0x01, 0x01 };

const TInt   KWimProvACIFFileOidLengthTaged      = 14;

const TInt   KWimJavaOidLengthTaged              = 16;

const TInt   KJavaFilePathMaxLength                = 8;

// FORWARD DECLARATION
class CWimUtilityFuncs;

// CLASS DECLARATION

/**
*  Class for handling Java Provisioning files.
*  
*  @since Series60 3.2
*/
class CWimJavaProvisioning : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CWimJavaProvisioning* NewL();
        
        /**
        * Destructor.
        */
        virtual ~CWimJavaProvisioning();

    public: // New functions
        
        /**
        * Write size of ACIF file to client memory
        * @param aMessage Client message
        * @return void
        */
        void ACIFFileSizeL( const RMessage2& aMessage );
        
        void ACFFileSizeL( const RMessage2& aMessage );
        /**
        * Write content of ACIF file to client memory
        * @param aMessage Client message
        * @return void
        */
        void ACIFFileContentL( const RMessage2& aMessage );
        
        void ACFFileContentL( const RMessage2& aMessage );
        
        void LabelAndPath( const RMessage2& aMessage ); 
        
        
    private:

        /**
        * C++ default constructor.
        */
        CWimJavaProvisioning();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Initialize path and size member variables with values got from WIMI.
        * @return void
        */
        void InitializePathsAndSizes();
        
        void InitializeSize();
        /**
        * Parse size of ACIF file if not given in DODF-JavaProv file
        * @return void
        */
        void ParseACIFFileSize();

       
    private:    // Data
        // Reference to WIM. Not owned.
        WIMI_Ref_t*               iWimRef;
        
        // Path of bootstrap file. Length is 0 if not exists.
        TBuf8<KJavaFilePathMaxLength> iPathACIF;
        
        TBuf8<KJavaFilePathMaxLength> iPathACF;
        // Size of ACIF file
        TInt                      iACIFFileSize;
        
        TInt                      iACFFileSize;
        // Flag to tell if provisiong file infos are initialized
        TBool                     iFileInfosInitialized;
        
        // Pointer to CWimUtilityFuncs. Owned.
        CWimUtilityFuncs*         iWimUtilityFuncs;
    };

#endif      // CWIMJAVAPROVISIONING_H
    
// End of File