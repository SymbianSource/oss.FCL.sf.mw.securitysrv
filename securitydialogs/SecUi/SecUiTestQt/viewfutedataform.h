/*
* ====================================================
*  Name        : viewfutedataform.h
*  Part of     : fute/listdemo
*  Description : Provides DataForm for general settings 
*                or add item data form of an itemview application. 
*                One instance of this class even supports 
*                populating both forms if operations are done sequentially one after another. 
*                In this case application should get handle to HbDataFormModel after populating data form
*                (as HbDataFormModel is cleared only in populate* function).
*              
*  Version     : %version: 1 %
*
*  Copyright (c) 2009 Nokia.  All rights reserved.
*  This material, including documentation and any related computer
*  programs, is protected by copyright controlled by Nokia.  All
*  rights are reserved.  Copying, including reproducing, storing,
*  adapting or translating, any or all of this material requires the
*  prior written consent of Nokia.  This material also contains
*  confidential information which may not be disclosed to others
*  without the prior written consent of Nokia.
* ====================================================
*/

#ifndef VIEWFUTEDATAFORM_H
#define VIEWFUTEDATAFORM_H

#include <QObject>
#include <hbdataform.h>

#include <hbabstractitemview.h>
#include <hbscrollarea.h>
#include <hbdataformviewitem.h>
#include <hblistviewitem.h>

class HbDataFormModel;
class HbListWidgetItem;
class HbIndexFeedback;
class QStandardItem;

class ViewFuteDataForm : public HbDataForm
{
    Q_OBJECT

public:

    enum Activity {
        None,
        Settings,
        AddItem
    };

    enum SettingsIndex {
        LaunchInPopup,
        Frame,
        Recycling,
        UniformItem,
        RichText,
        SelectionMode,
        ArrangeMode,
        ScrollingStyle,
        ClampingStyle,
        FrictionEnabled,
        Orientation,
        Mirroring,
        ScrollHint,
        GraphicsSize,
        StretchingStyle,
        MaximumRowCount,
        MinimumRowCount,
        InteractiveScrollBar,
        IndexFeedbackPolicy,
        Animations,
        PixmapCache,
        IconLoadPolicy,
        SettingsIndexLast
    };

    enum AddItemIndex {
        PrimaryText,
        SecondaryText,
        ThirdText,
        LeftColumn,
        RightColumn,
        Type,
        Background,
        ItemCount,
        AddItemIndexLast
    };


    explicit ViewFuteDataForm(HbAbstractItemView &view,
                              QGraphicsItem *parent = 0);
    virtual ~ViewFuteDataForm();

    HbDataFormModel *dataModel() const;

    // Returns next free index
    int nextIndex() const;

    // resets internal state of this object including model
    virtual void initialise();  // reset() is reserved by base class

    Activity action() const;

    // S E T T I N G S
    virtual void populateSettings();

    // Setting default input data. item: enum value of SettingsIndex or AddItemIndex, if not custom item
    virtual void setInputData(  Activity    action,
                                int         item, 
                                QVariant    data);

    void populateLaunchInPopup();
    void populateStretchingStyle();
    void populateGraphicsSize();
    void populateMaximumRowCount();
    void populateMinimumRowCount();
    void populateFrame();
    void populateRecycling();
    void populateUniformItem();
    void populateRichText();
    virtual void populateSelectionMode();
    void populateArrangeMode();
    void populateScrollingStyle();
    void populateClampingStyle();
    void populateFrictionEnabled();
    // needs Qt::Orientation as input data. Data type int. Default value Qt:Portrait
    void populateOrientation();
    // needs Qt::LayoutDirection as input data. Data type int. Default value Qt:LeftToRight
    void populateMirroring();
    // needs HbAbstractItemView::ScrollHint as input data. Data type int. 
    //  Default value HbAbstractItemView::EnsureVisible
    void populateScrollHint();
    void populateInteractiveScrollBar();
    void populateIndexFeedbackPolicy();
    void populateEnabledAnimations();
    void populatePixmapCacheEnabled();
    void populateIconLoadPolicy();

    // S E T T I N G S  results
    // Pushes results to prototype and view
    virtual void resolveSettingsResults();

    bool getLaunchInPopup();
    // true: default layout
    HbListViewItem::GraphicsSize getGraphicsSize();
    HbListViewItem::StretchingStyle getStretchingStyle();
    int getMaximumRowCount();
    int getMinimumRowCount();

    bool getFrame();
    bool getRecycling();
    bool getUniformItem();
    Qt::TextFormat getRichText();
    virtual HbAbstractItemView::SelectionMode getSelectionMode();
    bool getArrangeMode();
    HbScrollArea::ScrollingStyle getScrollingStyle();
    HbScrollArea::ClampingStyle getClampingStyle();
    bool getFrictionEnabled();
    Qt::Orientation getOrientation();
    Qt::LayoutDirection getMirroring();
    HbAbstractItemView::ScrollHint getScrollHint();
    bool getInteractiveScrollBar();
    int getIndexFeedbackPolicy();
    HbAbstractItemView::ItemAnimations getEnabledAnimations();
    bool getPixmapCacheEnaled();
    HbAbstractItemView::IconLoadPolicy getIconLoadPolicy();

    // A D D  I T E M
    virtual void populateAddItem();

    void populatePrimaryText();
    void populateSecondaryText();
    void populateThirdText();
    void populateLeftColumn();
    void populateRightColumn();
    void populateType();
    void populateBackground();

    // needs input data as int: default 1
    void populateAddItemCount();

    // A D D  I T E M  results: new item created. 
    // Returns 0, if any data not set
    virtual QList <HbListWidgetItem *> getListWidgetItems();
    // or 
    virtual QList <QStandardItem *> getStandardItems();

    QString getPrimaryText();
    QString getSecondaryText();
    QString getThirdText();
    QVariant getLeftColumn();
    QVariant getRightColumn();
    Hb::ModelItemType getType();
    QVariant getBackground();
    int getAddItemCount();

    void setIndexFeedback(HbIndexFeedback* indexFeedback);

private slots:
    void formatTriggered();
    void addFormating();

protected:
    // Return count of items added. previousItem: 1 or SettingsIndex
    virtual int populateCustomSettingsItem(int previousItem);

    Activity            activity;
    HbDataFormModel     *settingsFormModel;
    HbAbstractItemView  *view;
    HbListWidgetItem    *item;
    int                 counter;
    int                 settingsIndexes[SettingsIndexLast];
    int                 addItemIndexes[AddItemIndexLast];
    QVariant            settingsData[SettingsIndexLast];
    QVariant            addItemData[AddItemIndexLast];
    HbIndexFeedback     *mIndexFeedback;
};

#endif // VIEWFUTEDATAFORM_H
