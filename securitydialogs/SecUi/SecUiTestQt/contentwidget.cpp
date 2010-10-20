#include <e32debug.h>

#ifdef _DEBUG
#define RDEBUG( x, y ) RDebug::Printf( "%s %s (%u) %s=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, x, y );
#else
#define RDEBUG( x, y )
#endif

#include <devicelockaccessapi.h>
#include <secuicodequerydialog.h>
#include <gsmerror.h>
#include <secuisecuritysettings.h>
#include <secui.h>
#include <secuisecurityhandler.h>
#include <keyguardaccessapi.h>
#include <avkondomainpskeys.h> // KPSUidAvkonDomain, KAknKeyguardStatus, TAknKeyguardStatus
#include <startupdomainpskeys.h> // KStartupSecurityCodeQueryStatus
#include <coreapplicationuisdomainpskeys.h> // KCoreAppUIsAutolockStatus
#include <hwrmdomainpskeys.h>
#include <settingsinternalcrkeys.h>
#include <keylockpolicyapi.h>
#include <etelmm.h>
#include <rmmcustomapi.h>
#include <securitynotification.h>
#include <centralrepository.h>

const TInt KPhoneIndex(0);
const TInt KTriesToConnectServer(2);
const TInt KTimeBeforeRetryingServerConnection(50000);
const TUid KAutolockUid =
    {
    0x100059B5
    };
#include <Etel3rdParty.h>

_LIT( KMmTsyModuleName, "PhoneTsy");

#include <QStandardItemModel>
#include <QStandardItem>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QGraphicsTextItem>
#include <QGraphicsLinearLayout>
#include <QDirModel>
#include <QFileSystemWatcher>
#include <QItemSelectionModel>
#include <QBrush>
#include <QPointer>
#include <QDebug>
#include <QTimer>
#include <QFileSystemModel>

#include <hbapplication.h>
#include <hbmainwindow.h>
#include <hbinstance.h>
#include <hbnamespace.h>
#include <hbaction.h>
#include <hbmenu.h>
#include <hbtoolbar.h>
#include <hblabel.h>
#include <hbglobal.h>
#include <hbicon.h>
#include <hbview.h>
#include <hbpushbutton.h>
#include <hbtextitem.h>
#include <hbdataformmodel.h>
#ifdef HB_EFFECTS
#include <hbeffect.h>
#endif // HB_EFFECTS
#include <hbtreeview.h>
#include <hbtreeviewitem.h>
#include <hbinputdialog.h>
#include <hbframebackground.h>
#include <hbnotificationdialog.h>
#include <hbmodeliterator.h>

#include "../../Autolock/PubSub/securityuisprivatepskeys.h"

#include "contentwidget.h"
#include "dirviewitem.h"
#include "mailtreeviewitem.h"
#include "modelfactory.h"
#include "treedataform.h"
#include "greenoddviewitem.h"
#include "brownevenviewitem.h"

// model names for submenu
const QStringList KModelNames = (QStringList() << "Default" << "Simple" << "Deep" << "Flat" << /*"Mail" <<*/"Mixed" << "QDirModel" << "QFileSystemModel" << "GreenOddBrownEven");

// orientation submenu items
const QStringList TextStyles = (QStringList() << "Primary" << "Secondary");
const QStringList CustomWidgets = (QStringList() << "Empty" << "Zoom slider" << "Volume Slider" << "Progress bar" << "Button" << "Text Editor");
const QStringList LeftColumnWidgets = (QStringList() << "Empty" << "Icon" << "Text");
const QStringList RightColumnWidgets = (QStringList() << "Empty" << "Icon" << "Text");
const QStringList MiddleColumnWidgets = (QStringList() << "Empty" << "Four Small Icons" << "Three Large Icons" << "Label" << "Zoom slider" << "Volume Slider" << "Progress bar"
        << "Button" << "Text Editor");

// Custom role for storing the tree item depth.
const int KMyCustomDepthRole = Qt::UserRole + 18;

class BannerLabel : public HbLabel
    {
public:
    BannerLabel(QGraphicsItem *parent) :
        HbLabel(parent)
        {
        QFont currentfont(font());
        currentfont.setBold(true);
        currentfont.setPixelSize(18);
        setFont(currentfont);
        setAlignment(Qt::AlignCenter);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        }

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
        {
        QPen oldPen = painter->pen();
        painter->setPen(QPen(QColor(200, 200, 200, 150)));
        painter->setBrush(QBrush(QColor(200, 200, 200, 150), Qt::SolidPattern));
        painter->drawRect(option->rect);
        painter->setPen(oldPen);
        HbLabel::paint(painter, option, widget);
        }
    };

class HbFileSystemTreeView : public HbTreeView
    {

public:
    explicit HbFileSystemTreeView(QGraphicsItem *parent = 0) :
        HbTreeView(parent)
        {
        }

    virtual ~HbFileSystemTreeView()
        {
        }

protected:
    virtual void emitActivated(const QModelIndex &modelIndex);

protected slots:
virtual void modelLayoutChanged();

private:
    QPersistentModelIndex mTopIndex;

    };

void HbFileSystemTreeView::emitActivated(const QModelIndex &modelIndex)
    {
    QList<HbAbstractViewItem *> visibleItems = this->visibleItems();
    int count = visibleItems.count();
    if (count > 0)
        {
        mTopIndex = visibleItems.at(0)->modelIndex();
        }
    else
        {
        mTopIndex = QPersistentModelIndex();
        }
    emit activated(modelIndex);
    }

void HbFileSystemTreeView::modelLayoutChanged()
    {
    if (!mTopIndex.isValid())
        {
        mTopIndex = modelIterator()->nextIndex(rootIndex());
        }
    scrollTo(mTopIndex, HbAbstractItemView::PositionAtTop);
    }

// ======== MEMBER FUNCTIONS ========
ContentWidget::ContentWidget(QString& imagesDir, HbMainWindow *mainWindow) :
    HbView(0), mImagesDir(imagesDir), mWindow(mainWindow), mTreeView(0), mRemoveModelAction(0), mFileWatcher(0), mModelType(noModel), mMainMenu(0), mCountAdded(0), mMute(false),
            mSoftKeyQuitAction(new HbAction(Hb::QuitNaviAction, this)), mSoftKeyConfirmAction(new HbAction(Hb::ConfirmNaviAction, this)), mSoftKeyBackAction(new HbAction(
                    Hb::BackNaviAction, this)), mSoftKeyDoneAction(new HbAction(Hb::DoneNaviAction, this)), mInfoLabel(new BannerLabel(this)), mMainlayout(0), mForm(0),
            mDetailView(0), mTextOfNewItem("Added item"), mScrollHint(HbAbstractItemView::EnsureVisible), mDepth(1), mPopupModel(0), mTimer(0), mItemsToAdd(0)
    {
    mSoftKeyQuitAction->setText("Quit");
    connect(mSoftKeyQuitAction, SIGNAL(triggered()), this, SLOT(backButtonClicked()));
    connect(mSoftKeyConfirmAction, SIGNAL(triggered()), this, SLOT(confirmDelete()));
    mSoftKeyBackAction->setText("Back");
    connect(mSoftKeyBackAction, SIGNAL(triggered()), this, SLOT(backButtonClicked()));

    setNavigationAction( mSoftKeyQuitAction);

    mMainlayout = new QGraphicsLinearLayout(Qt::Vertical);
    mMainlayout->setContentsMargins(0, 0, 0, 0);
    setLayout( mMainlayout);

    updateTextLabel(0);
    mInfoLabel->hide();

    // parameter can be whatever at initialisation phase except, if populateFileSystemModel is populated
    createAndInitTreeView( treeModelSimple);
    populateTreeModelSimple();

    setMainMenu();

#ifdef HB_EFFECTS
    HbEffect::add("listItem", ":RadioButtonList/resources/animation.xml");
#endif // HB_EFFECTS
    }

ContentWidget::~ContentWidget()
    {
    if (mModelType == dirModel)
        {
        // Model is owned by ContentWidget.
        delete mTreeView->model();
        }
    }

void ContentWidget::changeMirroring()
    {
    if (HbApplication::layoutDirection() == Qt::LeftToRight)
        {
        HbApplication::setLayoutDirection(Qt::RightToLeft);
        mMirroring->setText("Turn mirroring off");
        }
    else
        {
        HbApplication::setLayoutDirection(Qt::LeftToRight);
        mMirroring->setText("Turn mirroring on");
        }
    }

void ContentWidget::scrollToTarget()
    {
    if (mTreeView)
        {
        mTreeView->scrollTo(mTarget, mScrollHint);
        }
    }

void ContentWidget::changeOrientation()
    {
    if (mainWindow()->orientation() == Qt::Vertical)
        {
        mWindow->setOrientation(Qt::Horizontal);
        mOrientationSwitch->setText("Change to portrait");
        }
    else
        {
        mWindow->setOrientation(Qt::Vertical);
        mOrientationSwitch->setText("Change to landscape");
        }
    }

void ContentWidget::showSettings()
    {
    mWindow->removeView(this);

    mForm = new TreeDataForm(*mTreeView, this);
    mForm->setHeading("Tree Settings");

    HbView *newView = mWindow->addView(mForm);
    newView->setNavigationAction(mSoftKeyBackAction);
    postEvents();

    mForm->setInputData(ViewFuteDataForm::Settings, ViewFuteDataForm::ScrollHint, mScrollHint);
    mForm->setDepth(mDepth);
    mForm->setDirViewItemEnabled(mModelType == dirModel);

    mForm->setIndentation(mTreeView->indentation());

    mForm->populateSettings();
    HbDataFormModel *settingsFormModel = mForm->dataModel();
    mForm->setModel(settingsFormModel);
    }

void ContentWidget::keyPressEvent(QKeyEvent *event)
    {
    if (event->key() == Qt::Key_H)
        {
        mWindow->setOrientation(Qt::Horizontal);
        event->accept();
        }
    else if (event->key() == Qt::Key_V)
        {
        mWindow->setOrientation(Qt::Vertical);
        event->accept();
        }
    else if (event->key() == Qt::Key_S)
        {
        mTreeView->scrollTo(mTreeView->model()->index(0, 0), mScrollHint);
        }
    else if (event->key() == Qt::Key_R)
        {
        qreal left, top, right, bottom = 0;
        mMainlayout->getContentsMargins(&left, &top, &right, &bottom);
        if (left != 0)
            {
            mMainlayout->setContentsMargins(0, 0, 0, 0);
            }
        else
            {
            mMainlayout->setContentsMargins(9, 9, 9, 9);
            }
        }
    else
        {
        HbView::keyPressEvent(event);
        }
    }

void ContentWidget::refresh()
    {
    if (mTreeView && qobject_cast<QDirModel *> (mTreeView->model()))
        {
        qobject_cast<QDirModel *> (mTreeView->model())->refresh();
        }
    }

void ContentWidget::removeModel()
    {
    mRemoveModelAction->setEnabled(false);

    if (mModelType != noModel)
        {
        delete mTreeView->model();
        mTreeView->setModel(0);
        }
    mModelType = noModel;
    mTarget = QModelIndex();
    }

void ContentWidget::launchInPopup()
    {
    mPopupModel = ModelFactory::populateTreeModelMixed();

    HbTreeView *popupTree = new HbTreeView();
    popupTree->setItemPixmapCacheEnabled(true);

    popupTree->setIndentation(0);
    popupTree->setModel(mPopupModel);

    // inherit some properties from view
    if (mTreeView)
        {
        popupTree->setSelectionMode(mTreeView->selectionMode());
        }

    HbDialog *popup = new HbDialog();
    connect(popup, SIGNAL(aboutToClose()), this, SLOT(aboutToClose()));
    popup->setAttribute(Qt::WA_DeleteOnClose);
    popup->setDismissPolicy(HbPopup::TapOutside);
    popup->setTimeout(HbPopup::NoTimeout);
    HbLabel *label = new HbLabel(tr("View in popup"));
    popup->setHeadingWidget(label);
    popup->setContentWidget(popupTree);
    if (mWindow->orientation() == Qt::Vertical)
        {
        popup->setMinimumWidth(2 * mWindow->layoutRect().width() / 3);
        }
    else
        {
        popup->setMinimumWidth(mWindow->layoutRect().width() / 2);
        }

    popup->show();
    }

void ContentWidget::changeModel(HbAction* action)
    {
    // removeModel resets mModelType
    removeModel();
    int modelType = action->data().toInt();
    if (modelType != noModel)
        {
        createAndInitTreeView(modelType);

        mRemoveModelAction->setEnabled(true);

        switch (modelType)
            {
            case treeModelDefault:
                populateTreeModelDefault();
                break;
            case treeModelSimple:
                populateTreeModelSimple();
                break;
            case treeModelDeep:
                populateTreeModelDeep();
                break;
            case treeModelFlat:
                populateTreeModelFlat();
                break;
                /*case treeModelMail:
                 populateTreeModelMail();
                 break;*/
            case treeModelMixed:
                populateTreeModelMixed();
                break;
            case dirModel:
                populateDirModel();
                break;
            case fileSystemModel:
                populateFileSystemModel();
                break;
            case greenOddBrownEvenModel:
                populateGreenOddBrownEvenModel();
                break;
            default:
                // error
                break;
            }
        expand(mTreeView->model(), mTreeView->rootIndex(), mDepth);
        }
    resetItemManagementMenu();
    }

void ContentWidget::editItem(HbAction* action)
    {
    int itemOperation = action->data().toInt();

    switch (itemOperation)
        {
        case addItemOperation:
            addItem();
            break;
        case changeItemOperation:
            changeItem();
            break;
        case removeItemOperation:
            removeItems();
            break;
        case removeAllItemsOperation:
            removeAllItems();
            break;
        case expandAllOperation:
            expandAll();
            break;
        case collapseAllOperation:
            collapseAll();
            break;
        case autoInsertOneOperation:
            autoInsertOne();
            break;
        case autoRemoveOneOperation:
            autoRemoveOne();
            break;
        case simulateVisible:
            // just checkable item - nothing to do
            break;
        case resetDuringSimulation:
            // just checkable item - nothing to do
            break;
        case simulateMultipleFastInsertsOperation:
            simulateMultipleFastInserts();
            break;
        case simulateMultipleFastRemovalsOperation:
            simulateMultipleFastRemovals();
            break;
        case selectAll:
            mTreeView->selectAll();
            break;
        case unselect:
            mTreeView->clearSelection();
            break;
        default:
            // error
            break;
        }
    }

void ContentWidget::insertItemAboveTarget()
    {
    if (mTarget.isValid())
        {
        //bool ok = false;
        //QString text = HbInputDialog::getText("Enter item text:", mTextOfNewItem, &ok);
HbInputDialog        ::queryText("Enter item text:",this ,SLOT(insertItemAboveTargetClosed(int)),mTextOfNewItem);
        //if (ok) {
        //    mTextOfNewItem = text;

        //    QStandardItemModel* model = qobject_cast<QStandardItemModel *>(mTreeView->model());
        //    int row = mTarget.row();
        //    model->insertRow(row, mTarget.parent());

        // Set text.
        //    if (mTextOfNewItem != QString()) {
        //        QModelIndex index = model->index(row, 0, mTarget.parent());
        //        QStandardItem* newItem = model->itemFromIndex(index);
        //        newItem->setText(mTextOfNewItem);
        //    }
        //}
        }
    }

void ContentWidget::insertItemBelowTarget()
    {
    if (mTarget.isValid())
        {
        //bool ok = false;
        //QString text = HbInputDialog::getText("Enter item text:", mTextOfNewItem, &ok);
HbInputDialog        ::queryText("Enter item text:",this ,SLOT(insertItemBelowTargetClosed(int)),mTextOfNewItem);
        //if (ok) {
        //    mTextOfNewItem = text;

        //    QStandardItemModel* model = qobject_cast<QStandardItemModel *>(mTreeView->model());
        //    int row = mTarget.row() + 1;
        //    model->insertRow(row, mTarget.parent());

        // Set text.
        //    if (mTextOfNewItem != QString()) {
        //        QModelIndex index = model->index(row, 0, mTarget.parent());
        //        QStandardItem* newItem = model->itemFromIndex(index);
        //        newItem->setText(mTextOfNewItem);
        //    }
        //}
        }
    }

void ContentWidget::populateTreeModelDefault()
    {
    if (mTreeView)
        {
        delete mTreeView->model();

        QStandardItemModel* model = ModelFactory::populateTreeModelDefault();
        mTreeView->setModel(model, new HbTreeViewItem);
        mTreeView->setIndentation(-1);

        connect(mTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(selectionChanged(QItemSelection, QItemSelection)));

        mModelType = treeModelDefault;
        }
    }

void ContentWidget::populateTreeModelSimple()
    {
    if (mTreeView)
        {
        delete mTreeView->model();

        // Keeps following line here: it is easiest debugable model
        //QStandardItemModel* model = ModelFactory::populateTreeModelSimpleOfSimplest();
        QStandardItemModel* model = ModelFactory::populateTreeModelSimple();
        mTreeView->setModel(model, new HbTreeViewItem);
        mTreeView->setIndentation(-1);

        connect(mTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(selectionChanged(QItemSelection, QItemSelection)));

        mModelType = treeModelSimple;
        }
    }

void ContentWidget::populateGreenOddBrownEvenModel()
    {
    if (mTreeView)
        {
        delete mTreeView->model();

        QStandardItemModel* model = ModelFactory::populateGreenOddBrownEvenModel();

        QList<HbAbstractViewItem *> prototypes;
        HbListViewItem *prototype1 = new HbTreeViewItem(mTreeView);
        prototype1->resize(0, 0);
        prototypes.append(prototype1);

        GreenOddViewItem *prototype2 = new GreenOddViewItem(mTreeView);
        prototype2->resize(0, 0);
        prototypes.append(prototype2);

        BrownEvenViewItem *prototype3 = new BrownEvenViewItem(mTreeView);
        prototype3->resize(0, 0);
        prototypes.append(prototype3);

        mTreeView->setItemPrototypes(prototypes);
        mTreeView->setIndentation(-1);

        mTreeView->setModel(model);

        connect(mTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(selectionChanged(QItemSelection, QItemSelection)));

        mModelType = greenOddBrownEvenModel;
        }
    }

void ContentWidget::populateTreeModelDeep()
    {
    if (mTreeView)
        {
        delete mTreeView->model();

        QStandardItemModel* model = ModelFactory::populateTreeModelDeep();
        mTreeView->setModel(model, new HbTreeViewItem);
        mTreeView->setIndentation(-1);

        connect(mTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(selectionChanged(QItemSelection, QItemSelection)));

        mModelType = treeModelDeep;
        }
    }

void ContentWidget::populateTreeModelFlat()
    {
    if (mTreeView)
        {
        delete mTreeView->model();
        mTreeView->setItemRecycling(true);

        QStandardItemModel* model = ModelFactory::populateTreeModelFlat();
        mTreeView->setModel(model, new HbTreeViewItem);
        mTreeView->setIndentation(-1);

        connect(mTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(selectionChanged(QItemSelection, QItemSelection)));

        mModelType = treeModelFlat;
        }
    }

/*void ContentWidget::populateTreeModelMail()
 {
 if (mTreeView) {
 delete mTreeView->model();
 mTreeView->setItemRecycling(true);

 QStandardItemModel* model = ModelFactory::populateTreeModelMail();
 mTreeView->setModel(model, new MailTreeViewItem);
 mTreeView->setIndentation(0);

 connect(mTreeView->selectionModel(), 
 SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
 this, 
 SLOT(selectionChanged(QItemSelection, QItemSelection)));

 mModelType = treeModelMail;
 }
 }*/

void ContentWidget::populateDirModel()
    {
    if (mTreeView)
        {
        delete mTreeView->model();

        QDirModel* model = new QDirModel();
        model->setSorting(QDir::DirsFirst);

        mTreeView->setModel(model, new HbTreeViewItem);
        mTreeView->setIndentation(-1);

        connect(mTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(selectionChanged(QItemSelection, QItemSelection)));

        if (!mFileWatcher)
            {
            mFileWatcher = new QFileSystemWatcher();
            mFileWatcher->addPath(mImagesDir);
            connect(mFileWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(refresh()));
            connect(mFileWatcher, SIGNAL(fileChanged(QString)), this, SLOT(refresh()));
            }
        mModelType = dirModel;
        }
    }

void ContentWidget::populateFileSystemModel()
    {
    if (mTreeView)
        {
        delete mTreeView->model();

        QFileSystemModel* model = new QFileSystemModel();
        QString myComputer = model->myComputer().toString();
        model->setRootPath(myComputer);
        mTreeView->setModel(model, new HbTreeViewItem);
        mTreeView->setIndentation(-1);

        QDir dir("");
        QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        int count = 0;
        for (int i = 0; i < count; i++)
            {
            model->setRootPath(dirs.at(i));
            }

        connect(mTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(selectionChanged(QItemSelection, QItemSelection)));

        mModelType = fileSystemModel;
        }
    }

void ContentWidget::populateTreeModelMixed()
    {
    if (mTreeView)
        {
        delete mTreeView->model();
        mTreeView->setItemRecycling(true);

        QStandardItemModel* model = ModelFactory::populateTreeModelMixed();
        mTreeView->setModel(model, new HbTreeViewItem);

        connect(mTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(selectionChanged(QItemSelection, QItemSelection)));

        mModelType = treeModelMixed;
        }
    }

void ContentWidget::setMainMenu()
    {
    if (!mMainMenu)
        {
        mMainMenu = new HbMenu();

        // -----------------------------------------
        // Submenu for selecting/removing the model.
        // -----------------------------------------
        HbMenu *subMenu = mMainMenu->addMenu("Model");
        connect(subMenu, SIGNAL(triggered(HbAction*)),this, SLOT(changeModel(HbAction*)));

        int i(noModel);
        HbAction *action = subMenu->addAction("Remove Model");
        action->setData(QVariant(i));
        mRemoveModelAction = action;

        QString temporaryString;
        foreach (temporaryString , KModelNames)
            {
            HbAction *subAction = subMenu->addAction(temporaryString);
            i++;
            subAction->setData ( QVariant(i) );
            }

        // Settings menuitem.
        HbAction *settingsAction = mMainMenu->addAction("Settings");
        connect(settingsAction, SIGNAL(triggered()), this, SLOT(showSettings()));

        // -------------------------
        // Submenu for editing item.
        // -------------------------
        mItemSubMenu = mMainMenu->addMenu("Item");
        connect(mItemSubMenu, SIGNAL(triggered(HbAction*)),this, SLOT(editItem(HbAction*)));

        HbAction* subMenuAction = mItemSubMenu->addAction("Add");
        subMenuAction->setData((int) addItemOperation);

        subMenuAction = mItemSubMenu->addAction("Change");
        subMenuAction->setData((int) changeItemOperation);

        subMenuAction = mItemSubMenu->addAction("Remove");
        subMenuAction->setData((int) removeItemOperation);

        subMenuAction = mItemSubMenu->addAction("Remove All");
        subMenuAction->setData((int) removeAllItemsOperation);

        subMenuAction = mItemSubMenu->addAction("Expand All");
        subMenuAction->setData((int) expandAllOperation);

        subMenuAction = mItemSubMenu->addAction("Colapse All");
        subMenuAction->setData((int) collapseAllOperation);

        subMenuAction = mItemSubMenu->addAction("Insert one in 3 sec");
        subMenuAction->setData((int) autoInsertOneOperation);

        subMenuAction = mItemSubMenu->addAction("Remove one in 3 sec");
        subMenuAction->setData((int) autoRemoveOneOperation);

        subMenuAction = mItemSubMenu->addAction("Simulate visible items");
        subMenuAction->setCheckable(true);
        subMenuAction->setData((int) simulateVisible);
        mCheckableItemActions.insert(simulateVisible, subMenuAction);

        subMenuAction = mItemSubMenu->addAction("Reset during simulation");
        subMenuAction->setCheckable(true);
        subMenuAction->setData((int) resetDuringSimulation);
        mCheckableItemActions.insert(resetDuringSimulation, subMenuAction);

        subMenuAction = mItemSubMenu->addAction("Simulate fast inserts");
        subMenuAction->setData((int) simulateMultipleFastInsertsOperation);

        subMenuAction = mItemSubMenu->addAction("Simulate fast removals");
        subMenuAction->setData((int) simulateMultipleFastRemovalsOperation);

        subMenuAction = mItemSubMenu->addAction("Select all");
        subMenuAction->setData((int) selectAll);

        subMenuAction = mItemSubMenu->addAction("Clear selected");
        subMenuAction->setData((int) unselect);

        // ---------------------------
        // Submenu for target actions.
        // ---------------------------
        mTargetActionsSubMenu = mMainMenu->addMenu("Target item");

        mScrollToAction = mTargetActionsSubMenu->addAction("Scroll to it");
        connect(mScrollToAction, SIGNAL(triggered()), this, SLOT(scrollToTarget()));

        mInsertAboveTargetAction = mTargetActionsSubMenu->addAction("Insert item above");
        connect(mInsertAboveTargetAction, SIGNAL(triggered()), this, SLOT(insertItemAboveTarget()));

        mInsertBelowTargetAction = mTargetActionsSubMenu->addAction("Insert item below");
        connect(mInsertBelowTargetAction, SIGNAL(triggered()), this, SLOT(insertItemBelowTarget()));

        mInsertBelowTargetAction = mTargetActionsSubMenu->addAction("Expand folder");
        connect(mInsertBelowTargetAction, SIGNAL(triggered()), this, SLOT(expandTargetItem()));

        mInsertBelowTargetAction = mTargetActionsSubMenu->addAction("Collapse folder");
        connect(mInsertBelowTargetAction, SIGNAL(triggered()), this, SLOT(collapseTargetItem()));

        // ---------------------------
        // Submenu for popup
        // ---------------------------
        HbMenu *optionsSubMenu = mMainMenu->addMenu("Popup test");

        HbAction *item1 = optionsSubMenu->addAction("Launch into Popup");
        connect(item1, SIGNAL(triggered()), this, SLOT(launchInPopup()));

        // Landscape/portrait menuitem.
        mOrientationSwitch = mMainMenu->addAction("Change to landscape");
        connect(mOrientationSwitch, SIGNAL(triggered()), this, SLOT(changeOrientation()));

        // Mirroring menuitem.
        mMirroring = mMainMenu->addAction("Turn mirroring on");
        connect(mMirroring, SIGNAL(triggered()), this, SLOT(changeMirroring()));

        resetItemManagementMenu();
        }

    // HbView takes the ownership.
    setMenu( mMainMenu);
    }

void ContentWidget::addItem()
    {
    if ((mModelType != dirModel) && (mModelType != noModel))
        {
        mWindow->removeView(this);
        mForm = new TreeDataForm(*mTreeView, this);
        HbView *newView = mWindow->addView(mForm);
        newView->setNavigationAction(mSoftKeyBackAction);

        postEvents();

        mForm->setHeading("New Item");
        mForm->populateAddItem();
        }
    }

void ContentWidget::doAddItem()
    {
    postEvents();
    QList<QStandardItem *> items = mForm->getStandardItems();
    if (items.count() > 0)
        {
        foreach (QStandardItem *item, items)
            {
            doAddItem(-1, item);
            }
        mTreeView->scrollTo(items.at(0)->index(), mScrollHint);
        }
    }

void ContentWidget::doAddItem(int pos, QStandardItem *item)
    {
    if (item)
        {
        QStandardItemModel *model = static_cast<QStandardItemModel *> (mTreeView->model());

        QModelIndex index;
        if (pos == -1)
            {
            index = mTreeView->currentIndex();
            }
        else
            {
            index = mTreeView->modelIterator()->index(pos);
            }

        if (index.isValid())
            {
            QStandardItem *parent = model->itemFromIndex(index);
            parent->setChild(parent->rowCount(), item);
            }
        else
            {
            model->appendRow(item);
            }

        expandUpwards(mTreeView->model(), item->index());
        }
    }

void ContentWidget::changeItem()
    {
    QStandardItemModel *model = static_cast<QStandardItemModel *> (mTreeView->model());
    QStandardItem *item = model->itemFromIndex(mTreeView->currentIndex());
    QVariant value = item->data(Qt::DisplayRole);
    if (value.isValid())
        {
        if (value.canConvert<QString> ())
            {
            item->setData("First text changed, isn't it?", Qt::DisplayRole);
            }
        else if (value.canConvert<QStringList> ())
            {
            QStringList strings = value.toStringList();
            if (strings.count())
                {
                strings.removeAt(0);
                }
            strings.insert(0, "First text changed, isn't it?");
            item->setData(strings, Qt::DisplayRole);
            }
        }
    }

void ContentWidget::removeItems()
    {
    if (mTreeView->selectionMode() != HbAbstractItemView::MultiSelection)
        {
        mTreeView->setSelectionMode(HbAbstractItemView::MultiSelection);
        }

    setNavigationAction( mSoftKeyConfirmAction);

    mMainlayout->insertItem(0, mInfoLabel);
    mInfoLabel->show();

    mItemSubMenu->menuAction()->setEnabled(false);
    }

void ContentWidget::doRemoveItems()
    {
    QStandardItemModel *model = static_cast<QStandardItemModel *> (mTreeView->model());
    QItemSelectionModel *selectionModel = mTreeView->selectionModel();
    QModelIndexList indexes = selectionModel->selectedIndexes();

    // For debug: For each model index to be deleted, calculate and save the depth in the tree.
    foreach (QModelIndex index, indexes)
        {
        int depth = calculateDepth(index);
        QStandardItem *item = model->itemFromIndex(index);
        item->setData(depth, KMyCustomDepthRole);
        }

    int count = indexes.count();

    // Create a copy of the selected indexes but using the persistent model indices.
    QVector<QPersistentModelIndex> persistentIndexes(count);
    qCopy(indexes.begin(), indexes.end(), persistentIndexes.begin());

    // For debug: Print the content of the list.
    for (int i = 0; i < count; i++)
        {
        QPersistentModelIndex index = persistentIndexes.at(i);
        int depth = index.data(KMyCustomDepthRole).toInt();
        qDebug() << "Item:" << i << index << ", Depth:" << depth;
        }

    // Delete all items by using persistent model indices, which cannot get invalid
    // during the deletion process (i.e. no need to sort them).
    for (int i = 0; i < count; i++)
        {
        QPersistentModelIndex persistentIndex = persistentIndexes.at(i);
        model->removeRow(persistentIndex.row(), persistentIndex.parent());
        }
    }

void ContentWidget::removeAllItems()
    {
    QStandardItemModel *newModel = new QStandardItemModel;
    mTreeView->setModel(newModel, new HbTreeViewItem);
    }

void ContentWidget::expandAll()
    {
    RDEBUG("0", 0);

    HbModelIterator *modelIterator = mTreeView->modelIterator();
    QModelIndex index = modelIterator->nextIndex(QModelIndex());
    while (index.isValid())
        {
        mTreeView->setExpanded(index, true);
        index = modelIterator->nextIndex(index);
        }
    }

void ContentWidget::collapseAll()
    {
    RDEBUG("0", 0);

    HbModelIterator *modelIterator = mTreeView->modelIterator();
    QModelIndex index = modelIterator->previousIndex(QModelIndex());
    while (index.isValid() && index != modelIterator->rootIndex())
        {
        mTreeView->setExpanded(index, false);
        index = modelIterator->previousIndex(index);
        }
    }

void ContentWidget::resetItemManagementMenu()
    {
    bool enabled = false;
    if ((mModelType != dirModel) && (mModelType != noModel))
        {
        enabled = true;
        }
    mItemSubMenu->menuAction()->setEnabled(enabled);
    }

void ContentWidget::doCommand(int index)
    {
    TInt ret = KErrNone;
    RDEBUG("index", index);
    int itemValue = index;
    switch (itemValue)
        {
        ///////////////////////
        case 00:
            {
            RDEBUG("Nothing to do. String selected", itemValue);
            }
            break;
        case 01:
            {
            RDEBUG("DeviceLockOff", 0);
            CDevicelockAccessApi* iDevicelockAccess = CDevicelockAccessApi::NewL();
            RDEBUG("0", 0);
            ret = iDevicelockAccess->DisableDevicelock();
            RDEBUG("ret", ret);
            delete iDevicelockAccess;
            }
            break;
        case 02:
            {
            CKeyguardAccessApi* iKeyguardAccess = CKeyguardAccessApi::NewL();
            RDEBUG("KeyguardOn+Note", 0);
            ret = iKeyguardAccess->EnableKeyguard(ETrue);
            RDEBUG("ret", ret);
            delete iKeyguardAccess;
            }
            break;
        case 03:
            {
            RDEBUG("KeyguardOff", 0);
            CKeyguardAccessApi* iKeyguardAccess = CKeyguardAccessApi::NewL();
            RDEBUG("0", 0);
            ret = iKeyguardAccess->DisableKeyguard(ETrue);
            RDEBUG("ret", ret);
            delete iKeyguardAccess;
            }
            break;
        case 04:
            {
            RDEBUG("OfferDevicelock", 0);
            CDevicelockAccessApi* iDevicelockAccess = CDevicelockAccessApi::NewL();
            RDEBUG("0", 0);
            // TInt errProp = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsTestCode, 12345 );
            ret = iDevicelockAccess->OfferDevicelock();
            RDEBUG("ret", ret);
            delete iDevicelockAccess;
            }
            break;
        case 05:
            {
            RDEBUG("KeyguardOn-Note", 0);
            CKeyguardAccessApi* iKeyguardAccess = CKeyguardAccessApi::NewL();
            RDEBUG("0", 0);
            ret = iKeyguardAccess->EnableKeyguard(EFalse);
            RDEBUG("ret", ret);
            delete iKeyguardAccess;
            }
            break;
        case 06:
            {
            RDEBUG("Wait20-DeviceLockOff", 0);
            for (int ii = 20; ii > 0; ii--)
                {
                RDebug::Printf("%s %s (%u) ii=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ii);
                User::After(1000 * 1000);
                }
            CDevicelockAccessApi* iDevicelockAccess = CDevicelockAccessApi::NewL();
            RDEBUG("0", 0);
            ret = iDevicelockAccess->DisableDevicelock();
            RDEBUG("ret", ret);
            delete iDevicelockAccess;
            }
            break;
        case 07:
            {
            RDEBUG("Wait20-KeyguardOff", 0);
            for (int ii = 20; ii > 0; ii--)
                {
                RDebug::Printf("%s %s (%u) ii=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ii);
                User::After(1000 * 1000);
                }
            CKeyguardAccessApi* iKeyguardAccess = CKeyguardAccessApi::NewL();
            RDEBUG("0", 0);
            ret = iKeyguardAccess->DisableKeyguard(ETrue);
            RDEBUG("ret", ret);
            delete iKeyguardAccess;
            }
            break;
        case 0x08:
            {
            RDEBUG("Wait20-ShowKeysLockedNote", 0);
            for (int ii = 20; ii > 0; ii--)
                {
                RDebug::Printf("%s %s (%u) ii=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ii);
                User::After(1000 * 1000);
                }
            CKeyguardAccessApi* iKeyguardAccess = CKeyguardAccessApi::NewL();
            RDEBUG("0", 0);
            ret = iKeyguardAccess->ShowKeysLockedNote();
            RDEBUG("ret", ret);
            delete iKeyguardAccess;
            }
            break;

        case 0x09:
            {
            RDEBUG("DeviceLockOn", 0);
            CDevicelockAccessApi* iDevicelockAccess = CDevicelockAccessApi::NewL();
            RDEBUG("0", 0);
            ret = iDevicelockAccess->EnableDevicelock(EDevicelockManual);
            RDEBUG("0", 0);
            delete iDevicelockAccess;
            }
            break;
            ///////////////////////
        case 10:
            {
            RDEBUG("Call1", 0);
            TBuf<0x100> title;
            title.Zero();
            HBufC* stringHolder = CSecuritySettings::TranslateLC(_L("txt_devicelocking_dialog_lock_code"), 0); // old txt_pin_code_dialog_sec_code. Perhaps txt_devicelocking_dialog_lock_code_unlock
            title.Append(stringHolder->Des());
            CleanupStack::PopAndDestroy(stringHolder);
            RDEBUG("0", 0);

            TBuf<0x100> title2;
            title2.Zero();
            HBufC* stringHolder2 = CSecuritySettings::TranslateLC(_L("not_found"), 0); // old txt_pin_code_dialog_sec_code. Perhaps txt_devicelocking_dialog_lock_code_unlock
            title2.Append(stringHolder2->Des());
            CleanupStack::PopAndDestroy(stringHolder2);
            RDEBUG("0", 0);

            CTelephony *iTelephony = CTelephony::NewL();
            CTelephony::TTelNumber telNumber(_L("+358504821987"));

            CTelephony::TCallId iCallId;
            CTelephony::TCallParamsV1 callParams;
            callParams.iIdRestrict = CTelephony::ESendMyId;
            CTelephony::TCallParamsV1Pckg callParamsPckg(callParams);
            TRequestStatus stat;
            iTelephony->DialNewCall(stat, callParamsPckg, telNumber, iCallId);
            User::WaitForRequest(stat);
            delete iTelephony;

            RDEBUG("0", 0);
            }
            break;
        case 11:
            {
            RDEBUG("ChangePinL", 0);
            CSecuritySettings* iSecSettings;
            iSecSettings = CSecuritySettings::NewL();
            TSecUi::InitializeLibL();
            RDEBUG("0", 0);
            // TInt errProp = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsTestCode, 1234 );
            ret = KErrNone;
            iSecSettings->ChangePinL();
            RDEBUG("0", 0);
            TSecUi::UnInitializeLib();
            RDEBUG("0", 0);
            delete iSecSettings;
            }
            break;
        case 12:
            {
            RDEBUG("IsLockEnabledL", 0);
            CSecuritySettings* iSecSettings;
            iSecSettings = CSecuritySettings::NewL();
            RDEBUG("0", 0);
            TSecUi::InitializeLibL();
            RDEBUG("0", 0);

            RDEBUG("RMobilePhone::ELockPhoneDevice", RMobilePhone::ELockPhoneDevice);
            ret = iSecSettings->IsLockEnabledL(RMobilePhone::ELockPhoneDevice); // 0
            RDEBUG("ret", ret);

            RDEBUG("RMobilePhone::ELockPhoneDevice", RMobilePhone::ELockICC);
            ret = iSecSettings->IsLockEnabledL(RMobilePhone::ELockICC); // 1
            RDEBUG("ret", ret);

            RDEBUG("RMobilePhone::ELockPhoneDevice", RMobilePhone::ELockPhoneToICC);
            ret = iSecSettings->IsLockEnabledL(RMobilePhone::ELockPhoneToICC); // 2
            RDEBUG("ret", ret);

            RDEBUG("RMobilePhone::ELockPhoneDevice", RMobilePhone::ELockPin2);
            ret = iSecSettings->IsLockEnabledL(RMobilePhone::ELockPin2); // 5
            RDEBUG("ret", ret);

            RDEBUG("RMobilePhone::ELockPhoneDevice", RMobilePhone::ELockUniversalPin);
            ret = iSecSettings->IsLockEnabledL(RMobilePhone::ELockUniversalPin); // 9
            RDEBUG("ret", ret);

            RDEBUG("0", 0);
            TSecUi::UnInitializeLib();
            RDEBUG("0", 0);
            delete iSecSettings;
            RDEBUG("end", 0x99);
            }
            break;
        case 13:
            {
            RDEBUG("AskSecCodeL", 0);
            CSecuritySettings* iSecSettings;
            iSecSettings = CSecuritySettings::NewL();
            TSecUi::InitializeLibL();
            RDEBUG("0", 0);
            // TInt errProp = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsTestCode, 1234 );
            ret = KErrNone;
            ret = iSecSettings->AskSecCodeL();
            RDEBUG("ret", ret);
            TSecUi::UnInitializeLib();
            RDEBUG("0", 0);
            delete iSecSettings;
            }
            break;
        case 14:
            {
            RDEBUG("AskPin2L", 0);
            CSecuritySettings* iSecSettings;
            iSecSettings = CSecuritySettings::NewL();
            TSecUi::InitializeLibL();
            RDEBUG("0", 0);
            // TInt errProp = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsTestCode, 1234 );
            ret = KErrNone;
            ret = iSecSettings->AskPin2L();
            RDEBUG("ret", ret);
            TSecUi::UnInitializeLib();
            RDEBUG("0", 0);
            delete iSecSettings;
            }
            break;
        case 15:
            {
            RDEBUG("GetFdnMode", 0);
            		RMobilePhone::TMobilePhoneFdnStatus fdnMode;
            CSecuritySettings* iSecSettings;
            iSecSettings = CSecuritySettings::NewL();
            TSecUi::InitializeLibL();
            RDEBUG("0", 0);
            // TInt errProp = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsTestCode, 1234 );
            ret = KErrNone;
            ret = iSecSettings->GetFdnMode(fdnMode);
            RDEBUG("ret", ret);
            TSecUi::UnInitializeLib();
            RDEBUG("0", 0);
            delete iSecSettings;
            }
            break;
        case 16:
            {
            RDEBUG("IsUpinBlocked", 0);
            CSecuritySettings* iSecSettings;
            iSecSettings = CSecuritySettings::NewL();
            TSecUi::InitializeLibL();
            RDEBUG("0", 0);
            // TInt errProp = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsTestCode, 1234 );
            ret = KErrNone;
            ret = iSecSettings->IsUpinBlocked();
            RDEBUG("ret", ret);
            TSecUi::UnInitializeLib();
            RDEBUG("0", 0);
            delete iSecSettings;
            }
            break;
        case 17:
            {
            RDEBUG("ChangeSecCodeL", 0);
            CSecuritySettings* iSecSettings;
            iSecSettings = CSecuritySettings::NewL();
            TSecUi::InitializeLibL();
            RDEBUG("0", 0);
            // TInt errProp = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsTestCode, 1234 );
            ret = KErrNone;
            iSecSettings->ChangeSecCodeL();
            RDEBUG("ret", ret);
            TSecUi::UnInitializeLib();
            RDEBUG("0", 0);
            delete iSecSettings;
            }
            break;
        case 18:
            {
            RDEBUG("ChangeAutoLockPeriodL=30", 0);
            CSecuritySettings* iSecSettings;
            iSecSettings = CSecuritySettings::NewL();
            TSecUi::InitializeLibL();
            RDEBUG("0", 0);
            // TInt errProp = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsTestCode, 1234 );
            ret = KErrNone;
            ret = iSecSettings->ChangeAutoLockPeriodL(30);
            RDEBUG("ret", ret);
            TSecUi::UnInitializeLib();
            RDEBUG("0", 0);
            delete iSecSettings;
            }
            break;
        case 19:
            {
            RDEBUG("ChangeAutoLockPeriodL=00", 0);
            CSecuritySettings* iSecSettings;
            iSecSettings = CSecuritySettings::NewL();
            TSecUi::InitializeLibL();
            RDEBUG("0", 0);
            // TInt errProp = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsTestCode, 1234 );
            ret = KErrNone;
            ret = iSecSettings->ChangeAutoLockPeriodL(0);
            RDEBUG("ret", ret);
            TSecUi::UnInitializeLib();
            RDEBUG("0", 0);
            delete iSecSettings;
            }
            break;
            ///////////////////////
        case 20:
            {
            RDEBUG("Notif.EPin1Required", 0);
            static const TUid KSecurityNotifierUid =
                {
                0x10005988
                };
            TInt err(KErrGeneral);
            err=err;
            RNotifier iNotifier;
            err = iNotifier.Connect();
            RDEBUG("0", 0);
            TSecurityNotificationPckg iParams;
            iParams().iEvent = 2;	// EPin1Required
            RDEBUG("0", 0);
            iParams().iStartup = ETrue;
            TPckgBuf<TInt> iPinResult;
            TRequestStatus stat;
            RDEBUG("0", 0);
            iNotifier.StartNotifierAndGetResponse(stat, KSecurityNotifierUid, iParams, iPinResult);
            RDEBUG("0", 0);
            User::WaitForRequest(stat);
            RDEBUG("0", 0);
            err = stat.Int();
            RDEBUG("err", 0);
            err = iNotifier.CancelNotifier(KSecurityNotifierUid);
            RDEBUG("err", err);
            iNotifier.Close();
            err = iPinResult();
            RDEBUG("err", err);
            }
            break;
        case 21:
            {
            RDEBUG("EPin1Required", 0);
		   					RMobilePhone	iPhone;

								TInt err( KErrGeneral);
            		err=err;
								TInt thisTry( 0);
								RTelServer iTelServer;
								RMmCustomAPI iCustomPhone;
								while ( ( err = iTelServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
								{
								User::After( KTimeBeforeRetryingServerConnection );
								}
								err = iTelServer.LoadPhoneModule( KMmTsyModuleName );
								RTelServer::TPhoneInfo PhoneInfo;
								err = iTelServer.SetExtendedErrorGranularity( RTelServer::EErrorExtended ) ;
								err = iTelServer.GetPhoneInfo( KPhoneIndex, PhoneInfo ) ;
								err = iPhone.Open( iTelServer, PhoneInfo.iName ) ;
								err = iCustomPhone.Open( iPhone ) ;
		   					RDEBUG("err", err);

						    CSecurityHandler* handler = new(ELeave) CSecurityHandler(iPhone);
						    // TSecUi::InitializeLibL(); 
								RMobilePhone::TMobilePhoneSecurityEvent iEvent;
						    iEvent = RMobilePhone::EPin1Required;
						    TInt result = KErrNone;
								RDEBUG("iEvent", iEvent);
						    handler->HandleEventL(iEvent, result);
								RDEBUG("result", result);
						    TSecUi::UnInitializeLib();  
								delete handler;
								RDEBUG("end", 0x99);
            }
        case 22:
            {
            RDEBUG("EPin2Required", 0);
		   					RMobilePhone	iPhone;

								TInt err( KErrGeneral);
		            err=err;
								TInt thisTry( 0);
								RTelServer iTelServer;
								RMmCustomAPI iCustomPhone;
								while ( ( err = iTelServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
								{
								User::After( KTimeBeforeRetryingServerConnection );
								}
								err = iTelServer.LoadPhoneModule( KMmTsyModuleName );
								RTelServer::TPhoneInfo PhoneInfo;
								err = iTelServer.SetExtendedErrorGranularity( RTelServer::EErrorExtended ) ;
								err = iTelServer.GetPhoneInfo( KPhoneIndex, PhoneInfo ) ;
								err = iPhone.Open( iTelServer, PhoneInfo.iName ) ;
								err = iCustomPhone.Open( iPhone ) ;
		   					RDEBUG("err", err);

						    CSecurityHandler* handler = new(ELeave) CSecurityHandler(iPhone);
						    // TSecUi::InitializeLibL(); 
								RMobilePhone::TMobilePhoneSecurityEvent iEvent;
						    iEvent = RMobilePhone::EPin2Required;
						    TInt result = KErrNone;
								RDEBUG("iEvent", iEvent);
						    handler->HandleEventL(iEvent, result);
								RDEBUG("result", result);
						    TSecUi::UnInitializeLib();  
								delete handler;
								RDEBUG("end", 0x99);
            }
        case 23:
            {
            RDEBUG("EPhonePasswordRequired", 0);
		   					RMobilePhone	iPhone;

								TInt err( KErrGeneral);
								err=err;
								TInt thisTry( 0);
								RTelServer iTelServer;
								RMmCustomAPI iCustomPhone;
								while ( ( err = iTelServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
								{
								User::After( KTimeBeforeRetryingServerConnection );
								}
								err = iTelServer.LoadPhoneModule( KMmTsyModuleName );
								RTelServer::TPhoneInfo PhoneInfo;
								err = iTelServer.SetExtendedErrorGranularity( RTelServer::EErrorExtended ) ;
								err = iTelServer.GetPhoneInfo( KPhoneIndex, PhoneInfo ) ;
								err = iPhone.Open( iTelServer, PhoneInfo.iName ) ;
								err = iCustomPhone.Open( iPhone ) ;
		   					RDEBUG("err", err);

						    CSecurityHandler* handler = new(ELeave) CSecurityHandler(iPhone);
						    // TSecUi::InitializeLibL(); 
								RMobilePhone::TMobilePhoneSecurityEvent iEvent;
						    iEvent = RMobilePhone::EPhonePasswordRequired;
						    TInt result = KErrNone;
								RDEBUG("iEvent", iEvent);
						    handler->HandleEventL(iEvent, result);
								RDEBUG("result", result);
						    TSecUi::UnInitializeLib();  
								delete handler;
								RDEBUG("end", 0x99);
            }
        case 24:
            {
            RDEBUG("EPuk1Required", 0);
		   					RMobilePhone	iPhone;

								TInt err( KErrGeneral);
								err=err;
								TInt thisTry( 0);
								RTelServer iTelServer;
								RMmCustomAPI iCustomPhone;
								while ( ( err = iTelServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
								{
								User::After( KTimeBeforeRetryingServerConnection );
								}
								err = iTelServer.LoadPhoneModule( KMmTsyModuleName );
								RTelServer::TPhoneInfo PhoneInfo;
								err = iTelServer.SetExtendedErrorGranularity( RTelServer::EErrorExtended ) ;
								err = iTelServer.GetPhoneInfo( KPhoneIndex, PhoneInfo ) ;
								err = iPhone.Open( iTelServer, PhoneInfo.iName ) ;
								err = iCustomPhone.Open( iPhone ) ;
		   					RDEBUG("err", err);

						    CSecurityHandler* handler = new(ELeave) CSecurityHandler(iPhone);
						    // TSecUi::InitializeLibL(); 
								RMobilePhone::TMobilePhoneSecurityEvent iEvent;
						    iEvent = RMobilePhone::EPuk1Required;
						    TInt result = KErrNone;
								RDEBUG("iEvent", iEvent);
						    handler->HandleEventL(iEvent, result);
								RDEBUG("result", result);
						    TSecUi::UnInitializeLib();  
								delete handler;
								RDEBUG("end", 0x99);
            }
        case 25:
            {
            RDEBUG("EPuk2Required", 0);
		   					RMobilePhone	iPhone;

								TInt err( KErrGeneral);
								err=err;
								TInt thisTry( 0);
								RTelServer iTelServer;
								RMmCustomAPI iCustomPhone;
								while ( ( err = iTelServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
								{
								User::After( KTimeBeforeRetryingServerConnection );
								}
								err = iTelServer.LoadPhoneModule( KMmTsyModuleName );
								RTelServer::TPhoneInfo PhoneInfo;
								err = iTelServer.SetExtendedErrorGranularity( RTelServer::EErrorExtended ) ;
								err = iTelServer.GetPhoneInfo( KPhoneIndex, PhoneInfo ) ;
								err = iPhone.Open( iTelServer, PhoneInfo.iName ) ;
								err = iCustomPhone.Open( iPhone ) ;
		   					RDEBUG("err", err);

						    CSecurityHandler* handler = new(ELeave) CSecurityHandler(iPhone);
						    // TSecUi::InitializeLibL(); 
								RMobilePhone::TMobilePhoneSecurityEvent iEvent;
						    iEvent = RMobilePhone::EPuk2Required;
						    TInt result = KErrNone;
								RDEBUG("iEvent", iEvent);
						    handler->HandleEventL(iEvent, result);
								RDEBUG("result", result);
						    TSecUi::UnInitializeLib();  
								delete handler;
								RDEBUG("end", 0x99);
            }
        case 26:
            {
            RDEBUG("EUniversalPinRequired", 0);
		   					RMobilePhone	iPhone;

								TInt err( KErrGeneral);
								err=err;
								TInt thisTry( 0);
								RTelServer iTelServer;
								RMmCustomAPI iCustomPhone;
								while ( ( err = iTelServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
								{
								User::After( KTimeBeforeRetryingServerConnection );
								}
								err = iTelServer.LoadPhoneModule( KMmTsyModuleName );
								RTelServer::TPhoneInfo PhoneInfo;
								err = iTelServer.SetExtendedErrorGranularity( RTelServer::EErrorExtended ) ;
								err = iTelServer.GetPhoneInfo( KPhoneIndex, PhoneInfo ) ;
								err = iPhone.Open( iTelServer, PhoneInfo.iName ) ;
								err = iCustomPhone.Open( iPhone ) ;
		   					RDEBUG("err", err);

						    CSecurityHandler* handler = new(ELeave) CSecurityHandler(iPhone);
						    // TSecUi::InitializeLibL(); 
								RMobilePhone::TMobilePhoneSecurityEvent iEvent;
						    iEvent = RMobilePhone::EUniversalPinRequired;
						    TInt result = KErrNone;
								RDEBUG("iEvent", iEvent);
						    handler->HandleEventL(iEvent, result);
								RDEBUG("result", result);
						    TSecUi::UnInitializeLib();  
								delete handler;
								RDEBUG("end", 0x99);
            }
        case 27:
            {
            RDEBUG("EUniversalPukRequired", 0);
		   					RMobilePhone	iPhone;

								TInt err( KErrGeneral);
								err=err;
								TInt thisTry( 0);
								RTelServer iTelServer;
								RMmCustomAPI iCustomPhone;
								while ( ( err = iTelServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
								{
								User::After( KTimeBeforeRetryingServerConnection );
								}
								err = iTelServer.LoadPhoneModule( KMmTsyModuleName );
								RTelServer::TPhoneInfo PhoneInfo;
								err = iTelServer.SetExtendedErrorGranularity( RTelServer::EErrorExtended ) ;
								err = iTelServer.GetPhoneInfo( KPhoneIndex, PhoneInfo ) ;
								err = iPhone.Open( iTelServer, PhoneInfo.iName ) ;
								err = iCustomPhone.Open( iPhone ) ;
		   					RDEBUG("err", err);

						    CSecurityHandler* handler = new(ELeave) CSecurityHandler(iPhone);
						    // TSecUi::InitializeLibL(); 
								RMobilePhone::TMobilePhoneSecurityEvent iEvent;
						    iEvent = RMobilePhone::EUniversalPukRequired;
						    TInt result = KErrNone;
								RDEBUG("iEvent", iEvent);
						    handler->HandleEventL(iEvent, result);
								RDEBUG("result", result);
						    TSecUi::UnInitializeLib();  
								delete handler;
								RDEBUG("end", 0x99);
            }

            ///////////////////////
        case 31:
            RDEBUG("0", 0)
            ;
            break;

            ///////////////////////
        case 40:
            {
            	RDEBUG("KAknKeyguardStatus=8", 0);
            TInt val = -1;
            ret = RProperty::Set(KPSUidAvkonDomain, KAknKeyguardStatus, 8);
            RDEBUG("ret", ret);
            RDEBUG("0", 0);
            ret = RProperty::Get(KPSUidAvkonDomain, KAknKeyguardStatus, val);
            RDEBUG("ret", ret);
            RDEBUG("val", val);
            }
            break;
        case 41:
            {
            	RDEBUG("Pass=1234", 0);
            TInt errProp = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsTestCode, 1234 );
            RDEBUG("errProp", errProp);
            RDEBUG("KSecurityUIsTestCode", KSecurityUIsTestCode);
            }
            break;
        case 42:
            {
            	RDEBUG("Pass=12345", 0);
            TInt errProp = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsTestCode, 12345 );
            RDEBUG("errProp", errProp);
            RDEBUG("KSecurityUIsTestCode", KSecurityUIsTestCode);
            }
            break;
        case 43:
            {
            RDEBUG("Pass=20499", 0);
            TInt errProp = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsTestCode, 20499 );
            RDEBUG("errProp", errProp);
            RDEBUG("KSecurityUIsTestCode", KSecurityUIsTestCode);
            }
            break;
        case 44:
            {
            RDEBUG("Read-Prop", 0);
            TInt val = -1;
             ret = RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus,   val);
             RDebug::Printf( "%s %s (%u) ret=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ret );
             RDebug::Printf( "%s %s (%u) KCoreAppUIsAutolockStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, val );

             ret = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsLights, val);
             RDebug::Printf( "%s %s (%u) ret=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ret );
             RDebug::Printf( "%s %s (%u) KSecurityUIsLights=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, val );
             
             ret = RProperty::Get(KPSUidAvkonDomain, KAknKeyguardStatus, val);
             RDebug::Printf( "%s %s (%u) ret=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ret );
             RDebug::Printf( "%s %s (%u) KAknKeyguardStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, val );

             ret = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsDismissDialog,  val);
             RDebug::Printf( "%s %s (%u) ret=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ret );
             RDebug::Printf( "%s %s (%u) KSecurityUIsDismissDialog=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, val );

             ret = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery,   val);
             RDebug::Printf( "%s %s (%u) ret=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ret );
             RDebug::Printf( "%s %s (%u) KSecurityUIsSecUIOriginatedQuery=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, val );

             ret = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsLockInitiatorTimeHigh,  val);
             RDebug::Printf( "%s %s (%u) ret=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ret );
             RDebug::Printf( "%s %s (%u) KSecurityUIsLockInitiatorTimeHigh=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, val );

             ret = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsLockInitiatorTimeLow,   val);
             RDebug::Printf( "%s %s (%u) ret=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ret );
             RDebug::Printf( "%s %s (%u) KSecurityUIsLockInitiatorTimeLow=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, val );

             ret = RProperty::Get(KPSUidHWRM, KHWRMGripStatus,   val);
             RDebug::Printf( "%s %s (%u) ret=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ret );
             RDebug::Printf( "%s %s (%u) KHWRMGripStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, val );

             ret = RProperty::Get(KPSUidAvkonDomain, KAknKeyguardStatus, val);
             RDebug::Printf( "%s %s (%u) ret=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, ret );
             RDebug::Printf( "%s %s (%u) KAknKeyguardStatus=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, val );
            }
            break;
        case 45:
            {
            RDEBUG("Read-Prop8", 0);
            mItemsToAdd = -1;
            if(mTimer)
            	{
            	RDEBUG("stop", 0);
            	mTimer->stop();
            	}
            else
            	{
            	RDEBUG("new", 0);
					    mTimer = new QTimer(this);
					  	}
           	RDEBUG("1", 1);
				    mTimer->setSingleShot(false);
           	RDEBUG("2", 2);
				    connect(mTimer, SIGNAL(timeout()), this, SLOT(addOneToModel()));
           	RDEBUG("3", 3);
				    mTimer->start(1000);
           	RDEBUG("4", 4);
            }
            break;
        case 46:
            {
            RDEBUG("Stop-Prop8", 0);
            mItemsToAdd = -1;
            if(mTimer)
            	{
           		RDEBUG("stop", 1);
            	mTimer->stop();
            	}
           	RDEBUG("done Stop-Prop8", 0x99);
            }
            break;
        case 47:
            {
            	RDEBUG("EAutolockOff", 0);
            TInt val = -1;
            TInt errProp = RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus,	EAutolockOff);
            RDEBUG("errProp", errProp);
            RDEBUG("KCoreAppUIsAutolockStatus EAutolockOff", EAutolockOff);
            errProp = RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus,	val);
            RDEBUG("errProp", errProp);
            RDEBUG("val", val);
            }
            break;
        case 48:
            {
            	RDEBUG("EManualLocked", 0);
            TInt val = -1;
            TInt errProp = RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus,	EManualLocked);
            RDEBUG("errProp", errProp);
            RDEBUG("KCoreAppUIsAutolockStatus EManualLocked", EManualLocked);
            errProp = RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus,	val);
            RDEBUG("errProp", errProp);
            RDEBUG("val", val);
            }
            break;
        case 49:
            {
            	RDEBUG("EAutolockStatusUninitialized", 0);
            TInt val = -1;
            TInt errProp = RProperty::Set(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus,	EAutolockStatusUninitialized);
            RDEBUG("errProp", errProp);
            RDEBUG("KCoreAppUIsAutolockStatus EAutolockStatusUninitialized", EAutolockStatusUninitialized);
            errProp = RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus,	val);
            RDEBUG("errProp", errProp);
            RDEBUG("val", val);
            }
            break;
            ///////////////////////
        case 50:
            {
            	RDEBUG("KeyguardTime=0s", 0);
            CRepository* repository = NULL;
         				TInt keyguardTime = 0;
         				TInt cRresult = 0;
         				cRresult = cRresult;
         				repository = CRepository::NewL(KCRUidSecuritySettings);
         				cRresult = repository->Get(KSettingsAutomaticKeyguardTime, keyguardTime);	// in seconds
								RDEBUG("cRresult", cRresult);
								RDEBUG("keyguardTime", keyguardTime);
								repository->Set(KSettingsAutomaticKeyguardTime, 0);	// in seconds
         				cRresult = repository->Get(KSettingsAutomaticKeyguardTime, keyguardTime);	// in seconds
								RDEBUG("cRresult", cRresult);
								RDEBUG("keyguardTime", keyguardTime);
								delete repository;
            }
            break;
        case 51:
            {
            	RDEBUG("KeyguardTime=10s", 0);
            CRepository* repository = NULL;
         				TInt keyguardTime = 0;
         				TInt cRresult = 0;
         				cRresult = cRresult;
         				repository = CRepository::NewL(KCRUidSecuritySettings);
         				cRresult = repository->Get(KSettingsAutomaticKeyguardTime, keyguardTime);	// in seconds
								RDEBUG("cRresult", cRresult);
								RDEBUG("keyguardTime", keyguardTime);
								repository->Set(KSettingsAutomaticKeyguardTime, 10);	// in seconds
         				cRresult = repository->Get(KSettingsAutomaticKeyguardTime, keyguardTime);	// in seconds
								RDEBUG("cRresult", cRresult);
								RDEBUG("keyguardTime", keyguardTime);
								delete repository;
            }
            break;
        case 52:
            {
            	RDEBUG("KeyguardTime=30s", 0);
            CRepository* repository = NULL;
         				TInt keyguardTime = 0;
         				TInt cRresult = 0;
         				cRresult = cRresult;
         				repository = CRepository::NewL(KCRUidSecuritySettings);
         				cRresult = repository->Get(KSettingsAutomaticKeyguardTime, keyguardTime);	// in seconds
								RDEBUG("cRresult", cRresult);
								RDEBUG("keyguardTime", keyguardTime);
								repository->Set(KSettingsAutomaticKeyguardTime, 30);	// in seconds
         				cRresult = repository->Get(KSettingsAutomaticKeyguardTime, keyguardTime);	// in seconds
								RDEBUG("cRresult", cRresult);
								RDEBUG("keyguardTime", keyguardTime);
								delete repository;
            }
            break;
        case 53:
            {
            	RDEBUG("KeyguardTime=10+60s", 0);
            CRepository* repository = NULL;
         				TInt keyguardTime = 0;
         				TInt cRresult = 0;
         				cRresult = cRresult;
         				repository = CRepository::NewL(KCRUidSecuritySettings);
         				cRresult = repository->Get(KSettingsAutomaticKeyguardTime, keyguardTime);	// in seconds
								RDEBUG("cRresult", cRresult);
								RDEBUG("keyguardTime", keyguardTime);
								repository->Set(KSettingsAutomaticKeyguardTime, 10+60);	// in seconds
         				cRresult = repository->Get(KSettingsAutomaticKeyguardTime, keyguardTime);	// in seconds
								RDEBUG("cRresult", cRresult);
								RDEBUG("keyguardTime", keyguardTime);
								delete repository;
            }
            break;
        case 54:
            {
            	RDEBUG("AutoLockTime=0m", 0);
            CRepository* repository = NULL;
         				TInt lockTime = 0;
         				TInt cRresult = 0;
         				cRresult = cRresult;
         				repository = CRepository::NewL(KCRUidSecuritySettings);
         				cRresult = repository->Get(KSettingsAutoLockTime, lockTime);	// in minutes
								RDEBUG("cRresult", cRresult);
								RDEBUG("lockTime", lockTime);
								repository->Set(KSettingsAutoLockTime, 0);	// in minutes
         				cRresult = repository->Get(KSettingsAutoLockTime, lockTime);	// in minutes
								RDEBUG("cRresult", cRresult);
								RDEBUG("lockTime", lockTime);
								delete repository;
            }
            break;
        case 55:
            {
            	RDEBUG("AutoLockTime=1m", 0);
            CRepository* repository = NULL;
         				TInt lockTime = 0;
         				TInt cRresult = 0;
         				cRresult = cRresult;
         				repository = CRepository::NewL(KCRUidSecuritySettings);
         				cRresult = repository->Get(KSettingsAutoLockTime, lockTime);	// in minutes
								RDEBUG("cRresult", cRresult);
								RDEBUG("lockTime", lockTime);
								repository->Set(KSettingsAutoLockTime, 1);	// in minutes
         				cRresult = repository->Get(KSettingsAutoLockTime, lockTime);	// in minutes
								RDEBUG("cRresult", cRresult);
								RDEBUG("lockTime", lockTime);
								delete repository;
            }
            break;
        case 56:
            {
            	RDEBUG("AutoLockTime=2m", 0);
            CRepository* repository = NULL;
         				TInt lockTime = 0;
         				TInt cRresult = 0;
         				cRresult = cRresult;
         				repository = CRepository::NewL(KCRUidSecuritySettings);
         				cRresult = repository->Get(KSettingsAutoLockTime, lockTime);	// in minutes
								RDEBUG("cRresult", cRresult);
								RDEBUG("lockTime", lockTime);
								repository->Set(KSettingsAutoLockTime, 2);	// in minutes
         				cRresult = repository->Get(KSettingsAutoLockTime, lockTime);	// in minutes
								RDEBUG("cRresult", cRresult);
								RDEBUG("lockTime", lockTime);
								delete repository;
            }
            break;
        case 57:
            {
            	RDEBUG("AutoLockTime=65535m", 0);
            CRepository* repository = NULL;
         				TInt lockTime = 0;
         				TInt cRresult = 0;
         				cRresult = cRresult;
         				repository = CRepository::NewL(KCRUidSecuritySettings);
         				cRresult = repository->Get(KSettingsAutoLockTime, lockTime);	// in minutes
								RDEBUG("cRresult", cRresult);
								RDEBUG("lockTime", lockTime);
								repository->Set(KSettingsAutoLockTime, 65535);	// in minutes
         				cRresult = repository->Get(KSettingsAutoLockTime, lockTime);	// in minutes
								RDEBUG("cRresult", cRresult);
								RDEBUG("lockTime", lockTime);
								delete repository;
            }
            break;
        case 58:
            {
            	RDEBUG("read", 0);
            CRepository* repository = NULL;
         				TInt keyguardTime = 0;
         				TInt cRresult = 0;
         				cRresult = cRresult;
         				repository = CRepository::NewL(KCRUidSecuritySettings);
         				cRresult = repository->Get(KSettingsAutomaticKeyguardTime, keyguardTime);	// in seconds
								RDEBUG("cRresult", cRresult);
								RDEBUG("lockTime", keyguardTime);
         				TInt lockTime = 0;
         				cRresult = repository->Get(KSettingsAutoLockTime, lockTime);	// in minutes
								RDEBUG("cRresult", cRresult);
								RDEBUG("lockTime", lockTime);
								delete repository;
            }
            break;
            ///////////////////////
        case 60:
            {
            	RDEBUG("Wait30+Cancel_P&S", 0);
            mItemsToAddExt = 61;
            if(mTimerExt)
            	{
            	RDEBUG("stop", 0);
            	mTimerExt->stop();
            	}
            else
            	{
            	RDEBUG("new", 0);
					    mTimerExt = new QTimer(this);
					  	}
           	RDEBUG("1", 1);
				    mTimerExt->setSingleShot(true);
           	RDEBUG("2", 2);
				    connect(mTimerExt, SIGNAL(timeout()), this, SLOT(addOneToModelExt()));
           	RDEBUG("3", 3);
				    mTimerExt->start(1000);
           	RDEBUG("4", 4);
            }
            break;
        case 61:
            {
            	RDEBUG("Cancel_P&S", 0);
            	TInt err = RProperty::Set(KPSUidSecurityUIs, KSecurityUIsDismissDialog, ESecurityUIsDismissDialogOn );
								RDEBUG("err", err);
		            for(int ii=5;ii>0;ii--)
		            	{
		            	RDEBUG("ii", ii);
		            	User::After(1000*1000);
		            	}
            }
            break;
        case 62:
            {
            	RDEBUG("TSecUi::InitializeLibL", 0);
					    TSecUi::InitializeLibL(); 
            	RDEBUG("0", 0);
            }
        case 63:
            {
            	RDEBUG("TSecUi::UnInitializeLib", 0);
					    TSecUi::UnInitializeLib(); 
            	RDEBUG("0", 0);
            }
        case 64:
            {
            	RDEBUG("Wait30+Cancel_P&S", 0);
            mItemsToAddExt = 61;
            if(mTimerExt)
            	{
            	RDEBUG("stop", 0);
            	mTimerExt->stop();
            	}
            else
            	{
            	RDEBUG("new", 0);
					    mTimerExt = new QTimer(this);
					  	}
           	RDEBUG("1", 1);
				    mTimerExt->setSingleShot(true);
           	RDEBUG("2", 2);
				    connect(mTimerExt, SIGNAL(timeout()), this, SLOT(addOneToModelExt()));
           	RDEBUG("3", 3);
				    mTimerExt->start(1000);
           	RDEBUG("4", 4);
            }
        case 65:
            {
            	RDEBUG("CancelSecCodeQuery", 0);
		   					RMobilePhone	iPhone;

								TInt err = KErrGeneral;
								err = err;
								TInt thisTry( 0);
								RTelServer iTelServer;
								RMmCustomAPI iCustomPhone;
								while ( ( err = iTelServer.Connect() ) != KErrNone && ( thisTry++ ) <= KTriesToConnectServer )
								{
								User::After( KTimeBeforeRetryingServerConnection );
								}
								err = iTelServer.LoadPhoneModule( KMmTsyModuleName );
								RTelServer::TPhoneInfo PhoneInfo;
								err = iTelServer.SetExtendedErrorGranularity( RTelServer::EErrorExtended ) ;
								err = iTelServer.GetPhoneInfo( KPhoneIndex, PhoneInfo ) ;
								err = iPhone.Open( iTelServer, PhoneInfo.iName ) ;
								err = iCustomPhone.Open( iPhone ) ;
		   					RDEBUG("err", err);

						    CSecurityHandler* handler = new(ELeave) CSecurityHandler(iPhone);
						    TInt result = KErrNone;
						    result = result;
								RDEBUG("err", err);
						    handler->CancelSecCodeQuery();
						    RDEBUG("0", 0);
								delete handler;
						    RDEBUG("end", 0x99);
            }
        case 66:
            {
            	RDEBUG("not supported EStdKeyDeviceF", 0);
            	/*
            		TApaTaskList tasklist( iCoeEnv->WsSession() );
                TApaTask autolocktask = tasklist.FindApp( KAutolockUid );
                if ( autolocktask.Exists() )
                    {
                    TKeyEvent keyEvent;
                    RDebug::Printf( "%s %s (%u) EStdKeyDeviceF=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, EStdKeyDeviceF );	// 0xb3
                    keyEvent.iCode = EStdKeyDeviceF;
                    keyEvent.iScanCode = EKeyDeviceF;
                    keyEvent.iRepeats = 0;
                    autolocktask.SendKey( keyEvent );
                    }
              */
            	RDEBUG("-1", -1);
            }
            ///////////////////////
        default:
            RDEBUG("default itemValue", itemValue)
            ;
            break;
        }
RDEBUG("real end", 0x99)
      }
void ContentWidget::itemActivated(const QModelIndex& index)
    {
    TInt ret = KErrNone;
    ret = ret;
    RDEBUG("0", 0);
    HbTreeViewItem *viewItem = qobject_cast<HbTreeViewItem*> (mTreeView->itemByIndex(index));
    RDEBUG("0", 0);
    QStandardItemModel *model = static_cast<QStandardItemModel*> (mTreeView->model());
    QStandardItem *myItem = model->itemFromIndex(index);
    QString itemText = myItem->text();
    itemText = itemText.left(2);
    int itemValue = itemText.toInt();
    doCommand(itemValue);
    }

void ContentWidget::backButtonClicked()
    {
    RDEBUG("0", 0);

    if (mWindow->currentView() != this)
        {
        if (mForm)
            {
            postEvents();
            if (mForm->action() == ViewFuteDataForm::Settings)
                {
                doSettings();
                }
            else if (mForm->action() == ViewFuteDataForm::AddItem)
                {
                doAddItem();
                }
            mWindow->removeView(mForm);
            mForm->deleteLater();
            mForm = 0;
            }

        mWindow->addView(this);
        postEvents();
        }
    else
        {
        qApp->quit();
        }
    }

void ContentWidget::confirmDelete()
    {
    RDEBUG("0", 0);

    doRemoveItems();

    mInfoLabel->hide();
    mMainlayout->removeItem(mInfoLabel);
    mTreeView->setSelectionMode(HbAbstractItemView::NoSelection);
    setNavigationAction( mSoftKeyQuitAction);

    mItemSubMenu->menuAction()->setEnabled(true);
    }

void ContentWidget::createAndInitTreeView(int newModelType)
    {
    RDEBUG("0", 0);

    bool treeViewChange = false;
    if (mModelType == fileSystemModel || newModelType == fileSystemModel)
        {
        treeViewChange = true;
        }
    if (!mTreeView || treeViewChange)
        {
        if (mTreeView)
            {
            resetTreeView();
            }
        if (newModelType == fileSystemModel)
            {
            mTreeView = new HbFileSystemTreeView(this);
            }
        else
            {
            mTreeView = new HbTreeView(this);
            }
        initTreeView();
        }
    }

void ContentWidget::initTreeView()
    {
    RDEBUG("0", 0);

    connect(mTreeView, SIGNAL(activated(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));

    connect(mTreeView,
            SIGNAL(longPressed(HbAbstractViewItem*, QPointF)),
            this,
            SLOT(onLongPressed(HbAbstractViewItem*, QPointF)));

    if (mTreeView->selectionModel())
        {
        connect(mTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(selectionChanged(QItemSelection, QItemSelection)));
        }

    setMenu( mMainMenu);

    mTreeView->setFocus();
    mTreeView->setLongPressEnabled(true);
    mTreeView->setItemPixmapCacheEnabled(true);

    if (mMainlayout)
        {
        mMainlayout->addItem(mTreeView);
        }
    }

void ContentWidget::resetTreeView()
    {
    RDEBUG("0", 0);

    delete mTreeView->model();

    disconnect(mTreeView, SIGNAL(activated(QModelIndex)), this, SLOT(itemActivated()));

    mMainlayout->removeItem(mTreeView);

    delete mTreeView;
    mTreeView = 0;
    mTarget = QModelIndex();
    }

Q_DECLARE_METATYPE( QModelIndex)
void ContentWidget::setTargetItemActionTriggered()
    {
    RDEBUG("0", 0);

    QAction *action = qobject_cast<QAction *> (sender());
    if (action)
        {
        mTarget = action->data().value<QModelIndex> ();
        if (mTarget.isValid())
            {
            HbNotificationDialog *popup = new HbNotificationDialog;
            popup->setText("Target set successfully.");
            popup->setTimeout(700);
            popup->setAttribute(Qt::WA_DeleteOnClose);
            popup->show();
            }
        }
    }

void ContentWidget::setRootItemActionTriggered()
    {
    RDEBUG("0", 0);

    QAction *action = qobject_cast<QAction *> (sender());
    if (action)
        {
        QModelIndex index = action->data().value<QModelIndex> ();
        mTreeView->setRootIndex(index);
        if (index.isValid())
            {
            HbNotificationDialog *popup = new HbNotificationDialog;
            popup->setText("Root item set successfully.");
            popup->setTimeout(700);
            popup->setAttribute(Qt::WA_DeleteOnClose);
            popup->show();
            }
        mTarget = QModelIndex();
        }
    }

void ContentWidget::onLongPressed(HbAbstractViewItem* listViewItem, const QPointF& coords)
    {
    RDEBUG("0", 0);

    Q_UNUSED(listViewItem);

    HbMenu *contextMenu = new HbMenu();
    HbAction *contextAction1 = contextMenu->addAction("Set as target item");
    HbAction *contextAction2 = contextMenu->addAction("Set as root item");

    QVariant modelIndex = qVariantFromValue(listViewItem->modelIndex());
    contextAction1->setData(modelIndex);
    contextAction2->setData(modelIndex);

    connect(contextAction1, SIGNAL(triggered()), SLOT(setTargetItemActionTriggered()));
    connect(contextAction2, SIGNAL(triggered()), SLOT(setRootItemActionTriggered()));

    contextMenu->setAttribute(Qt::WA_DeleteOnClose);
    contextMenu->setPreferredPos(coords);
    contextMenu->show();
    }

void ContentWidget::selectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
    {
    Q_UNUSED(selected)
    Q_UNUSED(deselected)
    QItemSelectionModel *selectionModel = mTreeView->selectionModel();
    QModelIndexList indexes = selectionModel->selectedIndexes();
    updateTextLabel(indexes.count());
    }

void ContentWidget::updateTextLabel(int count)
    {
    RDEBUG("0", 0);

    if (count == 0)
        {
        mInfoLabel->setPlainText("No items selected");
        }
    else
        {
        QString text = QString::number(count);
        text.append(" items selected");
        mInfoLabel->setPlainText(text);
        }
    }

void ContentWidget::doSettings()
    {
    RDEBUG("0", 0);

    mForm->resolveSettingsResults();
    mScrollHint = mForm->getScrollHint();

    int mDepthTemp = mDepth;
    mDepth = mForm->getDepth();
    if (mDepthTemp != mDepth)
        {
        collapse(mTreeView->model(), mTreeView->rootIndex(), mDepth);
        expand(mTreeView->model(), mTreeView->rootIndex(), mDepth);
        }
    }

int ContentWidget::calculateDepth(QModelIndex m) const
    {
    int depth = 1;
    RDEBUG("0", 0);

    while (m.parent() != QModelIndex())
        {
        depth++;
        m = m.parent();
        }
    return depth;
    }

void ContentWidget::expand(const QAbstractItemModel *model, const QModelIndex &parent, int depth)
    {
    RDEBUG("0", 0);

    if (model && depth > 1 && (parent == mTreeView->rootIndex() || parent.isValid()))
        {
        int rows = model->rowCount(parent);
        for (int j = 0; j < rows; j++)
            {
            QModelIndex index = model->index(j, 0, parent);
            if (index.isValid() && model->rowCount(index) > 0)
                {
                if (!mTreeView->isExpanded(index))
                    {
                    mTreeView->setExpanded(index, true);
                    }
                expand(model, index, depth - 1);
                }
            }
        }
    }

void ContentWidget::expandUpwards(const QAbstractItemModel *model, const QModelIndex &index)
    {
    RDEBUG("0", 0);

    if (model && index != mTreeView->rootIndex() && index.isValid())
        {
        if (!mTreeView->isExpanded(index))
            {
            mTreeView->setExpanded(index, true);
            }
        expandUpwards(model, index.parent());
        }
    }

void ContentWidget::collapse(const QAbstractItemModel *model, const QModelIndex &parent, int depth)
    {
    RDEBUG("0", 0);

    if (model && depth >= 1 && (parent == mTreeView->rootIndex() || parent.isValid()))
        {
        int rows = model->rowCount(parent);
        for (int j = 0; j < rows; j++)
            {
            QModelIndex index = model->index(j, 0, parent);
            if (index.isValid() && model->rowCount(index) > 0)
                {
                if (calculateDepth(index) >= depth && mTreeView->isExpanded(index))
                    {
                    mTreeView->setExpanded(index, false);
                    }
                collapse(model, index, depth);
                }
            }
        }
    }

void ContentWidget::postEvents()
    {
    RDEBUG("0", 0);

    // When widgets are added or removed from main window text items 
    // get font change event, which layout everything again.
    // Use case add item & scroll hint PositionAtBottom fails,
    // if those posted events are not flushed first.

    //TODO: to be wholly when proved that problmes putting following into comments are not too big
    //QCoreApplication::sendPostedEvents();
    //QCoreApplication::sendPostedEvents();
    //QCoreApplication::sendPostedEvents();
    }

void ContentWidget::aboutToClose()
    {
    delete mPopupModel;
    mPopupModel = 0;
    }

void ContentWidget::autoInsertOne()
    {
    RDEBUG("0", 0);

    // add item to model after three seconds
    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(addOneToModel()));
    timer->start(3000);
    }
void ContentWidget::autoRemoveOne()
    {
    // remove item from model after three seconds
    QModelIndex index = mTreeView->currentIndex();
    if (!index.isValid())
        {
        QAbstractItemModel *model = mTreeView->model();
        index = model->index((model->rowCount() / 2), 0, mTreeView->rootIndex());
        }

    mItemsToRemove.clear();
    mItemsToRemove.append(index);

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(removeFromModel()));
    timer->start(3000);
    }

void ContentWidget::addOneToModel()
    {
    RDEBUG("mItemsToAdd", mItemsToAdd);
		if(mItemsToAdd>0)
			mItemsToAdd--;
		if(mItemsToAdd==0)
			return;

    TInt ret=KErrNone;
    ret = ret;
    TInt val=-1;
		ret = RProperty::Get(KPSUidCoreApplicationUIs, KCoreAppUIsAutolockStatus,	val);
		RDEBUG("KCoreAppUIsAutolockStatus", val);
		ret = RProperty::Get(KPSUidAvkonDomain, KAknKeyguardStatus, val);
		RDEBUG("KAknKeyguardStatus", val);
		ret = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsLights,	val);
		RDEBUG("KSecurityUIsLights", val);
		ret = RProperty::Get(KPSUidSecurityUIs, KSecurityUIsSecUIOriginatedQuery,	val);
		RDEBUG("KSecurityUIsSecUIOriginatedQuery", val);
    }

void ContentWidget::addOneToModelExt()
    {
    RDEBUG("mItemsToAddExt", mItemsToAddExt);
		if(mItemsToAddExt>0)
			mItemsToAddExt--;
		if(mItemsToAddExt==0)
			return;

    TInt ret=KErrNone;
    ret=ret;
    TInt val=-1;
    val=val;
		doCommand(mItemsToAddExt);
    }

void ContentWidget::removeFromModel()
    {
    if (mItemsToRemove.count() > 0)
        {
        int count = mItemsToRemove.count();

        for (int i = 0; i < count; ++i)
            {
            mTreeView->model()->removeRow(mItemsToRemove.takeLast().row());
            }
        }
    }

void ContentWidget::simulateMultipleFastInserts()
    {
    RDEBUG("0", 0);

    if (!mTimer)
        {
        mTimer = new QTimer(this);
        connect(mTimer, SIGNAL(timeout()), this, SLOT(timerReadyForInsert()));
        }
    mItemsToAdd = 50;
    mTimer->start(100);

    }

void ContentWidget::simulateMultipleFastRemovals()
    {
    if (!mTimer)
        {
        mTimer = new QTimer(this);
        connect(mTimer, SIGNAL(timeout()), this, SLOT(timerReadyForRemoval()));
        }
    mItemsToAdd = 50;

    mTimer->start(100);

    }

void ContentWidget::timerReadyForRemoval()
    {
    RDEBUG("0", 0);

    if (mItemsToAdd == 10)
        {
        HbAction *action = mCheckableItemActions.value(resetDuringSimulation);
        if (action)
            {
            if (action->isChecked() && mTreeView->model())
                {
                mItemsToAdd--;
                mTreeView->reset();
                return;
                }
            }
        }

    if (mItemsToAdd > 0 && mTreeView->model() && mTreeView->model()->rowCount() > 0)
        {
        int pos = 0;
        bool removeVisible = false;
        HbAction *action = mCheckableItemActions.value(simulateVisible);
        if (action)
            {
            removeVisible = action->isChecked();
            }

        if (removeVisible)
            {
            QList<HbAbstractViewItem *> visibleItems = mTreeView->visibleItems();
            if (visibleItems.count())
                {
                pos = rand() % visibleItems.count();
                pos = visibleItems.at(pos)->modelIndex().row();
                }
            }
        else if (mTreeView->model()->rowCount() > 0)
            {
            pos = rand() % mTreeView->model()->rowCount();
            }

        mTimer->setInterval(2 + rand() % 10);
        mTreeView->model()->removeRows(pos, 1);
        mItemsToAdd--;
        }
    else
        {
        mTimer->stop();
        delete mTimer;
        mTimer = 0;
        }
    }

void ContentWidget::timerReadyForInsert()
    {
    RDEBUG("0", 0);

    if (mItemsToAdd == 10)
        {
        HbAction *action = mCheckableItemActions.value(resetDuringSimulation);
        if (action)
            {
            if (action->isChecked() && mTreeView->model())
                {
                mItemsToAdd--;
                mTreeView->reset();
                return;
                }
            }
        }

    if (mItemsToAdd > 0 && mTreeView->model())
        {
        int pos = 0;

        bool insertAsVisible = false;
        HbAction *action = mCheckableItemActions.value(simulateVisible);
        if (action)
            {
            insertAsVisible = action->isChecked();
            }

        if (insertAsVisible)
            {
            QList<HbAbstractViewItem *> visibleItems = mTreeView->visibleItems();
            if (visibleItems.count())
                {
                pos = rand() % visibleItems.count();
                pos = visibleItems.at(pos)->modelIndex().row();
                }
            }
        else if (mTreeView->model()->rowCount() > 0)
            {
            pos = rand() % mTreeView->model()->rowCount();
            }

        mTimer->setInterval(2 + rand() % 10);

        if (mModelType == treeModelMixed)
            {
            QStandardItem *item = new QStandardItem;

            if (rand() % 7)
                {
                QString text;
                QVariantList strings;
                HbIcon icon(QString(":/demo/remixevent"));
                if (rand() % 10)
                    {
                    text = "text " + QString::number(strings.size() + 1);
                    for (int i = rand() % 5; i > 0; i--)
                        {
                        text.append(" and");
                        };
                    strings << text;
                    }
                if (rand() % 5)
                    {
                    text = "text " + QString::number(strings.size() + 1);
                    for (int i = rand() % 20; i > 0; i--)
                        {
                        text.append(" and");
                        };
                    strings << text;
                    }
                if (rand() % 3)
                    {
                    text = "text " + QString::number(strings.size() + 1);
                    for (int i = rand() % 30; i > 0; i--)
                        {
                        text.append(" and");
                        };
                    strings << text;
                    }
                QVariantList icons;

                if (rand() % 15)
                    {
                    icons << icon;
                    }
                else
                    {
                    icons << QVariant();
                    }

                if (rand() % 5)
                    {
                    icons << icon;
                    }
                item->setData(icons, Qt::DecorationRole);
                item->setData(strings, Qt::DisplayRole);
                }
            else
                {
                // separator
                item->setData(Hb::SeparatorItem, Hb::ItemTypeRole);
                item->setData(QVariant("Separator"), Qt::DisplayRole);
                }
            mItemsToAdd--;
            doAddItem(pos, item);
            }
        else if (mModelType != dirModel)
            {
            QStandardItem *item = new QStandardItem();
            item->setData(mItemsToAdd == 1 ? QString("Simulated item %1 - last item!!!").arg(mItemsToAdd--) : QString("Simulated item %1").arg(mItemsToAdd--), Qt::DisplayRole);
            doAddItem(pos, item);
            }
        }
    else
        {
        mTimer->stop();
        delete mTimer;
        mTimer = 0;
        }
    }

void ContentWidget::insertItemAboveTargetClosed(int action)
    {
    RDEBUG("0", 0);

    HbInputDialog *dlg = static_cast<HbInputDialog*> (sender());
    QString text = dlg->value().toString();
    //if(dlg->actions().first() == action) {
    if (action == HbDialog::Accepted)
        {
        mTextOfNewItem = text;

        QStandardItemModel* model = qobject_cast<QStandardItemModel *> (mTreeView->model());
        int row = mTarget.row();
        model->insertRow(row, mTarget.parent());

        // Set text.
        if (mTextOfNewItem != QString())
            {
            QModelIndex index = model->index(row, 0, mTarget.parent());
            QStandardItem* newItem = model->itemFromIndex(index);
            newItem->setText(mTextOfNewItem);
            }
        }
    else if (action == HbDialog::Rejected)
        {
        return;
        }
    }

void ContentWidget::insertItemBelowTargetClosed(int action)
    {
    HbInputDialog *dlg = static_cast<HbInputDialog*> (sender());
    QString text = dlg->value().toString();
    //  if(dlg->actions().first() == action) {
    if (action == HbDialog::Accepted)
        {
        mTextOfNewItem = text;

        QStandardItemModel* model = qobject_cast<QStandardItemModel *> (mTreeView->model());
        int row = mTarget.row() + 1;
        model->insertRow(row, mTarget.parent());

        // Set text.
        if (mTextOfNewItem != QString())
            {
            QModelIndex index = model->index(row, 0, mTarget.parent());
            QStandardItem* newItem = model->itemFromIndex(index);
            newItem->setText(mTextOfNewItem);
            }
        }
    else if (action == HbDialog::Rejected)
        return;

    }

void ContentWidget::expandTargetItem()
    {
    if (!mTreeView->isExpanded(mTarget))
        {
        mTreeView->setExpanded(mTarget, true);
        }
    expand(mTreeView->model(), mTarget, 999);

    }

void ContentWidget::collapseTargetItem()
    {
    if (mTreeView->isExpanded(mTarget))
        {
        mTreeView->setExpanded(mTarget, false);
        }
    collapse(mTreeView->model(), mTarget, 999);
    }

// TODO
/*
 verify Autolock is running


 */
