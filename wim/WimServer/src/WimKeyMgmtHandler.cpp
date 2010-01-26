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
* Description:  Key management services
*
*/



// INCLUDE FILES
#include    "WimKeyMgmtHandler.h"
#include    "WimSession.h"
#include    "WimConsts.h"
#include    "WimMemMgmt.h"
#include    "WimUtilityFuncs.h"
#include    "WimTrace.h"


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimKeyMgmtHandler::CWimKeyMgmtHandler
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimKeyMgmtHandler::CWimKeyMgmtHandler()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimKeyMgmtHandler::CWimKeyMgmtHandler | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimKeyMgmtHandler::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimKeyMgmtHandler::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimKeyMgmtHandler::ConstructL | Begin"));
    iWimUtilFuncs = CWimUtilityFuncs::NewL();
    }

// -----------------------------------------------------------------------------
// CWimKeyMgmtHandler::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimKeyMgmtHandler* CWimKeyMgmtHandler::NewL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimKeyMgmtHandler::NewL | Begin"));
    CWimKeyMgmtHandler* self = new( ELeave ) CWimKeyMgmtHandler;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }
    
// Destructor
CWimKeyMgmtHandler::~CWimKeyMgmtHandler()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimKeyMgmtHandler::~CWimKeyMgmtHandler | Begin"));
    delete iWimUtilFuncs;
    }

// -----------------------------------------------------------------------------
// CWimKeyMgmtHandler::DoesKeyExistL
// Checks if the given key exists.
// -----------------------------------------------------------------------------
//
void CWimKeyMgmtHandler::DoesKeyExistL( const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimKeyMgmtHandler::DoesKeyExistL | Begin"));
    HBufC8* buf = iWimUtilFuncs->DesLC( 0, aMessage );
    TUint8* keyHash = const_cast<TUint8*>( buf->Des().Ptr() );
    WIMI_Ref_t* tmpKeyRef = NULL;
    WIMI_STAT status = WIMI_GetKeyByHash( keyHash, &tmpKeyRef );
    if ( status == WIMI_Ok )
        {
        free_WIMI_Ref_t( tmpKeyRef );
        }
    CleanupStack::PopAndDestroy( buf );
    aMessage.Complete( CWimUtilityFuncs::MapWIMError( status ) );
    }

// -----------------------------------------------------------------------------
// CWimKeyMgmtHandler::GetKeyDetailsL
// Fetches the details of the given key.
// -----------------------------------------------------------------------------
//
void CWimKeyMgmtHandler::GetKeyDetailsL( const RMessage2& aMessage )
    { 
    _WIMTRACE(_L("WIM | WIMServer | CWimKeyMgmtHandler::GetKeyDetailsL | Begin"));
    WIMI_Ref_t* pKeyRef = const_cast<WIMI_Ref_pt>( aMessage.Ptr0() );
        
    WIMI_Ref_t* pWimRef;    
    TUint8 keyType;
    TUint8 keyNumber;
    TUint8 pinNumber;
    TUint16 usage;
    TUint16 key_length;
    WIMI_BinData_t pt_label;
    WIMI_BinData_t pt_keyid;

    WIMI_STAT callStatus = WIMI_GetKeyInfo( pKeyRef,
                                            &pWimRef,
                                            NULL,
                                            &keyType,
                                            &keyNumber,
                                            &pinNumber,
                                            &usage,
                                            &pt_keyid,
                                            &pt_label,
                                            &key_length );

    if ( callStatus == WIMI_Ok )
        {
        // Code MAY NOT leave before pWimRef, pt_keyid.pb_buf, and
        // pt_label.pb_buf are deallocated.

        free_WIMI_Ref_t( pWimRef );

        TPckgBuf<TKeyInfo> keyInfoPckg;
        TInt readErr = aMessage.Read( 1, keyInfoPckg );
        if( readErr )
            {
            WSL_OS_Free( pt_label.pb_buf );
            WSL_OS_Free( pt_keyid.pb_buf );
            // Code can leave after this point.
            User::Leave( readErr );
            }

        keyInfoPckg().iLabel.Copy( TPtr8(
                pt_label.pb_buf, pt_label.ui_buf_length,
                pt_label.ui_buf_length ) );

        keyInfoPckg().iKeyId.Copy( TPtr8(
                pt_keyid.pb_buf, pt_keyid.ui_buf_length,
                pt_keyid.ui_buf_length ) );

        keyInfoPckg().iType = keyType;
        keyInfoPckg().iUsage = usage;
        keyInfoPckg().iLength = key_length;
        keyInfoPckg().iKeyNumber = keyNumber;
        keyInfoPckg().iPinNumber = pinNumber;

        WSL_OS_Free( pt_label.pb_buf );
        WSL_OS_Free( pt_keyid.pb_buf );
        // Code can leave after this point.

        aMessage.WriteL( 1, keyInfoPckg );
        }
    aMessage.Complete( CWimUtilityFuncs::MapWIMError( callStatus ) );
    }


// -----------------------------------------------------------------------------
// CWimKeyMgmtHandler::GetKeyListL
// Fetches the list of keys in a Wim.
// -----------------------------------------------------------------------------
//
void CWimKeyMgmtHandler::GetKeyListL(
    const RMessage2& aMessage,
    CWimMemMgmt* aWimMgmt ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimKeyMgmtHandler::GetKeyListL | Begin"));
    HBufC8* keyInfo = HBufC8::NewLC( KLabelLen );
    TPtr8 ptr = keyInfo->Des();

    WIMI_STAT status;
    WIMI_Ref_t* wimRef = aWimMgmt->WimRef();
    TUint16 keyNum = 0;
    WIMI_RefList_t refList;

    status = WIMI_GetKeyListByWIM( wimRef, &keyNum, &refList );
    if ( status == WIMI_Ok )
        {
        for ( TUint8 keyIndex = 0; keyIndex < keyNum; keyIndex++ )
            {
            ptr.AppendNum( ( TInt32 ) refList[keyIndex], EDecimal );
            ptr.Append( _L8(" ") ); //Space character
            }
        aWimMgmt->AppendWIMRefLstL( refList );
        }
    aMessage.WriteL( 0, keyInfo->Des() );
    TPckg<TInt> pckg( keyNum );
    aMessage.WriteL( 1, pckg );
    CleanupStack::PopAndDestroy( keyInfo );
    aMessage.Complete( CWimUtilityFuncs::MapWIMError( status ) );
    }

//  End of File  
