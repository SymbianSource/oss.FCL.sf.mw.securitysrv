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
 * Description:
 *
 */

#ifndef SECCODEEDIT_H
#define SECCODEEDIT_H

// System includes
#include <hblineedit.h>

// Class declaration
class SecCodeEdit : public HbLineEdit
{
    Q_OBJECT

public:
    explicit SecCodeEdit(const QString &text, QGraphicsItem *parent = 0);
    virtual ~SecCodeEdit();
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

signals:
    void clicked();
};

#endif //SECCODEEDIT_H
