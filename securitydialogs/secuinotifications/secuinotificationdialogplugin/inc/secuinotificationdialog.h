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
* Description: SecUi notification plugin dialog.
*
*/

#ifndef SECUINOTIFICATIONDIALOG_H
#define SECUINOTIFICATIONDIALOG_H

// #define RDEBUG( x, y ) RDebug::Printf( "%s %s (%u) %s=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, x, y );
#define RDEBUG( x, y )

#include <hbdialog.h>                   // HbDialog
#include <hbdevicedialoginterface.h>    // HbDeviceDialogInterface
#include <hbwidget.h>       // HbWidget
#include <hblineedit.h>       // HbWidget
#include <hblabel.h>

#include <qmobilityglobal.h>

QTM_BEGIN_NAMESPACE
    class QValueSpacePublisher;
    class QValueSpaceSubscriber;
QTM_END_NAMESPACE
QTM_USE_NAMESPACE

/**
 * SW Install notification widget class.
 */
class SecUiNotificationDialog : public HbDialog, public HbDeviceDialogInterface
{
    Q_OBJECT

public:     // constructor and destructor
    SecUiNotificationDialog(const QVariantMap &parameters);
    virtual ~SecUiNotificationDialog();

public:     // from HbDeviceDialogInterface
    bool setDeviceDialogParameters(const QVariantMap &parameters);
    int deviceDialogError() const;
    void closeDeviceDialog(bool byClient);
    HbDialog *deviceDialogWidget() const;

signals:
    void deviceDialogClosed();
    void deviceDialogData(const QVariantMap &data);

protected:  // from HbPopup (via HbDialog)
    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);

private:    // new functions
    bool constructDialog(const QVariantMap &parameters);
    void sendResult(int accepted);

private slots:
    void handleAccepted();
    void handleCancelled();
    void handleMemorySelectionChanged(const QString &text);
    void handleCodeTopChanged(const QString &text);
    void handleCodeBottomChanged(const QString &text);
    void handleCodeTopContentChanged();
		void saveFocusWidget(QWidget*,QWidget*);
		void handlebut1Changed();
		void handlebut2Changed();
		void handlebut3Changed();

public slots:    
    void subscriberKSecurityUIsDismissDialogChanged();
private:
    Q_DISABLE_COPY(SecUiNotificationDialog)

    int mLastError;
    bool mShowEventReceived;
    QVariantMap mResultMap;
    HbLineEdit *codeTop;
    HbLineEdit *codeBottom;
    HbAction *okAction;
    HbAction *cancelAction;
    HbLabel *title;
    int queryType;
    int lMinLength;
    int lMaxLength;
    int queryDual;
    int lEmergencySupported;
    int isEmergency;
    QValueSpaceSubscriber *subscriberKSecurityUIsDismissDialog;
};

#endif // SECUINOTIFICATIONDIALOG_H
