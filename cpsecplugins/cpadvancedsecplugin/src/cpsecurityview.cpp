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
#include <qlist.h>
#include <qstring.h>
#include <qvector.h>
#include <qpair.h>

#include <hblineedit.h>
#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>
#include <hblabel.h>
#include <hbpushbutton.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <hbmessagebox.h>
#include <HbListWidget>
#include <HbListWidgetItem>
#include <HbGroupBox>
#include <hbpushbutton.h>
#include <hbmainwindow.h>
#include <hbabstractviewitem.h>
#include <HbMessageBox>

#include <memory>

#include <../../inc/cpsecplugins.h>

#include "cpsecurityview.h"
#include "cpsecmodview.h"
#include "cpcertview.h"
#include "cpsecmodmodel.h"
#include "cpsecmoduleinfoview.h"

CpSecurityView::CpSecurityView(QGraphicsItem *parent /*= 0*/)
	: CpBaseSettingView(0,parent),
	  mSecModUIModel(NULL),
	  mSecModView(NULL),
	  mPreView(NULL),
	  mCertView(NULL),
	  mCurrentView(NULL),
	  mPos(0),
	  mNote(NULL), mOkAction(NULL)
	{
	RDEBUG("0", 0);
	setTitle(hbTrId("txt_certificate_manager_setlabel_advanced_security"));

	std::auto_ptr<QGraphicsLinearLayout> layout(q_check_ptr(new QGraphicsLinearLayout(Qt::Vertical)));
    HbListWidget* listCertView = q_check_ptr(new HbListWidget(this));

    std::auto_ptr<HbListWidgetItem> authcert(q_check_ptr(new HbListWidgetItem()));
    authcert->setText(hbTrId("txt_certificate_manager_list_authority_certificate"));

    std::auto_ptr<HbListWidgetItem> trustedsitecert(q_check_ptr(new HbListWidgetItem()));
    trustedsitecert->setText(hbTrId("txt_certificate_manager_list_trusted_site_certific"));

    std::auto_ptr<HbListWidgetItem> personalcert(q_check_ptr(new HbListWidgetItem()));
    personalcert->setText(hbTrId("txt_certificate_manager_list_personal_certificates"));

    std::auto_ptr<HbListWidgetItem> devicecert(q_check_ptr(new HbListWidgetItem()));
    devicecert->setText(hbTrId("txt_certificate_manager_list_device_certificates"));

    listCertView->addItem(authcert.get());
    authcert.release();
    listCertView->addItem(trustedsitecert.get());
    trustedsitecert.release();
    listCertView->addItem(personalcert.get());
    personalcert.release();
    listCertView->addItem(devicecert.get());
    devicecert.release();

    connect(listCertView, SIGNAL(released(QModelIndex)), this, SLOT(displayCert(QModelIndex)));

    HbListWidget* listSecView = q_check_ptr(new HbListWidget(this));
    QMap<QString,QString> keystoreLabels;
    RDEBUG("0", 0);
    try
		{
		QT_TRAP_THROWING(
		mSecModUIModel = CSecModUIModel::NewL();
		keystoreLabels = mSecModUIModel->LoadTokenLabelsL(); )
		}
    catch(const std::exception& exception)
    	{
		QString error(exception.what());
		HbMessageBox::information(error);
		QT_RETHROW;
    	}

    QMapIterator<QString, QString> labelIter(keystoreLabels);
    if(keystoreLabels.count() != 0)
    	{
		while(labelIter.hasNext())
			{
			labelIter.next();
			std::auto_ptr<HbListWidgetItem> widget(q_check_ptr(new HbListWidgetItem()));
			widget->setText(labelIter.key());
			widget->setSecondaryText(labelIter.value());
			listSecView->addItem(widget.get());
			widget.release();
			}
		connect(listSecView, SIGNAL(activated(QModelIndex)), this, SLOT(showCodeView(QModelIndex)));
		connect(listSecView, SIGNAL(longPressed(HbAbstractViewItem*, QPointF )), this, SLOT(indicateLongPress(HbAbstractViewItem*, QPointF)));
    	}
    else
    	{
		std::auto_ptr<HbListWidgetItem> emptyWidget(q_check_ptr(new HbListWidgetItem()));
		emptyWidget->setText("(no security modules)");
    	listSecView->addItem(emptyWidget.get());
    	emptyWidget.release();
    	}
    RDEBUG("0", 0);
    std::auto_ptr<HbGroupBox> certificatesList(q_check_ptr(new HbGroupBox()));
	certificatesList->setHeading(hbTrId("txt_certificate_manager_setlabel_certificates"));
	certificatesList->setContentWidget(listCertView);
	certificatesList->setCollapsed(true);
	layout->addItem(certificatesList.get());
	certificatesList.release();

	std::auto_ptr<HbGroupBox> protectedContentList(q_check_ptr(new HbGroupBox()));
	protectedContentList->setHeading("Protected Content");
	protectedContentList->setCollapsed(true);
	layout->addItem(protectedContentList.get());
	protectedContentList.release();

	std::auto_ptr<HbGroupBox> securityModuleList(q_check_ptr(new HbGroupBox()));
	securityModuleList->setHeading("Security Module");
	securityModuleList->setContentWidget(listSecView);
	securityModuleList->setCollapsed(true);
	layout->addItem(securityModuleList.get());
	securityModuleList.release();

	mContextMenu = q_check_ptr(new HbMenu());

	setLayout(layout.get());
	layout.release();
	}


CpSecurityView::~CpSecurityView()
	{
	if(mSecModView)
		{
		mSecModView->deleteLater();
		}

	if(mPreView)
		{
		mPreView->deleteLater();
		}

	if(mCertView)
		{
		mCertView->deleteLater();
		}

	if(mCurrentView)
		{
		mCurrentView->deleteLater();
		}

	delete mSecModUIModel;

	delete mNote;
	delete mContextMenu;
	}

void CpSecurityView::showCodeView()
	{
	RDEBUG("0", 0);
	try
		{
		mSecModView = q_check_ptr(new CpSecModView(mPos, *mSecModUIModel));
		QObject::connect(mSecModView , SIGNAL(aboutToClose()), this, SLOT(viewDone()));
		mPreView = mainWindow()->currentView();   //suppose mPreView  is member variable of CpSecurityView
		mainWindow()->addView(mSecModView);
		mainWindow()->setCurrentView(mSecModView);
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}

void CpSecurityView::showCodeView( const QModelIndex& aModelIndex)
	{
	mPos = aModelIndex.row();
	showCodeView();
	}

void CpSecurityView::showWIMview()
	{
	RDEBUG("0", 0);
	try
		{
		QObject::connect(mSecModView , SIGNAL(aboutToClose()), this, SLOT(viewDone()));
		mPreView = mainWindow()->currentView();   //suppose mPreView  is member variable of CpSecurityView
		mainWindow()->addView(mSecModView);
		mainWindow()->setCurrentView(mSecModView);
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}

void CpSecurityView::viewDone()
	{
	try
		{
		mainWindow()->removeView(mSecModView);
		mSecModView->deleteLater();
		mSecModView= NULL;
		mainWindow()->setCurrentView(mPreView);
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}

void CpSecurityView::displayCert(const QModelIndex& modelIndex)
	{
	RDEBUG("0", 0);
	try
	{
	mCertView = q_check_ptr(new CpCertView(modelIndex));
	connect(mCertView , SIGNAL(aboutToClose()), this, SLOT(displayPrevious()));
	mPreView = mainWindow()->currentView();
	mainWindow()->addView(mCertView);
	mainWindow()->setCurrentView(mCertView);
	}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}

void CpSecurityView::displayPrevious()
	{
	RDEBUG("0", 0);
	try
		{
		mainWindow()->removeView(mCertView);
		mCertView->deleteLater();
		mCertView= NULL;
		mainWindow()->setCurrentView(mPreView);
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}

void CpSecurityView::indicateLongPress(HbAbstractViewItem *item,QPointF coords)
	{
	RDEBUG("0", 0);
	try
		{
		mContextMenu->clearActions();
		mPos = item->modelIndex().row();

		std::auto_ptr<HbAction> openModule(q_check_ptr(new HbAction("Open")));
		connect(openModule.get(), SIGNAL(triggered()), this, SLOT( showCodeView()));
		mContextMenu->addAction(openModule.get());
		openModule.release();

		if(mSecModUIModel->IsTokenDeletable(mPos))
			{
			std::auto_ptr<HbAction> deleteModule(q_check_ptr(new HbAction("Delete")));
			connect(deleteModule.get(), SIGNAL(triggered()), this, SLOT( deleteModule()));
			mContextMenu->addAction(deleteModule.get());
			deleteModule.release();
			}

		std::auto_ptr<HbAction> moduleInfo(q_check_ptr(new HbAction("Module Info")));
		connect(moduleInfo.get(), SIGNAL(triggered()), this, SLOT( moduleDetails()));
		mContextMenu->addAction(moduleInfo.get());
		moduleInfo.release();

		mContextMenu->setPreferredPos(coords);
		mContextMenu->open();
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}

void CpSecurityView::deleteModule()
	{
	RDEBUG("0", 0);
	try
		{

		delete mNote;
		mNote = NULL;

		mNote = new HbMessageBox(HbMessageBox::MessageTypeQuestion);
		mNote->setHeadingWidget(q_check_ptr(new HbLabel(tr("Delete..."))));
		mNote->setText("Delete keystore and password?");
		mNote->clearActions();
		mOkAction = q_check_ptr(new HbAction("Yes"));
		mNote->addAction(mOkAction);
		HbAction* cancelAction = q_check_ptr(new HbAction("No"));
		mNote->addAction(cancelAction);
		mNote->setTimeout(HbPopup::NoTimeout);
		mNote->setIconVisible (EFalse);
		mNote->open(this,SLOT(dialogClosed(HbAction*)));
		}
	catch(const std::exception& exception)
		{
	    HbMessageBox *box = new HbMessageBox(exception.what());
    	box->setAttribute(Qt::WA_DeleteOnClose);
    	box->open();
		}

}
void CpSecurityView::dialogClosed(HbAction* action)
{
	RDEBUG("0", 0);
	if (action != mOkAction)
		{
		return;
		}

	try
		{
		QT_TRAP_THROWING(mSecModUIModel->DeleteKeysL(mPos));
   		}
	catch(const std::exception& exception)
		{
	    HbMessageBox *box = new HbMessageBox(exception.what());
    	box->setAttribute(Qt::WA_DeleteOnClose);
    	box->open();
		}
	}

void CpSecurityView::moduleDetails()
	{
	RDEBUG("0", 0);
	try
		{
		QVector< QPair<QString,QString> > securityDetails;
		QT_TRAP_THROWING(securityDetails = mSecModUIModel->SecModDetailsL(mPos));
		mModuleinfoView = q_check_ptr(new CpSecmoduleInfoView(securityDetails));

		connect(mModuleinfoView , SIGNAL(aboutToClose()), this, SLOT(displayPreviousFromModInfo()));
		mPreView = mainWindow()->currentView();
		mainWindow()->addView(mModuleinfoView);
		mainWindow()->setCurrentView(mModuleinfoView);
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}

void CpSecurityView::displayPreviousFromModInfo()
	{
	RDEBUG("0", 0);
	try
		{
		mainWindow()->removeView(mModuleinfoView);
		mCertView->deleteLater();
		mCertView= NULL;
		mainWindow()->setCurrentView(mPreView);
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}

