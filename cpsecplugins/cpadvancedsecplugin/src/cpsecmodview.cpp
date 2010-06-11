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
#include <hbabstractviewitem.h>
#include <hbmainwindow.h>
#include <HbMessageBox>

#include <memory>

#include "cpsecmodview.h"
#include "cpsecmodmodel.h"
#include "cpmoduleview.h"

CpSecModView::CpSecModView(TInt currentPos, CSecModUIModel& secModUIModel, QGraphicsItem *parent /*= 0*/)
: CpBaseSettingView(0,parent),
  mSecModUIModel(secModUIModel),
  mPos(currentPos),
  mCurrentView(EAccessView),
  mAccessView(NULL),
  mPrevView(NULL)
	{
	try
	{
	QT_TRAP_THROWING(mSecModUIModel.OpenTokenL(mPos));
	QString title = mSecModUIModel.TokenLabelForTitle();
	setTitle(title);
	
	TInt count = 0;
	// display code view
	QT_TRAP_THROWING(count = mSecModUIModel.CheckCodeViewStringsL());
	
	std::auto_ptr<QGraphicsLinearLayout> layout(q_check_ptr(new QGraphicsLinearLayout(Qt::Vertical)));
	HbListWidget* listSecView = q_check_ptr(new HbListWidget(this)); 
	
	std::auto_ptr<HbListWidgetItem> moduleWidget(q_check_ptr(new HbListWidgetItem()));
	moduleWidget->setText("\tModule PIN");
	listSecView->addItem(moduleWidget.get());
	moduleWidget.release();
	
	if( count == 2 )
		{
		std::auto_ptr<HbListWidgetItem> signingWidget(q_check_ptr(new HbListWidgetItem()));
		signingWidget->setText("\tSigning PIN");
		listSecView->addItem(signingWidget.get());
		signingWidget.release();
		}
	
	connect(listSecView, SIGNAL(activated(QModelIndex)), this, SLOT(showNextView(QModelIndex)));
	
	layout->addItem(listSecView);				
	setLayout(layout.get());
	layout.release();	
	}
	catch(const std::exception& exception)
		{
		QString error(exception.what());
		HbMessageBox::information(error);
		QT_RETHROW;
		}
	}

CpSecModView::~CpSecModView()
	{
	if(mAccessView)
		{
		mAccessView->deleteLater();
		mAccessView = NULL;
		}
	if(mPrevView)
		{
		mPrevView->deleteLater();
		mPrevView = NULL;
		}
	}


void CpSecModView::showNextView( const QModelIndex& modelIndex )
	{
	try
		{
		mAccessView = q_check_ptr(new CpModuleView((TSecModViews)modelIndex.row(),mSecModUIModel));
		QObject::connect(mAccessView , SIGNAL(aboutToClose()), this, SLOT(displayPrevious())); 
		mPrevView = mainWindow()->currentView();   //suppose iPreView  is member variable of CpSecurityView
		mainWindow()->addView(mAccessView);
		mainWindow()->setCurrentView(mAccessView);
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}

void CpSecModView::displayPrevious()
	{
	try
		{
		mainWindow()->removeView(mAccessView);
		}
	catch(const std::exception& exception)
		{
		HbMessageBox::information(exception.what());
		}
	}

