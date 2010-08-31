/*
* Copyright (c) 2003-2009 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   Implementation of class CCertManUICertificateHelper
*
*/


#include <X509CertNameParser.h>
#include <certmanui.rsg>
#include <hash.h>                   // MD5 fingerprint
#include <aknmessagequerydialog.h>
#include <pkixcertchain.h>          // for validation
#include <aknnotewrappers.h>        // for warning & information notes
#include <sysutil.h>
#include <ErrorUI.h>
#include <unifiedkeystore.h>
#include <TrustedSitesStore.h>
#include <x509keys.h>               // TX509KeyFactory
#include <e32math.h>                // Pow
#include "Certmanui.hrh"
#include "CertmanuiCertificateHelper.h"
#include "CertmanuiCommon.h"
#include "CertmanuiSyncWrapper.h"
#include "CertManUILogger.h"

const TInt KFileCertStoreUid( 0x101F501A );
const TInt KTrustedServerCertStoreUid( 0x101FB66F );
const TInt KDeviceCertStoreUid( 0x101FB668 );
const TInt KDeviceKeyStoreUid( 0x101FB66A );

_LIT( KBlockSeparator, " " );

template <class T>
class CleanupResetAndDestroy
    {
public:
    inline static void PushL( T& aRef );
private:
    static void ResetAndDestroy( TAny *aPtr );
    };

template <class T>
inline void CleanupResetAndDestroyPushL( T& aRef );

template <class T>
inline void CleanupResetAndDestroy<T>::PushL( T& aRef )
    {
    CleanupStack::PushL( TCleanupItem( &ResetAndDestroy, &aRef ) );
    }

template <class T>
void CleanupResetAndDestroy<T>::ResetAndDestroy( TAny *aPtr )
    {
    if( aPtr )
        {
        static_cast<T*>( aPtr )->ResetAndDestroy();
        }
    }

template <class T>
inline void CleanupResetAndDestroyPushL( T& aRef )
    {
    CleanupResetAndDestroy<T>::PushL( aRef );
    }


// ---------------------------------------------------------
// CCertManUICertificateHelper::CCertManUICertificateHelper()
// Default C++ Constructor
// ---------------------------------------------------------
//
CCertManUICertificateHelper::CCertManUICertificateHelper( CCertManUIKeeper& aKeeper )
: iKeeper(aKeeper)
    {
  }

// ---------------------------------------------------------
// CCertManUICertificateHelper::~CCertManUICertificateHelper()
// Destructor
// ---------------------------------------------------------
//
CCertManUICertificateHelper::~CCertManUICertificateHelper()
    {
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::CutCertificateField(TPtrC aField)
// If CertLabel, Issuer and Owner length is over 80 characters,
// cut it down to 80
// Returns the cut certificate field.
// ---------------------------------------------------------
//
TPtrC CCertManUICertificateHelper::CutCertificateField( TPtrC aField )
    {
    CERTMANUILOGGER_ENTERFN(
         "CCertManUICertificateHelper::CutCertificateField" );

    TInt fieldLength = aField.Length();
    if ( fieldLength >= KMaxLengthTextCertLabelVisible )
        {
        TPtrC cutCertLabel = aField.Mid( 0, KMaxLengthTextCertLabelVisible );

        CERTMANUILOGGER_LEAVEFN(
           "CCertManUICertificateHelper::CutCertificateField" );

        return cutCertLabel;
        }

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUICertificateHelper::CutCertificateField" );

    return aField;
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::IsCertificateDeletable
// Check whether certificate is deletable
// ---------------------------------------------------------
//
TBool CCertManUICertificateHelper::IsCertificateDeletable( TInt aIndex,
    TInt aType ) const
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::IsCertificateDeletable" );
    TBool ret = ETrue;

    // If wim has not been enabled and certificate resides in smart card
    // we cannot delete it since we don't have smart card certificate
    // delete functionality in non WIM builds. So in non WIM builds all
    // smart card certificates are marked read-only.
    if( aIndex > -1 )
        {
        CCTCertInfo* entry;
        if ( aType == KCertTypeAuthority )
            {
            entry = iKeeper.iCALabelEntries[ aIndex ]->iCAEntry;
            }
        else if( aType == KCertTypeTrustedSite )
            {
            entry = iKeeper.iPeerLabelEntries[ aIndex ]->iPeerEntry;
            }
        else if( aType == KCertTypeDevice )
            {
            entry = iKeeper.iDeviceLabelEntries[ aIndex ]->iDeviceEntry;
            }
        else
            {
            entry = iKeeper.iUserLabelEntries[ aIndex ]->iUserEntry;
            }
        if ( !entry->IsDeletable() )
            {
            ret = EFalse;
            }
        }

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUICertificateHelper::IsCertificateDeletable" );
    return ret;
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::IsOneMarkedCertificateDeletable
// Check whether one marked certificate is deletable, or
// if none certificate is marked, check if the highlighted
// certificate is deletable
// ---------------------------------------------------------
//
TBool CCertManUICertificateHelper::IsOneMarkedCertificateDeletable
    ( CEikColumnListBox* aListBox, TInt aType ) const
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::IsOneMarkedCertificateDeletable" );

    const CArrayFix<TInt>* array  = aListBox->SelectionIndexes();
    TInt markedCount = array->Count();
    TBool oneIsDeletable = EFalse;

    if ( 0 == markedCount )
        {
        TInt currentItem = aListBox->CurrentItemIndex();
        return IsCertificateDeletable( currentItem, aType );
        }

    for ( TInt i = 0; !oneIsDeletable && i < markedCount; i++ )
        {
        oneIsDeletable = IsCertificateDeletable( (*array)[i], aType );
        }

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUICertificateHelper::IsOneMarkedCertificateDeletable" );

    return oneIsDeletable;
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::MessageQueryViewDetailsL(TInt aIndex)
// Creates the whole of certificate details view
// Appends strings from both resources and CertManAPI to one
// message body text and displays it.
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::MessageQueryViewDetailsL( TInt aIndex,
    TInt aType,
    CEikonEnv* aEikonEnv )
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::MessageQueryViewDetailsL" );
     LOG_WRITE_FORMAT( "certificate index %i", aIndex );

    iKeeper.StartWaitDialogL( ECertmanUiWaitDialog );

    HBufC* message = HBufC::NewLC( KMaxLengthTextDetailsBody );
    TRAPD( err, CreateMessageBodyTextL( aIndex, aType, aEikonEnv, *message ) );

    iKeeper.CloseWaitDialogL();
    User::LeaveIfError( err );

    if( message->Length() )
        {
        HBufC* header = StringLoader::LoadLC( R_TEXT_RESOURCE_DETAILS_VIEW_HEADER );
        CAknMessageQueryDialog* dlg = CAknMessageQueryDialog::NewL( *message );
        dlg->PrepareLC( R_MESSAGE_QUERY_DETAILS_VIEW );
        dlg->QueryHeading()->SetTextL( *header );
        dlg->RunLD();
        CleanupStack::PopAndDestroy( header );
        }

    CleanupStack::PopAndDestroy( message );

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUICertificateHelper::MessageQueryViewDetailsL" );
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority::ValidateCertificateL(
// Checks if the certificate is corrupted
// Checks the validity period of the certificate
// Display only one warning note, in the following order:
// 1) Expired/Not yet valid
// 2) Corrupted
// 3) Not trusted
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::ValidateCertificateL(
    TInt aIndex, CCertificate* aDetails, const CCTCertInfo& aCertInfo, TInt aType )
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::ValidateCertificateL" );

    TInt poppableItems = 0;
    // Check allways first the validity period
    // Show Expired/Not yet valid notes
    const CValidityPeriod& validityPeriod = aDetails->ValidityPeriod();
    const TTime& startValue = validityPeriod.Start();
    const TTime& finishValue = validityPeriod.Finish();
    TTime current;
    current.UniversalTime();

    if ( startValue > current )
        {
        // certificate is not valid yet
        HBufC* string = StringLoader::LoadLC(
            R_WARNING_NOTE_TEXT_CERTIFICATE_NOT_VALID_YET );
        DisplayWarningNoteLD(string);
        }
    else if ( finishValue < current )
        {
        // certificate is expired
        HBufC* string = StringLoader::LoadLC(
            R_WARNING_NOTE_TEXT_CERTIFICATE_EXPIRED );
        DisplayWarningNoteLD( string );
        }
    else
        {
        // put here check for certificate status, show warning notes if needed
        TBool noValidationError = ETrue;
        CArrayFixFlat<TValidationError>* status = NULL;
        TCertificateFormat certificateFormat;

        if ( aType == KCertTypeAuthority )
            {
            certificateFormat = iKeeper.iCALabelEntries[aIndex]->
                                        iCAEntry->CertificateFormat();
            }
        else if ( aType == KCertTypeTrustedSite )
            {
            certificateFormat = iKeeper.iPeerLabelEntries[aIndex]->
                                        iPeerEntry->CertificateFormat();
            }
        else if ( aType == KCertTypeDevice )
            {
            certificateFormat = iKeeper.iDeviceLabelEntries[aIndex]->
                                        iDeviceEntry->CertificateFormat();
            }
        else
            {
            certificateFormat = iKeeper.iUserLabelEntries[aIndex]->
                                        iUserEntry->CertificateFormat();
            }

        switch ( certificateFormat )
            {
            case EX509Certificate:
                {
                status = ValidateX509CertificateL(
                    (CX509Certificate*)aDetails );
                break;
                }
            default:
                {
                status = new ( ELeave) CArrayFixFlat<TValidationError>( 1 );
                break;
                }
            }
        CleanupStack::PushL( status );
        poppableItems++;

        TInt errorCount = status->Count();
        TBool ready = EFalse;
        for ( TInt i = 0; i < errorCount && !ready; i++ )
            {
            TValidationError errorType = status->At(i);

            switch ( errorType )
                {
                case EValidatedOK:
                case EChainHasNoRoot:
                case EBadKeyUsage:
                // Ignore these errors
                    LOG_WRITE( "Ignored certificate validation error" );
                    break;

                default:
                // certificate is corrupted
                noValidationError = EFalse;
                HBufC* string = StringLoader::LoadLC(
                    R_WARNING_NOTE_TEXT_CERTIFICATE_CORRUPTED );
                DisplayWarningNoteLD( string );
                ready = ETrue;
                    break;
                }
            }

        if ( noValidationError  && (aType == KCertTypeAuthority ))
        // Check for last if the CA certificate has no clients,
        // ie. the trust state of every client is No
        // For user certificates we don't do the check
            {

            RArray<TUid> trusterUids;
            CleanupClosePushL( trusterUids );
            poppableItems++;

            iKeeper.iWrapper->GetApplicationsL(
                iKeeper.CertManager(), aCertInfo, trusterUids );

            if ( trusterUids.Count() == 0)
                {
                HBufC* string = StringLoader::LoadLC(
                    R_WARNING_NOTE_TEXT_CERTIFICATE_NOT_TRUSTED );
                DisplayWarningNoteLD( string );
                }
            }
        }
    CleanupStack::PopAndDestroy( poppableItems );

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUICertificateHelper::ValidateCertificateL" );
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::DetailsFieldDynamicL(
// Appends a field that has string from resources and string from CertManAPI
// to the details view message body
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::DetailsFieldDynamicL(
    HBufC& aMessage, TPtrC aValue, TInt aResourceOne, TInt aResourceTwo )
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::DetailsFieldDynamicL" );

    DetailsResourceL( aMessage, aResourceOne );
    DetailsDynamicL( aMessage, aValue, aResourceTwo );
    aMessage.Des().Append( KCertManUIDetailsViewEnter );

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUICertificateHelper::DetailsFieldDynamicL" );
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::DetailsResourceL(
// Reads line from resources
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::DetailsResourceL(
    HBufC& aMessage, TInt aResourceOne )
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::DetailsResourceL" );

    HBufC* stringHolder = StringLoader::LoadLC( aResourceOne );
    aMessage.Des().Append( stringHolder->Des() );
    CleanupStack::PopAndDestroy();  // stringHolder
    aMessage.Des().Append( KCertManUIDetailsViewEnter );

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUICertificateHelper::DetailsResourceL" );
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::DetailsFieldResourceL(
// Appends a field that has two strings from resources
// to the details view message body
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::DetailsFieldResourceL(
    HBufC& aMessage, TInt aResourceOne,
    TInt aResourceTwo)
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::DetailsFieldResourceL" );

    DetailsResourceL( aMessage, aResourceOne );
    DetailsResourceL( aMessage, aResourceTwo );
    aMessage.Des().Append( KCertManUIDetailsViewEnter );

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUICertificateHelper::DetailsFieldResourceL" );
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::DevideToBlocks
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::DevideToBlocks( const TDesC8& aInput, TPtr aOutput )
    {
    const TInt KBlockLength = 2;
    TInt blockIndex = 0;
    for ( TInt j = 0 ; j < aInput.Length() ; j++ )
        {
        if ( blockIndex == KBlockLength )
            {
            aOutput.Append( KBlockSeparator );
            blockIndex = 0;
            }
        aOutput.AppendNumFixedWidthUC( (TUint)(aInput[ j ]), EHex, 2 );
        ++blockIndex;
        }
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::DisplayWarningNoteLD(HBufC* aString)
// Displays the warning note for corrupted/expired/not yet valid certificate
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::DisplayWarningNoteLD( HBufC* aString )
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::DisplayWarningNoteLD" );

    CAknWarningNote* note = new ( ELeave ) CAknWarningNote( ETrue );
    note->ExecuteLD( *aString );
    CleanupStack::PopAndDestroy();  // aString

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUICertificateHelper::DisplayWarningNoteLD" );
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::ValidateX509CertificateL(CX509Certificate* aDetails)
// Checks if the certificate is corrupted
// Checks the validity period of the certificate
// Display only one warning note, in the following order:
// 1) Expired/Not yet valid
// 2) Corrupted
// 3) Not trusted
// ---------------------------------------------------------
//
CArrayFixFlat<TValidationError>* CCertManUICertificateHelper::
    ValidateX509CertificateL( CX509Certificate* aDetails )
    {
    TInt poppableItems = 0;
    CArrayFixFlat<TValidationError>* status =
        new ( ELeave ) CArrayFixFlat<TValidationError>( 1 );
    CleanupStack::PushL( status ); //This is returned, so it isn't destroyed at the end.

    TTime GMTTime;
    GMTTime.UniversalTime(); // Get Universal Time
    RPointerArray<CX509Certificate> certArray( &aDetails, 1 );

    CPKIXCertChain* chain = CPKIXCertChain::NewLC(
        iKeeper.iRfs, aDetails->Encoding(), certArray );
    poppableItems++;

    CPKIXValidationResult* result = CPKIXValidationResult::NewLC();
    poppableItems++;
    iKeeper.iWrapper->ValidateX509RootCertificateL( result, GMTTime, chain );

    TValidationStatus validationStatus = result->Error();
    //Set reserve space. One for error, other for warnings.
    status->SetReserveL( 1 + result->Warnings().Count() );
    status->AppendL(validationStatus.iReason);
    for ( TUint8 i = 0; i < result->Warnings().Count(); i++ )
        {
        status->AppendL( result->Warnings().At(0).iReason );
        }

    CleanupStack::PopAndDestroy( poppableItems );    //  All but status.
    CleanupStack::Pop();        //  status
    return status;
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::DetailsDynamicL(
// HBufC& aMessage, TPtrC aValue, TInt aResourceOne)
// Reads dynamic text from CertManAPI, if the string is empty
// put a not defined text from the resource in its place
// KMaxLengthTextCertLabel = 510, used by CertLabel(), Issuer(), Owner()
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::DetailsDynamicL(
    HBufC& aMessage, TPtrC aValue, TInt aResourceOne )
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::DetailsDynamicL" );

    HBufC* buf = HBufC::NewLC( KMaxLengthTextCertLabel );
    buf->Des() = aValue;
    buf->Des().TrimLeft();
    // Cut CertLabel after fourth semi colon
    TPtrC trimmedCertLabel = CutCertificateField( buf->Des() );
    buf->Des().Copy( trimmedCertLabel );
    TInt length = buf->Des().Length();
    if ( length == 0 )
        {
        DetailsResourceL( aMessage, aResourceOne );
        }
    else
        {
        //LRM (Left-to-Right mark 200E)
        const TInt KLRMark = 0x200E;
        aMessage.Des().Append( buf->Des() );
        aMessage.Des().Append(KLRMark);
        aMessage.Des().Append( KCertManUIDetailsViewEnter );
        }
    CleanupStack::PopAndDestroy();  // buf

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUICertificateHelper::DetailsDynamicL" );
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority::ConfirmationQueryDeleteCertL()
// Deletes either certificate in focus if there are no marked items,
// or deletes the marked items
// Shows read-only information notes if certificates are not deletable
// Returns ETrue if at least one certificate is deleted
// ---------------------------------------------------------
//
TBool CCertManUICertificateHelper::ConfirmationQueryDeleteCertL(
    TInt aType,
    CEikColumnListBox* aListBox )
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::ConfirmationQueryDeleteCertL" );

    TBool result = EFalse;

    const CArrayFix<TInt>* array = aListBox->SelectionIndexes();
    TInt focusPosition = aListBox->CurrentItemIndex();
    CCTCertInfo* entry;
    TInt queryOk = 0;
    // comes here when there is no selection, can be read-only
    // certificate if Clear key is pressed, must check
    if ( array->Count() == 0 && IsCertificateDeletable( focusPosition, aType ) )
        {
        if( aType == KCertTypeAuthority )
            {
            entry = iKeeper.iCALabelEntries[ focusPosition ]->iCAEntry;
            }
        else if( aType == KCertTypeTrustedSite )
            {
            entry = iKeeper.iPeerLabelEntries[ focusPosition ]->iPeerEntry;
            }
        else if( aType == KCertTypeDevice )
            {
            entry = iKeeper.iDeviceLabelEntries[ focusPosition ]->iDeviceEntry;
            }
        else
            {
            entry = iKeeper.iUserLabelEntries[ focusPosition ]->iUserEntry;
            }
        queryOk = DeleteSingleItemQueryL( *entry );
        if ( queryOk )
            {
            if (IsCertificateDeletable( focusPosition, aType ))
                {
                DeleteSingleItemL( *entry, focusPosition, aListBox, aType );


                AknListBoxUtils::HandleItemRemovalAndPositionHighlightL(
                    aListBox, focusPosition, ETrue );
                result = ETrue;
                }
            else
                {
                ReadOnlySingleItemNoteL( *entry );
                }
            }
        }
    else if ( IsOneMarkedCertificateDeletable(aListBox, aType) ) //if (array->Count()=>1)
        {
        if ( aType == KCertTypeAuthority )
            {
            entry = iKeeper.iCALabelEntries[ array->At(0) ]->iCAEntry;
            }
        else if ( aType == KCertTypeTrustedSite )
            {
            entry = iKeeper.iPeerLabelEntries[ array->At(0) ]->iPeerEntry;
            }
        else if( aType == KCertTypeDevice )
            {
            entry = iKeeper.iDeviceLabelEntries[ array->At(0) ]->iDeviceEntry;
            }
        else
            {
            entry = iKeeper.iUserLabelEntries[ array->At(0) ]->iUserEntry;
            }

        if (array->Count()==1)
            {
            queryOk = DeleteSingleItemQueryL( *entry );
            }
        else
            {
            HBufC* prompt = StringLoader::LoadLC(
                R_CONFIRMATION_QUERY_DELETE_CERT_PLURAL, array->Count() );
            queryOk = DisplayConfirmationLD( prompt );
            }
        if ( queryOk )
            {
            TInt readonlyCount = 0;
            TInt singleReadOnlyPosition = 0;
            TInt markedPosition = 0;
            TKeyArrayFix ownKey( 0, ECmpTInt );
            CONST_CAST(CArrayFix<int>*, array)->Sort( ownKey ); //lint !e665 expression macro param ok
            TInt markedCount = array->Count();
            CArrayFixFlat<TInt>* deletedIndexes = new (ELeave)CArrayFixFlat<TInt>( 4 );
            CleanupStack::PushL( deletedIndexes );

            iKeeper.StartWaitDialogL( ECertmanUiDeleteDialog );
            CleanupCloseWaitDialogPushL( iKeeper );

            for ( TInt i = markedCount; i > 0; i-- )
                {
                markedPosition = array->At( i-1 );
                // move focus up the list if marked items were
                // between focus and the beginning of the list
                if ( focusPosition > markedPosition )
                    {
                    focusPosition--;
                    }

                // Check whether certificate is deletable

                if( aType == KCertTypeAuthority )
                    {
                    entry = iKeeper.iCALabelEntries[ markedPosition ]->iCAEntry;
                    }
                else if( aType == KCertTypeTrustedSite )
                    {
                    entry = iKeeper.iPeerLabelEntries[ markedPosition ]->iPeerEntry;
                    }
                else if( aType == KCertTypeDevice )
                    {
                    entry = iKeeper.iDeviceLabelEntries[ markedPosition ]->iDeviceEntry;
                    }
                else
                    {
                    entry = iKeeper.iUserLabelEntries[ markedPosition ]->iUserEntry;
                    }

                if (IsCertificateDeletable(markedPosition, aType))
                    {
                    DeleteSingleItemL( *entry, markedPosition, aListBox, aType );
                    deletedIndexes->AppendL(markedPosition);
                    result = ETrue;
                    }
                else // Cannot be deleted
                    {
                    readonlyCount++;
                    singleReadOnlyPosition = markedPosition;
                    }
                }

            CleanupStack::PopAndDestroy();    // closes wait dialog

            if ( readonlyCount == 1 )
                {
                if( aType == KCertTypeAuthority  )
                    {
                    entry = iKeeper.iCALabelEntries[ singleReadOnlyPosition ]->iCAEntry;
                    }
                else if( aType == KCertTypeTrustedSite  )
                    {
                    entry = iKeeper.iPeerLabelEntries[ singleReadOnlyPosition ]->iPeerEntry;
                    }
                else if( aType == KCertTypeDevice )
                    {
                    entry = iKeeper.iDeviceLabelEntries[ singleReadOnlyPosition ]->iDeviceEntry;
                    }
                else
                    {
                    entry = iKeeper.iUserLabelEntries[ singleReadOnlyPosition ]->iUserEntry;
                    }

                ReadOnlySingleItemNoteL( *entry );
                }
            else if ( readonlyCount > 1 )
                {
                HBufC* string = StringLoader::LoadLC(
                    R_INFO_NOTE_TEXT_CERTIFICATE_READ_ONLY_MULTIPLE,
                    readonlyCount );
                DisplayNoteLD( string );
                }
            else
                { // For lint
                }

            aListBox->ClearSelection();
            AknListBoxUtils::HandleItemRemovalAndPositionHighlightL(
                aListBox, focusPosition, *deletedIndexes);
            CleanupStack::PopAndDestroy(deletedIndexes);

            }
        }
    else
        {
        result = EFalse;
        }

    aListBox->DrawNow();//lint !e539

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUICertificateHelper::ConfirmationQueryDeleteCertL" );


    return result;
    }

// ---------------------------------------------------------
// CCertManUIViewAuthority::DeleteSingleItemQueryL(CCTCertInfo& aEntry)
// Gets the certificate name (or No label/No subject) for the Confirmation note
// ---------------------------------------------------------
//
TInt CCertManUICertificateHelper::DeleteSingleItemQueryL( CCTCertInfo& aEntry )
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::DeleteSingleItemQueryL" );

    TInt selection = 0;

    HBufC* buf = HBufC::NewLC( KMaxLengthTextCertLabel );
    buf->Des() = aEntry.Label();
    buf->Des().TrimLeft();
    // Cut CertLabel after fourth semi colon
    TPtrC trimmedCertLabel = CutCertificateField( buf->Des() );
    buf->Des().Copy(trimmedCertLabel);
    TInt length = buf->Des().Length();
    if ( length == 0 )
        {
        // here the resource name is _NO_LABEL, but it returns only "Delete certificate?"
        // not any "No label" or "No subject" string
        HBufC* prompt = StringLoader::LoadLC(
            R_CONFIRMATION_QUERY_DELETE_CERT_NO_LABEL );
        selection = DisplayConfirmationLD( prompt );
        }
    else
        {
        HBufC* prompt = StringLoader::LoadLC(
            R_CONFIRMATION_QUERY_DELETE_CERT, buf->Des() );
        selection = DisplayConfirmationLD(prompt);
        }
    CleanupStack::PopAndDestroy();  // buf

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUICertificateHelper::DeleteSingleItemQueryL" );

    return selection;
    }


// ---------------------------------------------------------
// CCertManUICertificateHelper::DeleteSingleItemL(CCrCertEntry& aEntry, TInt aPosition, TInt aType)
// Deletes one item from CertManAPI and from listbox
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::DeleteSingleItemL( CCTCertInfo& aEntry,
    TInt aPosition,
    CEikColumnListBox* aListBox,
    TInt aType )
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::DeleteSingleItemL" );

    TInt status( KErrNone );

    if ( aType == KCertTypeTrustedSite )
        {
        status = iKeeper.iWrapper->DeleteCertL(
                                              iKeeper.CertManager(),
                                              aEntry,
                                              KCMTrustedServerTokenUid
                                              );
        }
    else if( aType == KCertTypeDevice )
        {
        status = iKeeper.iWrapper->DeleteCertL(
                                              iKeeper.CertManager(),
                                              aEntry,
                                              KCMDeviceCertStoreTokenUid
                                              );
        }
    else
        {
        status = iKeeper.iWrapper->DeleteCertL( iKeeper.CertManager(), aEntry );
        }

    // Update the list after delete. In case of a error, a note show inside DeleteCertL.
    if ( status == KErrNone )
        {
        MDesCArray *itemList = aListBox->Model()->ItemTextArray();
        CDesCArray* itemArray = STATIC_CAST( CDesCArray*, itemList );
        itemArray->Delete( aPosition );
        }

    CERTMANUILOGGER_LEAVEFN( "CCertManUICertificateHelper::DeleteSingleItemL" ); //lint !e539
    }


// ---------------------------------------------------------
// CCertManUICertificateHelper::MoveKeyL(const TCTKeyAttributeFilter& aFilter,
//                       TUid aSourceStore, TUid aSourceStore )
// Moves a key from one store to another
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::MoveKeyL(
    const TCTKeyAttributeFilter& aFilter,
    const TUid aSourceStore,
    const TUid aTargetStore )
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::MoveCertL" );

    iKeeper.iWrapper->MoveKeyL( iKeeper.KeyManager(), aFilter, aSourceStore, aTargetStore );

    CERTMANUILOGGER_LEAVEFN( "CCertManUICertificateHelper::MoveCertL" );
    }


// ---------------------------------------------------------
// CCertManUICertificateHelper::MoveCertL(CCrCertEntry& aEntry,
//                        TInt aPosition, CEikColumnListBox* aListBox,
//                        TUid aSourceStore, TUid aSourceStore )
// Moves a certificate from one store to another
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::MoveCertL( CCTCertInfo& aEntry,
    TInt aPosition,
    CEikColumnListBox* aListBox,
    const TUid aSourceStore,
    const TUid aTargetStore )
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::MoveCertL" );

    TInt certCount(0);

    TRAPD( err, certCount = iKeeper.iWrapper->MoveCertL( iKeeper.CertManager(), aEntry, aSourceStore, aTargetStore ));

    // Update the list after moving the certificate
    if ( err == KErrNone )
        {
        MDesCArray *itemList = aListBox->Model()->ItemTextArray();
        CDesCArray* itemArray = STATIC_CAST( CDesCArray*, itemList );
        if ( certCount == 1  )
            {
            // Delete single item
            itemArray->Delete( aPosition );
            AknListBoxUtils::HandleItemRemovalAndPositionHighlightL(
                          aListBox, aPosition, ETrue );
            }
         else
            {
            // More than one certificate moved. Empty listbox items.
            // It will be updated later
            itemArray->Reset();
            }
        }
    else
        {
        User::Leave( err );
        }

    CERTMANUILOGGER_LEAVEFN( "CCertManUICertificateHelper::MoveCertL" );
    }


// ---------------------------------------------------------
// CCertManUICertificateHelper::ReadOnlySingleItemNoteL(CCTCertInfo& aEntry)
// Gets the certificate name (or No label/No subject) for the read-only note
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::ReadOnlySingleItemNoteL(CCTCertInfo& aEntry )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUICertificateHelper::ReadOnlySingleItemNoteL" );

    HBufC* buf = HBufC::NewLC( KMaxLengthTextCertLabel );
    buf->Des() = aEntry.Label();
    buf->Des().TrimLeft();
    // Cut CertLabel after fourth semi colon
    TPtrC trimmedCertLabel = CutCertificateField( buf->Des() );
    buf->Des().Copy(trimmedCertLabel);
    TInt length = buf->Des().Length();
    if ( length == 0 )
        {
        // here the resource name is _NO_LABEL, but it returns only "Cannot delete certificate"
        // not any "No label" or "No subject" string
        HBufC* stringHolder = StringLoader::LoadLC(
                                R_INFO_NOTE_TEXT_CERTIFICATE_READ_ONLY_NO_LABEL );
        DisplayNoteLD( stringHolder );
        }
    else
        {
        HBufC* stringHolder = StringLoader::LoadLC(
                                R_INFO_NOTE_TEXT_CERTIFICATE_READ_ONLY, buf->Des() );
        DisplayNoteLD( stringHolder );
        }
    CleanupStack::PopAndDestroy();  // buf


    CERTMANUILOGGER_LEAVEFN( "CCertManUICertificateHelper::ReadOnlySingleItemNoteL" );
    }
// ---------------------------------------------------------
// CCertManUICertificateHelper::DisplayConfirmationLD(HBufC* aString)
// Displays Delete yes/no confirmation notes
// ---------------------------------------------------------
//
TInt CCertManUICertificateHelper::DisplayConfirmationLD( HBufC* aString )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUICertificateHelper::DisplayConfirmationLD" );

    CAknQueryDialog* dlg = CAknQueryDialog::NewL( CAknQueryDialog::ENoTone );
    TInt selection = dlg->ExecuteLD( R_CERTMANUI_CONFIRMATION_QUERY_YES_NO, *aString );
    // Takes ownership and destroys aString
    CleanupStack::PopAndDestroy();  // aString

    CERTMANUILOGGER_LEAVEFN( "CCertManUICertificateHelper::DisplayConfirmationLD" );

    return selection;
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::DisplayNoteLD(HBufC* aString)
// Displays read-only information notes
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::DisplayNoteLD(HBufC* aString)
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUICertificateHelper::DisplayNoteLD" );

    CAknInformationNote* note = new ( ELeave ) CAknInformationNote( ETrue );
    note->ExecuteLD( *aString );
    CleanupStack::PopAndDestroy();  // aString

    CERTMANUILOGGER_LEAVEFN( "CCertManUICertificateHelper::DisplayNoteLD" );
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::FFSSpaceBelowCriticalLevelL
// Handle deleting when disk is getting full
// ---------------------------------------------------------
//
TBool CCertManUICertificateHelper::FFSSpaceBelowCriticalLevelL(
    TBool aShowErrorNote, TInt aBytesToWrite )
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::FFSSpaceBelowCriticalLevelL" );

    TBool ret = EFalse;
    if ( SysUtil::FFSSpaceBelowCriticalLevelL(
        &(CCoeEnv::Static()->FsSession()), aBytesToWrite ) )
        {
        ret = ETrue;
        if ( aShowErrorNote )
            {
            CErrorUI* errorUi = CErrorUI::NewLC( *(CCoeEnv::Static()) );
            errorUi->ShowGlobalErrorNoteL( KErrDiskFull );
            CleanupStack::PopAndDestroy();  // errorUi
            }
        }

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUICertificateHelper::FFSSpaceBelowCriticalLevelL" );

    return ret;
    }
// ---------------------------------------------------------
// CCertManUICertificateHelper::HandleMarkableListCommandL( TInt aCommand )
// Handles marking and unmarking of the list.
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::HandleMarkableListCommandL( TInt aCommand,
                                                    CEikColumnListBox* aListBox )
    {
    CERTMANUILOGGER_ENTERFN(
        "CCertManUICertificateHelper::HandleMarkableListCommandL" );

    TInt index;
    switch ( aCommand )
        {
        case ECertManUICmdMark:
            {
            index = aListBox->CurrentItemIndex();
            if ( index >= 0 )
                {
                // This is done to make sure listbox updates its state
                // to make shift+arrow work correctly after shift+ok
                aListBox->View()->UpdateSelectionL( CListBoxView::EDisjointSelection );
                aListBox->View()->SelectItemL( index );
                }
            break;
            }
        case ECertManUICmdUnmark:
            {
            index = aListBox->CurrentItemIndex();
            if ( index >= 0 )
                {
                // This is done to make sure listbox updates its state
                // to make shift+arrow work correctly after shift+ok
                aListBox->View()->UpdateSelectionL(
                      CListBoxView::EDisjointSelection );
                aListBox->View()->DeselectItem( index );
                }
            break;
            }
        case ECertManUICmdMarkAll:
            {
            aListBox->ClearSelection();
            TInt count = aListBox->Model()->NumberOfItems();
            if ( count )
                {
                CArrayFixFlat<TInt>* selection =
                    new( ELeave ) CArrayFixFlat<TInt>( 1 );
                CleanupStack::PushL( selection );
                selection->SetReserveL( count );
                for( TInt i = 0; i < count; i++ )
                    {
                    selection->AppendL( i );
                    }
                aListBox->SetSelectionIndexesL( selection );
                CleanupStack::PopAndDestroy();  // selection
                }
            break;
            }
        case ECertManUICmdUnmarkAll:
            {
            aListBox->ClearSelection();
            break;
            }
        default:
      break;
      }

    CERTMANUILOGGER_LEAVEFN(
        "CCertManUICertificateHelper::HandleMarkableListCommandL" );
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::GetKeyUsageAndLocationL( CCTCertInfo* aCertEntry,
//                                                      TUid* aKeyLocation)
// Returns key usage and location.
// ---------------------------------------------------------
//
TKeyUsagePKCS15 CCertManUICertificateHelper::GetKeyUsageAndLocationL( CCTCertInfo* aCertEntry,
                                                                     TUid* aKeyLocation )
    {
    TKeyUsagePKCS15 keyUsage;

    CERTMANUILOGGER_ENTERFN( "CCertManUICertificateHelper::GetKeyUsageAndLocationL" );

    // Check whether we have key for this certificate
    RMPointerArray<CCTKeyInfo> keyEntry;
    TCTKeyAttributeFilter filter;
    filter.iKeyId = aCertEntry->SubjectKeyId();

    iKeeper.iWrapper->ListL( iKeeper.KeyManager(), &keyEntry, filter );

    if (keyEntry.Count())
        {
        keyUsage = keyEntry[0]->Usage();
        // Get Location
        aKeyLocation->iUid = keyEntry[0]->Token().TokenType().Type().iUid ;
        }
    else
        {
        keyUsage = EPKCS15UsageNone;
        }

    keyEntry.Close();
    LOG_WRITE_FORMAT( "Key usage : %X", keyUsage );
    CERTMANUILOGGER_LEAVEFN( "CCertManUICertificateHelper::GetKeyUsageAndLocationL" );
    return keyUsage;
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::SetLocationInfoL( (HBufC& aMessage,
//                                               TBool aCertificate,
//                                               TUid* aLocUid)
// Adds certificate/private key's location info to certificate details
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::SetLocationInfoL(
    HBufC& aMessage,
    TBool aCertificate,
    TUid* aLocUid)
    {
    TInt location = 0;
    TInt locationRes = 0;

    CERTMANUILOGGER_ENTERFN( "CCertManUICertificateHelper::SetLocationInfo" );

    switch ( aLocUid->iUid )
        {
        case KFileCertStoreUid:
        case KTrustedServerCertStoreUid:
        case KDeviceCertStoreUid:
        case KDeviceKeyStoreUid:
        case KTokenTypeFileKeystore:
            location = R_TEXT_RESOURCE_DETAILS_VIEW_LOCATION_PHONE_MEMORY;
            break;

        case KWIMCertStoreUid:
            location = R_TEXT_RESOURCE_DETAILS_VIEW_LOCATION_SMART_CARD;
            break;

        default:
            if ( aCertificate )
                {
                location = R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED;
                }
            else
                {
                location = R_TEXT_RESOURCE_DETAILS_VIEW_NO_PRIVATE_KEY;
                }
            break;
        }

    if ( aCertificate )
        {
        locationRes = R_TEXT_RESOURCE_DETAILS_VIEW_CERTIFICATE_LOCATION;
        }
    else
        {
        locationRes = R_TEXT_RESOURCE_DETAILS_VIEW_PRIVATE_KEY_LOCATION;
        }

    DetailsFieldResourceL( aMessage, locationRes, location );

    CERTMANUILOGGER_LEAVEFN( "CCertManUICertificateHelper::SetLocationInfo" );
    }

// ---------------------------------------------------------
// CCertManUICertificateHelper::CreateMessageBodyTextL()
// ---------------------------------------------------------
//
void CCertManUICertificateHelper::CreateMessageBodyTextL( TInt aIndex, TInt aType,
        CEikonEnv* aEikonEnv, HBufC& aMessage )
    {
    CERTMANUILOGGER_ENTERFN( "CCertManUICertificateHelper::CreateMessageBodyTextL" );

    // Use certificate index from previous view
    HBufC8* urlBuf = NULL;
    CCTCertInfo* entry = NULL;
    CCertificate* details = NULL;
    CUnifiedCertStore*& store = iKeeper.CertManager();

    if( aType == KCertTypeAuthority )
        {
        entry = ( iKeeper.iCALabelEntries )[ aIndex ]->iCAEntry;
        }
    else if( aType == KCertTypeTrustedSite )
        {
        entry = ( iKeeper.iPeerLabelEntries )[ aIndex ]->iPeerEntry;
        }
    else if( aType == KCertTypeDevice )
        {
        entry = ( iKeeper.iDeviceLabelEntries )[ aIndex ]->iDeviceEntry;
        }
    else
        {
        entry = ( iKeeper.iUserLabelEntries )[ aIndex ]->iUserEntry;
        }

    TCertificateFormat certiticateFormat = entry->CertificateFormat();

    if( certiticateFormat != EX509CertificateUrl )
        {
        TInt err = iKeeper.iWrapper->GetCertificateL( store, *entry, details );
        CleanupStack::PushL( details );
        User::LeaveIfError( err );

        ValidateCertificateL( aIndex, details, *entry, aType );
        }
    else
        {
        urlBuf = HBufC8::NewLC( entry->Size() );
        TPtr8 urlPtr = urlBuf->Des();
        TInt err = iKeeper.iWrapper->GetUrlCertificateL( store, *entry, urlPtr );
        User::LeaveIfError( err );
        }

    if( aType == KCertTypeTrustedSite )
        {
        CTrustSitesStore* trustedSitesStore = CTrustSitesStore::NewL();
        CleanupStack::PushL( trustedSitesStore );

        RPointerArray<HBufC> trustedSites;
        CleanupResetAndDestroyPushL( trustedSites );
        trustedSitesStore->GetTrustedSitesL( details->Encoding(), trustedSites );

        if( trustedSites.Count() > 0 )
            {
            if( trustedSites.Count() == 1 )
                {
                DetailsResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_SITE );
                }
            else
                {
                DetailsResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_SITES );
                }

            for( TInt i = 0; i < trustedSites.Count(); i++ )
                {
                aMessage.Des().Append( *trustedSites[ i ] );
                aMessage.Des().Append( KCertManUIDetailsViewEnter );
                }
            aMessage.Des().Append( KCertManUIDetailsViewEnter );
            }

        CleanupStack::PopAndDestroy( &trustedSites );
        CleanupStack::PopAndDestroy( trustedSitesStore );
        }
    else
        {
        DetailsFieldDynamicL( aMessage, entry->Label(),
                R_TEXT_RESOURCE_DETAILS_VIEW_LABEL,
                R_TEXT_RESOURCE_VIEW_NO_LABEL_DETAILS );
        }

    // certificate issuer and owner
    if( certiticateFormat == EX509Certificate )
        {
        HBufC* owner = NULL;
        X509CertNameParser::SubjectFullNameL( *(CX509Certificate*)details, owner );
        CleanupStack::PushL( owner );

        HBufC* issuer = NULL;
        X509CertNameParser::IssuerFullNameL( *(CX509Certificate*)details, issuer );
        CleanupStack::PushL( issuer );

        DetailsFieldDynamicL( aMessage, issuer->Des(),
            R_TEXT_RESOURCE_DETAILS_VIEW_ISSUER,
            R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED );
        CleanupStack::PopAndDestroy( issuer );

        // certificate subject
        DetailsFieldDynamicL( aMessage, owner->Des(),
            R_TEXT_RESOURCE_DETAILS_VIEW_SUBJECT,
            R_TEXT_RESOURCE_VIEW_NO_SUBJECT_DETAILS );
        CleanupStack::PopAndDestroy( owner );
        }

    // Get key usage and location. This is done only client certificates.
    const TInt KLocationUnknown = 0;
    TUid keyLocation = { KLocationUnknown };

    if(( aType == KCertTypePersonal ) || ( aType == KCertTypeDevice ))
        {
        TInt usage;
        switch( GetKeyUsageAndLocationL( entry, &keyLocation ) )
            {
            case EPKCS15UsageSignSignRecover:
            case EPKCS15UsageSignDecrypt:
            case EPKCS15UsageSign:
                usage = R_TEXT_RESOURCE_DETAILS_VIEW_CLIENT_AUTHENTICATION;
                break;
            case EPKCS15UsageNonRepudiation:
                usage = R_TEXT_RESOURCE_DETAILS_VIEW_DIGITAL_SIGNING;
                break;
            default:
                usage = R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED;
                break;
            }
        DetailsFieldResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_KEY_USAGE, usage );
        }

    if( certiticateFormat == EX509Certificate )
        {
        // Certificate validity period
        // Hometime's offset to UTC
        TLocale locale;
        TTimeIntervalSeconds offSet = locale.UniversalTimeOffset();

        DetailsResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_VALID_FROM );
        const CValidityPeriod& validityPeriod = details->ValidityPeriod();
        TTime startValue = validityPeriod.Start();
        startValue += offSet;

        HBufC* dateFormatString = aEikonEnv->AllocReadResourceLC( R_QTN_DATE_USUAL_WITH_ZERO );

        TBuf<KMaxLengthTextDateString> startString;
        startValue.FormatL( startString, *dateFormatString );
        AknTextUtils::DisplayTextLanguageSpecificNumberConversion( startString );
        aMessage.Des().Append( startString );
        aMessage.Des().Append( KCertManUIDetailsViewEnterEnter );

        DetailsResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_VALID_UNTIL );
        TTime finishValue = validityPeriod.Finish();
        finishValue += offSet;
        TBuf<KMaxLengthTextDateString> finishString;

        finishValue.FormatL( finishString, *dateFormatString );
        AknTextUtils::DisplayTextLanguageSpecificNumberConversion( finishString );
        aMessage.Des().Append( finishString );
        aMessage.Des().Append( KCertManUIDetailsViewEnterEnter );

        CleanupStack::PopAndDestroy( dateFormatString );

        TUid certLoc;
        certLoc.iUid = entry->Token().TokenType().Type().iUid;
        SetLocationInfoL( aMessage, ETrue, &certLoc );
        }
    else if( certiticateFormat == EX509CertificateUrl )
        {
        HBufC* urlBuf16 = HBufC::NewLC( urlBuf->Length() );
        TPtr urlPtr = urlBuf16->Des();
        urlPtr.Copy(*urlBuf);
        DetailsFieldDynamicL( aMessage, urlBuf16->Des(),
                R_TEXT_RESOURCE_DETAILS_VIEW_CERTIFICATE_LOCATION,
                R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED );
        CleanupStack::PopAndDestroy( urlBuf16 );
        }
    else
        {
        // nothing
        }

    // certificate format
    TInt fieldType = 0;
    TInt fieldType2 = 0;
    switch( certiticateFormat )
        {
        case EX509Certificate:
        case EX509CertificateUrl:
            {
            fieldType = R_TEXT_RESOURCE_DETAILS_VIEW_CERT_FORMAT_X509;
            break;
            }
         default:
            {
            fieldType = R_TEXT_RESOURCE_DETAILS_VIEW_NOT_DEFINED;
            break;
            }
        }
    DetailsFieldResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_CERT_FORMAT, fieldType );

    // Private key location, only for user and device certicates
    if(( aType == KCertTypePersonal ) || ( aType == KCertTypeDevice ))
        {
        SetLocationInfoL( aMessage, EFalse, &keyLocation );
        }

    if( certiticateFormat == EX509Certificate )
        {
        __UHEAP_MARK;

        // certificate algorithms

        // digest algorithm
        TAlgorithmId algorithmId = details->SigningAlgorithm().DigestAlgorithm().Algorithm();
        switch( algorithmId )
            {
            case EMD2:
                {
                fieldType = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_MD2;
                break;
                }
            case EMD5:
                {
                fieldType = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_MD5;
                break;
                }
            case ESHA1:
                {
                fieldType = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_SHA1;
                break;
                }
            default:
                {
                fieldType = R_TEXT_RESOURCE_DETAILS_VIEW_UNKNOWN;
                break;
                }
            }

        // public-key algorithm
        algorithmId = details->SigningAlgorithm().AsymmetricAlgorithm().Algorithm();
        switch( algorithmId )
            {
            case ERSA:
                {
                fieldType2 = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_RSA;
                break;
                }
            case EDSA:
                {
                fieldType2 = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_DSA;
                break;
                }
            case EDH:
                {
                fieldType2 = R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM_DH;
                break;
                }
            default:
                {
                fieldType2 = R_TEXT_RESOURCE_DETAILS_VIEW_UNKNOWN;
                }
            }

        // If other algorithm is unknown
        if( fieldType == R_TEXT_RESOURCE_DETAILS_VIEW_UNKNOWN ||
                fieldType2 == R_TEXT_RESOURCE_DETAILS_VIEW_UNKNOWN )
            {
            DetailsFieldResourceL( aMessage,
                    R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM,
                    R_TEXT_RESOURCE_DETAILS_VIEW_UNKNOWN );
            }
        else // Both are known.
            {
            DetailsResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_ALGORITHM );
            HBufC* stringHolder = StringLoader::LoadLC( fieldType );
            aMessage.Des().Append( stringHolder->Des() );
            CleanupStack::PopAndDestroy( stringHolder );
            stringHolder = StringLoader::LoadLC( fieldType2 );
            aMessage.Des().Append( stringHolder->Des() );
            CleanupStack::PopAndDestroy( stringHolder );
            aMessage.Des().Append( KCertManUIDetailsViewEnterEnter );
            }

        // certificate serial number
        DetailsResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_SERIAL_NUMBER );
        TPtrC8 serialNumber = details->SerialNumber();
        TBuf<KMaxLengthTextSerialNumberFormatting> buf2;

        for( TInt i = 0; i < serialNumber.Length(); i++ )
           {
           buf2.Format( KCertManUIDetailsViewHexaFormat, serialNumber[i] );
           aMessage.Des().Append( buf2 );
           }

        aMessage.Des().Append( KCertManUIDetailsViewEnterEnter );

        // certificate fingerprint SHA-1
        DetailsResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_FINGERPRINT );
        TPtrC8 sha1_fingerprint = details->Fingerprint();
        DevideToBlocks( sha1_fingerprint, aMessage.Des() );
        aMessage.Des().Append( KCertManUIDetailsViewEnterEnter );

        // certificate fingerprint MD5
        DetailsResourceL( aMessage, R_TEXT_RESOURCE_DETAILS_VIEW_FINGERPRINT_MD5 );

        CMD5* md5 = CMD5::NewL();
        CleanupStack::PushL( md5 );
        TBuf8<20> fingerprint = md5->Hash( details->Encoding() );
        CleanupStack::PopAndDestroy( md5 );

        DevideToBlocks( fingerprint, aMessage.Des() );
        aMessage.Des().Append( KCertManUIDetailsViewEnterEnter );

        // Public key
        // qtn_cm_public_key looks like this "Public key (%0U %1N bit):"
        // so DetailsResource cannot be used to get it.

        // We already know the public key algorithm, it has been put into fieldType2 above.
        HBufC* pubkeyHolder = StringLoader::LoadLC( fieldType2 );  // %0U

        CDesCArrayFlat* strArray = new (ELeave) CDesCArrayFlat( 1 );
        CleanupStack::PushL( strArray );

        strArray->InsertL( 0, *pubkeyHolder );

        // Get public key already here to get length of it for string loader.
        const CSubjectPublicKeyInfo& publicKey = details->PublicKey();
        TPtrC8 keyData = publicKey.KeyData();
        TInt size = keyData.Size();  // %1N

        // To get key size decode DER encoded key got from the certificate.
        TX509KeyFactory key;

        // There doesn't seem to be definition of MAX_INT anywhere so calculate it.
        TReal maxInt;
        TReal intBits = sizeof( TInt ) * 8;
        Math::Pow( maxInt, 2, intBits );
        maxInt = ( maxInt / 2 ) - 1;

        switch( algorithmId )
            {
            case ERSA:
                {
                const CRSAPublicKey* keyRSA = key.RSAPublicKeyL( keyData );

                const TInteger& n = keyRSA->N();

                TUint keySizeN = n.BitCount();

                // Play it safe.
                if( keySizeN < maxInt )
                    {
                    size = keySizeN;
                    }

                delete keyRSA;

                break;
                }
            case EDSA:
                {
                TPtrC8 params = publicKey.EncodedParams();
                const CDSAPublicKey* keyDSA = key.DSAPublicKeyL( params, keyData );

                const TInteger& y = keyDSA->Y();

                TUint keySizeY = y.BitCount();

                // Play it safe.
                if( keySizeY < maxInt )
                    {
                    size = keySizeY;
                    }

                delete keyDSA;

                break;
                }
            // There doesn't seem to be TX509KeyFactory function for DH keys.
            // If the key is DH or unknown, just multiply length of the key
            // in bytes with 8. It is not correct but at least gives an idea.
            // Without setting something to size 'unknown' text should be used
            // below for the string which is much more error prone than setting
            // at least something.
            case EDH:
            default:
                {
                size = 8 * keyData.Size();
                }
            }

        CArrayFixFlat<TInt>* intArr = new (ELeave) CArrayFixFlat<TInt>( 1 );
        CleanupStack::PushL( intArr );
        intArr->AppendL( size );

        // Load string and set algorithm and key length strings.
        HBufC* stringHolder = StringLoader::LoadLC( R_TEXT_RESOURCE_DETAILS_VIEW_PUBLIC_KEY,
                *strArray, *intArr );
        aMessage.Des().Append( stringHolder->Des() );
        aMessage.Des().Append( KCertManUIDetailsViewEnter );

        // Finally append the public key.
        DevideToBlocks( keyData, aMessage.Des() );

        const TInt KItemsInCleanupStack = 4; // stringHolder, intArr, strArray, pubkeyHolder
        CleanupStack::PopAndDestroy( KItemsInCleanupStack, pubkeyHolder );

        __UHEAP_MARKEND;
        }
    else
        {
        // SetUrlCertAlgorihm()
        }

    if( certiticateFormat != EX509CertificateUrl )
        {
        CleanupStack::PopAndDestroy( details );
        }
    else
        {
        CleanupStack::PopAndDestroy( urlBuf );
        }

    aMessage.Des().Append( KCertManUIDetailsViewEnter );

    CERTMANUILOGGER_LEAVEFN( "CCertManUICertificateHelper::CreateMessageBodyTextL" );
    }


// End of file
