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
* Description:  Services for authentication object management
*
*/



// INCLUDE FILES
#include    "WimServer.h"
#include    "Wimi.h"            //WIMI definitions
#include    "WimClsv.h"
#include    "WimAuthObjHandler.h"
#include    "WimMemMgmt.h"
#include    "WimResponse.h"
#include    "WimUtilityFuncs.h"
#include    "WimSecurityDlgHandler.h"
#include    "WimTrace.h"
#include    "WimCallbackImpl.h"
#include    "WimCleanup.h"
#include    "JavaUtils.h"
#include    <random.h>

#ifdef _DEBUG
_LIT( KWimAuthObjPanic, "WimAuthObj" );
#endif


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::CWimAuthObjHandler
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimAuthObjHandler::CWimAuthObjHandler() : CActive( EPriorityStandard )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimAuthObjHandler::CWimAuthObjHandler | Begin"));
    iDisablePINBlocked = EFalse;
    iUnblockPING = EFalse;
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimAuthObjHandler::ConstructL | Begin"));
    CActiveScheduler::Add( this );
    iWimUtilFuncs = CWimUtilityFuncs::NewL();
    iWimSecDlg = CWimSecurityDlgHandler::NewL();
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimAuthObjHandler* CWimAuthObjHandler::NewL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimAuthObjHandler::NewL | Begin"));
    CWimAuthObjHandler* self = new( ELeave ) CWimAuthObjHandler;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }
    
// Destructor
CWimAuthObjHandler::~CWimAuthObjHandler()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimAuthObjHandler::~CWimAuthObjHandler | Begin"));
    Cancel();
    delete iWimUtilFuncs;
    delete iWimSecDlg;
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::GetPINCountL
// Fetches the count of PINs in WIM.
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::GetPINCountL( const RMessage2& aMessage ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimAuthObjHandler::GetPINCountL | Begin"));
    WIMI_Ref_pt pWimStruct = const_cast<WIMI_Ref_pt>( aMessage.Ptr0() ); 
    TUint16 pinNum;
    WIMI_RefList_t refList;
    WIMI_STAT status = WIMI_Ok; 
    
    status = WIMI_GetPINList( pWimStruct, &pinNum, &refList );
    
    if ( status == WIMI_Ok )
        {
        free_WIMI_RefList_t( refList );
        TPckgBuf<TUint> pckg( pinNum );
        aMessage.WriteL( 1, pckg );
        }
    aMessage.Complete( CWimUtilityFuncs::MapWIMError( status ) );
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::GetPINRefListL
// Get reference list for PINs.
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::GetPINRefListL( 
    const RMessage2& aMessage, 
    CWimMemMgmt* aWimMgmt )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimAuthObjHandler::GetPINRefListL | Begin"));
    WIMI_Ref_pt pWimStruct = const_cast<WIMI_Ref_pt>( aMessage.Ptr0() );
    __ASSERT_ALWAYS( pWimStruct, User::Leave( KErrArgument ) );
    HBufC8* buf = iWimUtilFuncs->DesLC( 2, aMessage );
    TUint32* pinArr = ( TUint32* )( buf->Des().Ptr() );
    TUint16 pinNum;
    WIMI_RefList_t pinRefLst;
    WIMI_STAT status = WIMI_Ok;
       
    status = WIMI_GetPINList( pWimStruct, &pinNum, &pinRefLst );
    
    if ( status == WIMI_Ok )
        {
        aWimMgmt->AppendWIMRefLstL( pinRefLst );    // takes ownership
        TPckgBuf<TUint32> lstAddr( ( TUint32 ) pinRefLst );
        for ( TUint8 index = 0; index < pinNum; index++ )
            {
            pinArr[index] = ( TUint32 ) pinRefLst[index];
            }
            
        iPinGRef =  const_cast< WIMI_Ref_pt >( pinRefLst[0] );
        
        aMessage.WriteL( 1, lstAddr );
        aMessage.WriteL( 2, buf->Des() );
        }
    CleanupStack::PopAndDestroy( buf );
    aMessage.Complete( CWimUtilityFuncs::MapWIMError( status ) );
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::GetPINInfoL
// Get detailed information about the PIN.
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::GetPINInfoL( const RMessage2& aMessage ) 
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimAuthObjHandler::GetPINInfoL | Begin"));
    WIMI_Ref_pt pPinStruct = const_cast<WIMI_Ref_pt>( aMessage.Ptr0() );
    __ASSERT_ALWAYS( pPinStruct, User::Leave( KErrArgument ) );
    TUint8 flags;
    TUint8 minLength;
    TUint8 pinNumber = 0;
    WIMI_BinData_t ptLabel;
    WIMI_STAT status = WIMI_Ok;
    
    if ( pPinStruct )
        {
        iPinValue.Zero();
        TRequestStatus status1;
        CWimCallBack::SetSignTextRequestStatus( &status1 );
        status = WIMI_VerifyPINReq( NULL,pPinStruct, 
                             ( TUint8 )iPinValue.Length(),
                             ( TUint8* )iPinValue.Ptr() );
            
        if ( status != WIMI_Ok )
            {
        	aMessage.Complete( CWimUtilityFuncs::MapWIMError( status ) );
        	return;
            }
             
        User::WaitForRequest( status1 );   
        CWimCallBack::SetSignTextRequestStatus( NULL );          
        WIMI_Ref_t* cmeWimRef = NULL;
        
        status = WIMI_GetPINStatus( pPinStruct,
                                    &cmeWimRef,
                                    &flags,
                                    &minLength,
                                    &pinNumber,
                                    &ptLabel );

        if ( status == WIMI_Ok )
            {
            // Code MAY NOT leave before cmeWimRef and ptLabel.pb_buf are deallocated.

            TPckgBuf<TWimPinStruct> infoPckg;
            TInt readErr = aMessage.Read( 1, infoPckg );
            if( readErr )
                {
                free_WIMI_Ref_t( cmeWimRef );
                WSL_OS_Free( ptLabel.pb_buf );
                // Code can leave after this point.
                User::Leave( readErr );
                }

            TBuf<KLabelLen> label;
            label.Copy( TPtr8( ptLabel.pb_buf,
                               ptLabel.ui_buf_length, 
                               ptLabel.ui_buf_length ) );
            
            // Possible values of iStatus are:
            // EEnabled		    = 0x80,
            // EChangeDisabled	= 0x40,
            // EUnblockDisabled	= 0x20,
            // EDisableAllowed	= 0x10,
        
            if( status1.Int() == KErrLocked )
                {
                infoPckg().iStatus = flags & KPINStatusMask;
                infoPckg().iStatus = infoPckg().iStatus | KWimAuthObjectBlocked;	
                }
            else if( iDisablePINBlocked )
                {
                infoPckg().iStatus = flags & KPINStatusMask;
            	infoPckg().iStatus = infoPckg().iStatus | KWimAuthObjectBlocked;
                }
            else
                {
                infoPckg().iStatus = flags & KPINStatusMask;	
                }
            
            infoPckg().iPinNumber = pinNumber;
            infoPckg().iLabel = label;

            TInt writeErr = aMessage.Write( 1, infoPckg );

            free_WIMI_Ref_t( cmeWimRef );
            WSL_OS_Free( ptLabel.pb_buf );
            // Code can leave after this point.
            User::LeaveIfError( writeErr );
            }
        }
    else
        {
        status = WIMI_ERR_BadParameters;
        }
    aMessage.Complete( CWimUtilityFuncs::MapWIMError( status ) );
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::GetPINsInfoL
// Get detailed information about the all of the PINs on a certain SecMod.
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::GetPINsInfoL( const RMessage2& aMessage ) 
    {
    //Get pin number and reflist
    _WIMTRACE(_L("WIM | WIMServer | CWimAuthObjHandler::GetPINsInfoL | Begin"));
    WIMI_Ref_pt pWimStruct = const_cast<WIMI_Ref_pt>( aMessage.Ptr0() ); 
    
    TUint16 pinNum;
    WIMI_RefList_t refList;
    WIMI_STAT status = WIMI_Ok;
   
    status = WIMI_GetPINList( pWimStruct, &pinNum, &refList );
    CleanupPushWimRefListL( refList );

    CArrayFixFlat<TWimPinStruct>* pinsInfo = new( ELeave ) CArrayFixFlat<TWimPinStruct>( pinNum );
    CleanupStack::PushL( pinsInfo );

    RArray<WIMI_Ref_pt> pinRefsArray;
    CleanupClosePushL( pinRefsArray );
   
    if( status == WIMI_Ok )
        {
        if( pinNum != aMessage.Int2() )
            {
            status = WIMI_ERR_Internal;
            CleanupStack::PopAndDestroy( 3, refList );  // pinRefsArray, pinsInfo, refList
            aMessage.Complete( CWimUtilityFuncs::MapWIMError( status ) );
            return;
            }
        for ( TUint8 index = 0; index < pinNum; index++ )
            {
            pinRefsArray.Append( refList[index] );
            }
        }
    else
        {
        CleanupStack::PopAndDestroy( 3, refList );  // pinRefsArray, pinsInfo, refList
        aMessage.Complete( CWimUtilityFuncs::MapWIMError( status ) );
        return;
        }

    //Get pin details bt pin reference
    for ( TUint8 index = 0; index < pinNum; index++ )
        {
        WIMI_Ref_pt pPinStruct = pinRefsArray[ index ];

	    __ASSERT_ALWAYS( pPinStruct, User::Leave( KErrArgument ) );

        TUint8 flags;
        TUint8 minLength;
        TUint8 pinNumber = 0;
        WIMI_BinData_t ptLabel;
        WIMI_STAT status = WIMI_Ok;
        
        TRequestStatus status1;
        
        iPinValue.Zero();
        
        CWimCallBack::SetSignTextRequestStatus( &status1 );

        status = WIMI_VerifyPINReq( NULL,pPinStruct, 
                 ( TUint8 )iPinValue.Length(),
                 ( TUint8* )iPinValue.Ptr() );
        
        _WIMTRACE2(_L("WIM | WIMServer | CWimAuthObjHandler::GetPINsInfoL status %d "), status );    

        if ( status != WIMI_Ok )
            {
            CleanupStack::PopAndDestroy( 3, refList );  // pinRefsArray, pinsInfo, refList
            aMessage.Complete( CWimUtilityFuncs::MapWIMError( status ) );
            return;
            }

        User::WaitForRequest( status1 );

        CWimCallBack::SetSignTextRequestStatus( NULL );
        
        WIMI_Ref_t* cmeWimRef = NULL;
        
        status = WIMI_GetPINStatus( pPinStruct,
                        &cmeWimRef,
                        &flags,
                        &minLength,
                        &pinNumber,
                        &ptLabel );
        
        if ( status == WIMI_Ok )
            {
            free_WIMI_Ref_t( cmeWimRef );
            CleanupPushWimBufL( ptLabel );

            TWimPinStruct pinStruct;
            pinStruct.iLabel.Copy( TPtr8( ptLabel.pb_buf,
                    ptLabel.ui_buf_length,
                    ptLabel.ui_buf_length ) );
            CleanupStack::PopAndDestroy( ptLabel.pb_buf );

            // Possible values of iStatus are:
            // EEnabled		    = 0x80,
            // EChangeDisabled	= 0x40,
            // EUnblockDisabled	= 0x20,
            // EDisableAllowed	= 0x10,

            if( status1.Int() == KErrLocked )
                {
                pinStruct.iStatus = flags & KPINStatusMask;
                pinStruct.iStatus = pinStruct.iStatus | KWimAuthObjectBlocked;
                }
            else if( iDisablePINBlocked )
                {
                pinStruct.iStatus = flags & KPINStatusMask;
                pinStruct.iStatus = pinStruct.iStatus | KWimAuthObjectBlocked;
                }
            else
                {
                pinStruct.iStatus = flags & KPINStatusMask;
                }
            pinStruct.iPinNumber = pinNumber;

            pinsInfo->AppendL( pinStruct );
            }
        else
            {
            status = WIMI_ERR_BadParameters;
            break;
            }
        }

    TInt16 length = ( TInt16 )pinsInfo->Count()*( sizeof( TWimPinStruct ) );

    TPtrC8 pinInfoPtr;
    
    pinInfoPtr.Set( (TText8*)&(*pinsInfo)[0], length );  

    if( status == WIMI_Ok )
        {
        aMessage.WriteL( 1, pinInfoPtr );
        }

    CleanupStack::PopAndDestroy( 3, refList );  // pinRefsArray, pinsInfo, refList

    aMessage.Complete( CWimUtilityFuncs::MapWIMError( status ) );
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::VerifyDisabledPINRequestL
// Verifies Disabled PIN number.
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::VerifyDisabledPINRequestL( 
     const RMessage2& aMessage )
    {
    __ASSERT_DEBUG( iResponseID == NULL, User::Panic( KWimAuthObjPanic, KErrGeneral ) );
    iResponseID = new ( ELeave )CWimResponse( aMessage );
    iResponseID->iOpcode = EVerifyDisabledPinReq;
    
    if( iDisablePINBlocked ) 
    	{   
        iResponseID->iStatus = WIMI_ERR_PINBlocked;
        iResponseID->CompleteMsgAndDelete();
        iResponseID = NULL;
    	}
    else
    	{    
        iResponseID->iStatus = WIMI_Ok;
        iResponseID->CompleteMsgAndDelete();
        iResponseID = NULL;
    	}
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::VerifyPINRequestL
// Verifies PIN number.
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::VerifyPINRequestL(
    const RMessage2& aMessage,
    TBool aShowDialog )
    {
    _WIMTRACE(_L("WIM|WIMServer|CWimAuthObjHandler::VerifyPINRequestL|Begin"));

    __ASSERT_DEBUG( iResponseID == NULL, User::Panic( KWimAuthObjPanic, KErrGeneral ) );
    iResponseID = new ( ELeave )CWimResponse( aMessage );
    iResponseID->iOpcode = EVerifyPinReq;
    
    __ASSERT_DEBUG( iTrId == NULL, User::Panic( KWimAuthObjPanic, KErrGeneral ) );
    iTrId = iWimUtilFuncs->TrIdLC( iResponseID, EWimMgmtReq );
    CleanupStack::Pop( iTrId );
    
    iShowDialog = aShowDialog;

    if ( aShowDialog ) // Ask PIN from user
        {
        _WIMTRACE(_L("WIM|WIMServer|CWimAuthObjHandler::Verify|Show dialog"));
        TPckgBuf<TPINParams> infoPckg;
        aMessage.ReadL( 1, infoPckg );
        _WIMTRACE(_L("WIM|WIMServer|CWimAuthObjHandler::Verify|PINParamas read"));
        
        iPinParams.iPINLabel = infoPckg().iPINLabel;
        iPinParams.iTokenLabel = infoPckg().iTokenLabel;
        iPinParams.iMinLength = infoPckg().iMinLength;
	    iPinParams.iMaxLength = infoPckg().iMaxLength;
	    
        _WIMTRACE(_L("WIM|WIMServer|CWimAuthObjHandler::Verify|Call dialog"));
        
        iRetry = EFalse;
        
        VerifyPINAskPIN();
         
        }
    else // Do not ask PIN from user, just try if PIN is blocked with empty PIN
        {
        iRequest = EVerifyPINAskPIN;
        iStatus = KRequestPending;
        SetActive();
        iPinValue.Zero();
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone );
        }
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::VerifyPINAskPIN
//
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::VerifyPINAskPIN()
    {
	iRequest = EVerifyPINAskPIN;
    iStatus = KRequestPending;
    SetActive();
    iWimSecDlg->EnterPIN( iRetry,
                          iPinParams,
                          iPinValue,
                          iStatus );
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::ContinueVerifyPINReqL
// Continue verify PIN request. Calls WIMI.
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::ContinueVerifyPINReqL( ) 
    {
    __ASSERT_DEBUG( iResponseID, User::Panic( KWimAuthObjPanic, KErrGeneral ) );
    WIMI_Ref_pt pPINref = const_cast<WIMI_Ref_pt>( (iResponseID->iMessage).Ptr0() );
     
    HBufC8* pinBuf = HBufC8::NewLC( iPinValue.Length() );
    TPtr8 pinPtr = pinBuf->Des();
    pinPtr.Copy( iPinValue );
    
    if ( iShowDialog )
        {
        iStatus = KRequestPending;
        TRequestStatus* status = &iStatus;
    	
        iRequest = EVerifyPINCallBackResponse;
        SetActive();

        iResponseID->iStatus = WIMI_VerifyPINReq( iTrId,
                                             pPINref, 
                                             ( TUint8 )pinPtr.Length(),
                                             ( TUint8* )pinPtr.Ptr() );

        if ( iResponseID->iStatus != WIMI_Ok )
            {
            iRequest = EWIMIError;
            User::RequestComplete( status, iResponseID->iStatus );	 
            }
        }
    else
        {
        iResponseID->iStatus = WIMI_VerifyPINReq( iTrId,
                                         pPINref,
                                         ( TUint8 )pinPtr.Length(),
                                         ( TUint8* )pinPtr.Ptr() );

        if ( iResponseID->iStatus != WIMI_Ok )
            {
            iResponseID->CompleteMsgAndDelete();
            iResponseID = NULL;
            delete iTrId;
            iTrId = NULL;
            }
        }
        
    CleanupStack::PopAndDestroy( pinBuf );
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::CancelVerifyPin
// Cancel verify Pin operation
// -----------------------------------------------------------------------------
//    
void CWimAuthObjHandler::CancelVerifyPin( const RMessage2& aMessage )  
    {
    if( IsActive() )
        {
        Cancel();	
        }
        
    aMessage.Complete( KErrNone ); 
    } 

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::EnablePINReqL
// Enables/disables PIN query.
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::EnablePINReqL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM|WIMServer|CWimAuthObjHandler::EnablePINReqL|Begin"));

    __ASSERT_DEBUG( iResponseID == NULL, User::Panic( KWimAuthObjPanic, KErrGeneral ) );
    iResponseID = new( ELeave )CWimResponse( aMessage );
    iResponseID->iOpcode =EEnablePINReq;
    
    __ASSERT_DEBUG( iTrId == NULL, User::Panic( KWimAuthObjPanic, KErrGeneral ) );
    iTrId = iWimUtilFuncs->TrIdLC( iResponseID, EWimMgmtReq );
    CleanupStack::Pop( iTrId );
    
    TPckgBuf<TPINStateRequest> statePckg;
    aMessage.ReadL( 1, statePckg );

    TPckgBuf<TPINParams> infoPckg;
    aMessage.ReadL( 2, infoPckg );
    
    iPinParams.iPINLabel = infoPckg().iPINLabel;
    iPinParams.iTokenLabel = infoPckg().iTokenLabel;
    iPinParams.iMinLength = infoPckg().iMinLength;
	iPinParams.iMaxLength = infoPckg().iMaxLength;

    iEnablePinQuery = statePckg().iEnable;
    
    iRetry = EFalse;
    
    EnablePINAskPIN();

    }
    
void CWimAuthObjHandler::EnablePINAskPIN()
    {
    if ( iEnablePinQuery )
        {
        iRequest = EEnablePINAskPIN;
        }
    else
        {
        iRequest = EDisablePINAskPIN;
        }
   
    iStatus = KRequestPending;
	SetActive();
    // Query PIN code from the user by using security dialog
    iWimSecDlg->EnablePIN( iEnablePinQuery,
                           iRetry,
                           iPinParams,
                           iPinValue,
                           iStatus );
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::ContinueEnablePINReqL
// Continue enable/disable PIN query request. Calls WIMI.
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::ContinueEnablePINReqL() 
    {
    __ASSERT_DEBUG( iResponseID, User::Panic( KWimAuthObjPanic, KErrGeneral ) );
    WIMI_Ref_pt pPINref = const_cast<WIMI_Ref_pt>( (iResponseID->iMessage).Ptr0() );

    HBufC8* pinBuf = HBufC8::NewLC( iPinValue.Length() );
    TPtr8 pinPtr = pinBuf->Des();
    pinPtr.Copy( iPinValue );
    
    iStatus = KRequestPending;
    TRequestStatus* status = &iStatus;
    
    if ( iEnablePinQuery )
        {
        iRequest = EEnablePINCallBackResponse;
        }
    else
        {
        iRequest = EDisablePINCallBackResponse;
        }
    SetActive();

    iResponseID->iStatus = WIMI_EnablePINReq( iTrId,
                                             pPINref,          
                                             ( TUint8 )pinPtr.Length(),
                                             ( TUint8* )pinPtr.Ptr(),                     
                                             iEnablePinQuery );

    if ( iResponseID->iStatus != WIMI_Ok )   
        {
        iRequest = EWIMIError;
        User::RequestComplete( status, iResponseID->iStatus );
        }
    CleanupStack::PopAndDestroy( pinBuf );
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::CancelEnablePinL
// Cancel enable Pin operation
// -----------------------------------------------------------------------------
//    
void CWimAuthObjHandler::CancelEnablePin( const RMessage2& aMessage )  
    {
    if( IsActive() )
        {
        Cancel();	
        }
        
    aMessage.Complete( KErrNone ); 
    } 

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::ChangePINRequestL
// Changes a PIN number.
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::ChangePINRequestL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimAuthObjHandler::ChangePINRequestL | Begin"));

    __ASSERT_DEBUG( iResponseID == NULL, User::Panic( KWimAuthObjPanic, KErrGeneral ) );
    iResponseID = new ( ELeave ) CWimResponse( aMessage );
    iResponseID->iOpcode = EChangePINReq;
    
    __ASSERT_DEBUG( iTrId == NULL, User::Panic( KWimAuthObjPanic, KErrGeneral ) );
    iTrId = iWimUtilFuncs->TrIdLC( iResponseID, EWimMgmtReq );
    CleanupStack::Pop( iTrId ); 

    TPckgBuf<TPINParams> infoPckg;
    aMessage.ReadL( 1, infoPckg );
    
    iPinParams.iPINLabel = infoPckg().iPINLabel;
    iPinParams.iTokenLabel = infoPckg().iTokenLabel;
    iPinParams.iMinLength = infoPckg().iMinLength;
	iPinParams.iMaxLength = infoPckg().iMaxLength;
    
    iRetry = EFalse;
    
    ChangePINAskPIN();
    }
   
// -----------------------------------------------------------------------------
// CWimAuthObjHandler::ChangePINAskPIN
//
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::ChangePINAskPIN()
    {
	iRequest = EChangePINAskPIN;
	iStatus = KRequestPending;
	SetActive();
    // Query PIN code from the user by using security dialog
    iWimSecDlg->ChangePINValue( iRetry,
                                iPinParams, 
                                iOldPinValue, 
                                iNewPinValue, 
                                iStatus );
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::ContinueChangePINReqL
// Continue change PIN request. Calls WIMI.
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::ContinueChangePINReqL() 
    {
    __ASSERT_DEBUG( iResponseID, User::Panic( KWimAuthObjPanic, KErrGeneral ) );
    WIMI_Ref_pt pPINref = const_cast<WIMI_Ref_pt>( (iResponseID->iMessage).Ptr0() );
  
    HBufC8* pinOldBuf = HBufC8::NewLC( iOldPinValue.Length() );
    HBufC8* pinNewBuf = HBufC8::NewLC( iNewPinValue.Length() );

    TPtr8 oldPin = pinOldBuf->Des();
    oldPin.Copy( iOldPinValue );
    TPtr8 newPin = pinNewBuf->Des();
    newPin.Copy( iNewPinValue );
    
    iStatus = KRequestPending;
    TRequestStatus* status = &iStatus;
    iRequest = EChangePINCallBackResponse;
    SetActive();

    iResponseID->iStatus = WIMI_ChangePINReq( iTrId,
                                             pPINref,
                                             ( TUint8 )oldPin.Length(),
                                             ( TUint8* )oldPin.Ptr() ,
                                             ( TUint8 )newPin.Length(),
                                             ( TUint8* )newPin.Ptr() );
    if ( iResponseID->iStatus != WIMI_Ok )
        {
        iRequest = EWIMIError;
        User::RequestComplete( status, iResponseID->iStatus );
        }
    CleanupStack::PopAndDestroy( 2, pinOldBuf ); // pinNewBuf, pinOldBuf
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::CancelChangePin
// Cancel change Pin operation
// -----------------------------------------------------------------------------
//    
void CWimAuthObjHandler::CancelChangePin( const RMessage2& aMessage )  
    {
    if( IsActive() )
        {
        Cancel();	
        }
        
    aMessage.Complete( KErrNone ); 
    }     

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::UnblockPinReqL
// Unblocks blocked PIN.
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::UnblockPinReqL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimAuthObjHandler::UnblockPinReqL | Begin"));

    __ASSERT_DEBUG( iResponseID == NULL, User::Panic( KWimAuthObjPanic, KErrGeneral ) );
    iResponseID = new ( ELeave ) CWimResponse( aMessage );
    iResponseID->iOpcode = EUnblockPinReq;
    
    __ASSERT_DEBUG( iTrId == NULL, User::Panic( KWimAuthObjPanic, KErrGeneral ) );
    iTrId = iWimUtilFuncs->TrIdLC( iResponseID, EWimMgmtReq );
    CleanupStack::Pop( iTrId );
    
    TPckgBuf<TPINParams> blockedPINPckg;
    aMessage.ReadL( 1, blockedPINPckg );
    
    iPinParams.iPINLabel = blockedPINPckg().iPINLabel;
    iPinParams.iTokenLabel = blockedPINPckg().iTokenLabel;
    iPinParams.iMinLength = blockedPINPckg().iMinLength;
	iPinParams.iMaxLength = blockedPINPckg().iMaxLength;

    iUnblockingPinParams.iPINLabel = KUnblockingPinLabel;
    iUnblockingPinParams.iTokenLabel = blockedPINPckg().iTokenLabel;
    iUnblockingPinParams.iMinLength = blockedPINPckg().iMinLength;
    iUnblockingPinParams.iMaxLength = KMaxPINLength;

    iRetry = EFalse;
    
    UnblockPINAskPIN();
    }   

void CWimAuthObjHandler::UnblockPINAskPIN()
    {
	iRequest = EUnblockPINAskPIN;
	iStatus = KRequestPending;
    SetActive();
    // Query PIN code from the user by using security dialog
    iWimSecDlg->UnblockPIN( iRetry,
                            iPinParams,
                            iUnblockingPinParams,
                            iUnblockingPinValue, 
                            iNewPinValue, 
                            iStatus );
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::ContinueUnblockPinReqL
// Continue Unblock PIN request. Calls WIMI.
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::ContinueUnblockPinReqL() 
    {
    __ASSERT_DEBUG( iResponseID, User::Panic( KWimAuthObjPanic, KErrGeneral ) );
    WIMI_Ref_pt pPINref = const_cast< WIMI_Ref_pt>( (iResponseID->iMessage).Ptr0() );
    
    if( iPinGRef == pPINref )
        {
    	iUnblockPING = ETrue;
        }
    else
        {
        iUnblockPING = EFalse;	
        }
        
    HBufC8* unblockPinBuf = HBufC8::NewLC( iUnblockingPinValue.Length() );
    HBufC8* newPinBuf = HBufC8::NewLC( iNewPinValue.Length() );

    TPtr8 unblockingPin = unblockPinBuf->Des();
    unblockingPin.Copy( iUnblockingPinValue );

    TPtr8 newPin = newPinBuf->Des();
    newPin.Copy( iNewPinValue );
    
    iStatus = KRequestPending;
    TRequestStatus* status =&iStatus;
    iRequest = EUnblockPINCallBackResponse;
    SetActive();

    iResponseID->iStatus = WIMI_UnblockPINReq( iTrId,
                                              pPINref,
                                              ( TUint8 )unblockingPin.Length(),
                                              ( TUint8* )unblockingPin.Ptr(),
                                              ( TUint8 )newPin.Length(),
                                              ( TUint8* )newPin.Ptr() );
    
    if ( iResponseID->iStatus != WIMI_Ok )
        {
        iRequest = EWIMIError;
        User::RequestComplete( status, iResponseID->iStatus );
        }
    CleanupStack::PopAndDestroy( 2, unblockPinBuf ); // newPinBuf, unblockPinBuf
    
    }
    
// -----------------------------------------------------------------------------
// CWimAuthObjHandler::CancelUnblockPinL
// Cancel Unblock Pin operation
// -----------------------------------------------------------------------------
//    
void CWimAuthObjHandler::CancelUnblockPin( const RMessage2& aMessage )  
    {
    if( IsActive() )
        {
        Cancel();	
        }
        
    aMessage.Complete( KErrNone ); 
    }    

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::RunL
// Continue authentication request after PIN is asked
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::RunL()
    {
    _WIMTRACE3(_L("WIM|WIMServer|CWimAuthObjHandler::RunL|iRequest=%d, status=%d"), iRequest, iStatus.Int());

    switch ( iRequest )
        {
        // Verify PIN request
        case EVerifyPINAskPIN:
            {
            iRetry = ETrue;

            if( iStatus.Int() == KErrCancel )
                {
                _WIMTRACE2(_L("WIM|WIMServer|CWimAuthObjHandler::RunL|User cancel %d "), iStatus.Int());
                if( iResponseID )
                    {
                    iResponseID->iStatus = WIMI_ERR_UserCancelled;
                    iResponseID->CompleteMsgAndDelete();
                    iResponseID = NULL;
                    }
                if( iTrId )
                    {
                    delete iTrId;
                    iTrId = NULL;
                    }

                }
            else
                {
                if( iShowDialog )
                    {
                    CWimCallBack::SetSignTextRequestStatus( &iStatus );
                    }
                ContinueVerifyPINReqL();
                }
            break;
            }

        case EVerifyPINCallBackResponse:
            {
            CWimCallBack::SetSignTextRequestStatus( NULL );
            if( iStatus.Int() == WIMI_ERR_BadPIN )
                {
                VerifyPINAskPIN();
                }
            else if ( iStatus.Int() == WIMI_ERR_PINBlocked )
                {
                iRequest = EShowPINBlocked;
                iWimSecDlg->ShowPINBlocked( iPinParams, iStatus );
                SetActive();
                }
            else if ( iStatus.Int() == WIMI_ERR_CardIOError )
                {
                iRequest = EShowCardIsRemoved;
                iStatus = KRequestPending;
                TRequestStatus* status = &iStatus;
                SetActive();
                User::RequestComplete( status, KErrNone );
                }
            else if ( iStatus.Int() == WIMI_ERR_UserCancelled )
                {
                iStatus = KRequestPending;
                iRequest = EIdle;
                TRequestStatus* status = &iStatus;
                SetActive();
                User::RequestComplete( status, KErrNone );
                }
            else
                {
                iStatus = KRequestPending;
                iRequest = EIdle;
                TRequestStatus* status = &iStatus;
                SetActive();
                User::RequestComplete( status, KErrNone );
                }
            break;
            }

        // Change PIN request
        case EChangePINAskPIN:
            {
            iRetry = ETrue;
            if( iStatus.Int() == KErrCancel )
                {
                if( iResponseID )
                    {
                    iResponseID->iStatus = WIMI_ERR_UserCancelled;
                    iResponseID->CompleteMsgAndDelete();
                    iResponseID = NULL;
                    }
                if( iTrId )
                    {
                    delete iTrId;
                    iTrId = NULL;
                    }
                }
            else
                {
                CWimCallBack::SetSignTextRequestStatus( &iStatus );
                ContinueChangePINReqL();
                }
            break;
            }

        case EChangePINCallBackResponse:
            {
            CWimCallBack::SetSignTextRequestStatus( NULL );
            if( iStatus.Int() == WIMI_ERR_BadPIN )
                {
                ChangePINAskPIN();
                }
            else if ( iStatus.Int() == WIMI_ERR_PINBlocked )
                {
                iRequest = EShowPINBlocked;
                iWimSecDlg->ShowPINBlocked( iPinParams, iStatus );
                SetActive();
                }
             else if ( iStatus.Int() == WIMI_ERR_CardIOError )
                {
                iRequest = EShowCardIsRemoved;
                iStatus = KRequestPending;
                TRequestStatus* status = &iStatus;
                SetActive();
                User::RequestComplete( status, KErrNone );
                }
             else if ( iStatus.Int() == WIMI_ERR_UserCancelled )
                {
                iStatus = KRequestPending;
                iRequest = EIdle;
                TRequestStatus* status = &iStatus;
                SetActive();
                User::RequestComplete( status, KErrNone );
                }
             else
                {
                iStatus = KRequestPending;
                iRequest = EIdle;
                TRequestStatus* status = &iStatus;
                SetActive();
                User::RequestComplete( status, KErrNone );
                }
            break;
            }

        // Enable/Disable PIN query request
        case EEnablePINAskPIN:    // Flow trough
        case EDisablePINAskPIN:
            {
            iRetry = ETrue;
            if( iStatus.Int() == KErrCancel )
                {
                if( iResponseID )
                    {
                    iResponseID->iStatus = WIMI_ERR_UserCancelled;
                    iResponseID->CompleteMsgAndDelete();
                    iResponseID = NULL;
                    }
                if( iTrId )
                    {
                    delete iTrId;
                    iTrId = NULL;
                    }
                }
            else
                {
                CWimCallBack::SetSignTextRequestStatus( &iStatus );
                ContinueEnablePINReqL();
                }
            break;
            }

        case EEnablePINCallBackResponse:
        case EDisablePINCallBackResponse:
            {
            CWimCallBack::SetSignTextRequestStatus( NULL );
            if( iStatus.Int() == WIMI_ERR_BadPIN )
                {
                EnablePINAskPIN();
                }
            else if ( iStatus.Int() == WIMI_ERR_PINBlocked )
                {
                iRequest = EShowPINBlocked;
                iWimSecDlg->ShowPINBlocked( iPinParams, iStatus );
                SetActive();
                if( iEnablePinQuery )
                    {
                    iDisablePINBlocked = ETrue;
                    }
                }
             else if ( iStatus.Int() == WIMI_ERR_CardIOError )
                {
                iRequest = EShowCardIsRemoved;
                  iStatus = KRequestPending;
                TRequestStatus* status = &iStatus;
                SetActive();
                User::RequestComplete( status, KErrNone );
                  }
             else if ( iStatus.Int() == WIMI_ERR_UserCancelled )
                {
                iStatus = KRequestPending;
                iRequest = EIdle;
                TRequestStatus* status = &iStatus;
                SetActive();
                User::RequestComplete( status, KErrNone );
                }
            else
                {
                iStatus = KRequestPending;
                iRequest = EIdle;
                TRequestStatus* status = &iStatus;
                SetActive();
                User::RequestComplete( status, KErrNone );
                }
            break;
            }

        // Unblock PIN request
        case EUnblockPINAskPIN:
            {
            iRetry = ETrue;
            if( iStatus.Int() == KErrCancel )
                {
                if( iResponseID )
                    {
                    iResponseID->iStatus = WIMI_ERR_UserCancelled;
                    iResponseID->CompleteMsgAndDelete();
                    iResponseID = NULL;
                    }
                if( iTrId )
                    {
                    delete iTrId;
                    iTrId = NULL;
                    }
                }
            else
                {
                CWimCallBack::SetSignTextRequestStatus( &iStatus );
                ContinueUnblockPinReqL();
                }
            break;
            }
        case EUnblockPINCallBackResponse:
           {
            CWimCallBack::SetSignTextRequestStatus( NULL );
            if( iStatus.Int() == WIMI_ERR_BadPIN )
                {
                UnblockPINAskPIN();
                }
            else if ( iStatus.Int() == WIMI_ERR_PINBlocked )
                {
                iRequest = EShowPINBlocked;
                iWimSecDlg->ShowPINTotalBlocked( iPinParams, iStatus );
                SetActive();
                }
            else if ( iStatus.Int() == WIMI_ERR_CardIOError )
                {
                iRequest = EShowCardIsRemoved;
                iStatus = KRequestPending;
                TRequestStatus* status = &iStatus;
                SetActive();
                User::RequestComplete( status, KErrNone );
              }
            else if ( iStatus.Int() == WIMI_ERR_UserCancelled )
                {
                iStatus = KRequestPending;
                iRequest = EIdle;
                TRequestStatus* status = &iStatus;
                SetActive();
                User::RequestComplete( status, KErrNone );
                }
            else
                {
                iStatus = KRequestPending;
                iRequest = EIdle;
                TRequestStatus* status = &iStatus;
                SetActive();
                User::RequestComplete( status, KErrNone );

                if( iDisablePINBlocked && iUnblockPING )
                    {
                    iDisablePINBlocked = EFalse;
                    }
                }
            break;
           }

        case EShowPINBlocked:
           {
           if( iResponseID )
                {
                iResponseID->iStatus = WIMI_ERR_PINBlocked;
                iResponseID->CompleteMsgAndDelete();
                iResponseID = NULL;
                }
            if( iTrId )
                {
                delete iTrId;
                iTrId = NULL;
                }

           iStatus = KRequestPending;
           iRequest = EIdle;
           TRequestStatus* status = &iStatus;
           SetActive();
           User::RequestComplete( status, KErrNone );
           break;
           }

       case EShowCardIsRemoved:
           {
           if( iResponseID )
                {
                iResponseID->iStatus = WIMI_ERR_CardIOError;
                iResponseID->CompleteMsgAndDelete();
                iResponseID = NULL;
                }
            if( iTrId )
                {
                delete iTrId;
                iTrId = NULL;
                }

           iStatus = KRequestPending;
           iRequest = EIdle;
           TRequestStatus* status = &iStatus;
           SetActive();
           User::RequestComplete( status, KErrNone );
           break;
           }

        case EWIMIError:
           {
           CWimCallBack::SetSignTextRequestStatus( NULL );
           if( iResponseID )
                {
                iResponseID->iStatus = ( TUint )iStatus.Int();
                iResponseID->CompleteMsgAndDelete();
                iResponseID = NULL;
                }
            if( iTrId )
                {
                delete iTrId;
                iTrId = NULL;
                }
           break;
           }

        // Idle request, do nothing
        case EIdle:
            _WIMTRACE(_L("WIM|WIMServer|CWimAuthObjHandler::RunL|Idle"));
            break;

        // Illegal request
        default:
            _WIMTRACE(_L("WIM|WIMServer|CWimAuthObjHandler::RunL|Unknown request"));
            __ASSERT_DEBUG( EFalse, User::Panic( KWimAuthObjPanic, KErrNotSupported ) );
            break;
        }

    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::DoCancel
// Asynchronous request cancelled
// -----------------------------------------------------------------------------
//        
void CWimAuthObjHandler::DoCancel()
    {
    _WIMTRACE(_L("WIM|WIMServer|CWimAuthObjHandler::DoCancel"));
    if( iRequest == EEnablePINCallBackResponse  || 
        iRequest == EUnblockPINCallBackResponse ||
        iRequest == EDisablePINCallBackResponse ||
        iRequest == EChangePINCallBackResponse  ||
        iRequest == EVerifyPINCallBackResponse )
	    {
	    CWimCallBack::CancelPinOperation();	
	    }
	if( iRequest == EEnablePINAskPIN ||
	    iRequest == EUnblockPINAskPIN ||
	    iRequest == EDisablePINAskPIN ||
	    iRequest == EChangePINAskPIN ||
	    iRequest == EVerifyPINAskPIN ) 
	    {
		iWimSecDlg->Cancel();
	    }
    }

// -----------------------------------------------------------------------------
// CWimAuthObjHandler::RunError
// RunL leaved, handle error here. Note that message is completed by
// CWimResponse that is deleted by CleanupStack when some function leaves.
// -----------------------------------------------------------------------------
//
TInt CWimAuthObjHandler::RunError( TInt aError )
    {
    _WIMTRACE(_L("WIM|WIMServer|CWimAuthObjHandler::RunError"));
    CWimCallBack::SetSignTextRequestStatus( NULL );
    if( iResponseID )
        {
    	iResponseID->iStatus = ( TUint16 ) aError;
        iResponseID->CompleteMsgAndDelete();
        iResponseID = NULL;
        }
    
    if( iTrId )
        {
    	delete iTrId;
        iTrId = NULL;
        }
    
    return KErrNone;
    }
    
    
// -----------------------------------------------------------------------------
// CWimAuthObjHandler::RetrieveAuthObjectsInfo
// -----------------------------------------------------------------------------
//
void CWimAuthObjHandler::RetrieveAuthObjectsInfo( const RMessage2& aMessage ) 
    {
    //Get pin number and reflist
    _WIMTRACE(_L("WIM | WIMServer | RetrieveAuthObjectsInfo() | start "));
    WIMI_Ref_t* wimRef = WIMI_GetWIMRef( 0 );
    
    TInt count = aMessage.Int2();
    
    TInt authIdLength = count * sizeof( TInt );
    TInt authobjLength = count* sizeof( TJavaPINParams );
    
    RArray<TInt> authIdList( count );
    
    TInt temp = 0;
    for( TInt i = 0; i < count ; i++ )
        {
    	authIdList.Append(temp);
        }
        
    RArray<TJavaPINParams> authobjsList( count );
    
    TPtr8 authIdLstPtr( (TText8*)&authIdList[0], authIdLength, authIdLength );
      
    TInt err = aMessage.Read(0, authIdLstPtr );
    if ( err!= KErrNone )
       {
       aMessage.Complete( err );	
       return;
       }
   
    for ( TInt i = 0 ; i < authIdList.Count() ; i++ )
        {
    	TJavaPINParams authobject;
    	WIMI_STAT status = WIMI_Ok;
    	
    	TUint32 authId = authIdList[i];
    	
    	WIMI_BinData_t ptLabel;
    	TUint16 flags;
        TUint8 pinNumber = 0;
        TUint8 minLength;
        TUint8 maxLength;
        TUint8 pinType;
        TUint8 storedLength;
        TUint8 PinReference;
        TUint8 padChar;
        
        status = WIMI_GetAuthObjInfo( wimRef, 
                                      &authId,
                                      &ptLabel,
                                      &flags,
                                      &pinNumber,
                                      &minLength,
                                      &maxLength,
                                      &pinType,
                                      &storedLength,
                                      &PinReference,
                                      &padChar );                              
                                      
        if( status == WIMI_Ok )
            {
            TBuf<KLabelLen> label;
            label.Copy( TPtr8( ptLabel.pb_buf, 
                               ptLabel.ui_buf_length, 
                               ptLabel.ui_buf_length ) );
                               
        	authobject.iPINLabel = label;
        	authobject.iFlags = flags;
        	authobject.iAuthId = authId;
        	authobject.iPinNumber =pinNumber;
	        authobject.iMinLength = minLength;
	        authobject.iMaxLength = maxLength;
	        authobject.iPinType = pinType;
	        authobject.iStoredLength = storedLength;
	        authobject.iPinReference = PinReference;
	        authobject.iPadChar = padChar;
	        
	        authobjsList.Append( authobject );
            }
		else
			{
			_WIMTRACE2(_L("WIM | WIMServer | RetrieveAuthObjectsInfo() | WIMI_GetAuthObjInfo returns error %d"), status);
			//information did not found or error occured so return empty authOjbect
            TBuf<KLabelLen> label;
        	authobject.iPINLabel = label;
        	authobject.iFlags = KErrNotFound;
        	authobject.iPinNumber =KErrNotFound;
	        authobject.iMinLength = KErrNotFound;
	        authobject.iMaxLength = KErrNotFound;
	        authobject.iPinType = KErrNotFound;
	        authobject.iStoredLength = KErrNotFound;
	        authobject.iPinReference = KErrNotFound;
	        authobject.iPadChar = KErrNotFound;
	        authobjsList.Append( authobject );
			}
        }
        
    TPtrC8 authObjsInfoLstPtr;
    authObjsInfoLstPtr.Set( (TText8*)&authobjsList[0], authobjLength );
    
    err = aMessage.Write( 1, authObjsInfoLstPtr );
    free_WIMI_Ref_t( wimRef );
    
    aMessage.Complete( err );
    }

//  End of File  
