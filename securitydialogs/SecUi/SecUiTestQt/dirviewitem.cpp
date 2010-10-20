#include "dirviewitem.h"

#include <hbpushbutton.h>
#include <hbabstractitemview.h>
#include <hbtextitem.h>
#include <hbstyle.h>

const QString KExpandButtonItemName = "subitem-button";

DirViewItem::DirViewItem(QGraphicsItem* parent) : 
    HbTreeViewItem(parent),
    mExpandButton(0)
{
}

DirViewItem::~DirViewItem()
{
}

int DirViewItem::type() const
{
    return DirViewItem::Type;
}

HbAbstractViewItem *DirViewItem::createItem()
{
    return new DirViewItem(*this);
}

void DirViewItem::buttonReleased()
{
    if (isExpanded()) {
        setExpanded(false);
    } else {
        setExpanded(true);
    }
}

HbWidgetBase *DirViewItem::updateExpandItem()
{
    if (!mExpandButton) {
        mExpandButton = new HbPushButton(this);
        connect(mExpandButton, SIGNAL(released()), this, SLOT(buttonReleased()));
        HbStyle::setItemName(mExpandButton, KExpandButtonItemName);
    }

    if (isExpanded()) {
        mExpandButton->setText("Close");
    } else {
        mExpandButton->setText("Open");
    }

    return mExpandButton;
}

