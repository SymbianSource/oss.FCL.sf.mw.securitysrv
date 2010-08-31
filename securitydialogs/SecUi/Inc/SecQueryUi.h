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
* Description:  CSecQueryUi API can be used to implement UI dialogs for
*               SecUi plugins.
*/


#ifndef C_SECQUERYUI_H
#define C_SECQUERYUI_H

#include <e32base.h>                            // CActive
#include <hb/hbcore/hbdevicedialogsymbian.h>    // MHbDeviceDialogObserver
#include <etelmm.h>
#include <e32property.h>

#define ESecUiTypeMask  0x0FFFFFF
#define ESecUiBasicTypeMask    0x00F0000

#define ESecUiCancelSupported  0x1000000
#define ESecUiCancelNotSupported  0x0000000

#define ESecUiEmergencySupported  0x2000000
#define ESecUiEmergencyNotSupported  0x0000000

#define ESecUiAlphaSupported  0x4000000
#define ESecUiAlphaNotSupported  0x0000000

#define ESecUiSecretSupported  0x8000000
#define ESecUiSecretNotSupported  0x0000000

#define ESecUiBasicTypeCheck   0x0010000
#define ESecUiBasicTypeMultiCheck   0x0020000

class MSecQueryUiCertificateDetailsProvider;
class MSecQueryUiDrmDetailsProvider;
class CHbDeviceDialogSymbian;
class CHbSymbianVariantMap;
class CActiveSchedulerWait;
class CApaMaskedBitmap;

typedef TBuf<80> TSecUiPassword;

enum R_SECUI_ERROR_CODES
	{
	R_CODE_ERROR=0x58ED0100,
	R_SEC_BLOCKED,
	R_CONFIRMATION_NOTE,
	R_PIN_CODE_CHANGED_NOTE,
	R_PIN2_CODE_CHANGED_NOTE,
	R_PIN2_REJECTED,
	R_UPIN_CODE_CHANGED_NOTE,
	R_SIM_NOT_ALLOWED,
	R_SIM_ALLREADY_OFF,
	R_SIM_OFF,
	R_SIM_ALLREADY_ON,
	R_OPERATION_NOT_ALLOWED,
	R_SECUI_TEXT_AUTOLOCK_MUST_BE_ACTIVE,
	R_PIN_NOT_ALLOWED,
	R_REMOTELOCK_INVALID_CODE,
	R_SIM_ON,
	R_INSERT_SIM,
	R_CODES_DONT_MATCH,
	R_PIN_BLOCKED_NOTE,
	R_UPIN_NOT_ALLOWED,
	R_SECURITY_CODE_CHANGED_NOTE,
	R_SECUI_ERROR_CODE_LAST
	};

/**
 * CSecQueryUi provides UI dialogs for SW installation. UI dialogs are
 * global and they can be used in a non-UI code (like in SIF plugins).
 * SW install device dialog plugin implements the UI dialogs.
 *
 * @lib SecQueryUi.lib
 * @since 10.1
 */
typedef TBuf<80> TSecUiPassword;

class CSecQueryUi : public CActive, public MHbDeviceDialogObserver
    {
    public:  // constructor and destructor
        /**
         * Creates new CSecQueryUi object and pushes it into cleanup stack.
         * @returns CSecQueryUi* -- new CSecQueryUi object
         */
        IMPORT_C static CSecQueryUi* NewLC();

        /**
         * Creates new CSecQueryUi object.
         * @returns CSecQueryUi* -- new CSecQueryUi object
         */
        IMPORT_C static CSecQueryUi* NewL();

        /**
         * Destructor.
         */
        CSecQueryUi::~CSecQueryUi();

    public:  // new functions
        /**
         * Displays install confirmation query synchronously. Synchronous function
         * returns after the user has accepted or cancelled the query.
         * @param aAppDetails - other application details (like supplier)
         * @return TBool - ETrue if user accepted the query, EFalse otherwise
         */
        IMPORT_C TBool InstallConfirmationQueryL( TInt aType, RMobilePhone::TMobilePassword& password );
        IMPORT_C TInt  SecQueryDialog(const TDesC& aCaption, TDes& aDataText, TInt aMinLength,TInt aMaxLength,TInt aMode);
        /*
        IMPORT_C TBool InstallConfirmationQueryL( const TDesC& aAppName,
                const TDesC& aIconFile, const TDesC& aAppVersion, TInt aAppSize,
                const TDesC& aAppDetails );
        */

        /**
         * Displays information note. If the information note is already displayed, then
         * only the dialog text is updated. Information note is closed automatically after
         * pre-defined time (e.g. after 1.5 seconds). User can also press any key to close
         * the note faster.
         * @param aText - info text to be displayed
         */
        IMPORT_C void DisplayInformationNoteL( const TDesC& aText );

        /**
         * Displays warning note. If the warning note is already displayed, then only the note
         * text is updated. Warning note is closed automatically after pre-defined time (e.g.
         * after three seconds). User can also press any key to close the note faster.
         * @param aText - error text to be displayed
         */
        IMPORT_C void DisplayWarningNoteL( const TDesC& aText );

        /**
         * Displays error note. If the error note is already displayed, then only the dialog
         * text is updated. Error note is closed automatically after pre-defined time (e.g.
         * after three seconds). User can also press any key to close the note faster.
         * @param aText - error text to be displayed
         */
        IMPORT_C void DisplayErrorNoteL( const TDesC& aText );

        /**
         * Displays permanent note. If the permanent note is already displayed, then only the
         * dialog text is updated. User cannot cancel or close permanent notes. Permanent note
         * is displayed until ClosePermanentNote() is called. Delayed notes are displayed after
         * pre-defined delay time, and they may be closed before they appear on the screen.
         * @param aText - text to be displayed
         * @param aIsDelayed - ETrue if delayed dialog should be used
         */
        IMPORT_C void DisplayPermanentNoteL( const TDesC& aText );

        /**
         * Closes the permanent note.
         */
        IMPORT_C void ClosePermanentNote();

        /**
         * Displays progress note using aFinalValue. Use UpdateProgressNoteValueL() to
         * update the progress bar. If the progress note is already active, then only
         * the note text and final value are updated.
         * @param aText - text displayed in progress note
         * @param aFinalValue - final value of the progress bar
         */
        IMPORT_C void DisplayProgressNoteL( const TDesC& aText, TInt aFinalValue );

        /**
         * Updates the progress bar in progress note. Progress bar shows 0% when
         * aNewValue is zero (or less than zero), and full 100% when aNewValue is
         * equal (or more than) the final value set with SetProgressNoteFinalValueL().
         * @param aNewValue - new value for the progress bar
         */
        IMPORT_C void UpdateProgressNoteValueL( TInt aNewValue );

        /**
         * Closes the progress note. TRequestStatus set in DisplayProgressNoteL() is
         * completed with KErrNone. Note that it is completed with KErrCancel if user
         * cancels the note, or with error code if some error happens.
         */
        IMPORT_C void CloseProgressNoteL();

        /**
         * Displays wait note. If the wait note is already displayed, then only the note
         * text is updated. Wait note is like progress note, except that it is not known
         * how long the operation takes. User can cancel the operation as in progress note.
         * @param aText - text to be displayed
         * @param aStatus - request status that indicates if user cancels the note
         */
        IMPORT_C void DisplayWaitNoteL( const TDesC& aText, TRequestStatus& aStatus );

        /**
         * Closes the wait note.
         */
        IMPORT_C void CloseWaitNote();

        /**
         * Launches help with the help context aContext for application aUid.
         * @param aContext - help context to be displayed
         * @param aUid - application which help needs to be launched
         */
        IMPORT_C void LaunchHelpL( const TDesC& aContext, const TUid& aUid );

        /**
         * Launches SW Install application (UID 0x101F8512) help using the
         * help context aContext.
         * @param aContext - help context to be displayed
         */
        IMPORT_C void LaunchHelpL( const TDesC& aContext );

    protected:  // from CActive
        void DoCancel();
        void RunL();

    private:    // from MHbDeviceDialogObserver
        void DataReceived( CHbSymbianVariantMap& aData );
        void DeviceDialogClosed( TInt aCompletionCode );

    private:    // new functions
        CSecQueryUi();
        void ConstructL();
        void ClearParamsL();
        void ClearParamsAndSetNoteTypeL( TInt aType );
        void AddParamL( const TDesC& aKey, TInt aValue );
        void AddParamL( const TDesC& aKey, const TDesC& aValue );
        void DisplayDeviceDialogL();
        TInt WaitUntilDeviceDialogClosed();

    private:    // data
        CHbDeviceDialogSymbian* iDeviceDialog;
        CHbSymbianVariantMap* iVariantMap;
        CActiveSchedulerWait* iWait;
        TBool iIsDisplayingDialog;
        TInt iCompletionCode;
        TInt iReturnValue;
public:
	   // previoulsy it was RMobilePhone::TMobilePassword iPassword; but this was only 10 bytes
        TSecUiPassword iPassword;
    };


#endif  // C_SECQUERYUI_H

