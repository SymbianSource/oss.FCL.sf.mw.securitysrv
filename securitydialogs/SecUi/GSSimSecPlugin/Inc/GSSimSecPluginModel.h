/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Model for Device & SIM security plug-in.
*
*/


#ifndef GSSIMSECPLUGINMODEL_H
#define GSSIMSECPLUGINMODEL_H

// INCLUDES
#include <e32base.h>
#include <centralrepository.h>

// CONSTANTS
const TInt KGSBufSize128 = 128;

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DEFINITION
/**
*  CGSSimSecPluginModel is the model class of device & sim security plugin.
*  It provides functions to get and set setting values.
*  @lib GSSimSecPlugin.lib
*  @since Series 60_3.1

*/
class CGSSimSecPluginModel : public CBase
    {
    public:  // Constructor and destructor
        /**
        * Two-phased constructor
        */
        static CGSSimSecPluginModel* NewL();

        /**
        * Destructor
        */
        ~CGSSimSecPluginModel();
        
    public:
        /**
        * Returns the autolock period time (minutes). 
        *
        * @return TInt: period
        */
        TInt AutoLockPeriod();

        /**
        * Sets the autolock period. After this period device is locked.
        * @param aLockTime TInt (minutes)
        *
        * @return ETrue: no errors
        *         EFalse: an error has occurred
        */
        TBool SetAutoLockPeriod( const TInt aLockTime );

        /**
        * Returns SAT operations sate
        *
        * @return TInt 0: SAT operations off
        *         TInt 1: SAT operations on
        */
        TInt SatOperations();

        /**
        * Sets SAT operations on/off
        * @param aValue TInt (0 = off 1 =on)
        *
        * @return ETrue: no errors
        *         EFalse: an error has occurred
        */
        TBool SetSatOperations( const TInt aValue );

        /**
        * Checking if SAT operations supported
        * @return:
        * 0: not supported
        * 1: supported
        */
        TInt ConfirmSatOperationsSupport();

    private: // Private constructors
        /**
        * Default C++ contructor
        */
        CGSSimSecPluginModel();

        /**
        * Symbian OS default constructor
        * @return void
        */
        void ConstructL();
        
    private: // data
        CRepository* iSecurityRepository;
        CRepository* iPersonalizationRepository;
    
    };


#endif //GSSIMSECPLUGINMODEL_H

// End of File