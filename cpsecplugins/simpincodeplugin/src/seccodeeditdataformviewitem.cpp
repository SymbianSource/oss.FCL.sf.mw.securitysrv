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

// User includes
#include "seccodeeditdataformviewitem.h"
#include "seccodeedit.h"

// ======== MEMBER FUNCTIONS ========

/*!
    Constructor
*/
SecCodeEditDataFormViewItem::SecCodeEditDataFormViewItem(
    QGraphicsItem *parent): HbDataFormViewItem(parent)
{
}

/*!
    Destructor
*/
SecCodeEditDataFormViewItem::~SecCodeEditDataFormViewItem()
{
}

/*!
    Create Item
*/
HbAbstractViewItem *SecCodeEditDataFormViewItem::createItem()
{
    return new SecCodeEditDataFormViewItem( *this);
}
   
/*!
    Create Item
*/
bool SecCodeEditDataFormViewItem::canSetModelIndex(
    const QModelIndex &index) const
{
    int type = index.data(HbDataFormModelItem::ItemTypeRole).toInt();
    return type == SecCodeEditItem;
}

/*!
    Create Custom Widget
*/
HbWidget *SecCodeEditDataFormViewItem::createCustomWidget()
{
    int type = modelIndex().data(HbDataFormModelItem::ItemTypeRole).toInt();

    if (type == SecCodeEditItem) {
        SecCodeEdit *edit = new SecCodeEdit("1234");
        edit->setEchoMode(HbLineEdit::Password);
        edit->setReadOnly(true);
        return edit;
    }

    return 0;
}
