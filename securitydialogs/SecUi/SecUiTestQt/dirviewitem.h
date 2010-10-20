#ifndef DIRVIEWITEM_H
#define DIRVIEWITEM_H

#include <hbtreeviewitem.h>
#include <hbpushbutton.h>

#include <QPointer>

class DirViewItem : public HbTreeViewItem
{
    Q_OBJECT

public:

    explicit DirViewItem(QGraphicsItem *parent = 0);
    virtual ~DirViewItem();

    enum { Type = Hb::ItemType_Last + 10 };
    virtual int type() const;
    
    virtual HbAbstractViewItem* createItem();

protected slots:

    void buttonReleased();

protected:

    virtual HbWidgetBase *updateExpandItem();

private:

     QPointer<HbPushButton> mExpandButton;
};

#endif // DIRVIEWITEM_H
