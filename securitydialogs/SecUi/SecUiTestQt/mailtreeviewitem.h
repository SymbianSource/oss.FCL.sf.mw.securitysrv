#ifndef MAILVIEWITEM_H
#define MAILVIEWITEM_H

#include <hbtreeviewitem.h>
#include <hbabstractviewitem.h>

class HbLabel;
class HbFrameItem;
class HbAnchorLayout;
class HbWidgetBase;

#include <QPointer>
#include <QStyleOptionGraphicsItem>

class MailTreeViewItem : public HbTreeViewItem
{
    Q_OBJECT

public:

    explicit MailTreeViewItem(QGraphicsItem *parent = 0);
    virtual ~MailTreeViewItem();

    enum { Type = Hb::ItemType_Last + 11 };
    virtual int type() const;
    
    void updateChildItems();

    HbAbstractViewItem *createItem();
    bool canSetModelIndex(const QModelIndex &index) const;

    HbWidgetBase *updateExpandItem();

protected:

    void polishEvent();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    HbLabel *mSender;                   // Not owned
    HbLabel *mSubject;                  // Not owned
    HbLabel *mTime;                     // Not owned
    HbLabel *mDividerTitle;             // Not owned
    //HbLabel *mDividerIcon;              // Not owned
    HbFrameItem *mNewMsgIcon;            //
    HbLabel *mFrom;
    HbAnchorLayout* mLayout;            // Not owned
};

#endif // MAILVIEWITEM_H
