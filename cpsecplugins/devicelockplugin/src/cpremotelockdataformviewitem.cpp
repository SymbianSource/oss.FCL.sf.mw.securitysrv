/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "cpremotelockdataformviewitem.h"
#include <hblabel.h>
#include <QGraphicsSceneMouseEvent>



/*
 *****************************************************************
 * Name        : CpRemoteLockDataFormViewItem
 * Parameters  : QGraphicsItem*
 * Return value: None
 * Description : constructor
 *****************************************************************
 */
CpRemoteLockDataFormViewItem::CpRemoteLockDataFormViewItem(QGraphicsItem *parent )
: HbDataFormViewItem(parent)
{
}


/*
 *****************************************************************
 * Name        : ~CpRemoteLockDataFormViewItem
 * Parameters  : None
 * Return value: None
 * Description : destructor
 *****************************************************************
 */
CpRemoteLockDataFormViewItem::~CpRemoteLockDataFormViewItem()
{
}


/*
 *****************************************************************
 * Name        : createItem
 * Parameters  : None
 * Return value: HbAbstractViewItem*
 * Description : creates a HbAbstractViewItem
 *****************************************************************
 */
HbAbstractViewItem* CpRemoteLockDataFormViewItem::createItem()
{
	return new CpRemoteLockDataFormViewItem(*this);
}


/*
 *****************************************************************
 * Name        : canSetModelIndex
 * Parameters  : QModelIndex&
 * Return value: bool
 * Description : 
 *****************************************************************
 */
bool CpRemoteLockDataFormViewItem::canSetModelIndex(const QModelIndex &index) const
{
	int type = index.data(HbDataFormModelItem::ItemTypeRole).toInt();
	return ((type == CpRemoteLockItem) || (type == CpCodeEditItem));
}


/*
 *****************************************************************
 * Name        : createCustomWidget
 * Parameters  : None
 * Return value: HbWidget
 * Description : creates a custom widget
 *****************************************************************
 */
HbWidget *CpRemoteLockDataFormViewItem::createCustomWidget()
    {
    int type = modelIndex().data(HbDataFormModelItem::ItemTypeRole).toInt();
    if (type == CpCodeEditItem)
        {
        CpLockEdit *edit = new CpLockEdit("1234");
        edit->setEchoMode(HbLineEdit::Password);
        edit->setReadOnly(true);
        return edit;
        }
    else
        {
        return 0;
        }
}


/*
 *****************************************************************
 * Name        : CpLockEdit
 * Parameters  : QString&, QGraphicsitem
 * Return value: None
 * Description : constructor
 *****************************************************************
 */
CpLockEdit::CpLockEdit(const QString &text, QGraphicsItem *parent /*= 0*/)
: HbLineEdit(text,parent)
{
}


/*
 *****************************************************************
 * Name        : CpLockEdit
 * Parameters  : None
 * Return value: None
 * Description : destructor
 *****************************************************************
 */
CpLockEdit::~CpLockEdit()
    {
    
    }


/*
 *****************************************************************
 * Name        : mousePressEvent
 * Parameters  : QGraphicsSceneMouseEvent*
 * Return value: None
 * Description : handles mouse events
 *****************************************************************
 */
void CpLockEdit::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        event->ignore();
        return;
    }

    if (rect().contains(event->pos())) {
        emit clicked();
        event->accept();
    }
    else {
        event->ignore();
    } 
}
