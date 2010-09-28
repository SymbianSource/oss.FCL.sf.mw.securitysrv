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


#ifndef CPREMOTELOCKDATAFORMVIEWITEM_H
#define CPREMOTELOCKDATAFORMVIEWITEM_H

//#include <seccodeuiglobal.h>
#include <hbdataformviewitem.h>
#include <hbdataformmodelitem.h>
#include <hblineedit.h>

class CpRemoteLockDataFormViewItem : public HbDataFormViewItem
{
	Q_OBJECT
public:
	enum { CpRemoteLockItem = HbDataFormModelItem::CustomItemBase + 10,
           CpCodeEditItem = CpRemoteLockItem + 1 };

	/* Constructor */
    explicit CpRemoteLockDataFormViewItem(QGraphicsItem *parent = 0);
    /* Destructor */
    virtual ~CpRemoteLockDataFormViewItem();
    /* creates a view item */
    virtual HbAbstractViewItem* createItem();
    /**/
	virtual bool canSetModelIndex(const QModelIndex &index) const;
protected:
	/* creates a custom widget */
    virtual HbWidget* createCustomWidget();
};


class CpLockEdit : public HbLineEdit
{
    Q_OBJECT
public:
    /* constructor */
    explicit CpLockEdit(const QString &text, QGraphicsItem *parent = 0);
    /* destructor */
    virtual ~CpLockEdit();
    /* handles mouse events*/
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
protected:
    /*
           Reimplemented from HbWidgetBase. Called by the framework when 
           a gesture event has been received.
           @param event The received event
       */
       void gestureEvent(QGestureEvent *event);
signals:
    /* signal when edit box is clicked*/
    void clicked();
};

#endif //CPREMOTELOCKDATAFORMVIEWITEM_H

