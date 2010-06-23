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

#include "secuinotificationdialog.h"
#include "secuinotificationdialogpluginkeys.h"
#include "secuinotificationcontentwidget.h"
// #include <hbpopupbase.h>        // HbPopupBase::NoTimeout
#include <hblabel.h>
#include <hbaction.h>
#include <QDebug>
#include <e32debug.h>
#include <cphcltemergencycall.h>
#include <SCPServerInterface.h>	// for TARM error codes while validating new lock code
#include <QString>
#include <QDialogButtonBox>
#include <e32property.h>

QTM_USE_NAMESPACE

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
        const QVariantMap &parameters) : HbDialog(), mLastError(KNoError)
{
		RDEBUG("0", 0);
    constructDialog(parameters);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::~SecUiNotificationDialog()
// ----------------------------------------------------------------------------
//
SecUiNotificationDialog::~SecUiNotificationDialog()
{
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
    return mLastError;
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::closeDeviceDialog
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::closeDeviceDialog(bool byClient)
{
    Q_UNUSED(byClient);
		RDEBUG("0", 0);
    close();
		RDEBUG("mShowEventReceived", mShowEventReceived);

    // If show event has been received, close is signalled from hide event.
    // If not, hide event does not come and close is signalled from here.
    if (!mShowEventReceived) {
        emit deviceDialogClosed();
    }
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
		RDEBUG("close", 0);
		close();
		RDEBUG("deviceDialogClosed", 0);
		emit deviceDialogClosed();
		RDEBUG("deviceDialogClosed", 1);
    // old method was   emit deviceDialogClosed();
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::showEvent
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::showEvent(QShowEvent *event)
{
		RDEBUG("0", 0);
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
            
            }
			const TUint32 KSecurityUIsTestCode  = 0x00000307;
			TInt value = 0;
			TInt err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsTestCode, value );
			RDEBUG("KSecurityUIsTestCode err", err);
			RDEBUG("faking value", value);
			if(value>0 && mShowEventReceived==true)	// show happens 2 times. Dialog can be closed only the second.
				{
				QString myString = "";
				myString += QString("%1").arg(value);
				qDebug() << "SecUiNotificationDialog::faking myString=" << myString;
		    codeTop->setText( myString );
		    TInt err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsTestCode, 0 );	// clear after using it
				qDebug() << "SecUiNotificationDialog::calling handleAccepted=" << myString;
		    emit handleAccepted();
				// handleAccepted already   emit closeDeviceDialog(false);	// false means "not by client", although it's not really used
				}
			}
    mShowEventReceived = true;
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::constructDialog()
// ----------------------------------------------------------------------------
//
bool SecUiNotificationDialog::constructDialog(const QVariantMap &parameters)
    {
		RDEBUG("0", 0);
    setTimeout(HbPopup::NoTimeout);
    setDismissPolicy(HbPopup::NoDismiss);
    setModal(true);
    mShowEventReceived = false;

    // Title
    if (parameters.contains(KDialogTitle)) {
        QString titleText = parameters.value(KDialogTitle).toString();
        QString titleAttempts = "";
   			if(titleText.indexOf('|')>0)
    				{	// if separator, take only first part
    				titleText = titleText.left(titleText.indexOf('|'));
    				}
   			if(titleText.indexOf('#')>0)
    				{	// if separator, take only first part
    				titleAttempts = titleText.right(titleText.length()-titleText.indexOf('#')-1);
    				qDebug() << "SecUiNotificationDialog::titleAttempts=" << titleAttempts;
    				int nAttempts = titleAttempts.toInt();
    				RDEBUG("nAttempts", nAttempts);
    				titleText = titleText.left(titleText.indexOf('#'));
    				if(nAttempts>0)
    					titleText = titleText + " attempts=" + QString::number(nAttempts);
    				}
        title = new HbLabel(titleText);
        setHeadingWidget(title);
    }

	    if (parameters.contains(KEmergency)) {
					RDEBUG("KEmergency", 1);
	        QString emergencyText = parameters.value(KEmergency).toString();
	        qDebug() << emergencyText;
	        if(!emergencyText.compare("emergencyYes"))
	        	{
	        	RDEBUG("emergencyYes", 1);
	        	isEmergency = 1;
	        	okAction->setEnabled(true);
	        	okAction->setText("Call");
    				return true;
	        	}
	        if(!emergencyText.compare("emergencyNo"))
	        	{
	        	RDEBUG("emergencyNo", 1);
	        	isEmergency = 0;
	        	okAction->setEnabled(false);	// 112 -> 1122 (=password) . This is handled by   < lMinLength 
	        	okAction->setText("Ok");
    				return true;
	        	}
	    }
	    // after TARM validation.
	    if (parameters.contains(KInvalidNewLockCode)) {
					RDEBUG("KInvalidNewLockCode", 0);
	        QString invalidText = parameters.value(KInvalidNewLockCode).toString();
	        qDebug() << invalidText;

	        title->setPlainText("Lock Code");	// TODO take from the original one
	        QString invalidStr = invalidText.right(invalidText.length()-invalidText.indexOf('#')-1);
	        int invalidNumber = invalidStr.toInt();
	        RDEBUG("invalidNumber", invalidNumber);
	        if(invalidNumber<0)
	        	{
	        	RDEBUG("invalidNumber<0", invalidNumber );
	        	// nothing to do
	        	}
				if(invalidNumber==EDeviceLockAutolockperiod)
	        	{
	        	RDEBUG("EDeviceLockAutolockperiod", invalidNumber );
	        	title->setPlainText("EDeviceLockAutolockperiod");
	        	}
	        if(invalidNumber==EDeviceLockMaxAutolockPeriod)
	        	{
	        	RDEBUG("EDeviceLockAutolockperiod", invalidNumber );
	        	title->setPlainText("EDeviceLockMaxAutolockPeriod");
	        	}
	        if(invalidNumber==EDeviceLockMinlength)
	        	{
	        	RDEBUG("EDeviceLockMinlength", invalidNumber );
	        	title->setPlainText("EDeviceLockMinlength");
	        	}
	        if(invalidNumber==EDeviceLockMaxlength)
	        	{
	        	RDEBUG("EDeviceLockMaxlength", invalidNumber );
	        	title->setPlainText("EDeviceLockMaxlength");
	        	}
	        if(invalidNumber==EDeviceLockRequireUpperAndLower)
	        	{
	        	RDEBUG("EDeviceLockRequireUpperAndLower", invalidNumber );
	        	title->setPlainText("EDeviceLockRequireUpperAndLower");
	        	}
	        if(invalidNumber==EDeviceLockRequireCharsAndNumbers)
	        	{
	        	RDEBUG("EDeviceLockMaxlength", invalidNumber );
	        	title->setPlainText("EDeviceLockMaxlength");
	        	}
	        if(invalidNumber==EDeviceLockAllowedMaxRepeatedChars)
	        	{
	        	RDEBUG("EDeviceLockAllowedMaxRepeatedChars", invalidNumber );
	        	title->setPlainText("EDeviceLockAllowedMaxRepeatedChars");
	        	}
	        if(invalidNumber==EDeviceLockHistoryBuffer)
	        	{
	        	RDEBUG("EDeviceLockHistoryBuffer", invalidNumber );
	        	title->setPlainText("EDeviceLockHistoryBuffer");
	        	}
	        if(invalidNumber==EDeviceLockPasscodeExpiration)
	        	{
	        	RDEBUG("EDeviceLockPasscodeExpiration", invalidNumber );
	        	title->setPlainText("EDeviceLockPasscodeExpiration");
	        	}
	        if(invalidNumber==EDeviceLockMinChangeTolerance)
	        	{
	        	RDEBUG("EDeviceLockMinChangeTolerance", invalidNumber );
	        	title->setPlainText("EDeviceLockMinChangeTolerance");
	        	}
	        if(invalidNumber==EDeviceLockMinChangeInterval)
	        	{
	        	RDEBUG("EDeviceLockMinChangeInterval", invalidNumber );
	        	title->setPlainText("EDeviceLockMinChangeInterval");
	        	}
	        if(invalidNumber==EDeviceLockDisallowSpecificStrings)
	        	{
	        	RDEBUG("EDeviceLockDisallowSpecificStrings", invalidNumber );
	        	title->setPlainText("EDeviceLockDisallowSpecificStrings");
	        	}
	        if(invalidNumber==EDeviceLockAllowedMaxAtempts)
	        	{
	        	RDEBUG("EDeviceLockAllowedMaxAtempts", invalidNumber );
	        	title->setPlainText("EDeviceLockAllowedMaxAtempts");
	        	}
	        if(invalidNumber==EDeviceLockConsecutiveNumbers)
	        	{
	        	RDEBUG("EDeviceLockConsecutiveNumbers", invalidNumber );
	        	title->setPlainText("EDeviceLockConsecutiveNumbers");
	        	}
	        if(invalidNumber==EDeviceLockMinSpecialCharacters)
	        	{
	        	RDEBUG("EDeviceLockMinSpecialCharacters", invalidNumber );
	        	title->setPlainText("EDeviceLockMinSpecialCharacters");
	        	}
	        if(invalidNumber==EDeviceLockSingleCharRepeatNotAllowed)
	        	{
	        	RDEBUG("EDeviceLockSingleCharRepeatNotAllowed", invalidNumber );
	        	title->setPlainText("EDeviceLockSingleCharRepeatNotAllowed");
	        	}
	        if(invalidNumber==EDevicelockConsecutiveCharsNotAllowed)
	        	{
	        	RDEBUG("EDevicelockConsecutiveCharsNotAllowed", invalidNumber );
	        	title->setPlainText("EDevicelockConsecutiveCharsNotAllowed");
	        	}
	        if(invalidNumber>=EDevicelockTotalPolicies)
	        	{
	        	RDEBUG("EDevicelockTotalPolicies", invalidNumber );
	        	title->setPlainText("EDevicelockTotalPolicies");
	        	}
	        if(invalidNumber<0)	// everything is ok
	        	{
	        	okAction->setEnabled(true);	// TODO check this : invalid -> valid. This allows verif ?
	        	okAction->setText("Ok");
	        	codeBottom->setEnabled(true);
	        	}
	        else
	        	{
	        	okAction->setEnabled(false);
	        	codeBottom->setEnabled(false);
	        	codeBottom->setText("");
	        	okAction->setText("Ok");
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

    okAction = new HbAction(tr("Ok"));
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
    
    cancelAction = new HbAction(tr("Cancel"));    // qtTrId("txt_common_button_cancel")
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
    subscriberKSecurityUIsDismissDialog = new QValueSpaceSubscriber(
            "/KPSUidSecurityUIs/KSecurityUIsDismissDialog", this);
    connect(subscriberKSecurityUIsDismissDialog, SIGNAL(contentsChanged()), this,
            SLOT(subscriberKSecurityUIsDismissDialogChanged()));
		RDEBUG("subscriberKSecurityUIsDismissDialog", 1);
	
		RDEBUG("check cancel", 0);
    if ((queryType & ESecUiCancelSupported)==ESecUiCancelSupported)
    	{
    		// nothing to do. Cancel is enabled by default
    	}
  	else
  		{
				RDEBUG("disable Cancel", 1);
  			cancelAction->setEnabled(false);
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
    QVariant acceptedValue(accepted);
		RDEBUG("0", 0);
    mResultMap.insert(KResultAccepted, acceptedValue);
		RDEBUG("0", 0);
		qDebug() << mResultMap;
    emit deviceDialogData(mResultMap);
		RDEBUG("0", 0);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleAccepted()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleAccepted()
{
		RDEBUG("0", 0);
		// okAction
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
                //could also use  if(index.row()!=selectedItems.count()-1) codeTopText+= "|";
                codeTopText+= "|";
                }
         mResultMap.insert(KCodeTopIndex,  codeTopText);
    	}
    else
        codeTopText = codeTop->text();
    // TODO check last time for codeBottom
   	qDebug() << "codeTopText=" << codeTopText;
    sendResult(KErrNone);
    close();	// this is needed because Cancel doesn't automatically closes the dialog
		RDEBUG("close", 0);
		emit deviceDialogClosed();
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleCancelled()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleCancelled()
{
		RDEBUG("0", 0);
    sendResult(KErrCancel);
    close();	// this is needed because Cancel doesn't automatically closes the dialog
		RDEBUG("close", 0);
		emit deviceDialogClosed();
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleMemorySelectionChanged()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleMemorySelectionChanged(const QString &text)
    {
		RDEBUG("0", 0);
    	qDebug() << text;
    QVariant memorySelection(text);
    mResultMap.insert(KSelectedMemoryIndex, memorySelection);
    //TODO: do we need emit here, or would it be better to send all data at the end?
    //emit deviceDialogData(mResultMap);
    }

void SecUiNotificationDialog::handleCodeTopContentChanged()
    {
		RDEBUG("0", 0);
    	qDebug() << codeTop->text();
    	handleCodeTopChanged(codeTop->text());
    }

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleCodeTopChanged()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleCodeTopChanged(const QString &text)
    {
		RDEBUG("0", 0);
    	qDebug() << "SecUiNotificationDialog::handleCodeTopChanged=" << text ;
    	if(queryDual)
    		{
    		codeBottom->setText("");	// any change resets the verification.
    		}
    	if( queryType == 0x1000004 )	// new codeLock
    		{	// ChangeSecCodeParamsL change RMobilePhone::ESecurityCodePhonePassword
			    QVariant codeTopVar(text);
			    mResultMap.insert(KCodeTopIndex, codeTopVar);
					sendResult(KErrCompletion);	// send the current password back to the client for further TARM validation. This is done on any key-press, not in the OK
    		}
    	if(text.length() < lMinLength )
    		{
    		qDebug() << "SecUiNotificationDialog::handleCodeTopChanged too short:" << text ;
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
    		// TODO might use a flag to avoid re-setting
    		qDebug() << "SecUiNotificationDialog::handleCodeTopChanged long enough:" << text ;
    		okAction->setText("Ok");
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
    	qDebug() << "SecUiNotificationDialog::handleCodeBottomChanged" << text ;
    	qDebug() << "SecUiNotificationDialog::handleCodeBottomChanged. codeTop=" << codeTop->text() ;
    	// TODO compare 
    	if(text.length() < lMinLength )
    		{
    		qDebug() << "SecUiNotificationDialog::handleCodeBottomChanged too short:" << text ;
    		okAction->setEnabled(false);
    		}
    	else
    		{
    		// TODO might use a flag to avoid re-setting
    		qDebug() << "SecUiNotificationDialog::handleCodeBottomChanged long enough:" << text ;
    		if(codeTop->text()==text)
    			{
    			qDebug() << "SecUiNotificationDialog::handleCodeBottomChanged codes match:" << text ;
	    		okAction->setEnabled(true);
	    		}
	    	else
	    		{
    			qDebug() << "SecUiNotificationDialog::handleCodeBottomChanged codes not match:" << text ;
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
    	qDebug() << "codeTopText";
    	qDebug() << codeTopText;
    	codeTopText = codeTopText + "1" ;
    	qDebug() << "codeTopText+1";
    	qDebug() << codeTopText;
    	codeTop->setText(codeTopText);
    }
// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handlebut3Changed()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handlebut3Changed()
    {
    	qDebug() << "SecUiNotificationDialog::handlebut3Changed";
    	QString codeTopText = codeTop->text();
    	qDebug() << "codeTopText";
    	qDebug() << codeTopText;
    	codeTopText = codeTopText + "5" ;
    	qDebug() << "codeTopText+5";
    	codeTop->setEchoMode(HbLineEdit::PasswordEchoOnEdit);
    	qDebug() << codeTopText;
    	codeTop->setText(codeTopText);
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
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::subscriberKSecurityUIsDismissDialogChanged()
    {

    RDEBUG("0", 0);
    TInt aDismissDialog = ESecurityUIsDismissDialogUninitialized;
    TInt err = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsDismissDialog, aDismissDialog );
    RDEBUG("err", err);
	RDEBUG("aDismissDialog", aDismissDialog);
    if( aDismissDialog == ESecurityUIsDismissDialogOn )
    	{
		err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsDismissDialog, ESecurityUIsDismissDialogProcessing );
		RDEBUG("err", err);
		// TODO perhaps do this only if Cancel is allowed?
		RDEBUG("sendResult(KErrCancel)", KErrCancel);	// another option is KErrDied
		sendResult(KErrCancel);	// similar to     emit handleCancelled();
		RDEBUG("close", 0);
		close();
		RDEBUG("emit closeDeviceDialog", 0);
		emit deviceDialogClosed();
		// RDEBUG("emit closeDeviceDialog", 0);
		// this is old method    emit closeDeviceDialog(false);	// false means "not by client", although it's not really used
		RDEBUG("all emited", 0);
		err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsDismissDialog, ESecurityUIsDismissDialogDone );	// clear after using it
		RDEBUG("err", err);
    	}
	}