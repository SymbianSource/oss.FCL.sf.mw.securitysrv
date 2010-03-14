/*
* Copyright (c) 2002-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Certificate management methods
*
*/



// INCLUDE FILES
#include    "Wimi.h"            // wimi definitions
#include    "WimCertHandler.h"
#include    "WimMemMgmt.h"
#include    "WimSession.h"
#include    "WimResponse.h"
#include    "WimUtilityFuncs.h"
#include    <asn1enc.h>         // asn.1 encoding
#include    <asn1dec.h>         // asn.1 decoding
#include    <x509cert.h>        // cx509certificate
#ifndef SYMBIAN_ENABLE_SPLIT_HEADERS
#include    <x509keys.h>        // cx509rsapublickey
#else
#include    <x509keys.h>        // cx509rsapublickey
#include    <x509keyencoder.h>        // cx509rsapublickey
#endif
#include    <wtlscert.h>        // cwtlscertificate
#include    <wtlskeys.h>        // cwtlsrsapublickey
#include    "WimTrace.h"
#include    "WimCallbackImpl.h"
#include    "WimCleanup.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CWimCertHandler::CWimCertHandler
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CWimCertHandler::CWimCertHandler()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::CWimCertHandler | Begin"));
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CWimCertHandler::ConstructL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::ConstructL | Begin"));
    iWimUtilFuncs = CWimUtilityFuncs::NewL();
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CWimCertHandler* CWimCertHandler::NewL()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::NewL | Begin"));
    CWimCertHandler* self = new( ELeave ) CWimCertHandler;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// Destructor
CWimCertHandler::~CWimCertHandler()
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::~CWimCertHandler | Begin"));
    delete iWimUtilFuncs;

    for( TInt index = 0; index < iCertRefLst.Count(); ++index )
        {
        WIMI_Ref_t* ref = reinterpret_cast< WIMI_Ref_t* >( iCertRefLst[ index ] );
        _WIMTRACE2(_L("WIM | WIMServer | CWimCertHandler::~CWimCertHandler, -ref 0x%08x"), ref);
        free_WIMI_Ref_t( ref );
        iCertRefLst[ index ] = 0;
        }
    iCertRefLst.Reset();
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::GetCertificatesFromWimL
// Fetches certificates from WIM card.
// -----------------------------------------------------------------------------
//
void CWimCertHandler::GetCertificatesFromWimL(
    const RMessage2& aMessage,
    CWimMemMgmt* aWimMgmt )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::GetCertificatesFromWimL | Begin"));
    WIMI_STAT callStatus = WIMI_Ok;
    TUint8 certNum = 0;
    WIMI_Ref_t* temp = NULL;

    HBufC8* certRefLst = iWimUtilFuncs->DesLC( 0, aMessage );
    HBufC8* certInfoLst = iWimUtilFuncs->DesLC( 1, aMessage );

    TWimEntryType certEntryType = ( TWimEntryType ) aMessage.Int2();

    TUint32* pCertRefLst = ( TUint32* )( certRefLst->Des().Ptr() );
    TWimCertInfo* pCertInfoLst = ( TWimCertInfo* )( certInfoLst->Des().Ptr() );

    __ASSERT_ALWAYS( aWimMgmt, User::Leave( KErrArgument ) );
    temp = MainWimRef( *aWimMgmt );

    if ( temp )
        {
        if ( EWimEntryTypeAll == certEntryType ||
             EWimEntryTypeCA == certEntryType )
            {
            callStatus = GetCertificateFromWimRefL( temp, WIMI_CU_CA,
                                                    certNum, pCertRefLst,
                                                    pCertInfoLst, aMessage );

            }
        if ( callStatus == WIMI_Ok && ( EWimEntryTypeAll == certEntryType ||
                                   EWimEntryTypePersonal == certEntryType ) )
            {
            callStatus = GetCertificateFromWimRefL( temp, WIMI_CU_Client,
                                                    certNum, pCertRefLst,
                                                    pCertInfoLst, aMessage );

            }

        //record the ref for sanity checking, deallocate old refs first
        for( TInt index = 0; index < iCertRefLst.Count(); ++index )
            {
            WIMI_Ref_t* ref = reinterpret_cast< WIMI_Ref_t* >( iCertRefLst[ index ] );
            _WIMTRACE2(_L("WIM | WIMServer | CWimCertHandler::GetCertificatesFromWimL, -ref 0x%08x"), ref);
            free_WIMI_Ref_t( ref );
            iCertRefLst[ index ] = 0;
            }
        iCertRefLst.Reset();
        for( TInt index = 0; index < certNum; ++index )
            {
            _WIMTRACE2(_L("WIM | WIMServer | CWimCertHandler::GetCertificatesFromWimL, +ref 0x%08x"), pCertRefLst[ index ]);
            iCertRefLst.AppendL( pCertRefLst[ index ] );
            }

        aMessage.WriteL( 0, certRefLst->Des() );
        aMessage.WriteL( 1, certInfoLst->Des() );
        }
    else
        {
        callStatus = WIMI_ERR_OutOfMemory;
        }
    CleanupStack::PopAndDestroy( 2, certRefLst );   // certInfoLst, certRefLst
    aMessage.Complete( CWimUtilityFuncs::MapWIMError( callStatus ) );
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::GetCertificateFromWimRefL
// Fetches certificate from the WIM card.
// -----------------------------------------------------------------------------
//
WIMI_STAT CWimCertHandler::GetCertificateFromWimRefL(
    WIMI_Ref_t* aTmpWimRef,
    TInt8 aUsage,
    TUint8& aCertNum,
    TUint32* aCertRefLst,
    TWimCertInfo* aCertInfoLst,
    const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::GetCertificateFromWimRefL | Begin"));
    TUint8 tempCrtCount;
    WIMI_RefList_t refList = NULL;
    WIMI_STAT callStatus = WIMI_ERR_BadReference;

    if ( aTmpWimRef )
        {
        callStatus = WIMI_GetCertificateListByWIM( aTmpWimRef,
                                                   aUsage,
                                                   &tempCrtCount,
                                                   &refList );

        if ( callStatus == WIMI_Ok )
            {
            // If the certifcate length is 0,
            // the step increases by 1.
            TInt step = 0;
            TInt err = KErrNone;
            for ( TUint8 certIndex = 0; certIndex < tempCrtCount; certIndex++ )
                {
                TInt current = aCertNum + certIndex - step;
                err = CopyCertificateInfo( aCertInfoLst[current], refList[certIndex], aMessage );

                if ( err == KErrNone )
                    {
                    // transfers ownership of refList item to aCertRefLst
                    aCertRefLst[current] = reinterpret_cast< TUint32 >( refList[certIndex] );
                    refList[certIndex] = NULL;
                    }
                else // KErrArgument
                    {
                    // ingore certificate info and continue with the next
                	step++;
                    free_WIMI_Ref_t( refList[certIndex] );
                    }
                }
            //variable step is equal to the number of certificate in CDF whose
            //length is set as 0.
            aCertNum = static_cast< TUint8 >( aCertNum + tempCrtCount - step );

            // Because list items are moved to aCertRefLst, only refList array
            // needs to be freed. Cannot use free_WIMI_RefList_t() as it would
            // delete also items contained in refList.
            WSL_OS_Free( refList );
            }
        }

    return callStatus;
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::CopyCertificateInfo
// Copies certificate information to client's allocated memory area.
// This function MAY NOT leave.
// -----------------------------------------------------------------------------
//
TInt CWimCertHandler::CopyCertificateInfo(
    TWimCertInfo& aCertInfo,
    WIMI_Ref_t* aCert,
    const RMessage2& /*aMessage*/ )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::CopyCertificateInfo | Begin"));
    WIMI_Ref_t* tempRef = NULL;
    WIMI_BinData_t ptLabel;
    WIMI_BinData_t ptKeyID;
    WIMI_BinData_t ptCAID;
    WIMI_BinData_t ptIssuerHash;
    WIMI_BinData_t ptTrustedUsage;
    TUint8 uiCDFRefs;
    TUint8 usage;
    TUint8 type;
    TUint16 certLen;
    TUint8 modifiable = 0;
    WIMI_STAT callStatus = WIMI_GetCertificateInfo(
                                aCert,
                                &tempRef,
                                &ptLabel,
                                &ptKeyID, /* Key Id (hash)*/
                                &ptCAID,
                                &ptIssuerHash,
                                &ptTrustedUsage,
                                &uiCDFRefs,
                                &usage,  /* 0 = client, 1 = CA */
                                &type,   /* WTLSCert(1),
                                            X509Cert(2),
                                            X968Cert(3),
                                            CertURL(4) */
                                &certLen,   /* cert. content or URL length */
                                &modifiable );

    if ( callStatus == WIMI_Ok )
        {
        free_WIMI_Ref_t( tempRef );

        // To make sure the length of the certificate is not zero
        _WIMTRACE2(_L("WIM | WIMServer | CWimCertHandler::CopyCertificateInfoL | certLen %d"), certLen);
        if ( certLen == 0 )
            {
            WSL_OS_Free( ptLabel.pb_buf );
            WSL_OS_Free( ptKeyID.pb_buf );
            WSL_OS_Free( ptCAID.pb_buf );
            WSL_OS_Free( ptIssuerHash.pb_buf );
            WSL_OS_Free( ptTrustedUsage.pb_buf );
        	return KErrArgument;
            }

        // it is x509cert
        if ( type == WIMI_CT_X509 && certLen != 0 )
            {
            //use this rough sanity checking for temp
            if ( certLen < 10 )
                {
                _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::CopyCertificateInfoL | cert is malformated, return KErrArgument"));
                WSL_OS_Free( ptLabel.pb_buf );
                WSL_OS_Free( ptKeyID.pb_buf );
                WSL_OS_Free( ptCAID.pb_buf );
                WSL_OS_Free( ptIssuerHash.pb_buf );
                WSL_OS_Free( ptTrustedUsage.pb_buf );
                return KErrArgument;
                }
            }
        _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::CopyCertificateInfoL | X509 check ok"));

        aCertInfo.iLabel.Copy(
            TPtr8(
                ptLabel.pb_buf,
                ptLabel.ui_buf_length,
                ptLabel.ui_buf_length ) );

        aCertInfo.iKeyId.Copy(
            TPtr8(
                ptKeyID.pb_buf,
                ptKeyID.ui_buf_length,
                ptKeyID.ui_buf_length ) );

        aCertInfo.iCAId.Copy(
            TPtr8(
                ptCAID.pb_buf,
                ptCAID.ui_buf_length,
                ptCAID.ui_buf_length ) );

        aCertInfo.iIssuerHash.Copy(
            TPtr8(
                ptIssuerHash.pb_buf,
                ptIssuerHash.ui_buf_length,
                ptIssuerHash.ui_buf_length ) );

        aCertInfo.iUsage = usage;
        aCertInfo.iType = type;
        aCertInfo.iCertlen = certLen;
        aCertInfo.iModifiable = modifiable;

        // Certificate location
        aCertInfo.iCDFRefs = iWimUtilFuncs->MapCertLocation( uiCDFRefs );

        //Allocate enough memory for OID
        aCertInfo.iTrustedUsageLength = ptTrustedUsage.ui_buf_length * 16;

        WSL_OS_Free( ptLabel.pb_buf );
        WSL_OS_Free( ptKeyID.pb_buf );
        WSL_OS_Free( ptCAID.pb_buf );
        WSL_OS_Free( ptIssuerHash.pb_buf );
        WSL_OS_Free( ptTrustedUsage.pb_buf );
        }

    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::GetExtrasFromWimL
// Fetches certificate's extra info from WIM card. In this case it is
// trusted usage info.
// -----------------------------------------------------------------------------
//
void CWimCertHandler::GetExtrasFromWimL(
    const RMessage2& aMessage,
    CWimMemMgmt* aWimMgmt )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::GetExtrasFromWimL | Begin"));

    WIMI_STAT callStatus = WIMI_Ok;
    TInt8 certUsage = 0;

    HBufC8* keyIdBuf = iWimUtilFuncs->DesLC( 0, aMessage );
    TPtrC8 keyIdHash = keyIdBuf->Des();

    __ASSERT_ALWAYS( aWimMgmt, User::Leave( KErrArgument ) );
    WIMI_Ref_t* wimTempRef = MainWimRef( *aWimMgmt );

    if ( wimTempRef )
        {
        TWimEntryType certEntryType = static_cast< TWimEntryType >( aMessage.Int2() );
        switch ( certEntryType )
            {
            case EWimEntryTypeCA:
                {
                certUsage = WIMI_CU_CA;
                break;
                }
            case EWimEntryTypeAll: //Flow through
            case EWimEntryTypePersonal:
                {
                certUsage = WIMI_CU_Client;
                break;
                }
            default:
                {
                callStatus = WIMI_ERR_BadParameters;
                break;
                }
            }

        if ( callStatus == WIMI_Ok )
            {
            callStatus = GetExtrasFromWimRefL( wimTempRef,
                                               certUsage,
                                               keyIdHash,
                                               aMessage );
            }
        }
    else
        {
        callStatus = WIMI_ERR_BadReference;
        }

    CleanupStack::PopAndDestroy( keyIdBuf );
    aMessage.Complete( CWimUtilityFuncs::MapWIMError( callStatus ) );
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::GetExtrasFromWimRefL
// Fetches extra information (e.g. certs trusted usage) from the WIM card.
// -----------------------------------------------------------------------------
//
WIMI_STAT CWimCertHandler::GetExtrasFromWimRefL(
    WIMI_Ref_t* aTmpWimRef,
    TInt8 aUsage,
    TDesC8& aKeyHash,
    const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::GetExtrasFromWimRefL | Begin"));

    TUint8 tempCertCount = 0;
    WIMI_RefList_t certRefList = NULL;
    WIMI_STAT callStatus = WIMI_Ok;
    TInt certIndex = 0;
    TPtrC8 keyHash;

    if ( aTmpWimRef )
        {
        // List all certificates (by WIM and usage)
        callStatus = WIMI_GetCertificateListByWIM( aTmpWimRef,
                                                   aUsage,
                                                   &tempCertCount,
                                                   &certRefList );
        }
    else
        {
        callStatus = WIMI_ERR_BadReference;
        }

    if ( callStatus == WIMI_Ok )
        {
        CleanupPushWimRefListL( certRefList );

        WIMI_Ref_t* tempRef = NULL;
        WIMI_BinData_t ptLabel;
        WIMI_BinData_t ptKeyID;
        WIMI_BinData_t ptCAID;
        WIMI_BinData_t ptIssuerHash;
        WIMI_BinData_t ptTrustedUsage;
        TUint8 uiCDFRefs;
        TUint8 usage;
        TUint8 certType;
        TUint16 certLen;
        TUint8 modifiable = 0;

        for ( TInt i = 0; i < tempCertCount; i++ )
            {
            // Get info for each certificate until we find valid cert
            callStatus = WIMI_GetCertificateInfo( certRefList[i],
                                                  &tempRef,
                                                  &ptLabel,
                                                  &ptKeyID,
                                                  &ptCAID,
                                                  &ptIssuerHash,
                                                  &ptTrustedUsage,
                                                  &uiCDFRefs,
                                                  &usage,
                                                  &certType,
                                                  &certLen,
                                                  &modifiable );
            if ( callStatus == WIMI_Ok )
                {
                free_WIMI_Ref_t( tempRef );

                // Code MAY NOT leave before ptLabel.pb_buf, ptKeyID.pb_buf,
                // ptCAID.pb_buf, ptIssuerHash.pt_buf, and ptTrustedUsage.pb_buf
                // are deallocated.

                keyHash.Set( ptKeyID.pb_buf, ptKeyID.ui_buf_length );

                // Compare given and fetched key hash
                if ( keyHash.Compare( aKeyHash ) == 0 &&
                     certType == WIMI_CT_X509 ) //Match
                    {
                    certIndex = i; // Found one
                    i = tempCertCount; // Stop looping
                    callStatus = WIMI_Ok;
                    }
                else // Cert not supported
                    {
                    callStatus = WIMI_ERR_UnsupportedCertificate;
                    }

                WSL_OS_Free( ptLabel.pb_buf );
                WSL_OS_Free( ptKeyID.pb_buf );
                WSL_OS_Free( ptCAID.pb_buf );
                WSL_OS_Free( ptIssuerHash.pb_buf );
                WSL_OS_Free( ptTrustedUsage.pb_buf );
                // Code can leave after this point.
                }
            }

        if ( callStatus == WIMI_Ok )
            {
            CopyCertExtrasInfoL( certRefList[certIndex], aMessage );
            }

        CleanupStack::PopAndDestroy( certRefList );
        }

    return callStatus;
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::CopyCertExtrasInfoL
// Copies certs extra information to client's allocated memory area.
// -----------------------------------------------------------------------------
//
void CWimCertHandler::CopyCertExtrasInfoL(
    WIMI_Ref_t* aCert,
    const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::CopyCertExtrasInfoL | Begin"));
    TUint8 pushedItemCount = 0;
    WIMI_Ref_t* tempref = NULL;
    WIMI_BinData_t ptLabel;
    WIMI_BinData_t ptKeyID;
    WIMI_BinData_t ptCAID;
    WIMI_BinData_t ptIssuerHash;
    WIMI_BinData_t ptTrustedUsage;
    TUint8 uiCDFRefs;
    TUint8 usage;
    TUint8 type;
    TUint16 certlen;
    TUint8 modifiable = 0;
    TBool found = ETrue;

    TCertExtrasInfo certExtraInfo;
    certExtraInfo.iCDFRefs = 0;
    certExtraInfo.iTrustedUsage = NULL;

    WIMI_STAT callStatus = WIMI_GetCertificateInfo(
                                aCert,
                                &tempref,
                                &ptLabel,
                                &ptKeyID, /* Key Id (hash)*/
                                &ptCAID,
                                &ptIssuerHash,
                                &ptTrustedUsage,
                                &uiCDFRefs,
                                &usage,  /* 0 = client, 1 = CA */
                                &type,
                                &certlen,   /* cert. content or URL length */
                                &modifiable);
    if ( callStatus == WIMI_Ok )
        {
        free_WIMI_Ref_t( tempref );
        WSL_OS_Free( ptLabel.pb_buf );
        WSL_OS_Free( ptKeyID.pb_buf );
        WSL_OS_Free( ptCAID.pb_buf );
        WSL_OS_Free( ptIssuerHash.pb_buf );

        CleanupPushWimBufL( ptTrustedUsage );
        pushedItemCount++;

        TPtrC8 undecodedUsage;
        undecodedUsage.Set( ptTrustedUsage.pb_buf );

        if ( ptTrustedUsage.ui_buf_length == 0 ) // No OIDs
            {
            found = EFalse;
            }

        // DECODE OIDs
        TASN1DecObjectIdentifier decoder;
        RPointerArray<HBufC> decodedOIDArray;
        CleanupResetAndDestroyPushL( decodedOIDArray );
        pushedItemCount++;
        HBufC* decodedOIDs = NULL;
        TInt oidsLength = 0;    // length of total OID buffer
        TInt err;

        for ( TInt position = 0; found; )   //Loop until no OIDs found anymore
            {
            if ( undecodedUsage.Length() > position ) //Don't go over buffer
                {
                TRAP( err, decodedOIDs = decoder.DecodeDERL( undecodedUsage, position ) );
                if ( err == KErrNone ) //Found OID
                    {
                    CleanupStack::PushL( decodedOIDs );
                    if ( decodedOIDs->Length() )
                        {
                        found = ETrue;
                        decodedOIDArray.AppendL( decodedOIDs );
                        oidsLength += decodedOIDs->Length();
                        CleanupStack::Pop( decodedOIDs );
                        }
                    else    // Not found OID from buffer
                        {
                        found = EFalse;
                        CleanupStack::PopAndDestroy( decodedOIDs );
                        }
                    decodedOIDs = NULL;
                    }
                else    // Error in OID parsing -> Not found OID
                    {
                    found = EFalse;
                    }
                }
            else    // undecoded OID buffer seeked through
                {
                found = EFalse;
                }
            }

        _LIT( KDelimeter16, " " ); //Delimeter between OIDs
        TInt trustedUsagesBufMaxLength = oidsLength + decodedOIDArray.Count();
        TPtr trustedUsage( NULL, 0, trustedUsagesBufMaxLength );

        if ( oidsLength > 0 ) // OID's found
            {
            // Alloc new buffer for all OID's
            HBufC* trustedUsagesBuf = HBufC::NewLC( trustedUsagesBufMaxLength );
            pushedItemCount++;

            trustedUsage.Set( trustedUsagesBuf->Des() );

            // Add OID's to one buffer from separate buffers
            for ( TInt i = 0; i < decodedOIDArray.Count(); i++ )
                {
                if ( i == 0 ) //First one
                    {
                    trustedUsage.Copy( decodedOIDArray[i]->Des() );
                    }
                else // Append other OIDs, with delimeter
                    {
                    trustedUsage.Append( KDelimeter16 );
                    trustedUsage.Append( decodedOIDArray[i]->Des() );
                    }
                }
            }

        TPckgBuf<TCertExtrasInfo> wimCertExtraPckg( certExtraInfo );
        aMessage.ReadL( 1, wimCertExtraPckg );

        wimCertExtraPckg().iCDFRefs = iWimUtilFuncs->MapCertLocation( uiCDFRefs );

        if ( oidsLength > 0 ) // OID's found, write buffer to client
            {
            aMessage.WriteL( 3, trustedUsage );
            }
        aMessage.WriteL( 1, wimCertExtraPckg );

        CleanupStack::PopAndDestroy( pushedItemCount, ptTrustedUsage.pb_buf );
        }
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::GetCerticateCountL
// Fetches count of certicates in certain WIM card.
// -----------------------------------------------------------------------------
//
void CWimCertHandler::GetCerticateCountL(
    const RMessage2& aMessage,
    CWimMemMgmt* aWimMgmt ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::GetCerticateCountL | Begin"));
    WIMI_STAT callStatus = WIMI_Ok;
    TWimEntryType certEntryType = ( TWimEntryType )aMessage.Int1();

    __ASSERT_ALWAYS( certEntryType != EWimEntryTypeAll ||
        certEntryType != EWimEntryTypeCA ||
        certEntryType != EWimEntryTypePersonal, User::Leave( KErrArgument ) );

    WIMI_Ref_t* wimRef = NULL;
    TUint8 certCount = 0;

    __ASSERT_ALWAYS( aWimMgmt, User::Leave( KErrArgument ) );
    wimRef = MainWimRef( *aWimMgmt );

    if ( wimRef )
        {
        if ( EWimEntryTypeAll == certEntryType ||
            EWimEntryTypeCA == certEntryType )
            {
            callStatus = GetCertificateCountByWIM( wimRef,
                                                   certCount,
                                                   WIMI_CU_CA );
            }

        if ( callStatus == WIMI_Ok && ( EWimEntryTypeAll == certEntryType ||
                                     EWimEntryTypePersonal == certEntryType ) )
            {
            callStatus = GetCertificateCountByWIM( wimRef,
                                                   certCount,
                                                   WIMI_CU_Client );
            }
        }
    else
        {
        callStatus = WIMI_ERR_OutOfMemory;
        }

    if ( callStatus == WIMI_Ok )
        {
        _WIMTRACE2(_L("WIM | WIMServer | CWimCertHandler::GetCerticateCountL | count=%d"), certCount);
        TPckgBuf<TUint8> pckg( certCount );
        aMessage.WriteL( 0, pckg );
        }
    aMessage.Complete( CWimUtilityFuncs::MapWIMError( callStatus ) );
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::GetCertificateCountByWIM
// Fetches count of certicates in certain WIM card.
// This function MAY NOT leave.
// -----------------------------------------------------------------------------
//
WIMI_STAT CWimCertHandler::GetCertificateCountByWIM(
    WIMI_Ref_t* aRef,
    TUint8& aCertCount,
    TUint8 aUsage ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::GetCertificateCountByWIM | Begin"));

    // Get the number of certificates from smart card.
    TUint8 certNum = 0;
    WIMI_RefList_t refList ;
    WIMI_STAT callStatus = WIMI_GetCertificateListByWIM( aRef,
                                                         aUsage,
                                                         &certNum,
                                                         &refList );

    if ( callStatus != WIMI_Ok )
        {
    	return callStatus;
        }

    // Find out how many certificate has zero length
    TInt certLenZero = 0;
    TInt certMalformat = 0;
    for ( TInt i = 0; i < certNum ; i++ )
	    {
	    WIMI_Ref_t* tempRef = NULL;
	    WIMI_BinData_t ptLabel;
	    WIMI_BinData_t ptKeyID;
	    WIMI_BinData_t ptCAID;
	    WIMI_BinData_t ptIssuerHash;
	    WIMI_BinData_t ptTrustedUsage;
	    TUint8 uiCDFRefs;
	    TUint8 usage;
	    TUint8 type;
	    TUint16 certLen;
	    TUint8 modifiable = 0;
	    callStatus = WIMI_GetCertificateInfo(
	                                refList[i],
	                                &tempRef,
	                                &ptLabel,
	                                &ptKeyID, /* Key Id (hash)*/
	                                &ptCAID,
	                                &ptIssuerHash,
	                                &ptTrustedUsage,
	                                &uiCDFRefs,
	                                &usage,  /* 0 = client, 1 = CA */
	                                &type,   /* WTLSCert(1),
	                                            X509Cert(2),
	                                            X968Cert(3),
	                                            CertURL(4) */
	                                &certLen,   /* cert. content or URL length */
	                                &modifiable );

	    if ( callStatus == WIMI_Ok )
	        {
	        free_WIMI_Ref_t( tempRef );

	        if ( certLen == 0 )
	            {
	            certLenZero++;
	            }

            // it is x509cert
            if ( type == WIMI_CT_X509 && certLen != 0 )
                {

                //use this rough sanity checking
                if ( certLen < 10 )
                    {
                    certMalformat++;
                    }
                }

            WSL_OS_Free( ptLabel.pb_buf );
            WSL_OS_Free( ptKeyID.pb_buf );
            WSL_OS_Free( ptCAID.pb_buf );
            WSL_OS_Free( ptIssuerHash.pb_buf );
            WSL_OS_Free( ptTrustedUsage.pb_buf );
	        }
	    }

    // only return the number of ceritifcates that have non-zero length
    if ( callStatus == WIMI_Ok )
        {
        aCertCount = static_cast< TUint8 >( aCertCount + certNum - certLenZero - certMalformat );
        }

    free_WIMI_RefList_t( refList );

    return callStatus;
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::StoreCertificateL
// Stores certificate to the WIM card.
// -----------------------------------------------------------------------------
//
void CWimCertHandler::StoreCertificateL(
    TWimServRqst /*aOpcode*/,
    const RMessage2& aMessage ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::StoreCertificateL | Begin"));
    aMessage.Complete( KErrNotSupported );
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::RemoveCertificateL
// Removes certificate from a WIM card.
// -----------------------------------------------------------------------------
//
void CWimCertHandler::RemoveCertificateL(
    const RMessage2& aMessage,
    CWimMemMgmt* /*aWimMgmt*/ ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::RemoveCertificateL | Begin"));
    aMessage.Complete( KErrNotSupported );
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::SanityCheck
// -----------------------------------------------------------------------------
//
TBool CWimCertHandler::SanityCheck( TUint32 aCertRef )
    {
    TInt certNum = iCertRefLst.Count();
    if ( certNum == 0 )
        {
        return EFalse;
        }

    for ( TInt index = 0; index < certNum; ++index )
        {
        if( aCertRef == iCertRefLst[ index ] )
            {
            return ETrue;
            }
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::GetCertificateDetailsL
// Fetches certificate details.
// -----------------------------------------------------------------------------
//
void CWimCertHandler::GetCertificateDetailsL(
    TWimServRqst aOpCode,
    const RMessage2& aMessage )
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimCertHandler::GetCertificateDetailsL | Begin"));
    TBool IsOk = SanityCheck( (TUint32)aMessage.Ptr0() );
    if ( ! IsOk  )
        {
        aMessage.Panic(_L("WIM"), KErrBadHandle );
        return;
        }

    //capability checking
    TUint8 usage = 255;
    WIMI_STAT callStatus = ResolveCertUsage( aMessage, usage );
    if ( WIMI_Ok == callStatus )
        {
        if( !CheckReadCapsForUsage( aMessage, usage  ) )
              {
              aMessage.Complete(KErrPermissionDenied);
              return;
              }
        }

    //capabilty ok, go to fetch the details of the certificate.
    WIMI_Ref_pt pCertRef = const_cast<WIMI_Ref_pt>( aMessage.Ptr0() );
    CWimResponse* responseID = new( ELeave ) CWimResponse( aMessage );
    CleanupStack::PushL( responseID );
    TWimReqTrId* trId = iWimUtilFuncs->TrIdLC( responseID, EWimMgmtReq );
    responseID->iOpcode = aOpCode;
    CleanupStack::Pop( 2, responseID ); // trId, responseID

    WIMI_STAT certReqStatus = WIMI_CertificateReq( trId, pCertRef );
    if ( certReqStatus != WIMI_Ok )
        {
        responseID->iStatus = certReqStatus;
        responseID->CompleteMsgAndDelete();
        delete trId;
        }
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::ResolveCertUsage
// Resolves usage (CA/User) for a certificate.
// -----------------------------------------------------------------------------
//
WIMI_STAT CWimCertHandler::ResolveCertUsage( const RMessage2& aMsg,
                                        TUint8& aUsage )
    {
    _WIMTRACE(_L("CWimServer::ResolveCertUsage | Begin"));

    // aMsg.Ptr0 contains reference to certificate

    WIMI_Ref_pt pCertRef = const_cast< WIMI_Ref_pt >( aMsg.Ptr0() );
    _WIMTRACE2(_L("CWimServer::ResolveCertUsage | Begin aMsg.Ptr0() = %d"), aMsg.Ptr0());

    WIMI_STAT callStatus = GetCertificateInfo( pCertRef, aUsage );

    _WIMTRACE(_L("CWimServer::ResolveCertUsage | End"));
    return callStatus;
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::GetCertificateInfo
// Fetches certificate info. Wrapper for WIMI call.
// -----------------------------------------------------------------------------
//
WIMI_STAT CWimCertHandler::GetCertificateInfo( WIMI_Ref_pt aCertRef,
                                          TUint8& aUsage )
    {
    _WIMTRACE(_L("CWimServer::GetCertificateInfo | Begin"));

    WIMI_Ref_pt     pWimRef = NULL;
    WIMI_BinData_t  ptLabel;
    WIMI_BinData_t  ptKeyID;
    WIMI_BinData_t  ptCAID;
    WIMI_BinData_t  ptIssuerHash;
    WIMI_BinData_t  ptTrustedUsage;
    TUint8          uiCDFRefs;
    TUint8          type;
    TUint16         certLen;
    TUint8          modifiable = 0;
    _WIMTRACE(_L("CWimServer::GetCertificateInfo | Begin 1"));
    WIMI_STAT callStatus = WIMI_GetCertificateInfo(
                                aCertRef,
                                &pWimRef,
                                &ptLabel,
                                &ptKeyID, // Key Id (hash)
                                &ptCAID,
                                &ptIssuerHash,
                                &ptTrustedUsage,
                                &uiCDFRefs,
                                &aUsage,  // 0 = client, 1 = CA
                                &type,    // WTLSCert(1),
                                          // X509Cert(2),
                                          // X968Cert(3),
                                          // CertURL(4)
                                &certLen, // cert. content or URL length
                                &modifiable );
    _WIMTRACE(_L("CWimServer::GetCertificateInfo | Begin 2"));
    // Don't need references anymore
    if ( callStatus == WIMI_Ok )
        {
        free_WIMI_Ref_t( pWimRef );
        WSL_OS_Free( ptLabel.pb_buf );
        WSL_OS_Free( ptKeyID.pb_buf );
        WSL_OS_Free( ptCAID.pb_buf );
        WSL_OS_Free( ptIssuerHash.pb_buf );
        WSL_OS_Free( ptTrustedUsage.pb_buf );
        }

    _WIMTRACE(_L("CWimServer::GetCertificateInfo | End"));
    return callStatus;
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::GetCertificateInfo
// Fetches certificate info. Wrapper for WIMI call.
// -----------------------------------------------------------------------------
//
TBool CWimCertHandler::CheckReadCapsForUsage( const RMessage2& aMsg,
                               TUint8 aUsage )
    {
    TBool result = EFalse;

    switch ( aUsage )
        {
        case WIMI_CU_CA:
            {
            // CA certificate reading doesn't require any capability.
            _WIMTRACE(_L("CWimCertHandler::CheckReadCapsForUsage: CA cert read req, OK."));
            result = ETrue;
            break;
            }
        case WIMI_CU_Client:
            {
            // User certificate reading requires ReadUserData capability.
            if ( aMsg.HasCapability( ECapabilityReadUserData ))
                {
                result = ETrue;
                _WIMTRACE(_L("CWimCertHandler::CheckReadCapsForUsage: User cert read capa PASS"));
                }
            else
                {
                result = EFalse;
                _WIMTRACE(_L("CWimCertHandler::CheckReadCapsForUsage: User cert read capa FAIL"));
                }
            break;
            }
        default:
            {
            _WIMTRACE(_L("CWimCertHandler::CheckReadCapsAccordingToUsage: FAIL:Unknown usage."));
            result = EFalse;
            break;
            }
        }
    _WIMTRACE(_L("CWimCertHandler::CheckReadCapsForUsage | End"));
    return result;
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::ExportPublicKeyL
// Export public key from certificate.
// -----------------------------------------------------------------------------
//
void CWimCertHandler::ExportPublicKeyL( const RMessage2& aMessage ) const
    {
    _WIMTRACE(_L("WIM | WIMServer | CWimPublicKeyHandler::ExportPublicKeyL | Begin"));

    TUint8 certType = 0;

    TPckgBuf<TExportPublicKey> keyExportPckg;
    aMessage.ReadL( 0, keyExportPckg );

    HBufC8* publicKeyBuf = HBufC8::NewLC( KPublicKeyLength );

    TBuf8<KKeyIdLen> keyIdBuf = keyExportPckg().iKeyId;

    WIMI_STAT callStatus = WIMI_Ok;
    WIMI_BinData_t ptCertData;
    ptCertData.pb_buf = NULL;
    ptCertData.ui_buf_length = 0;
    WIMI_Ref_t* pCert = NULL;
    WIMI_TransactId_t trId = ( void* )EInitializationCertListHashing;
    TUint8 certCount = 0;
    WIMI_RefList_t certRefList = NULL;

    // Check firs the Client certs
    callStatus = WIMI_GetCertificateListByKeyHash( ( TUint8* )keyIdBuf.Ptr(),
                                                   WIMI_CU_Client,
                                                   &certCount,
                                                   &certRefList );
    if ( callStatus == WIMI_Ok )
        {
        if ( certCount == 0 ) // Not found client certs
            {
            free_WIMI_RefList_t( certRefList );
            certRefList = NULL;

            // Check CA certs
            callStatus = WIMI_GetCertificateListByKeyHash(
                                                    ( TUint8* )keyIdBuf.Ptr(),
                                                    WIMI_CU_CA,
                                                    &certCount,
                                                    &certRefList );
            }
        }

    if ( callStatus == WIMI_Ok )
        {
        CleanupPushWimRefListL( certRefList );
        if ( certCount == 0 ) // No certificates found for key
            {
            CleanupStack::PopAndDestroy( certRefList );
            callStatus = WIMI_ERR_CertNotFound;
            }
        }

    if ( callStatus == WIMI_Ok )
        {
        WIMI_Ref_t* tempRef = NULL;
        WIMI_BinData_t ptLabel;
        WIMI_BinData_t ptKeyID;
        WIMI_BinData_t ptCAID;
        WIMI_BinData_t ptIssuerHash;
        WIMI_BinData_t ptTrustedUsage;
        TUint8 uiCDFRefs;
        TUint8 usage;

        TUint16 certLen;
        TUint8 modifiable = 0;

        for ( TInt i = 0; i < certCount; i++ )
            {
            // Get info for each certificate until we find valid cert
            callStatus = WIMI_GetCertificateInfo( certRefList[i],
                                                  &tempRef,
                                                  &ptLabel,
                                                  &ptKeyID,
                                                  &ptCAID,
                                                  &ptIssuerHash,
                                                  &ptTrustedUsage,
                                                  &uiCDFRefs,
                                                  &usage,
                                                  &certType,
                                                  &certLen,
                                                  &modifiable );
            if ( callStatus == WIMI_Ok )
                {
                WSL_OS_Free( ptLabel.pb_buf );
                WSL_OS_Free( ptKeyID.pb_buf );
                WSL_OS_Free( ptCAID.pb_buf );
                WSL_OS_Free( ptIssuerHash.pb_buf );
                WSL_OS_Free( ptTrustedUsage.pb_buf );
                free_WIMI_Ref_t( tempRef );

                // Certificate has to be WTLS or X509, not URL
                if ( certType == WIMI_CT_X509 || certType == WIMI_CT_WTLS )
                    {
                    pCert = certRefList[i]; // Found one
                    i = certCount; // Stop looping
                    }
                else // Cert not supported
                    {
                    callStatus = WIMI_ERR_UnsupportedCertificate;
                    }
                }
            }

        // Retrieve certificate data from WIM
        if ( callStatus == WIMI_Ok )
            {
            callStatus = WIMI_CertificateReqT( trId, pCert, &ptCertData );
            if ( callStatus == WIMI_Ok )
                {
                CleanupPushWimBufL( ptCertData );

                TPtrC8 certPtr( ptCertData.pb_buf, ptCertData.ui_buf_length );
                TPtr8 publicKeyPtr = publicKeyBuf->Des();

                TRAPD( parseError, ParseCertPublicKeyL( certPtr, publicKeyPtr, certType ) );
                _WIMTRACE2(_L("WIM | WIMServer | CWimPublicKeyHandler::ExportPublicKeyL parseError %d"), parseError );
                if ( !parseError )
                    {
                    aMessage.WriteL( 1, publicKeyPtr );
                    }
                else
                    {
                    callStatus = WIMI_ERR_CertParseError;
                    }

                CleanupStack::PopAndDestroy( ptCertData.pb_buf );
                }
            }

        CleanupStack::PopAndDestroy( certRefList );
        }

    CleanupStack::PopAndDestroy( publicKeyBuf );

    _WIMTRACE2(_L("WIM | WIMServer | CWimPublicKeyHandler::ExportPublicKeyL callStatus %d"), callStatus);
    aMessage.Complete( CWimUtilityFuncs::MapWIMError( callStatus ) );
    _WIMTRACE(_L("WIM | WIMServer | CWimPublicKeyHandler::ExportPublicKeyL | End"));
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::ParseCertPublicKeyL
// Parse public key from certificate.
// -----------------------------------------------------------------------------
//
void CWimCertHandler::ParseCertPublicKeyL(
    const TDesC8& aCertData,
    TDes8& aPublicKey,
    const TUint8 aCertType ) const
    {
    _WIMTRACE2(_L("WIM | WIMServer | CWimPublicKeyHandler::ParseCertPublicKeyL | Begin, type %d"), aCertType);
    CCertificate* certificate = NULL;
    CRSAPublicKey* publicKey = NULL;

    switch ( aCertType )
        {
        case WIMI_CT_WTLS:
            {
            certificate = CWTLSCertificate::NewLC( aCertData );
            publicKey = CWTLSRSAPublicKey::NewLC( certificate->PublicKey().KeyData() );
            break;
            }

        case WIMI_CT_X509:
            {
            certificate = CX509Certificate::NewLC( aCertData );
            publicKey = CX509RSAPublicKey::NewLC( certificate->PublicKey().KeyData() );
            break;
            }
        default:
            {
            _WIMTRACE2(_L("WIM|WIMServer|CWimCertHandler::ParseCertPublicKeyL, type %d not supported"), aCertType);
            User::Leave( KErrNotSupported );
            break;
            }
        }

    TX509RSAKeyEncoder encoder( *publicKey, ESHA1 );
    CASN1EncBase* encoded = encoder.EncodeKeyLC();

    TUint pos = 0;
    aPublicKey.SetLength( KPublicKeyLength );
    // Check that Max. length is not exceeded
    if ( encoded->LengthDER() > static_cast< TUint >( KPublicKeyLength ) )
        {
        _WIMTRACE(_L("WIM|WIMServer|CWimCertHandler::ParseCertPublicKeyL, too long public key"));
        User::Leave( KErrBadDescriptor );
        }
    // Write encoded key to prealloced buffer
    encoded->WriteDERL( aPublicKey, pos );

    CleanupStack::PopAndDestroy( 3, certificate );  // encoded, publicKey, certificate
    _WIMTRACE(_L("WIM | WIMServer | CWimPublicKeyHandler::ParseCertPublicKeyL | End"));
    }

// -----------------------------------------------------------------------------
// CWimCertHandler::MainWimRef()
// Returns cached WIM ref using WIM memory manager
// -----------------------------------------------------------------------------
//
WIMI_Ref_t* CWimCertHandler::MainWimRef( CWimMemMgmt& aWimMgmt ) const
    {
    WIMI_Ref_t* ref = aWimMgmt.WimRef();
    if( !ref )
        {
        ref = WIMI_GetWIMRef( 0 );
        aWimMgmt.SetWIMRef( ref );    // takes ownership
        }
    return ref;
    }


//  End of File
