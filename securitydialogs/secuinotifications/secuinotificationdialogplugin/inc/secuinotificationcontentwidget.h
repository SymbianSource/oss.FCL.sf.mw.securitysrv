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
* Description: SecUi notification content widget.
*
*/

#ifndef SECUINOTIFICATIONCONTENTWIDGET_H
#define SECUINOTIFICATIONCONTENTWIDGET_H

#include <hbwidget.h>       // HbWidget
#include <hblineedit.h>       // HbWidget

class HbLabel;


class SecUiNotificationContentWidget : public HbWidget
{
    Q_OBJECT

public:
    SecUiNotificationContentWidget(QGraphicsItem *parent=0, Qt::WindowFlags flags=0);
    virtual ~SecUiNotificationContentWidget();

    void constructFromParameters(const QVariantMap &parameters);

signals:
    void memorySelectionChanged(const QString &text);
    void codeTopChanged(const QString &text);
    void but1Changed();
    void but2Changed();
    void but3Changed();

private:
    Q_DISABLE_COPY(SecUiNotificationContentWidget)

private:    // data
public:
		HbLineEdit *codeTop;
    int queryType;
};

#endif // SECUINOTIFICATIONCONTENTWIDGET_H
