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

#include <QStringList>
#include <QGraphicsLinearLayout>
#include <QModelIndex>

#include <hblineedit.h>
#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>
#include <hblabel.h>
#include <hbpushbutton.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <HbListWidget>
#include <HbListWidgetItem>
#include <HbGroupBox>
#include <hbpushbutton.h>
#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>
#include <hbabstractviewitem.h>
#include <hbtextitem.h>
#include <hbmainwindow.h>
#include <hblistview.h>
#include <QStandardItemModel>
#include <QModelIndexList>
#include <hbmessagebox.h>
#include <memory>

#include "cpcertdetailview.h"
#include "cpcertview.h"
#include "cpcertdatacontainer.h"
#include "cpcerttrustview.h"
#include "cpcertmanuisyncwrapper.h"


CpCertView::CpCertView(const QModelIndex& modelIndex, QGraphicsItem *parent /*= 0*/)
	: CpBaseSettingView(0,parent),
	  mPopup(NULL),
	  mPrevView(NULL),
	  mCurrentView(NULL),
	  mRefreshedView(NULL),
	  mListView(NULL),
	  mSelectAllView(NULL),
	  mOriginalView(NULL),
	  mNote(NULL)
	{
	
	mOriginalView = mainWindow()->currentView();
	
	HbMenu* menu = this->menu();   
	std::auto_ptr<HbAction> deleteAction(new HbAction(hbTrId("txt_common_menu_delete")));
	connect(deleteAction.get(), SIGNAL(triggered()), this, SLOT(deleteList()));    
	menu->addAction(deleteAction.get());
	deleteAction.release();
	
	std::auto_ptr<QGraphicsLinearLayout> layout(new QGraphicsLinearLayout(Qt::Vertical));
	
	HbDataForm *form = q_check_ptr(new HbDataForm(this));
	std::auto_ptr<HbDataFormModel> formModel(q_check_ptr(new HbDataFormModel()));
	form->setModel(formModel.get());
	formModel.release();
	
	HbListWidget* certificateList = q_check_ptr(new HbListWidget(this)); 
		
	CpCertView::TCertificateViews currentView = (CpCertView::TCertificateViews)modelIndex.row(); 
	QString title;
	if(currentView == EPersonalView)
		{
		title = "Move to Device";
		}
	else if(currentView == EDeviceView)
		{
		title = "Move to Personal";  
		}
	std::auto_ptr<HbAction> moveToDeviceAction(q_check_ptr(new HbAction(title)));   
	connect(moveToDeviceAction.get(), SIGNAL(triggered()), this, SLOT(moveCert()));    
	menu->addAction(moveToDeviceAction.get());
	moveToDeviceAction.release();
			
	setDetails(currentView);
	TInt count = 0;
	try
		{
		QT_TRAP_THROWING(mCertDataContainer = CpCertDataContainer::NewL());
		QT_TRAP_THROWING(count = refreshListL());
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		throw(exception);
		}
	
	for(int index = 0; index< count; index++)
		{
		QString certificateLabel = certLabel(index);
		std::auto_ptr<HbListWidgetItem> singleCert(q_check_ptr(new HbListWidgetItem()));
		singleCert->setText(certificateLabel);
		certificateList->addItem(singleCert.get());
		singleCert.release();
		}  // End of FOR loop
	
	connect(certificateList, SIGNAL(activated(QModelIndex)), this, SLOT(openCertFromList(QModelIndex)));   
	connect(certificateList, SIGNAL(longPressed(HbAbstractViewItem*, QPointF )), this, SLOT(indicateLongPress(HbAbstractViewItem*, QPointF))); 
	
	layout->addItem(certificateList);
	setLayout(layout.get());
	layout.release();
	}

CpCertView::~CpCertView()
	{
	delete mCertDataContainer;
	mCertDataContainer = NULL;
	
	if(mPrevView)
		{
		mPrevView->deleteLater();
		mPrevView= NULL;
		}
	if(mCurrentView)
		{
		mCurrentView->deleteLater();
		mCurrentView= NULL;
		}
	if(mRefreshedView)
		{
		mRefreshedView->deleteLater();
		mRefreshedView= NULL;
		}
	if(mListView)
		{
		mListView->deleteLater();
		mListView= NULL;
		}
		
	if(mSelectAllView)
	{
	mSelectAllView->deleteLater();
	mSelectAllView = NULL;
	}
	
	mSelectionIndex.Close();
	
	mIndexList.Close();
	
	delete mPopup;
	
	delete mNote;
	mNote = NULL;
	}
	
void CpCertView::setDetails(CpCertView::TCertificateViews currentView)
	{
	switch(currentView)
		{
		case EAuthorityView:
		setTitle(hbTrId("txt_certificate_manager_list_authority_certificate"));
		mCertView = EAuthorityView;
		break;
		
		case ETrustedView:
		setTitle(hbTrId("txt_certificate_manager_list_trusted_site_certific"));
		mCertView = ETrustedView;
		break;
		
		case EPersonalView:
		setTitle(hbTrId("txt_certificate_manager_list_personal_certificates"));
		mCertView = EPersonalView;
		break;
		
		case EDeviceView:
		setTitle(hbTrId("txt_certificate_manager_list_device_certificates"));
		mCertView = EDeviceView;
		break;
		}
	}

void CpCertView::indicateLongPress(HbAbstractViewItem *item,QPointF coords)
	{
	std::auto_ptr<HbMenu> contextMenu(q_check_ptr(new HbMenu()));
	mPos = item->modelIndex().row();   // Pos will tell you what is the certificate clicked in particular view.
	
	std::auto_ptr<HbAction> open(q_check_ptr(new HbAction(hbTrId("txt_common_menu_open"))));     
	connect(open.get(), SIGNAL(triggered()), this, SLOT( openCertificate()));    
	contextMenu->addAction(open.get());			
	open.release();
	
	QString moveTitle;
	
	if(mCertView == EAuthorityView)
		{
		std::auto_ptr<HbAction> trustSettings(q_check_ptr(new HbAction(hbTrId("txt_certificate_manager_menu_trust_settings")))); 
		connect(trustSettings.get(), SIGNAL(triggered()), this, SLOT(showTrustSettings()));    
		contextMenu->addAction(trustSettings.get());
		trustSettings.release();
		}
	else if(mCertView == EPersonalView)
		{
		moveTitle = hbTrId("txt_certificate_manager_menu_move_to_device_certif");
		}
	else if(mCertView == EDeviceView)
		{
		moveTitle = hbTrId("txt_certificate_manager_menu_move_to_personal_cert");
		}	
	std::auto_ptr<HbAction> moveCert(q_check_ptr(new HbAction(moveTitle)));     
	connect(moveCert.get(), SIGNAL(triggered()), this, SLOT(moveSelectedCert()));    
	contextMenu->addAction(moveCert.get());
	moveCert.release();
			
	if( certAt(mPos)->IsDeletable() )
		{
		std::auto_ptr<HbAction> menuDelete(q_check_ptr(new HbAction(hbTrId("txt_common_menu_delete")))); 
		connect(menuDelete.get(), SIGNAL(triggered()), this, SLOT(deleteCertificate()));    
		contextMenu->addAction(menuDelete.get());
		menuDelete.release();
		}
	contextMenu->open();
	contextMenu->setPreferredPos(coords);
	}

void CpCertView::openCertFromList(const QModelIndex& modelIndex)
	{	
	// Pos will tell you what is the certificate clicked in particular view.
	mPos = modelIndex.row();   
	openCertificate();
	}

void CpCertView::openCertificate()
	{	
	mCurrentView = q_check_ptr(new CpCertDetailView(mCertView,mPos,*mCertDataContainer));    
	connect(mCurrentView, SIGNAL(aboutToClose()), this, SLOT(displayPreviousView()));
	mPrevView = mainWindow()->currentView();   
	mainWindow()->addView(mCurrentView);
	mainWindow()->setCurrentView(mCurrentView);  		
	}
	
void CpCertView::displayPreviousView()  
	{
	mainWindow()->removeView(mCurrentView);    
	mCurrentView->deleteLater();
	mCurrentView= NULL;
	mainWindow()->setCurrentView(mPrevView);  	
	}
	
void CpCertView::deleteCertificate()
	{
	RArray<TInt> pos;
	pos.Append(mPos);
	QT_TRAP_THROWING(deleteCerts(pos));
	}		
	
void CpCertView::deleteList()
	{
	mSelectAll = EFalse;
	mPopup = new HbDialog();

	mPopup->setDismissPolicy(HbDialog::NoDismiss);
	// Set the label as heading widget
	mPopup->setHeadingWidget(q_check_ptr(new HbLabel(hbTrId("txt_certificate_manager_setlabel_certificates"))));
	
	std::auto_ptr<QGraphicsLinearLayout> layout(q_check_ptr(new QGraphicsLinearLayout(Qt::Vertical)));
		
	mSelectAllView = q_check_ptr(new HbListView(this));
	QStandardItemModel* selectAllModel = q_check_ptr(new QStandardItemModel(this));
	// Populate the model with content
	std::auto_ptr<QStandardItem> selectAllItem(q_check_ptr(new QStandardItem()));
	selectAllItem->setData(QString("Select All"),Qt::DisplayRole);
	selectAllModel->appendRow(selectAllItem.get());
	selectAllItem.release();
	
	connect(mSelectAllView, SIGNAL(activated(QModelIndex)), this, SLOT(selectAll()));
	mSelectAllView->setModel(selectAllModel);
	mSelectAllView->setSelectionMode(HbListView::MultiSelection);
	layout->addItem(mSelectAllView);
	
	mListView = q_check_ptr(new HbListView(this));
	// Connect to "activated" signal
	connect(mListView, SIGNAL(activated(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));
	
	// Create a model
	QStandardItemModel* model = q_check_ptr(new QStandardItemModel(this));
	TInt count=0;
	QT_TRAP_THROWING( count = refreshListL());
	
	for(TInt index = 0; index < count ; ++index)
		{
		const CCTCertInfo* cert = certAt(index);
		if( cert->IsDeletable() )
			{
			// Populate the model with content
			std::auto_ptr<QStandardItem> certItem(q_check_ptr(new QStandardItem()));
			QString certificateLabel = certLabel(index);
			certItem->setData( certificateLabel, Qt::DisplayRole);
			model->appendRow(certItem.get());
			mSelectionIndex.Append(index);
			certItem.release();
			}
		}	
	// Set the model to the list view
	mListView->setModel(model);
	mListView->setSelectionMode(HbListView::MultiSelection);
	layout->addItem(mListView);
	
	std::auto_ptr<HbWidget> widget( q_check_ptr(new HbWidget()));
	widget->setLayout(layout.get());
	layout.release();
	mPopup->setContentWidget(widget.get());
	widget.release();
	
	mPopup->setPrimaryAction(q_check_ptr(new HbAction(hbTrId("txt_common_opt_delete"))));
	mPopup->setSecondaryAction(q_check_ptr(new HbAction(hbTrId("txt_common_button_cancel"))));
	mPopup->setTimeout(HbPopup::NoTimeout);
	
	// Launch popup syncronously
	mPopup->open(this, SLOT(handleMultipleDelete(HbAction*)));

}

void CpCertView::handleMultipleDelete(HbAction* action)
{
	if(action == mPopup->primaryAction())
		{
		QItemSelectionModel *selectionModel = mListView->selectionModel();
		QModelIndexList mWidgetItemsToRemove = selectionModel->selectedIndexes();
		TInt deleteCount = mWidgetItemsToRemove.count();
		// start deleting from end of array so that the indexes do not changes of the ones
		// at the front.
		RArray<TInt> actualIndex;
		QT_TRAP_THROWING(CleanupClosePushL(actualIndex));
		for (TInt index = deleteCount-1; index>= 0 ; --index) 
			{
			TInt selectedItemIndex = mWidgetItemsToRemove[index].row();
			actualIndex.Append( mSelectionIndex[selectedItemIndex] );
			}
		deleteCerts(actualIndex);
		 CleanupStack::PopAndDestroy(&actualIndex);
		}
	mListView->deleteLater();
	mListView = NULL;
	mSelectAllView->deleteLater();
	mSelectAllView = NULL; 
	delete mPopup;
	mPopup = NULL;
	}

void CpCertView::moveCert()
	{
	mSelectAll = EFalse;
	mPopup = new HbDialog();
	mPopup->setDismissPolicy(HbDialog::NoDismiss);
	// Set the label as heading widget
	if(mCertView == EPersonalView)
		{
		mPopup->setHeadingWidget(q_check_ptr(new HbLabel(tr("Move To Device"))));
		}
	else if(mCertView == EDeviceView)
		{
		mPopup->setHeadingWidget(q_check_ptr(new HbLabel(tr("Move To Personal"))));
		}
		
	std::auto_ptr<QGraphicsLinearLayout> layout(q_check_ptr(new QGraphicsLinearLayout(Qt::Vertical)));
			
	mSelectAllView = q_check_ptr(new HbListView(this));
	QStandardItemModel* selectAllModel = q_check_ptr(new QStandardItemModel(this));
	// Populate the model with content
	std::auto_ptr<QStandardItem> selectAllItem(q_check_ptr(new QStandardItem()));
	selectAllItem->setData(QString("Select All"),Qt::DisplayRole);
	selectAllModel->appendRow(selectAllItem.get());
	selectAllItem.release();
	connect(mSelectAllView, SIGNAL(activated(QModelIndex)), this, SLOT(selectAll()));
	mSelectAllView->setModel(selectAllModel);
	mSelectAllView->setSelectionMode(HbListView::MultiSelection);
	layout->addItem(mSelectAllView);
	
	mListView = q_check_ptr(new HbListView(this));
	// Connect to "activated" signal
	connect(mListView, SIGNAL(activated(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));
	
	// Create a model
	QStandardItemModel* model = q_check_ptr(new QStandardItemModel(this));
	TInt count =0;
	QT_TRAP_THROWING(count = refreshListL());
	for(TInt index = 0; index < count ; ++index)
		{
		// Populate the model with content
		std::auto_ptr<QStandardItem> certItem(q_check_ptr(new QStandardItem()));
		QString certificateLabel = certLabel(index);
		certItem->setData( certificateLabel, Qt::DisplayRole);
		model->appendRow(certItem.get());
		mSelectionIndex.Append(index);
		certItem.release();
		}	
	
	// Set the model to the list view
	mListView->setModel(model);
	mListView->setSelectionMode(HbListView::MultiSelection);
	layout->addItem(mListView);
	
	std::auto_ptr<HbWidget> widget(q_check_ptr(new HbWidget()));
	widget->setLayout(layout.get());	
	layout.release();
	mPopup->setContentWidget(widget.get());
	widget.release();
	
	mPopup->setPrimaryAction(q_check_ptr(new HbAction(tr("Yes"))));
	mPopup->setSecondaryAction(q_check_ptr(new HbAction(tr("No"))));
	mPopup->setTimeout(HbPopup::NoTimeout);
	
	// Launch popup syncronously
	 mPopup->open(this, SLOT(handleMoveCertDialog(HbAction*)));
}

void CpCertView::handleMoveCertDialog(HbAction* action)
{	
	if(action == mPopup->primaryAction())
		{
		QItemSelectionModel *selectionModel = mListView->selectionModel();
		QModelIndexList mWidgetItemsToRemove = selectionModel->selectedIndexes();
		TInt deleteCount = mWidgetItemsToRemove.count();
		// start deleting from end of array so that the indexes do not changes of the ones
		// at the front.
		RArray<TInt> actualIndex;
		QT_TRAP_THROWING(CleanupClosePushL(actualIndex));
		 for (TInt index = deleteCount-1; index>= 0 ; --index) 
			 {
			 TInt selectedItemIndex = mWidgetItemsToRemove[index].row();
			 actualIndex.Append(mSelectionIndex[selectedItemIndex]);	 
			 }
		 moveCertList(actualIndex);
		 CleanupStack::PopAndDestroy(&actualIndex); 
		}
	mListView->deleteLater();
	mListView = NULL;	
	mSelectAllView->deleteLater();
	mSelectAllView = NULL;
	delete mPopup;
	mPopup = NULL;
	}

void CpCertView::selectAll()
	{
	if(mSelectAll == EFalse)
		{
		mListView->selectAll();
		mSelectAll= ETrue;
		}
	else
		{
		mListView->clearSelection();
		mSelectAll= EFalse;
		}
	}

void CpCertView::deleteCerts( RArray<TInt>& indexList )
    {
  	mIndexList = indexList;
	TInt count = indexList.Count();
	    
	mNote = new HbMessageBox(HbMessageBox::MessageTypeQuestion);
	QString deleteMsg;
	QString sCount;
	if(count == 1)
		{
		deleteMsg = "Delete %1?";
		const CCTCertInfo* entry = certAt(indexList[0]);
		sCount = QString((QChar*)entry->Label().Ptr(),entry->Label().Length());
		}
	else
		{
		deleteMsg = "Delete %1 items?";
		sCount.setNum(count);
		}
	
	deleteMsg = deleteMsg.arg(sCount);
	mNote->setText(deleteMsg);
	mNote->setPrimaryAction(q_check_ptr(new HbAction("Yes")));
	mNote->setSecondaryAction(q_check_ptr(new HbAction("No")));
	mNote->setTimeout(HbPopup::NoTimeout);
	mNote->open(this,SLOT(handleDeleteDialog(HbAction*)));

}

void CpCertView::handleDeleteDialog(HbAction* action)
{
	
	TInt count = mIndexList.Count();
	
	if (action != mNote->primaryAction() || count == 0 )
		{
		return;
		}
		
    RPointerArray<CCTCertInfo> errCerts;
    QT_TRAP_THROWING(CleanupClosePushL(errCerts));
    
    for(TInt index = 0; index < count; ++index)
    	{
		const CCTCertInfo* entry = certAt(mIndexList[index]);
		
		if( mCertView == EPersonalView || mCertView == EAuthorityView )
			{
			QT_TRAP_THROWING( mCertDataContainer->iWrapper->DeleteCertL( 
						mCertDataContainer->CertManager(), *entry ));
			}
		else if( mCertView == ETrustedView )
			{
			QT_TRAP_THROWING( mCertDataContainer->iWrapper->DeleteCertL( 
						mCertDataContainer->CertManager(),*entry, KCMTrustedServerTokenUid ));
			}
		else if( mCertView == EDeviceView )
			{
			QT_TRAP_THROWING( mCertDataContainer->iWrapper->DeleteCertL(
						mCertDataContainer->CertManager(), *entry, KCMDeviceCertStoreTokenUid ));
			}	
		QT_TRAP_THROWING( errCerts.AppendL(entry));
    	}
    
    if(errCerts.Count() > 0)
    	{
		QString message("Unable to delete the following certificate: \n");
		TInt count = errCerts.Count();
		for(TInt index=0;index<count;++index)
			{
			const TDesC& certLabel = errCerts[index]->Label();
			QString certName = QString((QChar*)certLabel.Ptr(),certLabel.Length());
			message.append(certName).append("\n");
			}
		HbMessageBox::warning(message);
    	}
    QT_TRAP_THROWING( count = refreshListL());
	refreshView(count);
	
	CleanupStack::PopAndDestroy(&errCerts);
	delete mNote;
	mNote = NULL;
	
    }

const CCTCertInfo* CpCertView::certAt(TInt index) const
	{
	CCTCertInfo* currentCert = NULL;
	switch(mCertView)
		{
		case EAuthorityView:
			{
			currentCert = mCertDataContainer->iCALabelEntries[ index ]->iCAEntry;
			break;
			}
		case ETrustedView:
			{
			currentCert = mCertDataContainer->iPeerLabelEntries[ index ]->iPeerEntry;
			break;
			}
		case EDeviceView:
			{
			currentCert = mCertDataContainer->iDeviceLabelEntries[ index ]->iDeviceEntry;
			break;
			}
		case EPersonalView:
			{
			currentCert = mCertDataContainer->iUserLabelEntries[ index ]->iUserEntry;
			break;
			}
		};
	return currentCert;
	}

QString CpCertView::certLabel(TInt index) const
	{
	CpCertManUICertData* certData = NULL;
	HBufC* label = NULL;
	TInt length = 0;
	switch(mCertView)
		{
		case EAuthorityView:
			{
			certData = mCertDataContainer->iCALabelEntries[index];
			label = certData->iCAEntryLabel;
			length = certData->iCAEntryLabel->Length();
			break;
			}
		case ETrustedView:
			{
			certData = mCertDataContainer->iPeerLabelEntries[index];
			label = certData->iPeerEntryLabel;
			length = certData->iPeerEntryLabel->Length();
			break;
			}
		case EPersonalView:
			{
			certData = mCertDataContainer->iUserLabelEntries[index];
			label = certData->iUserEntryLabel;
			length = certData->iUserEntryLabel->Length();
			break;
			}
		case EDeviceView:
			{
			certData = mCertDataContainer->iDeviceLabelEntries[index];
			label = certData->iDeviceEntryLabel;
			length = certData->iDeviceEntryLabel->Length();
			break;
			}
		}
	return QString((QChar*)label->Des().Ptr(), length);
	}

TInt CpCertView::refreshListL()
	{
	TInt count = 0;
	switch(mCertView)
		{
		case EAuthorityView:
			mCertDataContainer->RefreshCAEntriesL();
			count = mCertDataContainer->iCALabelEntries.Count();
			break;
		case ETrustedView:
			mCertDataContainer->RefreshPeerCertEntriesL();
			count = mCertDataContainer->iPeerLabelEntries.Count();
			break;
		case EPersonalView:
			mCertDataContainer->RefreshUserCertEntriesL();
			count = mCertDataContainer->iUserLabelEntries.Count();
			break;
		case EDeviceView:
			mCertDataContainer->RefreshDeviceCertEntriesL();
			count = mCertDataContainer->iDeviceLabelEntries.Count();
			break;
		};
	return count;
	}


void CpCertView::refreshView( TInt count )
	{
	if(mRefreshedView)
		{
		mRefreshedView->deleteLater();
		mRefreshedView = NULL;
		}

	mRefreshedView = q_check_ptr(new CpBaseSettingView());  
	switch(mCertView)
		{
		case EAuthorityView:
			{
			mRefreshedView->setTitle(hbTrId("txt_certificate_manager_list_authority_certificate"));
			break;
			}
		case ETrustedView:
			{
			mRefreshedView->setTitle(hbTrId("txt_certificate_manager_list_trusted_site_certific"));				
			break;
			}
		case EPersonalView:
			{
			mRefreshedView->setTitle(hbTrId("txt_certificate_manager_list_personal_certificates"));				
			break;
			}
		case EDeviceView:
			{
			mRefreshedView->setTitle(hbTrId("txt_certificate_manager_list_device_certificates"));				
			break;
			}
		}	
	
	HbMenu* menu = mRefreshedView->menu();   
		
	std::auto_ptr<HbAction> endAction( q_check_ptr(new HbAction(hbTrId("txt_common_opt_delete"))) );
	connect(endAction.get(), SIGNAL(triggered()), this, SLOT(deleteList()));    
	menu->addAction(endAction.get());
	endAction.release();
	
	QString title;
	if(mCertView == EPersonalView)
		{
		title = "Move to Device";   
		}
	else if(mCertView == EDeviceView)
		{
		title = "Move to Personal";  
		}
	std::auto_ptr<HbAction> moveAction(q_check_ptr(new HbAction(title)));
	connect(moveAction.get(), SIGNAL(triggered()), this, SLOT(moveCert()));    
	menu->addAction(moveAction.get());
	moveAction.release();
			
	std::auto_ptr<QGraphicsLinearLayout> layout(q_check_ptr(new QGraphicsLinearLayout(Qt::Vertical)));
			
	HbListWidget* mCertificateList = q_check_ptr(new HbListWidget(this)); 

	for(int index = 0; index< count; index++)
		{
		QString certificateLabel = certLabel(index);
		std::auto_ptr<HbListWidgetItem> singleCert(q_check_ptr(new HbListWidgetItem()));
		singleCert->setText(certificateLabel);
		mCertificateList->addItem(singleCert.get());
		singleCert.release();
		}  // end of for loop
	
	connect(mCertificateList, SIGNAL(activated(QModelIndex)), this, SLOT(openCertFromList(QModelIndex)));   
	connect(mCertificateList, SIGNAL(longPressed(HbAbstractViewItem*, QPointF )), this, SLOT(indicateLongPress(HbAbstractViewItem*, QPointF))); 
	
	layout->addItem(mCertificateList);
	
	// Refresh current view
	QObject::connect(mRefreshedView , SIGNAL(aboutToClose()), this, SLOT(viewDone()));
	mPrevView = mainWindow()->currentView();   
	mainWindow()->addView(mRefreshedView);
	mainWindow()->setCurrentView(mRefreshedView); 
	mRefreshedView->setLayout(layout.get());
	layout.release();
	}


void CpCertView::viewDone()  
	{
	mCurrentView = mainWindow()->currentView();
	mCurrentView->deleteLater();
	mCurrentView= NULL;
	mainWindow()->setCurrentView(mOriginalView);  	
	}
void CpCertView::showTrustSettings()
	{
	mCurrentView = q_check_ptr(new CpCertTrustView(mPos, *mCertDataContainer));    
	connect(mCurrentView , SIGNAL(aboutToClose()), this, SLOT(saveTrustSettings()));
	mPrevView = mainWindow()->currentView();   
	mainWindow()->addView(mCurrentView);
	mainWindow()->setCurrentView(mCurrentView); 
	}	

void CpCertView::saveTrustSettings()
	{
	((CpCertTrustView*)mCurrentView)->saveTrustSettings();
	displayPreviousView();
	}

void CpCertView::moveSelectedCert()
	{
	RArray<TInt> pos;
	pos.Append(mPos);
	moveCertList(pos);
	}

void CpCertView::moveCertList(RArray<TInt>& indexList)
	{
		
	mIndexList = indexList;
	mNote = new HbMessageBox(HbMessageBox::MessageTypeQuestion);
	mNote->setHeadingWidget(q_check_ptr(new HbLabel(hbTrId("txt_certificate_manager_info_move"))));
	if( mCertView == EPersonalView )
		{
		mNote->setText(hbTrId("txt_certificate_manager_info_device_certificates_c"));
		}
	else if( mCertView == EDeviceView )
		{
		mNote->setText("Use of Personal certificates may require user confirmation. Proceed?");
		}
	
	mNote->setPrimaryAction(q_check_ptr(new HbAction("Yes")));
	mNote->setSecondaryAction(q_check_ptr(new HbAction("No")));
	mNote->setTimeout(HbPopup::NoTimeout);
	mNote->setIconVisible (EFalse);
	mNote->open(this,SLOT(handleMoveDialog(HbAction*)));
}

void CpCertView::handleMoveDialog(HbAction* action)
{

	if (action != mNote->primaryAction())
	    {
		return;
	    }
	
	TInt count = mIndexList.Count();
	
	for(TInt index = 0 ; index < count; ++index)
		{
		CCTCertInfo* entry = NULL;
		if(mCertView == EPersonalView)
			{
			entry = mCertDataContainer->iUserLabelEntries[ mIndexList[index] ]->iUserEntry;
			}
		else if(mCertView == EDeviceView)
			{
			entry = mCertDataContainer->iDeviceLabelEntries[ mIndexList[index] ]->iDeviceEntry;
			}
			
		// Move key first
		TCTKeyAttributeFilter keyFilter;
		keyFilter.iKeyId = entry->SubjectKeyId();
		keyFilter.iPolicyFilter =  TCTKeyAttributeFilter::EAllKeys;
	
		TUid sourceCertStoreUid = TUid::Uid(0);
		TUid targetCertStoreUid = TUid::Uid(0);
		TUid sourceKeyStoreUid = TUid::Uid(0);
		TUid targetKeyStoreUid = TUid::Uid(0);
				
		if(mCertView == EPersonalView)
			{
			sourceKeyStoreUid = KCMFileKeyStoreTokenUid;
			targetKeyStoreUid = KCMDeviceKeyStoreTokenUid;
			sourceCertStoreUid = KCMFileCertStoreTokenUid;
			targetCertStoreUid = KCMDeviceCertStoreTokenUid;
			}
		else if(mCertView == EDeviceView)
			{
			sourceKeyStoreUid = KCMDeviceKeyStoreTokenUid;
			targetKeyStoreUid = KCMFileKeyStoreTokenUid;
			sourceCertStoreUid = KCMDeviceCertStoreTokenUid;
			targetCertStoreUid = KCMFileCertStoreTokenUid;
			}
		try
			{
			
			QT_TRAP_THROWING( mCertDataContainer->iWrapper->MoveKeyL( 
					mCertDataContainer->KeyManager(), keyFilter, sourceKeyStoreUid, targetKeyStoreUid ));
			
			// Move certificate
			QT_TRAP_THROWING( mCertDataContainer->iWrapper->MoveCertL( 
					mCertDataContainer->CertManager(), *entry, sourceCertStoreUid, targetCertStoreUid ) );

			}
		catch(const std::exception& exception)
			{
			QString error(exception.what());
			QT_TRAP_THROWING(mCertDataContainer->ShowErrorNoteL( error.toInt() ));
			User::Exit( KErrNone );
			}
		
		try
			{
			if(mCertView == EPersonalView)
				{
				QT_TRAP_THROWING( mCertDataContainer->RefreshUserCertEntriesL() );
				}
			else if(mCertView == EDeviceView)
				{
				QT_TRAP_THROWING( mCertDataContainer->RefreshDeviceCertEntriesL() );
				}
			}
		catch(const std::exception& exception)
			{
			QString error(exception.what());
			if (  error.toInt() == KErrCorrupt )
				{
				QT_TRAP_THROWING(mCertDataContainer->ShowErrorNoteL( error.toInt()) );
				User::Exit( KErrNone );
				}
			// have to call straight away the Exit
			// showing any error notes would corrupt the display
			User::Exit( error.toInt() );	
			}
		} // for
	// Refresh current view
	QT_TRAP_THROWING(refreshView(refreshListL()));	
	delete mNote;
	mNote = NULL;
	}

