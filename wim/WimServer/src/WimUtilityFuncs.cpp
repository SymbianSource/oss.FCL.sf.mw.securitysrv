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
* Description:  Implementation of the CWimUtilityFuncs class.
*
*/



// INCLUDE FILES
#include    "WimUtilityFuncs.h"
#include    "Wimi.h"                // WIMI definitions
#include    "WimDefs.h" 
#include    "WimTrace.h"

#include    <e32property.h>         // RProperty

#ifdef RD_STARTUP_CHANGE
#include    <startupdomainpskeys.h> // Property values
#else
#include    <PSVariables.h>         // Property values
#endif // RD_STARTUP_CHANGE

#ifdef BT_SAP_TEST_BY_CHARGER
#include    <PSVariables.h>         // Property values
#else
//#include    <BTSapInternalPSKeys.h> // BT Sap property values
#include    <BTSapDomainPSKeys.h>
#endif // BT_SAP_TEST_BY_CHARGER

//If want to use emulator hw, uncomment this line
#define __EMULATOR_HW__ 

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimUtilityFuncs::CWimUtilityFuncs
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimUtilityFuncs::CWimUtilityFuncs()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimUtilityFuncs::CWimUtilityFuncs | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimUtilityFuncs::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimUtilityFuncs::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimUtilityFuncs::ConstructL | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimUtilityFuncs::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimUtilityFuncs* CWimUtilityFuncs::NewL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimUtilityFuncs::NewL | Begin"));
    CWimUtilityFuncs* self = new( ELeave ) CWimUtilityFuncs;
    
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    return self;
    }

    
// Destructor
CWimUtilityFuncs::~CWimUtilityFuncs()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimUtilityFuncs::~CWimUtilityFuncs | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimUtilityFuncs::::MapWIMError
// Map WIMI errors to WIM errors
// -----------------------------------------------------------------------------
//
TInt CWimUtilityFuncs::MapWIMError( WIMI_STAT aStatus )
    {
    _WIMTRACE2(_L("WIM | WIMServer | CWimUtilityFuncs::MapWIMError aStatus=%d"), aStatus);
    TInt err = KErrUnknown; 
    
    switch ( aStatus ) 
        {
        case WIMI_Err:
        case WIMI_ERR_Internal:
             err = KErrCorrupt;
            break;
        case WIMI_ERR_OutOfMemory:
            err = KErrNoMemory;
            break;
        case WIMI_Ok:
            err = KErrNone;
            break;
        case WIMI_ERR_BadParameters:
            err = KErrArgument;
            break;
        case WIMI_ERR_ServerCertRejected:
            err = KErrCorrupt;
            break;
        case WIMI_ERR_CipherNotSupported:
        case WIMI_ERR_MACANotSupported:
            err = KErrNotSupported;
            break;
        case WIMI_ERR_UnsupportedCertificate:
            err = KErrNotSupported;
            break;
        case WIMI_ERR_SessionNotSet:
            err = KErrDisconnected;
            break;
        case WIMI_ERR_WrongKES:
            err = KErrCorrupt;
            break;
        case WIMI_ERR_DecodeError:
            err = KErrCorrupt;
            break;
        case WIMI_ERR_ExpiredReference:
            err = KErrArgument;
            break;
        case WIMI_ERR_NoKey:
            err = KErrNotFound;
            break;
        case WIMI_ERR_CertNotYetValid:
            err = KErrCorrupt;
            break;
        case WIMI_ERR_CertExpired:
            err = KErrTimedOut;
            break;
        case WIMI_ERR_UnknownCA:
            err = KErrArgument;
            break;
        case WIMI_ERR_CertParseError:
            err = KErrCorrupt;
            break;
        case WIMI_ERR_KeyStorageFull:
            err = KErrDirFull;
            break;
        case WIMI_ERR_BadKey:
            err = KErrArgument;
            break;
        case WIMI_ERR_CertStorageFull:
            err = KErrDirFull;
            break;
        case WIMI_ERR_BadCert:
            err = KErrArgument;
            break;
        case WIMI_ERR_PStorageError:
            err = KErrDirFull;
            break;
        case WIMI_ERR_CertNotFound:
            err = KErrNotFound;
            break;
        case WIMI_ERR_KeyNotFound:
            err = KErrNotFound;
            break;
        case WIMI_ERR_BadReference:
            err = KErrCorrupt;
            break;
        case WIMI_ERR_OperNotSupported:
            err = KErrNotSupported;
            break;
        case WIMI_ERR_BadPIN:
            err = KErrArgument;
            break;
        case WIMI_ERR_PINBlocked:
            err = KErrLocked;
            break;
        case WIMI_ERR_CardDriverInitError:
            err = KErrBadDriver;
            break;
        case WIMI_ERR_CardIOError:
            err = KErrHardwareNotAvailable;
            break;
        case WIMI_ERR_AlgorithmNotYetImplemented:
            err = KErrNotSupported;
            break;
        case WIMI_ERR_UserCancelled:
            err = KErrCancel;
            break;
        default:
            err = KErrUnknown;
        }
    return err;
    }

// -----------------------------------------------------------------------------
// CWimUtilityFuncs::TrIdLC
// Creates a new TWimReqTrId item.
// -----------------------------------------------------------------------------
//
TWimReqTrId* CWimUtilityFuncs::TrIdLC( TAny* aTrId, TWimReqType aReqType ) const 
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimUtilityFuncs::GetTrIdLC | Begin"));
    TWimReqTrId* reqTrId = new( ELeave ) TWimReqTrId;
    CleanupStack::PushL( reqTrId );
    reqTrId->iReqType = aReqType;
    reqTrId->iReqTrId = aTrId;
    return reqTrId;
    }

// -----------------------------------------------------------------------------
// CWimUtilityFuncs::DesLC
// Utility function that reads a descriptor from the clients address
// space. The caller has to free the returned HBufC8.
// -----------------------------------------------------------------------------
//
HBufC8* CWimUtilityFuncs::DesLC(
    const TInt aIndex,
    const RMessage2& aMessage ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimUtilityFuncs::GetDesLC | Begin"));

    TInt len = aMessage.GetDesLength( aIndex );    

    //User might cancelled operation, so we need to leave before
    //HBufC panics. We do not want panic at this point. Leave is Trapped in 
    //session
    if ( len < 0 )
        {
        User::Leave( KErrArgument );
        }

    HBufC8* buf = HBufC8::NewLC( len ); // free'd by caller
    TPtr8 ptr = buf->Des();
    aMessage.ReadL( aIndex, ptr );
    
    return buf;
    }

// -----------------------------------------------------------------------------
// CWimUtilityFuncs::SimState
// Get state of SIM/SWIM card. Uses system wide repository to get state.
// If card is OK return KErrNone, otherwise some other system wide state.
// -----------------------------------------------------------------------------
//
TInt CWimUtilityFuncs::SimState()
    {
    TInt simState = KErrNotReady;
    TInt stateError = 0;

    // Get SIM state

#ifdef RD_STARTUP_CHANGE
    stateError = RProperty::Get( KPSUidStartup,
                                 KPSSimStatus,
                                 simState );
#else
    stateError = RProperty::Get( KUidSystemCategory,
                                 KPSUidSIMStatusValue,
                                 simState );
#endif // RD_STARTUP_CHANGE
 
    // In WINS may not return real SIM state -> ignore
    #ifdef __WINS__
    stateError = 0;
    #endif

    if ( stateError == KErrNone ) // Got SIM state succesfully
        {
        // In WINS may not return real SIM state -> ignore
#ifdef __WINS__

#ifdef __EMULATOR_HW__

#ifdef RD_STARTUP_CHANGE
        simState = ESimUsable;
#else
        simState = EPSSimOk;
#endif // RD_STARTUP_CHANGE

#else  //__EMULATOR_HW__

#ifdef RD_STARTUP_CHANGE
        simState = ESimNotPresent;
#else
        simState = EPSSimNotPresent;
#endif // RD_STARTUP_CHANGE

#endif //__EMULATOR_HW__

#endif // __WINS__

        _WIMTRACE2(_L("WIM | WIMServer | CWimUtilityFuncs::SimState|simState=%d"),simState);

#ifdef RD_STARTUP_CHANGE
        if ( simState == ESimUsable )
#else
        if ( simState == EPSSimOk )
#endif // RD_STARTUP_CHANGE        
            {
            simState = KErrInUse;
#ifdef BT_SAP_TEST_BY_CHARGER
            // Test of BT Sap by charger. To emulate connected BT Sap, charger must be
            // connected and charging.
            stateError = RProperty::Get( KUidSystemCategory,
                                         KPSUidChargerStatusValue,
                                         simState );  
#else
            // Get state of BT Sap
            stateError = RProperty::Get( KPSUidBluetoothSapConnectionState,
                                         KBTSapConnectionState,
                                         simState );
#endif // BT_SAP_TEST_BY_CHARGER

       // In WINS may not return real SIM state -> ignore
#ifdef __WINS__
            stateError = 0;
#endif

            if ( KErrNone == stateError ) // Got BT SAP state succesfully
                {
       // In WINS may not return real SIM state -> ignore 
#ifdef __WINS__

#ifdef BT_SAP_TEST_BY_CHARGER
        simState = EPSChargerDisconnected;
#else
        simState = EBTSapNotConnected;
#endif // BT_SAP_TEST_BY_CHARGER

#endif // __WINS__            
               
                _WIMTRACE2(_L("WIM | WIMServer | CWimUtilityFuncs::SimState|simState=%d"),simState);
               
#ifdef BT_SAP_TEST_BY_CHARGER
                if ( EPSChargerConnected != simState )
#else
                if ( EBTSapConnected != simState )
#endif // BT_SAP_TEST_BY_CHARGER 
                    {
                    simState = KErrNone;
                    }
                else
                    {
                    simState = KErrInUse;
                    }               
                }
            else if ( KErrNotFound == stateError )
                {
                _WIMTRACE(_L("WIM | WIMServer | CWimUtilityFuncs::SimState|BTSap not found, WIM initialization continued"));
                simState = KErrNone;
                }
            else
                {
                _WIMTRACE2(_L("WIM | WIMServer | CWimUtilityFuncs::SimState|BTSap ERROR| stateError=%d"),stateError);    
                }
            }
        else // simState != ESimUsable/EPSSimOk
            {
            simState = KErrNotReady;
            }
        }
    _WIMTRACE2(_L("WIM | WIMServer | CWimUtilityFuncs::SimState|End|simState=%d"),simState); 
    return simState;
    }
// -----------------------------------------------------------------------------
// CWimCertHandler::MapCertLocation
// Map Wimlib certificate CDF to our own type
// -----------------------------------------------------------------------------
//
TUint8 CWimUtilityFuncs::MapCertLocation( const TUint8 aCertCDF ) const
    {
    TWimCertificateCDF certCDF;

    switch ( aCertCDF )
        {
        case WIMLIB_CERTIFICATES_CDF:
            {
            certCDF = EWimCertificatesCDF;
            break;
            }
        case WIMLIB_TRUSTEDCERTS_CDF:
            {
            certCDF = EWimTrustedCertsCDF;
            break;
            }
        case WIMLIB_USEFULCERTS_CDF:
            {
            certCDF = EWimUsefulCertsCDF;
            break;
            }
        default:
            {
            certCDF = EWimUnknownCDF;
            break;
            }
        }
    return ( TUint8 )certCDF;
    }

//  End of File  
