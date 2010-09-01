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


// INCLUDES
#include "GSSimSecPluginModel.h"

#include <settingsinternalcrkeys.h>

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES  

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS
const TInt KGSSettingOff = 0;
// default value for autolock period
const TInt KGSDefaultAutoLockTime = 0;

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// ============================= LOCAL FUNCTIONS ==============================

// ========================= MEMBER FUNCTIONS =================================

// ----------------------------------------------------------------------------
// CGSSimSecPluginModel::NewL
// 
// Symbian OS two-phased constructor
// ----------------------------------------------------------------------------
//
CGSSimSecPluginModel* CGSSimSecPluginModel::NewL()
    {
    CGSSimSecPluginModel* self = new( ELeave ) CGSSimSecPluginModel;
    CleanupStack::PushL( self );
    self->ConstructL();

    CleanupStack::Pop( self );
    return self;
    }


// ----------------------------------------------------------------------------
// CGSSimSecPluginModel::CGSSimSecPluginModel
// 
// 
// C++ default constructor can NOT contain any code, that might leave.
// ----------------------------------------------------------------------------
//
CGSSimSecPluginModel::CGSSimSecPluginModel()
    {
    
    }


// ----------------------------------------------------------------------------
// CGSSimSecPluginModel::ConstructL
// 
// Symbian OS default constructor can leave.
// ----------------------------------------------------------------------------
//
void CGSSimSecPluginModel::ConstructL()
    {
    iSecurityRepository = CRepository::NewL( KCRUidSecuritySettings );
    iPersonalizationRepository = 
        CRepository::NewL( KCRUidPersonalizationSettings );
    }


// ----------------------------------------------------------------------------
// CGSSimSecPluginModel::~CGSSimSecPluginModel
// 
// Destructor
// ----------------------------------------------------------------------------
//
CGSSimSecPluginModel::~CGSSimSecPluginModel()
    {
    delete iSecurityRepository;
    iSecurityRepository = NULL;

    delete iPersonalizationRepository;
    iPersonalizationRepository = NULL;
    }


// ----------------------------------------------------------------------------
// CGSSimSecPluginModel::AutoLockPeriod()
// 
// Reads Autolock period from .ini file and returns it
// ----------------------------------------------------------------------------
//
TInt CGSSimSecPluginModel::AutoLockPeriod()
    {
    TInt period = KGSSettingOff;
    
    if ( iSecurityRepository->
         Get( KSettingsAutoLockTime, period ) != KErrNone )
            {
            period = KGSDefaultAutoLockTime;
            iSecurityRepository->Set( KSettingsAutoLockTime, period );
            }
    
    return period;
    }
            
// ----------------------------------------------------------------------------
// CGSSimSecPluginModel::SetAutoLockPeriod
// 
// Writes Autolock period time to Cenrep
// ----------------------------------------------------------------------------
//
TBool CGSSimSecPluginModel::SetAutoLockPeriod( TInt aLockTime )
    {
    TInt ret = iSecurityRepository->Set( KSettingsAutoLockTime, aLockTime );
    
    return ret;
    }

// ----------------------------------------------------------------------------
// CGSSimSecPluginModel::SatOperations()
// 
// Reads SatOperations value from .ini file and returns it
// ----------------------------------------------------------------------------
//
TInt CGSSimSecPluginModel::SatOperations()
    {
    TInt value = KGSSettingOff;
    
    iPersonalizationRepository->Get( KSettingsConfirmSatOperations, value );
    
    return value;
    }


// ----------------------------------------------------------------------------
// CGSSimSecPluginModel::SetSatOperations
// 
// Write user changes to the .ini file
// ----------------------------------------------------------------------------
//
TBool CGSSimSecPluginModel::SetSatOperations( TInt aValue )
    {
    TInt ret = iPersonalizationRepository->
            Set( KSettingsConfirmSatOperations, aValue );
    
    return ret;
    }


// ----------------------------------------------------------------------------
// CGSSimSecPluginModel::ConfirmSatOperationsSupport
// 
// Get Confirm Sat Operations supported value
// ----------------------------------------------------------------------------
//
TInt CGSSimSecPluginModel::ConfirmSatOperationsSupport()
    {
    TInt ret = 0;
    iPersonalizationRepository->
        Get( KSettingsConfirmSatOperationsSupported, ret );
    
    return ret;
    }
  
// End of File
