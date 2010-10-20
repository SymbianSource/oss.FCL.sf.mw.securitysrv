#ifndef CONTENTWIDGET_H
#define CONTENTWIDGET_H

#include <hbview.h>
#include <hbradiobuttonlist.h>

#include <QPersistentModelIndex>

QT_BEGIN_NAMESPACE

class QFileSystemWatcher;
class QGraphicsTextItem;
class QGraphicsLinearLayout;
class QItemSelection;
class QStandardItem;

QT_END_NAMESPACE

class HbMenuItem;
class HbMainWindow;
class HbListView;
class HbMenu;
class HbAction;
class HbIcon;
class HbAbstractViewItem;
class HbLabel;
class HbListViewItem;
class HbTreeView;
class TreeDataForm;

class ContentWidget : public HbView
{
    Q_OBJECT

public:
    ContentWidget(QString& imagesDir, HbMainWindow *mainWindow);
    virtual ~ContentWidget();

public slots:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void onLongPressed(HbAbstractViewItem *index, const QPointF &coords);
    void aboutToClose();
    void launchInPopup();
    void setRootItemActionTriggered();
    void setTargetItemActionTriggered();

    void addOneToModel();
    void addOneToModelExt();
    void removeFromModel();

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    // refreshes model
    void refresh();
    void populateDirModel();
    void populateFileSystemModel();
    void populateGreenOddBrownEvenModel();
    void showSettings();
    void removeModel();
    void changeModel(HbAction* action);
    void editItem(HbAction* action);
    
    void insertItemAboveTarget();
    void insertItemBelowTarget();

    void setMainMenu();
    void resetItemManagementMenu();  
    void doCommand(int index);
    void itemActivated(const QModelIndex& index);
    void backButtonClicked();
    void confirmDelete();

    void addItem();
    void changeItem();

    void removeItems();
    void removeAllItems();

    void expandAll();
    void collapseAll();

    void changeMirroring();
    void scrollToTarget();
    void changeOrientation();

    void populateTreeModelDefault();
    void populateTreeModelSimple();
    void populateTreeModelDeep();
    void populateTreeModelFlat();
    //void populateTreeModelMail();
    void populateTreeModelMixed();

    void autoInsertOne();
    void autoRemoveOne();
    void simulateMultipleFastInserts();
    void simulateMultipleFastRemovals();
    void timerReadyForInsert();
    void timerReadyForRemoval();

    void insertItemAboveTargetClosed(int  action);
    void insertItemBelowTargetClosed(int action);
    void expandTargetItem();
    void collapseTargetItem();

private:
    void doAddItem();
    void doAddItem(int pos, QStandardItem *item);
    void doSettings();
    void doRemoveItems();

    void createAndInitTreeView(int newModelType); 
    void initTreeView();
    void resetTreeView();

    void updateTextLabel(int count);

    // Calculates the item's depth in the tree. Topmost items are on depth 1. 
    // This information is needed only for debug purposes.
    int calculateDepth(QModelIndex m) const;
    void expand(const QAbstractItemModel *model,
                const QModelIndex &parent, 
                int level );
    void expandUpwards( const QAbstractItemModel *model,
                        const QModelIndex &parent );
    void collapse(  const QAbstractItemModel *model,
                    const QModelIndex &parent, 
                    int  depth );
    void postEvents();

private:
    // submenu items for selecting model
    enum modelType {
        noModel = 0,
        treeModelDefault,
        treeModelSimple,
        treeModelDeep,
        treeModelFlat,
        //treeModelMail,
        treeModelMixed,
        dirModel,
        fileSystemModel,
        greenOddBrownEvenModel,
    };

    // submenu items for list item operation
    enum itemOperation {
        addItemOperation,
        changeItemOperation,
        removeItemOperation,
        removeAllItemsOperation,
        expandAllOperation,
        collapseAllOperation,
        autoInsertOneOperation,
        autoRemoveOneOperation,
        simulateVisible,
        resetDuringSimulation,
        simulateMultipleFastInsertsOperation,
        simulateMultipleFastRemovalsOperation,
        selectAll,
        unselect
    };

    // submenu items for list item operation
    enum orientationOperation {
        toggleOrientationOperation,
        customOrientationOperation
    };

    // submenu items for list item operation
    enum optionsOperation {
        optionsOperationLast // not in use 
    };

    QString&                mImagesDir;
    HbMainWindow            *mWindow;
    HbTreeView              *mTreeView;
                            
    HbAction                *mRemoveModelAction;
    HbAction                *mOrientationSwitch;
    HbAction                *mMirroring;

    HbAction                *mScrollToAction;
    HbAction                *mInsertAboveTargetAction;
    HbAction                *mInsertBelowTargetAction;

    HbMenu                  *mItemSubMenu;
    HbMenu                  *mTargetActionsSubMenu;
                            
    QFileSystemWatcher      *mFileWatcher;
    int                     mModelType;
                            
    HbMenu                  *mMainMenu;
    int                     mCountAdded;
    bool                    mMute;
    HbAction                *mSoftKeyQuitAction;
    HbAction                *mSoftKeyConfirmAction;
    HbAction                *mSoftKeyBackAction;
    HbAction                *mSoftKeyDoneAction;
    HbLabel                 *mInfoLabel;
    QGraphicsLinearLayout   *mMainlayout;
    TreeDataForm            *mForm;
    HbView                  *mDetailView;
    QPersistentModelIndex   mTarget;

    QString                 mTextOfNewItem;
    HbAbstractItemView::ScrollHint mScrollHint;
    int                     mDepth;
    QAbstractItemModel      *mPopupModel;

    QModelIndexList mItemsToRemove;
    QTimer *mTimer;
    int mItemsToAdd;
    QTimer *mTimerExt;
    int mItemsToAddExt;

    QMap<int, HbAction*>    mCheckableItemActions;
};

#endif // CONTENTWIDGET_H
