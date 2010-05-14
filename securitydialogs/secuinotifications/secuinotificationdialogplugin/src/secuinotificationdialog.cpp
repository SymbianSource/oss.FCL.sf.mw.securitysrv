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
#include <CPhCltEmergencyCall.h>
#include <SCPServerInterface.h>	// for TARM error codes while validating new lock code
#include <QString>
#include <QDialogButtonBox>

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

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::SecUiNotificationDialog()
// ----------------------------------------------------------------------------
//
SecUiNotificationDialog::SecUiNotificationDialog(
        const QVariantMap &parameters) : HbDialog(), mLastError(KNoError)
{
		qDebug() << "SecUiNotificationDialog::SecUiNotificationDialog";
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
		qDebug() << "SecUiNotificationDialog::setDeviceDialogParameters";
   return constructDialog(parameters);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::deviceDialogError()
// ----------------------------------------------------------------------------
//
int SecUiNotificationDialog::deviceDialogError() const
{
		qDebug() << "SecUiNotificationDialog::deviceDialogError";
		qDebug() << mLastError;
    return mLastError;
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::closeDeviceDialog
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::closeDeviceDialog(bool byClient)
{
    Q_UNUSED(byClient);
    close();
		qDebug() << "SecUiNotificationDialog::closeDeviceDialog";

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
		qDebug() << "SecUiNotificationDialog::hideEvent";
    HbDialog::hideEvent(event);
    emit deviceDialogClosed();
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::showEvent
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::showEvent(QShowEvent *event)
{
		qDebug() << "SecUiNotificationDialog::showEvent";
    HbDialog::showEvent(event);
    mShowEventReceived = true;
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::constructDialog()
// ----------------------------------------------------------------------------
//
bool SecUiNotificationDialog::constructDialog(const QVariantMap &parameters)
    {
		qDebug() << "SecUiNotificationDialog::constructDialog";
    setTimeout(HbPopup::NoTimeout);
    setDismissPolicy(HbPopup::NoDismiss);
    setModal(true);

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
    				qDebug() << "SecUiNotificationDialog::nAttempts=" << nAttempts;
    				titleText = titleText.left(titleText.indexOf('#'));
    				if(nAttempts>0)
    					titleText = titleText + " attempts=" + QString::number(nAttempts);
    				}
        title = new HbLabel(titleText);
        setHeadingWidget(title);
    }

	    if (parameters.contains(KEmergency)) {
					qDebug() << "SecUiNotificationDialog::KEmergency";
	        QString emergencyText = parameters.value(KEmergency).toString();
	        qDebug() << emergencyText;
	        if(!emergencyText.compare("emergencyYes"))
	        	{
	        	qDebug() << "SecUiNotificationDialog::KEmergency emergencyYes";
	        	isEmergency = 1;
	        	okAction->setEnabled(true);
	        	okAction->setText("Call");
    				return true;
	        	}
	        if(!emergencyText.compare("emergencyNo"))
	        	{
	        	qDebug() << "SecUiNotificationDialog::KEmergency emergencyNo";
	        	isEmergency = 0;
	        	okAction->setEnabled(false);	// 112 -> 1122 (=password) . This is handled by   < lMinLength 
	        	okAction->setText("Ok");
    				return true;
	        	}
	    }
	    
	    if (parameters.contains(KInvalidNewLockCode)) {
					qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode";
	        QString invalidText = parameters.value(KInvalidNewLockCode).toString();
	        qDebug() << invalidText;

	        title->setPlainText("Lock Code");	// TODO take from the original one
	        QString invalidStr = invalidText.right(invalidText.length()-invalidText.indexOf('#')-1);
	        int invalidNumber = invalidStr.toInt();
	        qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode invalidNumber" << invalidNumber;
	        if(invalidNumber<0)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode ???";
	        	// nothing to do
	        	}
/*
	        if(invalidNumber==EDeviceLockAutolockperiod)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockAutolockperiod";
	        	title->setPlainText("EDeviceLockAutolockperiod");
	        	}
	        if(invalidNumber==EDeviceLockMaxAutolockPeriod)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockMaxAutolockPeriod";
	        	title->setPlainText("EDeviceLockMaxAutolockPeriod");
	        	}
	        if(invalidNumber==EDeviceLockMinlength)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockMinlength";
	        	title->setPlainText("EDeviceLockMinlength");
	        	}
	        if(invalidNumber==EDeviceLockMaxlength)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockMaxlength";
	        	title->setPlainText("EDeviceLockMaxlength");
	        	}
	        if(invalidNumber==EDeviceLockRequireUpperAndLower)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockRequireUpperAndLower";
	        	title->setPlainText("EDeviceLockRequireUpperAndLower");
	        	}
	        if(invalidNumber==EDeviceLockRequireCharsAndNumbers)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockRequireCharsAndNumbers";
	        	title->setPlainText("EDeviceLockMaxlength");
	        	}
	        if(invalidNumber==EDeviceLockAllowedMaxRepeatedChars)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockAllowedMaxRepeatedChars";
	        	title->setPlainText("EDeviceLockAllowedMaxRepeatedChars");
	        	}
	        if(invalidNumber==EDeviceLockHistoryBuffer)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockHistoryBuffer";
	        	title->setPlainText("EDeviceLockHistoryBuffer");
	        	}
	        if(invalidNumber==EDeviceLockPasscodeExpiration)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockPasscodeExpiration";
	        	title->setPlainText("EDeviceLockPasscodeExpiration");
	        	}
	        if(invalidNumber==EDeviceLockMinChangeTolerance)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockMinChangeTolerance";
	        	title->setPlainText("EDeviceLockMinChangeTolerance");
	        	}
	        if(invalidNumber==EDeviceLockMinChangeInterval)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockMinChangeInterval";
	        	title->setPlainText("EDeviceLockMinChangeInterval");
	        	}
	        if(invalidNumber==EDeviceLockDisallowSpecificStrings)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockDisallowSpecificStrings";
	        	title->setPlainText("EDeviceLockDisallowSpecificStrings");
	        	}
	        if(invalidNumber==EDeviceLockAllowedMaxAtempts)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockAllowedMaxAtempts";
	        	title->setPlainText("EDeviceLockAllowedMaxAtempts");
	        	}
	        if(invalidNumber==EDeviceLockConsecutiveNumbers)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockConsecutiveNumbers";
	        	title->setPlainText("EDeviceLockConsecutiveNumbers");
	        	}
	        if(invalidNumber==EDeviceLockMinSpecialCharacters)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockMinSpecialCharacters";
	        	title->setPlainText("EDeviceLockMinSpecialCharacters");
	        	}
	        if(invalidNumber==EDeviceLockSingleCharRepeatNotAllowed)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDeviceLockSingleCharRepeatNotAllowed";
	        	title->setPlainText("EDeviceLockSingleCharRepeatNotAllowed");
	        	}
	        if(invalidNumber==EDevicelockConsecutiveCharsNotAllowed)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDevicelockConsecutiveCharsNotAllowed";
	        	title->setPlainText("EDevicelockConsecutiveCharsNotAllowed");
	        	}
	        if(invalidNumber>=EDevicelockTotalPolicies)
	        	{
	        	qDebug() << "SecUiNotificationDialog::KInvalidNewLockCode EDevicelockTotalPolicies";
	        	title->setPlainText("EDevicelockTotalPolicies");
	        	}
	        	*/
	        // always keep OK valid.
   				return true;
	    }
	
    // Content
    SecUiNotificationContentWidget *content = new SecUiNotificationContentWidget();
    content->constructFromParameters(parameters);
    setContentWidget(content);

		queryType = content->queryType;
		queryDual = content->queryDual;
		isEmergency = content->isEmergency;
		codeTop = content->codeTop;
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
		qDebug() << "SecUiNotificationDialog::queryType=";
		qDebug() << queryType;
    // Buttons
    if( (queryType & ESecUiTypeMaskLock))
    	{
    	// no need to create OK or Cancel
    	return true;
    	}

    okAction = new HbAction(tr("Ok"));
    okAction->setEnabled(false);	// initially the OK is disabled because codeTop is empty
    // setAction(okAction, QDialogButtonBox::AcceptRole);	// it's supposed to use this, when deprecated
    setPrimaryAction(okAction);
    connect(okAction, SIGNAL(triggered()), this, SLOT(handleAccepted()));
    
    cancelAction = new HbAction(tr("Cancel"));    // qtTrId("txt_common_button_cancel")
    connect(cancelAction, SIGNAL(triggered()), this, SLOT(handleCancelled()));
    // setAction(cancelAction, QDialogButtonBox::RejectRole);		// it's supposed to use this, when deprecated
    setSecondaryAction(cancelAction);

		qDebug() << "SecUiNotificationDialog check Cancel";
    if ((queryType & ESecUiCancelSupported)==ESecUiCancelSupported)
    	{
    		// nothing to do. Cancel is enabled by default
    	}
  	else
  		{
				qDebug() << "disable Cancel";
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
		qDebug() << "SecUiNotificationDialog::sendResult 1.2";
    QVariant acceptedValue(accepted);
		qDebug() << "SecUiNotificationDialog::sendResult 2";
    mResultMap.insert(KResultAccepted, acceptedValue);
		qDebug() << "SecUiNotificationDialog::sendResult 3";
		qDebug() << mResultMap;
    emit deviceDialogData(mResultMap);
		qDebug() << "SecUiNotificationDialog::sendResult end";
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleAccepted()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleAccepted()
{
		qDebug() << "SecUiNotificationDialog::handleAccepted";
		// okAction
		QString codeTopText;

		if( (queryType & ESecUiTypeMaskLock))
    	{
    	codeTopText = "Unlock-Request";
    	}
    else
    	{
    	codeTopText = codeTop->text();
    	}
    // TODO check last time for codeBottom
   	qDebug() << "codeTopText=" << codeTopText;
    sendResult(KErrNone);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleCancelled()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleCancelled()
{
		qDebug() << "SecUiNotificationDialog::handleCancelled";
    sendResult(KErrCancel);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleMemorySelectionChanged()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleMemorySelectionChanged(const QString &text)
    {
    	qDebug() << "SecUiNotificationDialog::handleMemorySelectionChanged";
    	qDebug() << text;
    QVariant memorySelection(text);
    mResultMap.insert(KSelectedMemoryIndex, memorySelection);
    //TODO: do we need emit here, or would it be better to send all data at the end?
    //emit deviceDialogData(mResultMap);
    }

void SecUiNotificationDialog::handleCodeTopContentChanged()
    {
    	qDebug() << "SecUiNotificationDialog::handleCodeTopContentChanged";
    	qDebug() << codeTop->text();
    	handleCodeTopChanged(codeTop->text());
    }

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleCodeTopChanged()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleCodeTopChanged(const QString &text)
    {
    	qDebug() << "SecUiNotificationDialog::handleCodeTopChanged";
    	qDebug() << "SecUiNotificationDialog::handleCodeTopChanged=" << text ;
    	if(queryDual)
    		{
    		codeBottom->setText("");	// any change resets the verification.
    		}
    	if( queryType == 0x1000004 )
    		{	// ChangeSecCodeParamsL change RMobilePhone::ESecurityCodePhonePassword
			    QVariant codeTop(text);
			    mResultMap.insert(KCodeTopIndex, codeTop);
					sendResult(KErrCompletion);	// send the current password back to the client for further TARM validation
    		}
    	if(text.length() < lMinLength )
    		{
    		qDebug() << "SecUiNotificationDialog::handleCodeTopChanged too short:" << text ;
    		okAction->setEnabled(false);

				if( lEmergencySupported && text.length() > 2 )	// emergency numbers need at least 3 digits
					{	// check whether it's a emergency number
					QVariant codeTop(text);
  				mResultMap.insert(KCodeTopIndex, codeTop);
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
    QVariant codeTop(text);
    mResultMap.insert(KCodeTopIndex, codeTop);
    }
// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleCodeBottomChanged()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleCodeBottomChanged(const QString &text)
    {
    	qDebug() << "SecUiNotificationDialog::handleCodeBottomChanged";
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
    	qDebug() << "SecUiNotificationDialog::handlebut1Changed";
    	codeTop->setText("1234");
    }
// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handlebut2Changed()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handlebut2Changed()
    {
    	qDebug() << "SecUiNotificationDialog::handlebut2Changed";
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
		qDebug() << "SecUiNotificationDialog::saveFocusWidget";
}
