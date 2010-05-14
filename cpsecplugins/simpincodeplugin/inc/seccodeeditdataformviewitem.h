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

#ifndef SECCODEEDITDATAFORMVIEWITEM_H
#define SECCODEEDITDATAFORMVIEWITEM_H

// System includes
#include <hbdataformviewitem.h>
#include <hbdataformmodelitem.h>

// User includes
#include "seccodeuiglobal.h"

// Class declaration
class SecCodeEditDataFormViewItem : public HbDataFormViewItem
{
    Q_OBJECT

public:
    enum { SecCodeEditItem = HbDataFormModelItem::CustomItemBase + 1 };
    explicit SecCodeEditDataFormViewItem(QGraphicsItem *parent = 0);
    virtual ~SecCodeEditDataFormViewItem();
    virtual HbAbstractViewItem* createItem();
    virtual bool canSetModelIndex(const QModelIndex &index) const;

protected:
    virtual HbWidget* createCustomWidget();
};

#endif //SECCODEEDITDATAFORMVIEWITEM_H
