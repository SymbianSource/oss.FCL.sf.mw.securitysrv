/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Displays untrusted certificate dialog.
*
*/

#include "CTUntrustedCertQuery.h"           // CCTUntrustedCertQuery
#include <hb/hbcore/hbsymbianvariant.h>     // CHbSymbianVariantMap

// Note that the dialog type string, the parameters name strings, and the return code
// name string and values must match to those defined in Qt-side untrusted certificate
// dialog (in untrustedcertificatedefinitions.h file).

// Device dialog type for untrusted certificate dialog
_LIT( KUntrustedCertificateDialog, "com.nokia.untrustedcert/1.0" );

// Variant map parameter names for untrusted certificate dialog
_LIT( KUntrustedCertEncodedCertificate, "cert" );   // bytearray, mandatory
_LIT( KUntrustedCertServerName, "host" );           // string, mandatory
_LIT( KUntrustedCertValidationError, "err" );       // int (TValidationError), mandatory
_LIT( KUntrustedCertTrustedSiteStoreFail, "tss" );  // any, prevents permanent acceptance

// Dialog return code name and values
_LIT( KUntrustedCertDialogResult, "result" );       // int
const TInt KUntrustedCertDialogRejected = 0;
const TInt KUntrustedCertDialogAccepted = 1;
const TInt KUntrustedCertDialogAcceptedPermanently = 2;

// TODO: replace with OST tracing
#ifdef _DEBUG
#include <e32debug.h>
#define TRACE(x)        RDebug::Printf(x)
#define TRACE1(x,y)     RDebug::Printf(x,y)
#else
#define TRACE(x)
#define TRACE1(x,y)
#endif


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CCTUntrustedCertQuery::NewLC()
// ---------------------------------------------------------------------------
//
CCTUntrustedCertQuery* CCTUntrustedCertQuery::NewLC(
        TValidationError aValidationError, const TDesC8& aCertificate,
        const TDesC& aServerName, TBool aCanHandlePermanentAccept )
    {
    TRACE( "CCTUntrustedCertQuery::NewLC" );
    CCTUntrustedCertQuery* self = new ( ELeave ) CCTUntrustedCertQuery(
            aValidationError, aCertificate, aServerName,
            aCanHandlePermanentAccept );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
// CCTUntrustedCertQuery::~CCTUntrustedCertQuery()
// ---------------------------------------------------------------------------
//
CCTUntrustedCertQuery::~CCTUntrustedCertQuery()
    {
    TRACE( "CCTUntrustedCertQuery::~CCTUntrustedCertQuery" );
    Cancel();
    delete iWait;
    delete iDeviceDialog;
    delete iVariantMap;
    }

// ---------------------------------------------------------------------------
// CCTUntrustedCertQuery::ShowQueryAndWaitForResponseL()
// ---------------------------------------------------------------------------
//
void CCTUntrustedCertQuery::ShowQueryAndWaitForResponseL( TResponse& aResponse )
    {
    TRACE( "CCTUntrustedCertQuery::ShowQueryAndWaitForResponseL" );
    if( !iDeviceDialog )
        {
        iDeviceDialog = CHbDeviceDialogSymbian::NewL();
        }
    if( !iVariantMap )
        {
        iVariantMap = CHbSymbianVariantMap::NewL();
        }

    CHbSymbianVariant *variant = NULL;
    variant = CHbSymbianVariant::NewL( &iCertificate, CHbSymbianVariant::EBinary );
    User::LeaveIfError( iVariantMap->Add( KUntrustedCertEncodedCertificate, variant ) );
    variant = CHbSymbianVariant::NewL( &iValidationError, CHbSymbianVariant::EInt );
    User::LeaveIfError( iVariantMap->Add( KUntrustedCertValidationError, variant ) );
    variant = CHbSymbianVariant::NewL( &iServerName, CHbSymbianVariant::EDes );
    User::LeaveIfError( iVariantMap->Add( KUntrustedCertServerName, variant ) );
    if( !iCanHandlePermanentAccept )
        {
        variant = CHbSymbianVariant::NewL( &iCanHandlePermanentAccept, CHbSymbianVariant::EBool );
        User::LeaveIfError( iVariantMap->Add( KUntrustedCertTrustedSiteStoreFail, variant ) );
        }

    User::LeaveIfError( iDeviceDialog->Show( KUntrustedCertificateDialog, *iVariantMap, this ) );

    iStatus = KRequestPending;
    SetActive();
    TRACE( "CCTUntrustedCertQuery::ShowQueryAndWaitForResponseL, wait start" );
    iWait->Start();
    TRACE( "CCTUntrustedCertQuery::ShowQueryAndWaitForResponseL, wait end" );
    TRACE1( "CCTUntrustedCertQuery::ShowQueryAndWaitForResponseL, iWaitCompletionCode=%d", iWaitCompletionCode );
    User::LeaveIfError( iWaitCompletionCode );
    TRACE1( "CCTUntrustedCertQuery::ShowQueryAndWaitForResponseL, iResponse=%d", iResponse );
    aResponse = iResponse;
    }

// ---------------------------------------------------------------------------
// CCTUntrustedCertQuery::DoCancel()
// ---------------------------------------------------------------------------
//
void CCTUntrustedCertQuery::DoCancel()
    {
    TRACE( "CCTUntrustedCertQuery::DoCancel begin" );
    if( iDeviceDialog )
        {
        TRACE( "CCTUntrustedCertQuery::DoCancel, iDeviceDialog->Cancel()" );
        iDeviceDialog->Cancel();
        }
    if( iWait && iWait->IsStarted() && iWait->CanStopNow() )
        {
        TRACE( "CCTUntrustedCertQuery::DoCancel, iWait->AsyncStop()" );
        iWaitCompletionCode = KErrCancel;
        iWait->AsyncStop();
        }
    TRACE( "CCTUntrustedCertQuery::DoCancel end" );
    }

// ---------------------------------------------------------------------------
// CCTUntrustedCertQuery::RunL()
// ---------------------------------------------------------------------------
//
void CCTUntrustedCertQuery::RunL()
    {
    TRACE1( "CCTUntrustedCertQuery::RunL, iStatus.Int()=%d", iStatus.Int() );
    iWaitCompletionCode = iStatus.Int();
    if( iWait )
        {
        iWait->AsyncStop();
        }
    }

// ---------------------------------------------------------------------------
// CCTUntrustedCertQuery::DataReceived()
// ---------------------------------------------------------------------------
//
void CCTUntrustedCertQuery::DataReceived( CHbSymbianVariantMap& aData )
    {
    TRACE( "CCTUntrustedCertQuery::DataReceived" );
    const CHbSymbianVariant* variant = aData.Get( KUntrustedCertDialogResult );
    if( variant )
        {
        TInt* value = variant->Value<TInt>();
        if( value )
            {
            switch( *value )
                {
                case KUntrustedCertDialogRejected:
                    TRACE( "CCTUntrustedCertQuery::DataReceived, rejected" );
                    iResponse = EQueryRejected;
                    break;
                case KUntrustedCertDialogAccepted:
                    TRACE( "CCTUntrustedCertQuery::DataReceived, accepted" );
                    iResponse = EQueryAccepted;
                    break;
                case KUntrustedCertDialogAcceptedPermanently:
                    TRACE( "CCTUntrustedCertQuery::DataReceived, accepted permanently" );
                    iResponse = EQueryAcceptedPermanently;
                    break;
                default:
                    __ASSERT_DEBUG( EFalse, User::Invariant() );
                    break;
                }
            }
        else
            {
            __ASSERT_DEBUG( EFalse, User::Invariant() );
            }
        }
    else
        {
        __ASSERT_DEBUG( EFalse, User::Invariant() );
        }
    }

// ---------------------------------------------------------------------------
// CCTUntrustedCertQuery::DeviceDialogClosed()
// ---------------------------------------------------------------------------
//
void CCTUntrustedCertQuery::DeviceDialogClosed( TInt aCompletionCode )
    {
    TRACE1( "CCTUntrustedCertQuery::DeviceDialogClosed aCompletionCode=%d", aCompletionCode );
    TRequestStatus* status( &iStatus );
    User::RequestComplete( status, aCompletionCode );
    }

// ---------------------------------------------------------------------------
// CCTUntrustedCertQuery::CCTUntrustedCertQuery()
// ---------------------------------------------------------------------------
//
CCTUntrustedCertQuery::CCTUntrustedCertQuery(
        TValidationError aValidationError, const TDesC8& aCertificate,
        const TDesC& aServerName, TBool aCanHandlePermanentAccept ) :
        CActive( CActive::EPriorityStandard ), iValidationError( aValidationError ),
        iCertificate( aCertificate ), iServerName( aServerName ),
        iCanHandlePermanentAccept( aCanHandlePermanentAccept ),
        iResponse( EQueryRejected )
    {
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------------------------
// CCTUntrustedCertQuery::ConstructL()
// ---------------------------------------------------------------------------
//
void CCTUntrustedCertQuery::ConstructL()
    {
    TRACE( "CCTUntrustedCertQuery::ConstructL" );
    iWait = new( ELeave ) CActiveSchedulerWait;
    }

