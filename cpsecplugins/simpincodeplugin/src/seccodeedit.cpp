/*
 * Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0""
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

// System includes
#include <QGraphicsSceneMouseEvent>

// User includes
#include "seccodeedit.h"

// ======== MEMBER FUNCTIONS ========

/*!
    Constructor
*/
SecCodeEdit::SecCodeEdit(const QString &text, QGraphicsItem *parent/*= 0*/)
    : HbLineEdit(text, parent)
{
}

/*!
    Destructor
*/
SecCodeEdit::~SecCodeEdit()
{
}

/*!
    Mouse Press Event
*/
void SecCodeEdit::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    if (rect().contains(event->pos())) {
        emit clicked();
        event->accept();
    } else {
        event->ignore();
    }
}
