/*
* Copyright (c) 2002-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:  Methods for token handling
*
*/



// INCLUDE FILES
#include    "WimTokenHandler.h"
#include    "WimMemMgmt.h"
#include    "WimClsv.h"
#include    "WimSession.h"
#include    "WimTimer.h"
#include    "WimSession.h"
#include    "WimUtilityFuncs.h"
#include    "Wimi.h"            //WIMI definitions
#include    "WimTrace.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimTokenHandler::CWimTokenHandler
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimTokenHandler::CWimTokenHandler()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTokenHandler::CWimTokenHandler | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimTokenHandler::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimTokenHandler::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTokenHandler::ConstructL | Begin"));
    iWimUtilFuncs = CWimUtilityFuncs::NewL();
    }

// -----------------------------------------------------------------------------
// CWimTokenHandler::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimTokenHandler* CWimTokenHandler::NewL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTokenHandler::NewL | Begin"));
    CWimTokenHandler* self = new( ELeave ) CWimTokenHandler;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// Destructor
CWimTokenHandler::~CWimTokenHandler()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTokenHandler::~CWimTokenHandler | Begin"));
    delete iWimUtilFuncs;
    }

// -----------------------------------------------------------------------------
// CWimTokenHandler::GetWIMInfoL
// Fetches WIM info from wimlib
// -----------------------------------------------------------------------------
//
void CWimTokenHandler::GetWIMInfoL(
    const RMessage2& aMessage,
    CWimMemMgmt* const aWimMgmt ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTokenHandler::GetWIMInfoL | Begin"));
    WIMI_Ref_pt pWimRef = const_cast<WIMI_Ref_pt>( aMessage.Ptr0() ); 
    
    __ASSERT_ALWAYS( pWimRef, User::Leave( KErrArgument ) );
    __ASSERT_ALWAYS( aWimMgmt, User::Leave( KErrArgument ) );
    if ( !aWimMgmt->ValidateWIMRefL( pWimRef ) )
        {
        _WIMTRACE(_L("WIM | WIMServer | CWimTokenHandler::GetWIMInfoL | KErrArgument"));
        User::LeaveIfError( KErrArgument ); 
        }
    WIMI_STAT status = WIMI_Ok;
    
    WIMI_Ref_pt pinRef;
    TUint16 flags;
    TUint8 seSet;
    TUint8 version;
    WIMI_BinData_t ptWimID;
    WIMI_BinData_t ptManufacturerID;
    WIMI_BinData_t ptLabel;
    TUint8 reader = 0; 
    TBool sim; 
    TPckgBuf<TWimSecModuleStruct> infoPckg;
    aMessage.ReadL( 1, infoPckg );
    
    status = WIMI_GetWIMInfo( pWimRef,
                              &flags,
                              &seSet,
                              &ptWimID,
                              &ptManufacturerID,
                              &ptLabel,
                              &reader, 
                              &pinRef, 
                              &sim, 
                              &version );
    if ( status == WIMI_Ok )
        {
        // Code MAY NOT leave before ptLabel.pb_buf, ptManufacturerID.pb_buf,
        // and ptWimID.pb_buf are deallocated. Next AppendWIMRefL() takes the
        // ownership of pinRef struct.
        TRAPD( err, aWimMgmt->AppendWIMRefL( pinRef ) );
        if( err )
            {
            WSL_OS_Free( ptLabel.pb_buf );
            WSL_OS_Free( ptManufacturerID.pb_buf );
            WSL_OS_Free( ptWimID.pb_buf );
            // Code can leave after this point.
            User::Leave( err );
            }
        infoPckg().iRefPinG = reinterpret_cast< TUint32 >( pinRef );
        
        TBuf<KLabelLen> label;
        label.Copy( TPtr8( 
                ptLabel.pb_buf,
                ptLabel.ui_buf_length,
                ptLabel.ui_buf_length ) );
        
        TBuf<KManufacturerLen> manufacturer;
        manufacturer.Copy( TPtr8(
                ptManufacturerID.pb_buf,
                ptManufacturerID.ui_buf_length,
                ptManufacturerID.ui_buf_length ) );

        TBuf<KSerialNumberLen> serialnumber;
        serialnumber.Copy( TPtr8(
                ptWimID.pb_buf,
                ptWimID.ui_buf_length,
                ptWimID.ui_buf_length ) );

        infoPckg().iVersion = version;
        infoPckg().iReader = reader;
        infoPckg().iLabel = label;
        infoPckg().iManufacturer = manufacturer;
        infoPckg().iSerialNumber = serialnumber;

        WSL_OS_Free( ptLabel.pb_buf );
        WSL_OS_Free( ptManufacturerID.pb_buf );
        WSL_OS_Free( ptWimID.pb_buf );
        // Code can leave after this point.

        aMessage.WriteL( 1, infoPckg );
        }

    aMessage.Complete( CWimUtilityFuncs::MapWIMError( status ) );
    }

// -----------------------------------------------------------------------------
// CWimTokenHandler::IsWIMOpenL
// Checks if the WIM is already opened.
// -----------------------------------------------------------------------------
//
void CWimTokenHandler::IsWIMOpenL(
    const RMessage2& aMessage,
    CWimTimer* const aTimer,
    CWimMemMgmt* const aWimMgmt ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTokenHandler::IsWIMOpenL | Begin"));
    WIMI_Ref_pt pWimRef = const_cast<WIMI_Ref_pt>( aMessage.Ptr0() );
    
    __ASSERT_ALWAYS( pWimRef, User::Leave( KErrArgument ) );
    __ASSERT_ALWAYS( aTimer, User::Leave( KErrArgument ) );
    if ( !aWimMgmt->ValidateWIMRefL( pWimRef ) ) 
        {
        _WIMTRACE(_L("WIM | WIMServer | CWimTokenHandler::IsWIMOpenL | KErrArgument"));
        User::LeaveIfError( KErrArgument ); 
        }

    TPckgBuf<TBool> pckg;
    
    if ( WIMI_IsWIMOpened( pWimRef ) )
        {
        pckg() = ETrue;
        aTimer->ResetTimer();
        }
    else
        {
        pckg() = EFalse;
        }

    aMessage.WriteL( 1, pckg );
    aMessage.Complete( KErrNone );
    }

// -----------------------------------------------------------------------------
// CWimTokenHandler::CloseWIM
// Closes connection to WIM card.
// -----------------------------------------------------------------------------
//
void CWimTokenHandler::CloseWIM( const RMessage2& aMessage ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimTokenHandler::CloseWIM | Begin"));
    WIMI_STAT status = KErrNone;
    WIMI_Ref_pt wimRef = const_cast<WIMI_Ref_pt>( aMessage.Ptr0() );

    if ( wimRef )
        {
        status = WIMI_CloseWIM( wimRef );
        }
    else
        {
        status = WIMI_ERR_BadParameters;
        }
    aMessage.Complete( CWimUtilityFuncs::MapWIMError( status ) );
    }

//  End of File  
