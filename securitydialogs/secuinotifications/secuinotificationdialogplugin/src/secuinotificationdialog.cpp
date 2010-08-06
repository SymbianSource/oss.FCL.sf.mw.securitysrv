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
* Description: SecUi notification plugin class.
*
*/

#include "secuinotificationdebug.h"
#include "secuinotificationdialog.h"
#include "secuinotificationdialogpluginkeys.h"
#include "secuinotificationcontentwidget.h"
// #include <hbpopupbase.h>        // HbPopupBase::NoTimeout
#include <hblabel.h>
#include <hbaction.h>
#include <QDebug>
#include <e32debug.h>
#include <hbtranslator.h>
#include <cphcltemergencycall.h>
#include <SCPServerInterface.h>	// for TARM error codes while validating new lock code
#include <QString>
#include <QDialogButtonBox>
#include <HbEditorInterface>
#include <e32property.h>

QTM_USE_NAMESPACE

const QString KTranslationsPath = "/resource/qt/translations/";
const QString KSecUiNotificationTranslationsFile = "secui";
const QString KCommonTranslationsFile = "common";

#include <qvaluespacesubscriber.h>
#include <qvaluespacepublisher.h>
#define ESecUiBasicTypeText    0x0000000
#define ESecUiBasicTypeCheck   0x0010000
#define ESecUiBasicTypeCheckMulti   0x0020000
#define ESecUiBasicTypeMask    0x00F0000

#define ESecUiCancelSupported  0x1000000
#define ESecUiCancelNotSupported  0x0000000

#define ESecUiEmergencySupported  0x2000000
#define ESecUiEmergencyNotSupported  0x0000000

#define ESecUiAlphaSupported  0x4000000
#define ESecUiAlphaNotSupported  0x0000000

#define ESecUiSecretSupported  0x8000000
#define ESecUiSecretNotSupported  0x0000000

#define ESecUiMaskFlags  0xFF000000
#define ESecUiMaskType   0x00FFFFFF

#define ESecUiTypeDeviceLock		0x00100000
#define ESecUiTypeKeyguard			0x00200000
#define ESecUiTypeClock  				0x00300000
#define ESecUiTypeScreensaver		0x00400000

#define ESecUiTypeMaskLock			0x00F00000

const TUid KPSUidSecurityUIs = { 0x100059b5 };
const TUint32 KSecurityUIsDismissDialog  = 0x00000309;

enum TSecurityUIsDismissDialogValues
    {
    ESecurityUIsDismissDialogUninitialized = 0,
    ESecurityUIsDismissDialogOn,
    ESecurityUIsDismissDialogProcessing,
    ESecurityUIsDismissDialogDone,
    ESecurityUIsDismissDialogLastValue
    };

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::SecUiNotificationDialog()
// ----------------------------------------------------------------------------
//
SecUiNotificationDialog::SecUiNotificationDialog(
        const QVariantMap &parameters) : HbDialog(), mCommonTranslator(0), mSecUiNotificationTranslator(0), mLastError(KNoError)
{
		RDEBUG("0", 0);
		TTime myTime;
    myTime.HomeTime();
		mMyId = I64LOW( myTime.Int64() );
    mCommonTranslator = new HbTranslator(KTranslationsPath, KCommonTranslationsFile);
    mSecUiNotificationTranslator = new HbTranslator(KTranslationsPath, KSecUiNotificationTranslationsFile);
    constructDialog(parameters);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::~SecUiNotificationDialog()
// ----------------------------------------------------------------------------
//
SecUiNotificationDialog::~SecUiNotificationDialog()
{
		RDEBUG("0", 0);
    delete mSecUiNotificationTranslator;
    delete mCommonTranslator;
		RDEBUG("0", 0);
    // TODO what about subscriberKSecurityUIsDismissDialog
    // and codeTop
		RDEBUG("1", 1);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::setDeviceDialogParameters()
// ----------------------------------------------------------------------------
//
bool SecUiNotificationDialog::setDeviceDialogParameters(const QVariantMap &parameters)
{
	 RDEBUG("0", 0);
   return constructDialog(parameters);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::deviceDialogError()
// ----------------------------------------------------------------------------
//
int SecUiNotificationDialog::deviceDialogError() const
{
		RDEBUG("mLastError", mLastError);
		RDEBUG("mMyId", mMyId);
    return mLastError;
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::closeDeviceDialog
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::closeDeviceDialog(bool byClient)
{
    Q_UNUSED(byClient);
		RDEBUG("mMyId", mMyId);
		RDEBUG("calling close()", 0);
    close();
		RDEBUG("mShowEventReceived", mShowEventReceived);

    // If show event has been received, close is signalled from hide event.
    // If not, hide event does not come and close is signalled from here.
    if (!mShowEventReceived) {
    		RDEBUG("emitting deviceDialogClosed", 0);
        emit deviceDialogClosed();
				if(1==0 && subscriberKSecurityUIsDismissDialog)
					{
						RDEBUG("disconnect subscriberKSecurityUIsDismissDialog", 0);
						disconnect(subscriberKSecurityUIsDismissDialog, SIGNAL(contentsChanged()), this, SLOT(subscriberKSecurityUIsDismissDialogChanged()));
						RDEBUG("disconnected subscriberKSecurityUIsDismissDialog", 1);
						if(1==1)
							{
							RDEBUG("deleting subscriberKSecurityUIsDismissDialog", 0);
							delete subscriberKSecurityUIsDismissDialog;
							subscriberKSecurityUIsDismissDialog = NULL;
							RDEBUG("deleted subscriberKSecurityUIsDismissDialog", 1);
							}
				 }
    }
    RDEBUG("0", 0);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::deviceDialogWidget
// ----------------------------------------------------------------------------
//
HbDialog *SecUiNotificationDialog::deviceDialogWidget() const
{
    return const_cast<SecUiNotificationDialog*>(this);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::hideEvent
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::hideEvent(QHideEvent *event)
{
		RDEBUG("0", 0);
    HbDialog::hideEvent(event);
		RDEBUG("mMyId", mMyId);
		RDEBUG("calling close()", 0);
		close();
		RDEBUG("emitting deviceDialogClosed", 0);
		emit deviceDialogClosed();
		RDEBUG("deviceDialogClosed", 1);
		if(1==0 && subscriberKSecurityUIsDismissDialog)
			{
				RDEBUG("disconnect subscriberKSecurityUIsDismissDialog", 0);
				disconnect(subscriberKSecurityUIsDismissDialog, SIGNAL(contentsChanged()), this, SLOT(subscriberKSecurityUIsDismissDialogChanged()));
				RDEBUG("disconnected subscriberKSecurityUIsDismissDialog", 1);
				if(1==1)
					{ 
					RDEBUG("deleting subscriberKSecurityUIsDismissDialog", 0);
					delete subscriberKSecurityUIsDismissDialog;
					subscriberKSecurityUIsDismissDialog = NULL;
					RDEBUG("deleted subscriberKSecurityUIsDismissDialog", 1);
					}
			}
    // old method was   emit deviceDialogClosed();
   RDEBUG("0", 0);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::showEvent
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::showEvent(QShowEvent *event)
{
		RDEBUG("0", 0);
		RDEBUG("mMyId", mMyId);
    HbDialog::showEvent(event);

		if(!(queryType & ESecUiTypeMaskLock))
			{	// not for the "lock icon"
			RDEBUG("check default.1", 0);
			if(codeTop!=NULL)
            {
            RDEBUG("check default.2", 0);
            if(codeTop->text().length()>0)	// there's a default value. Verify it and (might) enable OK
                {
                RDEBUG("check default.3", 0);
                handleCodeTopChanged(codeTop->text());
                }
            RDEBUG("setFocus", 0);
        		// remove setFocus from PIN query because Orbit can't handle it
      			// codeTop->setFocus();	// this should open the VKB . Doesn't seem to work when it's done on the widget, so it's done here.
            }	// codeTop!=NULL
            
			const TUint32 KSecurityUIsTestCode  = 0x00000307;
			TInt value = 0;
			TInt err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsTestCode, value );
			RDEBUG("KSecurityUIsTestCode err", err);
			RDEBUG("faking value", value);
			RDEBUG("mShowEventReceived", mShowEventReceived);
			if(value>0 && value<1000000 && mShowEventReceived==true)	// show happens 2 times. Dialog can be closed only the second.
				{
				QString myString = "";
				myString += QString("%1").arg(value);
				RDEBUGQT("myString", myString);
		    codeTop->setText( myString );
		    TInt err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsTestCode, 0 );	// clear after using it
		    RDEBUGQT("calling handleAccepted myString", myString);
		    emit handleAccepted();
				// handleAccepted already   emit closeDeviceDialog(false);	// false means "not by client", although it's not really used
				}
			}
    mShowEventReceived = true;
    RDEBUG("1", 1);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::constructDialog()
// ----------------------------------------------------------------------------
//
bool SecUiNotificationDialog::constructDialog(const QVariantMap &parameters)
    {
		RDEBUG("0", 0);
		RDEBUG("mMyId", mMyId);
    setTimeout(HbPopup::NoTimeout);
    setDismissPolicy(HbPopup::NoDismiss);
    setModal(true);
    mShowEventReceived = false;
    subscriberKSecurityUIsDismissDialog = NULL;
    titleWidget = NULL;
		RDEBUG("subscriberKSecurityUIsDismissDialog NULL", 0);

    subscriberKSecurityUIsDismissDialog = new QValueSpaceSubscriber("/KPSUidSecurityUIs/KSecurityUIsDismissDialog", this);
    if(subscriberKSecurityUIsDismissDialog)
    	{
    	RDEBUG("subscriberKSecurityUIsDismissDialog created", 1);
    	}
    else
    	{
    	RDEBUG("subscriberKSecurityUIsDismissDialog not created", 0);
    	}
    connect(subscriberKSecurityUIsDismissDialog, SIGNAL(contentsChanged()), this, SLOT(subscriberKSecurityUIsDismissDialogChanged()));
		RDEBUG("subscriberKSecurityUIsDismissDialog", 1);

    // Title
    // this is also done later in the widget
    // For now, it only says "Security"
    if (parameters.contains(KDialogTitle)) {
        QString titleText = parameters.value(KDialogTitle).toString();
        QString titleAttempts = "";
   			if(titleText.indexOf('|')>0)
    				{	// if separator, take only first part
    				titleText = titleText.left(titleText.indexOf('|'));
    				}
   			if(titleText.indexOf('$')>0)
    				{	// if separator, take only first part
    				titleAttempts = titleText.right(titleText.length()-titleText.indexOf('$')-1);
    				RDEBUGQT("titleAttempts", titleAttempts);
    				int nAttempts = titleAttempts.toInt();
    				RDEBUG("nAttempts", nAttempts);
    				titleText = titleText.left(titleText.indexOf('$'));
    				if(nAttempts>0)
    					titleText = titleText + " attempts=" + QString::number(nAttempts);
    				}
    		titleText = hbTrId("Security Query");	// this is the header, for any potential later error which needs to be displayed
        // This is created only if needed (i.e. errors for NewLockCode)
        // titleWidget = new HbLabel(titleText);
        // setHeadingWidget(titleWidget);
    }

	    if (parameters.contains(KEmergency)) {
					RDEBUG("KEmergency", 1);
	        QString emergencyText = parameters.value(KEmergency).toString();
	        RDEBUGQT("emergencyText", emergencyText);
	        if(!emergencyText.compare("emergencyYes"))
	        	{
	        	RDEBUG("emergencyYes", 1);
	        	isEmergency = 1;
	        	okAction->setEnabled(true);
	        	okAction->setText(hbTrId("Call"));
    				return true;
	        	}
	        if(!emergencyText.compare("emergencyNo"))
	        	{
	        	RDEBUG("emergencyNo", 1);
	        	isEmergency = 0;
	        	okAction->setEnabled(false);	// 112 -> 1122 (=password) . This is handled by   < lMinLength 
	        	okAction->setText(hbTrId("Ok"));
    				return true;
	        	}
	    }
	    // after TARM validation.
	    if (parameters.contains(KInvalidNewLockCode)) {
					RDEBUG("KInvalidNewLockCode", 0);
	        QString invalidText = parameters.value(KInvalidNewLockCode).toString();
	        RDEBUGQT("invalidText", invalidText);
	        QString newTitleText = "Lock Code";	// TODO take from the original one

	        QString invalidStr = invalidText.right(invalidText.length()-invalidText.indexOf('$')-1);
	        int invalidNumber = invalidStr.toInt();
	        RDEBUG("invalidNumber", invalidNumber);
	        if(invalidNumber<0)	// that is, -1
	        	{
	        	RDEBUG("invalidNumber<0", invalidNumber );
	        	// nothing to do
	        	}
				if(invalidNumber==EDeviceLockAutolockperiod)
	        	{
	        	RDEBUG("EDeviceLockAutolockperiod", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockAutolockperiod");
	        	}
	        if(invalidNumber==EDeviceLockMaxAutolockPeriod)
	        	{
	        	RDEBUG("EDeviceLockAutolockperiod", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockMaxAutolockPeriod");
	        	}
	        if(invalidNumber==EDeviceLockMinlength)
	        	{
	        	RDEBUG("EDeviceLockMinlength", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockMinlength");
	        	}
	        if(invalidNumber==EDeviceLockMaxlength)
	        	{
	        	RDEBUG("EDeviceLockMaxlength", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockMaxlength");
	        	}
	        if(invalidNumber==EDeviceLockRequireUpperAndLower)
	        	{
	        	RDEBUG("EDeviceLockRequireUpperAndLower", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockRequireUpperAndLower");
	        	}
	        if(invalidNumber==EDeviceLockRequireCharsAndNumbers)
	        	{
	        	RDEBUG("EDeviceLockMaxlength", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockMaxlength");
	        	}
	        if(invalidNumber==EDeviceLockAllowedMaxRepeatedChars)
	        	{
	        	RDEBUG("EDeviceLockAllowedMaxRepeatedChars", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockAllowedMaxRepeatedChars");
	        	}
	        if(invalidNumber==EDeviceLockHistoryBuffer)
	        	{
	        	RDEBUG("EDeviceLockHistoryBuffer", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockHistoryBuffer");
	        	}
	        if(invalidNumber==EDeviceLockPasscodeExpiration)
	        	{
	        	RDEBUG("EDeviceLockPasscodeExpiration", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockPasscodeExpiration");
	        	}
	        if(invalidNumber==EDeviceLockMinChangeTolerance)
	        	{
	        	RDEBUG("EDeviceLockMinChangeTolerance", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockMinChangeTolerance");
	        	}
	        if(invalidNumber==EDeviceLockMinChangeInterval)
	        	{
	        	RDEBUG("EDeviceLockMinChangeInterval", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockMinChangeInterval");
	        	}
	        if(invalidNumber==EDeviceLockDisallowSpecificStrings)
	        	{
	        	RDEBUG("EDeviceLockDisallowSpecificStrings", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockDisallowSpecificStrings");
	        	}
	        if(invalidNumber==EDeviceLockAllowedMaxAtempts)
	        	{
	        	RDEBUG("EDeviceLockAllowedMaxAtempts", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockAllowedMaxAtempts");
	        	}
	        if(invalidNumber==EDeviceLockConsecutiveNumbers)
	        	{
	        	RDEBUG("EDeviceLockConsecutiveNumbers", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockConsecutiveNumbers");
	        	}
	        if(invalidNumber==EDeviceLockMinSpecialCharacters)
	        	{
	        	RDEBUG("EDeviceLockMinSpecialCharacters", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockMinSpecialCharacters");
	        	}
	        if(invalidNumber==EDeviceLockSingleCharRepeatNotAllowed)
	        	{
	        	RDEBUG("EDeviceLockSingleCharRepeatNotAllowed", invalidNumber );
	        	newTitleText+=hbTrId("EDeviceLockSingleCharRepeatNotAllowed");
	        	}
	        if(invalidNumber==EDevicelockConsecutiveCharsNotAllowed)
	        	{
	        	RDEBUG("EDevicelockConsecutiveCharsNotAllowed", invalidNumber );
	        	newTitleText+=hbTrId("EDevicelockConsecutiveCharsNotAllowed");
	        	}
	        if(invalidNumber>=EDevicelockTotalPolicies)
	        	{
	        	RDEBUG("EDevicelockTotalPolicies", invalidNumber );
	        	newTitleText+=hbTrId("EDevicelockTotalPolicies");
	        	}
	        if( !titleWidget )
	        	{
	        	RDEBUG("creating titleWidget", 0 );
	        	titleWidget = new HbLabel(hbTrId("New lock code"));	// it will be changed later
          	setHeadingWidget(titleWidget);
          	}
          RDEBUG("setPlainText", 0 );
	        titleWidget->setPlainText(newTitleText);
	        
	        if(invalidNumber<0)	// everything is ok
	        	{
	        	okAction->setEnabled(true);	// this might fail in the scenario: check this : invalid -> valid. This allows verif ?
	        	okAction->setText(hbTrId("Ok"));
	        	codeBottom->setEnabled(true);
	        	}
	        else
	        	{
	        	okAction->setEnabled(false);
	        	codeBottom->setEnabled(false);
	        	codeBottom->setText("");
	        	okAction->setText(hbTrId("Ok"));
	        	}
	        // need to return because all objects are already created
   				return true;
	    }
	
    // Content
    SecUiNotificationContentWidget *content = new SecUiNotificationContentWidget();
    content->constructFromParameters(parameters);
    setContentWidget(content);

		queryType = content->queryType;
		queryDual = content->queryDual;
		isEmergency = content->isEmergency;
    codeTop=NULL;
		codeTop = content->codeTop;
    checkBox = content->checkbox;
    listWidget = content->listWidget;
		codeBottom = content->codeBottom;
		lMinLength = content->lMinLength;
		lMaxLength = content->lMaxLength;
		lEmergencySupported = content->lEmergencySupported;

    connect(content, SIGNAL(codeTopChanged(const QString &)), this, SLOT(handleCodeTopChanged(const QString &)));
    connect(content, SIGNAL(codeBottomChanged(const QString &)), this, SLOT(handleCodeBottomChanged(const QString &)));
    connect(content, SIGNAL(codeTopContentChanged()), this, SLOT(handleCodeTopContentChanged()));
    connect(content, SIGNAL(but1Changed()), this, SLOT(handlebut1Changed()));
    connect(content, SIGNAL(but2Changed()), this, SLOT(handlebut2Changed()));
    connect(content, SIGNAL(but3Changed()), this, SLOT(handlebut3Changed()));
		RDEBUG("queryType", queryType);
    // Buttons
    if( (queryType & ESecUiTypeMaskLock))
    	{
    	// no need to create OK or Cancel
    	return true;
    	}

    okAction = new HbAction(hbTrId("Ok"));
    RDEBUG("created HbAction okAction", 1);
    okAction->setEnabled(false);	// initially the OK is disabled because codeTop is empty
    if((queryType & ESecUiBasicTypeMask) ==ESecUiBasicTypeCheck) {
        okAction->setEnabled(true);
        setHeadingWidget(0); // had to remove this no multiline
    }
    else if ((queryType & ESecUiBasicTypeMask) ==ESecUiBasicTypeCheckMulti){
        okAction->setEnabled(true);
    }

    // setAction(okAction, QDialogButtonBox::AcceptRole);	// it's supposed to use this, when deprecated
    // setPrimaryAction(okAction);
    addAction(okAction);
    disconnect(okAction, SIGNAL(triggered()), this, SLOT(close()));	// the close will be done in handleAccepted
    connect(okAction, SIGNAL(triggered()), this, SLOT(handleAccepted()));
    
    cancelAction = new HbAction(hbTrId("Cancel"));    // qtTrId("txt_common_button_cancel")
    addAction(cancelAction);
    disconnect(cancelAction, SIGNAL(triggered()), this, SLOT(close()));	// the close will be done in handleCancelled
    connect(cancelAction, SIGNAL(triggered()), this, SLOT(handleCancelled()));
    // setAction(cancelAction, QDialogButtonBox::RejectRole);		// it's supposed to use this, when deprecated
    // setSecondaryAction(cancelAction);

		// this should had been set by Autolock, but just to be sure
    TInt ret = RProperty::Define(KPSUidSecurityUIs, KSecurityUIsDismissDialog,
            RProperty::EInt, TSecurityPolicy(TSecurityPolicy::EAlwaysPass),
            TSecurityPolicy(TSecurityPolicy::EAlwaysPass));
    RDEBUG("defined KSecurityUIsDismissDialog", ret);
    TInt aDismissDialog = -1;
    ret = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsDismissDialog, aDismissDialog );
    RDEBUG("ret", ret);
    RDEBUG("aDismissDialog", aDismissDialog);
    if(aDismissDialog==ESecurityUIsDismissDialogOn || aDismissDialog==ESecurityUIsDismissDialogProcessing)
    	{
    	RDebug::Printf( "potential error: %s %s (%u) aDismissDialog=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, aDismissDialog );
    	}
	
		RDEBUG("check cancel", 0);
    if ((queryType & ESecUiCancelSupported)==ESecUiCancelSupported)
    	{
    		// nothing to do. Cancel is enabled by default
    	}
  	else
  		{
				RDEBUG("disable Cancel", 1);
  			cancelAction->setEnabled(false);
  			cancelAction->setText("");
  			cancelAction->setVisible(false);
  		}
    
    return true;
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::sendResult()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::sendResult(int accepted)
{
		RDEBUG("0", 0);
		RDEBUG("mMyId", mMyId);
    QVariant acceptedValue(accepted);
		RDEBUG("0", 0);
    mResultMap.insert(KResultAccepted, acceptedValue);
		RDEBUG("0", 0);
		RDEBUGQT("mResultMap", mResultMap);
		RDEBUG("queryType", queryType);
		if(!(queryType & ESecUiTypeMaskLock))
			{	// the lock-icon should not reply
			RDEBUG("emit deviceDialogData", 0);
    	emit deviceDialogData(mResultMap);
    	}
		RDEBUG("1", 1);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleAccepted()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleAccepted()
{
		RDEBUG("0", 0);
		// okAction
		RDEBUG("mMyId", mMyId);
		QString codeTopText="";

		if( (queryType & ESecUiTypeMaskLock))
    	{
    	codeTopText = "Unlock-Request";
    	}
    else if( (queryType & ESecUiBasicTypeMask)==ESecUiBasicTypeCheck)
    	{
        codeTopText=(checkBox->isChecked() ?  "1":"0");
        mResultMap.insert(KCodeTopIndex,  codeTopText);
    	}
    else if( (queryType & ESecUiBasicTypeMask)==ESecUiBasicTypeCheckMulti)
    	{
        QItemSelectionModel *selectionModel = listWidget->selectionModel();
        QModelIndexList selectedItems = selectionModel->selectedIndexes();
        QModelIndex index;
        codeTopText="";
         foreach(index, selectedItems) { 
		 		 codeTopText+=QString::number(index.row());
                // could also use  if(index.row()!=selectedItems.count()-1) codeTopText+= "|";
                codeTopText+= "|";
                }
         mResultMap.insert(KCodeTopIndex,  codeTopText);
    	}
    else
        codeTopText = codeTop->text();
    // no need to check last time for codeBottom
   	RDEBUGQT("codeTopText", codeTopText);
    sendResult(KErrNone);
    RDEBUG("calling close()", 0);
    close();	// this is needed because Cancel doesn't automatically closes the dialog
		RDEBUG("emitting deviceDialogClosed", 0);
		emit deviceDialogClosed();
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleCancelled()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleCancelled()
{
		RDEBUG("0", 0);
		RDEBUG("mMyId", mMyId);
    sendResult(KErrCancel);
		RDEBUG("callingclose()", 0);
    close();	// this is needed because Cancel doesn't automatically closes the dialog
		RDEBUG("emitting deviceDialogClosed", 0);
		emit deviceDialogClosed();
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleMemorySelectionChanged()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleMemorySelectionChanged(const QString &text)
    {
		RDEBUG("0", 0);
		RDEBUG("mMyId", mMyId);
    RDEBUGQT("text", text);
    QVariant memorySelection(text);
    mResultMap.insert(KSelectedMemoryIndex, memorySelection);
    }

void SecUiNotificationDialog::handleCodeTopContentChanged()
    {
		RDEBUG("0", 0);
		RDEBUG("mMyId", mMyId);
    	RDEBUGQT("codeTop->text()", codeTop->text());
    	handleCodeTopChanged(codeTop->text());
    }

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleCodeTopChanged()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleCodeTopChanged(const QString &text)
    {
		RDEBUG("0", 0);
    	RDEBUGQT("text", text);
    	if(queryDual)
    		{
    		codeBottom->setText("");	// any change resets the verification.
	    	if( (queryType & ESecUiMaskType) == 0x000004  )	// new codeLock
	    		{	// ChangeSecCodeParamsL change RMobilePhone::ESecurityCodePhonePassword
				    QVariant codeTopVar(text);
				    mResultMap.insert(KCodeTopIndex, codeTopVar);
						sendResult(KErrCompletion);	// send the current password back to the client for further TARM validation. This is done on any key-press, not in the OK
	    		}
    		}
    	else if(text.length() < lMinLength )
    		{
    		RDEBUGQT("too short text", text);
    		okAction->setEnabled(false);
				RDEBUG("lEmergencySupported", lEmergencySupported);
				if( lEmergencySupported && text.length() > 2 )	// emergency numbers need at least 3 digits
					{	// check whether it's a emergency number
					QVariant codeTopVar(text);
  				mResultMap.insert(KCodeTopIndex, codeTopVar);
					sendResult(KErrAbort);	// send the current password back to the client. Perhaps it's an emergency number and decides to Ok->Call
					}
    		}
    	else if (text.length() >= lMinLength)
    		{
    		// might use a flag to avoid re-setting. But this complicates things if there's another initial verification
    		RDEBUGQT("long enough text", text);
    		okAction->setText(hbTrId("Ok"));
    		if(queryDual==0)	// only if Bottom is not used
    			okAction->setEnabled(true);
    		}
    QVariant codeTopVar(text);
    mResultMap.insert(KCodeTopIndex, codeTopVar);
    }
// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleCodeBottomChanged()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleCodeBottomChanged(const QString &text)
    {
		RDEBUG("0", 0);
    	RDEBUGQT("text", text);
    	RDEBUGQT("codeTop->text()", codeTop->text());
    	if(text.length() < lMinLength )
    		{
    		RDEBUGQT("too short text", text);
    		okAction->setEnabled(false);
    		}
    	else
    		{
    		// might use a flag to avoid re-setting. But it just complicates things.
    		RDEBUGQT("long enough text", text);
    		if(codeTop->text()==text)
    			{
    			// unless both codes match, don't allow OK. Note that the first field doesn't allow exit until the validations (i.e. NewLockCode) as succesfull
    			RDEBUGQT("codes match", text);
	    		okAction->setEnabled(true);
	    		}
	    	else
	    		{
    			RDEBUGQT("codes don't match", text);
					okAction->setEnabled(false);
	    		}
    		}
		// verification is not sent
    }
// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handlebut1Changed()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handlebut1Changed()
    {
		RDEBUG("0", 0);
    codeTop->setText("1234");
    }
// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handlebut2Changed()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handlebut2Changed()
    {
		RDEBUG("0", 0);
    	QString codeTopText = codeTop->text();
    	RDEBUGQT("codeTopText", codeTopText);
    	codeTopText = codeTopText + "1" ;
    	RDEBUGQT("codeTopText+1", codeTopText);
    	codeTop->setText(codeTopText);
    	
    	      RDEBUG("editorInterface", 0);
      			HbEditorInterface editorInterface(codeTop);
      			RDEBUG("actions", 0);
      			QList<HbAction *> vkbList = editorInterface.actions();
      			RDEBUG("count", 0);
      			int count = vkbList.count();
      			RDEBUG("got count", count);
		        for (int i = 0; i < count; i++)
		        		{
		        		RDEBUG("i", i);
		            HbAction *action = static_cast<HbAction *>(vkbList[i]);
		            RDEBUG("action", 0);
		          	}
		          	
		        RDEBUG("okVKBAction", 0);
		        okVKBAction = new HbAction(tr("Ok"));
		        RDEBUG("addAction", 0);
		        editorInterface.addAction(okVKBAction);
		        RDEBUG("addAction", 1);
		        connect(okVKBAction, SIGNAL(triggered()), this, SLOT(handleAccepted()));
		        RDEBUG("connect", 1);

      			QList<HbAction *> vkbList2 = editorInterface.actions();
      			RDEBUG("count", 0);
      			int count2 = vkbList2.count();
      			RDEBUG("got count2", count2);
		        for (int i = 0; i < count2; i++)
		        		{
		        		RDEBUG("i", i);
		            HbAction *action2 = static_cast<HbAction *>(vkbList2[i]);
		            RDEBUG("action2", 0);
		          	}
    
    }
// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handlebut3Changed()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handlebut3Changed()
    {
    	RDEBUG("0", 0);
    	QString codeTopText = codeTop->text();
    	RDEBUG("codeTopText", 0);
    	RDEBUGQT("codeTopText", codeTopText);
    	codeTopText = codeTopText + "5" ;
    	RDEBUGQT("codeTopText+5", codeTopText);
    	codeTop->setEchoMode(HbLineEdit::PasswordEchoOnEdit);
    	RDEBUGQT("codeTopText", codeTopText);
    	codeTop->setText(codeTopText);
    	RDEBUG("setFocus", 0);
    	codeTop->setFocus();
    	RDEBUG("1", 1);
    }

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::saveFocusWidget(QWidget*,QWidget*)
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::saveFocusWidget(QWidget*,QWidget*)
{
		RDEBUG("0", 0);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::subscriberKSecurityUIsDismissDialogChanged()
// A way for Autolock to dismiss any possible PIN dialog
// This doesn't dismiss the lockIcon because P&S is not connected
// Note: if this changes itself, then it becomes recursive
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::subscriberKSecurityUIsDismissDialogChanged()
    {

    RDEBUG("0", 0);
		RDEBUG("mMyId", mMyId);
    TInt aDismissDialog = ESecurityUIsDismissDialogUninitialized;
    TInt err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsDismissDialog, aDismissDialog );
    RDEBUG("err", err);
		RDEBUG("aDismissDialog", aDismissDialog);
    if( aDismissDialog == ESecurityUIsDismissDialogOn )
    	{
    	if(subscriberKSecurityUIsDismissDialog)
    		{
    		RDEBUG("subscriberKSecurityUIsDismissDialog", 1);
    		}
    	else
    		{
    		RDEBUG("! subscriberKSecurityUIsDismissDialog", 0);
    		}

    	if(this)
    		{
    		RDEBUG("this", 1);
    		}
    	else
    		{
    		RDEBUG("! this", 0);
    		}


			RDEBUG("disconnect subscriberKSecurityUIsDismissDialog", 0);
			disconnect(subscriberKSecurityUIsDismissDialog, SIGNAL(contentsChanged()), this, SLOT(subscriberKSecurityUIsDismissDialogChanged()));
			// this doesn't really disconnect, because somehow the events are still queued. This is a QtMobility error
			RDEBUG("disconnected subscriberKSecurityUIsDismissDialog", 1);
			
			RDEBUG("not set KSecurityUIsDismissDialog", ESecurityUIsDismissDialogProcessing);
			// can't set it because it does recursion
			// err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsDismissDialog, ESecurityUIsDismissDialogProcessing );
			RDEBUG("err", err);
			// only if Cancel is allowed
			if ((queryType & ESecUiCancelSupported)==ESecUiCancelSupported)
				{
				RDEBUG("sendResult(KErrCancel)", KErrCancel);	// another option is KErrDied
				sendResult(KErrCancel);	// similar to     emit handleCancelled();
				RDEBUG("calling close()", 0);
				err = close();
				RDEBUG("err", err);
				RDEBUG("emitting deviceDialogClosed", 0);
				emit deviceDialogClosed();
				// RDEBUG("emit closeDeviceDialog", 0);
				// this is old method    emit closeDeviceDialog(false);	// false means "not by client", although it's not really used
				RDEBUG("all emited", 0);
				}
			RDEBUG("not set KSecurityUIsDismissDialog", ESecurityUIsDismissDialogDone);
			// can't set it because it does recursion
			// err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsDismissDialog, ESecurityUIsDismissDialogDone );	// clear after using it
			RDEBUG("err", err);

			RDEBUG("reconnect subscriberKSecurityUIsDismissDialog", 0);
			connect(subscriberKSecurityUIsDismissDialog, SIGNAL(contentsChanged()), this, SLOT(subscriberKSecurityUIsDismissDialogChanged()));
			RDEBUG("reconnected subscriberKSecurityUIsDismissDialog", 1);
    	}
    RDEBUG("1", 1);
	}
