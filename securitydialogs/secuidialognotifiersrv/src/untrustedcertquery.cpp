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
* Description:  Displays untrusted certificate dialog
*
*/

#include "untrustedcertquery.h"             // CUntrustedCertQuery
#include <hb/hbcore/hbsymbianvariant.h>     // CHbSymbianVariantMap
#include "secuidialogstrace.h"           // TRACE macro

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


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// CUntrustedCertQuery::NewL()
// ---------------------------------------------------------------------------
//
CUntrustedCertQuery* CUntrustedCertQuery::NewL(
        TValidationError aValidationError, const TDesC8& aCertificate,
        const TDesC& aServerName, TBool aCanHandlePermanentAccept )
    {
    TRACE( "CUntrustedCertQuery::NewL" );
    CUntrustedCertQuery* self = new ( ELeave ) CUntrustedCertQuery(
            aValidationError, aCertificate, aServerName,
            aCanHandlePermanentAccept );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ---------------------------------------------------------------------------
// CUntrustedCertQuery::~CUntrustedCertQuery()
// ---------------------------------------------------------------------------
//
CUntrustedCertQuery::~CUntrustedCertQuery()
    {
    TRACE( "CUntrustedCertQuery::~CUntrustedCertQuery, begin" );
    Cancel();
    delete iWait;
    iWait = NULL;
    delete iDeviceDialog;
    iDeviceDialog = NULL;
    delete iVariantMap;
    iVariantMap = NULL;
    TRACE( "CUntrustedCertQuery::~CUntrustedCertQuery, end" );
    }

// ---------------------------------------------------------------------------
// CUntrustedCertQuery::ShowQueryAndWaitForResponseL()
// ---------------------------------------------------------------------------
//
void CUntrustedCertQuery::ShowQueryAndWaitForResponseL( TResponse& aResponse )
    {
    TRACE( "CUntrustedCertQuery::ShowQueryAndWaitForResponseL, begin" );
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

    TRACE( "CUntrustedCertQuery::ShowQueryAndWaitForResponseL, iWait start" );
    iWait->Start();
    TRACE( "CUntrustedCertQuery::ShowQueryAndWaitForResponseL, iWaitCompletionCode=%d",
            iWaitCompletionCode );
    User::LeaveIfError( iWaitCompletionCode );
    TRACE( "CUntrustedCertQuery::ShowQueryAndWaitForResponseL, iResponse=%d", iResponse );
    aResponse = iResponse;
    }

// ---------------------------------------------------------------------------
// CUntrustedCertQuery::DoCancel()
// ---------------------------------------------------------------------------
//
void CUntrustedCertQuery::DoCancel()
    {
    TRACE( "CUntrustedCertQuery::DoCancel, begin" );
    if( iDeviceDialog )
        {
        TRACE( "CUntrustedCertQuery::DoCancel, cancelling device dialog" );
        iDeviceDialog->Cancel();
        }

    // Have to complete the request here, because cancelled device dialog does not
    // call DeviceDialogClosed() that normally completes it. The request needs to
    // be completed since CActive::Cancel() waits until the request is completed.
    TRACE( "CUntrustedCertQuery::DoCancel, completing self with KErrCancel" );
    TRequestStatus* status( &iStatus );
    User::RequestComplete( status, KErrCancel );

    // Normally the above request complete would trigger running RunL(). Now RunL()
    // is not run since the active object is already cancelled. Hence, have to stop
    // the waiting here so that iWait->Start() returns.
    iWaitCompletionCode = KErrCancel;
    if( iWait && iWait->IsStarted() )
        {
        TRACE( "CUntrustedCertQuery::DoCancel, stopping iWait" );
        iWait->AsyncStop();
        }
    TRACE( "CUntrustedCertQuery::DoCancel, end" );
    }

// ---------------------------------------------------------------------------
// CUntrustedCertQuery::RunL()
// ---------------------------------------------------------------------------
//
void CUntrustedCertQuery::RunL()
    {
    TRACE( "CUntrustedCertQuery::RunL, iStatus.Int()=%d", iStatus.Int() );
    iWaitCompletionCode = iStatus.Int();
    if( iWait && iWait->IsStarted() )
        {
        TRACE( "CUntrustedCertQuery::RunL, stopping iWait" );
        iWait->AsyncStop();
        }
    }

// ---------------------------------------------------------------------------
// CUntrustedCertQuery::DataReceived()
// ---------------------------------------------------------------------------
//
void CUntrustedCertQuery::DataReceived( CHbSymbianVariantMap& aData )
    {
    TRACE( "CUntrustedCertQuery::DataReceived" );
    const CHbSymbianVariant* variant = aData.Get( KUntrustedCertDialogResult );
    if( variant )
        {
        TInt* value = variant->Value<TInt>();
        if( value )
            {
            switch( *value )
                {
                case KUntrustedCertDialogRejected:
                    TRACE( "CUntrustedCertQuery::DataReceived, rejected" );
                    iResponse = EQueryRejected;
                    break;
                case KUntrustedCertDialogAccepted:
                    TRACE( "CUntrustedCertQuery::DataReceived, accepted" );
                    iResponse = EQueryAccepted;
                    break;
                case KUntrustedCertDialogAcceptedPermanently:
                    TRACE( "CUntrustedCertQuery::DataReceived, accepted permanently" );
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
// CUntrustedCertQuery::DeviceDialogClosed()
// ---------------------------------------------------------------------------
//
void CUntrustedCertQuery::DeviceDialogClosed( TInt aCompletionCode )
    {
    TRACE( "CUntrustedCertQuery::DeviceDialogClosed, aCompletionCode=%d", aCompletionCode );
    if( IsActive() )
        {
        TRACE( "CUntrustedCertQuery::DeviceDialogClosed, request complete" );
        TRequestStatus* status( &iStatus );
        User::RequestComplete( status, aCompletionCode );
        }
    }

// ---------------------------------------------------------------------------
// CUntrustedCertQuery::CUntrustedCertQuery()
// ---------------------------------------------------------------------------
//
CUntrustedCertQuery::CUntrustedCertQuery(
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
// CUntrustedCertQuery::ConstructL()
// ---------------------------------------------------------------------------
//
void CUntrustedCertQuery::ConstructL()
    {
    TRACE( "CUntrustedCertQuery::ConstructL" );
    iWait = new( ELeave ) CActiveSchedulerWait;
    }

