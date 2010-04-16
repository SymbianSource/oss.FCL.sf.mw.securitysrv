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
*
* Description: SecUi notification content widget.
*
*/

#include "secuinotificationcontentwidget.h"
#include "secuinotificationdialogpluginkeys.h"
#include <QGraphicsLinearLayout>
#include <hblabel.h>
#include <hbpushbutton.h>
#include <hbcombobox.h>
#include <hblineedit.h>
#include <hbinputeditorinterface.h>
#include <QDebug>


// ----------------------------------------------------------------------------
// SecUiNotificationContentWidget::SecUiNotificationContentWidget()
// ----------------------------------------------------------------------------
//
SecUiNotificationContentWidget::SecUiNotificationContentWidget(
        QGraphicsItem *parent, Qt::WindowFlags flags) : HbWidget(parent, flags)
{
		qDebug() << "SecUiNotificationContentWidget::SecUiNotificationContentWidget";
}

// ----------------------------------------------------------------------------
// SecUiNotificationContentWidget::~SecUiNotificationContentWidget()
// ----------------------------------------------------------------------------
//
SecUiNotificationContentWidget::~SecUiNotificationContentWidget()
{
}

// ----------------------------------------------------------------------------
// SecUiNotificationContentWidget::constructFromParameters()
// ----------------------------------------------------------------------------
//
void SecUiNotificationContentWidget::constructFromParameters(const QVariantMap &parameters)
{
		qDebug() << "SecUiNotificationContentWidget::constructFromParameters";
		qDebug() << parameters;
    QGraphicsLinearLayout *mainLayout = new QGraphicsLinearLayout(Qt::Vertical);

    // TODO: add another layout for icon + text_block, and yet other for text_block

    // KApplicationIcon
    if (1==0 && parameters.contains(KApplicationIcon)) {
				qDebug() << "SecUiNotificationContentWidget::KApplicationIcon";
        QString iconName = parameters.value(KApplicationIcon).toString();
        HbLabel *iconLabel = new HbLabel;
        iconLabel->setIcon(HbIcon(iconName));
        mainLayout->addItem(iconLabel);
    }

    // KApplicationName + KApplicationVersion
    if (1==0 && parameters.contains(KApplicationName)) {
				qDebug() << "SecUiNotificationContentWidget::KApplicationName";
        QString appName = "";
        QString nameStr = parameters.value(KApplicationName).toString();
        appName = nameStr;
        HbLabel *appLabel = new HbLabel(appName);
        mainLayout->addItem(appLabel);
    }

    // KApplicationSize
    if (parameters.contains(KQueryType)) {
				qDebug() << "SecUiNotificationContentWidget::KQueryType";
        queryType = parameters.value(KQueryType).toUInt();
				qDebug() << queryType;
    }

    // KCodeTop
    if (parameters.contains(KCodeTop)) {
				qDebug() << "SecUiNotificationContentWidget::KCodeTop 1";
        codeTop = new HbLineEdit("");	// no default value
        // HbLineEdit *codeTop2 = new HbLineEdit;
				qDebug() << "SecUiNotificationContentWidget::KCodeTop 2";
    		HbEditorInterface editorInterface(codeTop);
    		editorInterface.setUpAsPhoneNumberEditor();
				qDebug() << "SecUiNotificationContentWidget::KCodeTop 3";
        connect(codeTop, SIGNAL(textChanged(const QString &)), this, SIGNAL(codeTopChanged(const QString &)));
    		mainLayout->addItem(codeTop);
        // mainLayout->addItem(codeTop2);

    		QGraphicsLinearLayout *mainLayoutButtons = new QGraphicsLinearLayout(Qt::Horizontal);
        HbPushButton *but1 = new HbPushButton("1234");
        HbPushButton *but2 = new HbPushButton("+1");
        HbPushButton *but3 = new HbPushButton("+5");
        connect(but1, SIGNAL(clicked()), this, SIGNAL(but1Changed()));
        connect(but2, SIGNAL(clicked()), this, SIGNAL(but2Changed()));
        connect(but3, SIGNAL(clicked()), this, SIGNAL(but3Changed()));
        mainLayoutButtons->addItem(but1);
        mainLayoutButtons->addItem(but2);
        mainLayoutButtons->addItem(but3);

        mainLayout->addItem(mainLayoutButtons);
        
        codeTop->setFocus();

    }

    // KCertificates
    // KDrmDetails

    setLayout(mainLayout);
    }

