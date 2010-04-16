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

#define ESecUiCancelSupported  0x1000000
#define ESecUiCancelNotSupported  0x0000000

#define ESecUiEmergencySupported  0x2000000
#define ESecUiEmergencyNotSupported  0x0000000

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
        HbLabel *title = new HbLabel(titleText);
        setHeadingWidget(title);
    }

    // Content
    SecUiNotificationContentWidget *content = new SecUiNotificationContentWidget();
    content->constructFromParameters(parameters);
    setContentWidget(content);
    connect(content, SIGNAL(codeTopChanged(const QString &)), this, SLOT(handleCodeTopChanged(const QString &)));
    connect(content, SIGNAL(but1Changed()), this, SLOT(handlebut1Changed()));
    connect(content, SIGNAL(but2Changed()), this, SLOT(handlebut2Changed()));
    connect(content, SIGNAL(but3Changed()), this, SLOT(handlebut3Changed()));
		codeTop = content->codeTop;
		queryType = content->queryType;
		qDebug() << "SecUiNotificationDialog::queryType=";
		qDebug() << queryType;
    // Buttons
    okAction = new HbAction(tr("Ok"));            // qtTrId("txt_common_button_ok")
    setPrimaryAction(okAction);
    connect(okAction, SIGNAL(triggered()), this, SLOT(handleAccepted()));
    cancelAction = new HbAction(tr("Cancel"));    // qtTrId("txt_common_button_cancel")
    connect(cancelAction, SIGNAL(triggered()), this, SLOT(handleCancelled()));
    setSecondaryAction(cancelAction);

		qDebug() << "SecUiNotificationDialog check Cancel";
    if (queryType & ESecUiCancelSupported)
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
void SecUiNotificationDialog::sendResult(bool accepted)
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
		QString codeTopText = codeTop->text();
   	qDebug() << "codeTopText";
   	qDebug() << codeTopText;
		if(!codeTopText.compare("1234111"))
				{
					qDebug() << "codeTopText is 1234111. Not exit";
					return;
				}
    sendResult(true);
}

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleCancelled()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleCancelled()
{
		qDebug() << "SecUiNotificationDialog::handleCancelled";
    sendResult(false);
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

// ----------------------------------------------------------------------------
// SecUiNotificationDialog::handleCodeTopChanged()
// ----------------------------------------------------------------------------
//
void SecUiNotificationDialog::handleCodeTopChanged(const QString &text)
    {
    	qDebug() << "SecUiNotificationDialog::handleCodeTopChanged";
    	qDebug() << "SecUiNotificationDialog::handleCodeTopChanged" << text ;
    QVariant codeTop(text);
    mResultMap.insert(KCodeTopIndex, codeTop);
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
