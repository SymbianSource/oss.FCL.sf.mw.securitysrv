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

#include <mctauthobject.h>
#include <mctkeystore.h>

#include <qstringlist.h>
#include <qgraphicslinearlayout.h>

#include <hblineedit.h>
#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>
#include <hblabel.h>
#include <hbpushbutton.h>
#include <hbmenu.h>
#include <hbaction.h>
#include <hbmessagebox.h>
#include <hblistwidget.h>
#include <hblistwidgetitem.h>
#include <hbgroupbox.h>
#include <hbpushbutton.h>
#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>
#include <hbmainwindow.h>
#include <hbabstractviewitem.h>

#include <memory>

#include "cpmoduleview.h"
#include "cpsecmodmodel.h"
#include "cpsecmodview.h"

CpModuleView::CpModuleView( CpSecModView::TSecModViews currentView, 
							CSecModUIModel& secModUIModel,
							QGraphicsItem *parent /*= 0*/ )
: CpBaseSettingView(0,parent),
  mCurrentView(currentView),
  mSecModUIModel(secModUIModel)
	{
	QString title = mSecModUIModel.TokenLabelForTitle();
	setTitle(title);
	
	if(currentView == CpSecModView::EAccessView)
		{	
		showAccessView();
		}
	else if(currentView == CpSecModView::ESignatureView)
		{
		showSignatureView(ETrue);
		}
	}

CpModuleView::~CpModuleView()
	{}

void CpModuleView::showAccessView()
	{
	HbMenu* menu = this->menu();   
	std::auto_ptr<HbAction> endAction(q_check_ptr(new HbAction("Module Info")));     
	connect(endAction.get(), SIGNAL(triggered()), this, SLOT(saveProv()));    

	menu->addAction(endAction.get());
	endAction.release();
	
	std::auto_ptr<QGraphicsLinearLayout> layout(q_check_ptr(new QGraphicsLinearLayout(Qt::Vertical)));
	
	std::auto_ptr<HbDataForm> form(q_check_ptr(new HbDataForm()));
	std::auto_ptr<HbDataFormModel> formModel(q_check_ptr(new HbDataFormModel()));
	form->setModel(formModel.get());
	formModel.release();
	
	QString titleName;
	if (mSecModUIModel.KeyStoreUID() == KTokenTypeFileKeystore)
		{
		titleName = "Phone key store code";
		}
	else
		{
		const TDesC& label = mSecModUIModel.AuthObj(KPinGSettIndex).Label();
		titleName = QString((QChar*)label.Ptr(), label.Length());
		}
	
	HbListWidget* accessDetails = q_check_ptr(new HbListWidget(this)); 
	
	std::auto_ptr<HbListWidgetItem> codeLabel(q_check_ptr(new HbListWidgetItem()));
	codeLabel->setText(titleName);
	accessDetails->addItem(codeLabel.get());
	codeLabel.release();

	std::auto_ptr<HbListWidgetItem> codeText(q_check_ptr(new HbListWidgetItem()));
	codeText->setText("****");
	accessDetails->addItem(codeText.get());
	codeText.release();

	std::auto_ptr<HbListWidgetItem> requestLabel(q_check_ptr(new HbListWidgetItem()));
	requestLabel->setText("Access Code Request");
	accessDetails->addItem(requestLabel.get());
	requestLabel.release();

	TUint32 status = mSecModUIModel.AuthStatus(KPinGSettIndex);
	std::auto_ptr<HbListWidgetItem> requestText(q_check_ptr(new HbListWidgetItem()));
	requestText->setText("Off");
	if (status & EEnabled)
		{
		requestText->setText("On");
		}	
	accessDetails->addItem(requestText.get());
	requestText.release();
	
	std::auto_ptr<HbListWidgetItem> statusLabel(q_check_ptr(new HbListWidgetItem()));
	statusLabel->setText("Status");
	accessDetails->addItem(statusLabel.get());
	statusLabel.release();
	
	std::auto_ptr<HbListWidgetItem> statusText(q_check_ptr(new HbListWidgetItem()));
	QString pinStatus = mSecModUIModel.PINStatus(KPinGSettIndex, ETrue);
	statusText->setText(pinStatus);
	accessDetails->addItem(statusText.get());
	statusText.release();

	connect(accessDetails, SIGNAL(longPressed(HbAbstractViewItem*, QPointF )), this, SLOT(indicateLongPress(HbAbstractViewItem*, QPointF)));
	connect(formModel.get(), SIGNAL(activated(QModelIndex)), this, SLOT(handleAccessView(QModelIndex)));
	
	layout->addItem(accessDetails);	
	layout->addItem(form.get());
	form.release();
	setLayout(layout.get());
	layout.release();
	}

void CpModuleView::showSignatureView(TBool showBlockedNote)
	{
	mCurrentView = CpSecModView::ESignatureView;
	std::auto_ptr<QGraphicsLinearLayout> layout(q_check_ptr(new QGraphicsLinearLayout(Qt::Vertical)));
	std::auto_ptr<HbDataForm> form(q_check_ptr(new HbDataForm()));
	std::auto_ptr<HbDataFormModel> formModel(q_check_ptr(new HbDataFormModel()));
		
	QVector< QPair<QString, TUint32> > authDetails = mSecModUIModel.AuthDetails();
	QVectorIterator< QPair<QString, TUint32> > authDetailsIter(authDetails);
	
	HbListWidget* accessDetails = q_check_ptr(new HbListWidget(this)); 
		
	while(authDetailsIter.hasNext())
		{
		authDetailsIter.next();
		QPair<QString, TUint32> pair;
		QString label = pair.first;
		TUint32 status = pair.second;
		QString blockedDetails = NULL;
		if ( status & EAuthObjectBlocked)
			{
			if (showBlockedNote)
				{
				QString totalBlocked(" is blocked. PUK code is needed to unblock the code");
				if (status & EUnblockDisabled)
					{
					totalBlocked =  " totalblocked, contact your module vendor.";
					}
				totalBlocked.prepend(label);
				HbMessageBox::information(totalBlocked);
				}
			blockedDetails = label.append(" Blocked");
		
		std::auto_ptr<HbListWidgetItem> statusLabel(q_check_ptr(new HbListWidgetItem()));
		statusLabel->setText(tr("Status"));
		accessDetails->addItem(statusLabel.get());
		statusLabel.release();

		std::auto_ptr<HbListWidgetItem> statusText(q_check_ptr(new HbListWidgetItem()));
		QString pinStatus = mSecModUIModel.PINStatus(KPinNrSettIndex, ETrue);
		statusText->setText(pinStatus);
		accessDetails->addItem(statusText.get());
		statusText.release();
		}
		else
			{
			std::auto_ptr<HbListWidgetItem> unblockedLabel(q_check_ptr(new HbListWidgetItem()));
			unblockedLabel->setText(label);
			accessDetails->addItem(unblockedLabel.get());
			unblockedLabel.release();
			
			std::auto_ptr<HbListWidgetItem> unblockedText(q_check_ptr(new HbListWidgetItem()));
			unblockedText->setText("****");
			accessDetails->addItem(unblockedText.get());
			unblockedText.release();
			}
		}
	form->setModel(formModel.get());
	formModel.release();
	layout->addItem(accessDetails);
	
	connect(accessDetails, SIGNAL(longPressed(HbAbstractViewItem*, QPointF )), this, SLOT(indicateLongPress(HbAbstractViewItem*, QPointF)));
	connect(formModel.get(), SIGNAL(activated(QModelIndex)), this, SLOT(handleSigView()));
		
	layout->addItem(form.get());
	form.release();
	setLayout(layout.get());
	layout.release();
	}

void CpModuleView::indicateLongPress(HbAbstractViewItem *item,QPointF coords)
	{
	try
		{
		std::auto_ptr<HbMenu> contextMenu(q_check_ptr(new HbMenu()));
		mPos = item->modelIndex().row();   
		
		if(mCurrentView == CpSecModView::EAccessView)
			{
			if( mPos == EIndexCodeLabel && mSecModUIModel.PinChangeable(KPinGSettIndex)
				|| mPos == EIndexCodeRequest && mSecModUIModel.PinRequestChangeable(KPinGSettIndex)
				|| mPos == EIndexCodeStatus )
				{
				std::auto_ptr<HbAction> changePIN(q_check_ptr(new HbAction("Change")));     
				connect(changePIN.get(), SIGNAL(triggered()), this, SLOT( handleAccessView()));    
				contextMenu->addAction(changePIN.get());
				changePIN.release();
				}
			if( mSecModUIModel.PinUnblockable(KPinGSettIndex) )
				{
				std::auto_ptr<HbAction> unblockPIN(q_check_ptr(new HbAction("Unblock")));     
				connect(unblockPIN.get(), SIGNAL(triggered()), this, SLOT( handleAccessView()));    
				contextMenu->addAction(unblockPIN.get());
				unblockPIN.release();
				}
			if( mSecModUIModel.PinOpen(KPinGSettIndex) )
				{
				std::auto_ptr<HbAction> closePIN(q_check_ptr(new HbAction("Close")));     
				connect(closePIN.get(), SIGNAL(triggered()), this, SLOT( handleAccessView()));    
				contextMenu->addAction(closePIN.get());
				closePIN.release();
				}
			}
		else if(mCurrentView == CpSecModView::ESignatureView)
			{
			if (mSecModUIModel.PinChangeable(KPinNrSettIndex))
				{
				std::auto_ptr<HbAction> changePIN(q_check_ptr(new HbAction("Change")));     
				connect(changePIN.get(), SIGNAL(triggered()), this, SLOT( handleSigViewCommand()));    
				contextMenu->addAction(changePIN.get());
				changePIN.release();
				}
			if (mSecModUIModel.PinUnblockable(KPinNrSettIndex))
				{
				std::auto_ptr<HbAction> unblockPIN(q_check_ptr(new HbAction("Unblock")));     
				connect(unblockPIN.get(), SIGNAL(triggered()), this, SLOT( handleSigViewCommand()));    
				contextMenu->addAction(unblockPIN.get());
				unblockPIN.release();
				}
			}
				
		contextMenu->open();
		contextMenu->setPreferredPos(coords);
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}

void CpModuleView::handleAccessView( const QModelIndex& modelIndex )
	{
	try
		{
		mPos = modelIndex.row();
		handleAccessView();
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}

void CpModuleView::handleAccessView()
	{
	try
		{
		if(mPos == EIndexCodeLabel)
			{
			QT_TRAP_THROWING(mSecModUIModel.ChangeOrUnblockPinL(KPinGSettIndex));        
			}
		else if(mPos == EIndexCodeRequest)
			{
			if (KErrCancel == mSecModUIModel.ChangeCodeRequest(KPinGSettIndex))
				{
				return;
				}
			}
		else if ( mPos == EIndexCodeStatus)
			{
			QT_TRAP_THROWING(mSecModUIModel.CloseAuthObjL(KPinGSettIndex)); 
			}
		showAccessView();
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}

void CpModuleView::handleSigView()
	{
	try
		{
		QT_TRAP_THROWING(mSecModUIModel.ChangeOrUnblockPinL(KPinNrSettIndex));
		showSignatureView(EFalse);
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}

void CpModuleView::handleSigViewCommand()
	{
	try
		{
		switch(mPos)
			{
			case ESecModUICmdChange:
				QT_TRAP_THROWING(mSecModUIModel.ChangePinNrL(mPos));
				showSignatureView(EFalse);
				break;
			case ESecModUICmdUnblock:
				QT_TRAP_THROWING(mSecModUIModel.UnblockPinNrL(mPos));
				showSignatureView(EFalse);
				break;
			};
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}


