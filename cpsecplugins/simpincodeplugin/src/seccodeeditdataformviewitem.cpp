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
#include <../../inc/cpsecplugins.h>

// ======== MEMBER FUNCTIONS ========

/*!
    Constructor
*/
SecCodeEditDataFormViewItem::SecCodeEditDataFormViewItem(
    QGraphicsItem *parent): HbDataFormViewItem(parent)
{
	RDEBUG("0", 0);
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
		RDEBUG("0", 0);
    return new SecCodeEditDataFormViewItem( *this);
}
   
/*!
    Create Item
*/
bool SecCodeEditDataFormViewItem::canSetModelIndex(
    const QModelIndex &index) const
{
    int type = index.data(HbDataFormModelItem::ItemTypeRole).toInt();
    RDEBUG("type", type);
    return type == SecCodeEditItem;
}

/*!
    Create Custom Widget
*/
HbWidget *SecCodeEditDataFormViewItem::createCustomWidget()
{
    int type = modelIndex().data(HbDataFormModelItem::ItemTypeRole).toInt();
    RDEBUG("type", type);

    if (type == SecCodeEditItem) {
        SecCodeEdit *edit = new SecCodeEdit("1234");
        edit->setEchoMode(HbLineEdit::Password);
        RDEBUG("edit->setReadOnly", 1);
        edit->setReadOnly(true);
        return edit;
    }
		RDEBUG("0", 0);
    return 0;
}
