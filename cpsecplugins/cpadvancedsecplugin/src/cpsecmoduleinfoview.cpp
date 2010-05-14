
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
#include <HbMenu.h>
#include <hbaction.h>
#include <HbMessageBox.h>
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
#include <HbListView.h>
#include <QStandardItemModel>
#include <QModelIndexList>

#include <memory>

#include "cpsecmoduleinfoview.h"


CpSecmoduleInfoView::CpSecmoduleInfoView(const QVector< QPair<QString,QString> >& securityDetails, QGraphicsItem *parent /*= 0*/)
	: CpBaseSettingView(0,parent)
	{
	setTitle(tr("Module info"));
		
	std::auto_ptr<QGraphicsLinearLayout> layout(q_check_ptr(new QGraphicsLinearLayout(Qt::Vertical)));
  	HbListWidget* listSecurityDetails = q_check_ptr(new HbListWidget(this)); 
	
  	QVectorIterator< QPair<QString, QString> > secDetIter(securityDetails);
	while(secDetIter.hasNext())
		{
		QPair<QString,QString> pair = secDetIter.next();
		
		std::auto_ptr<HbListWidgetItem> label(q_check_ptr(new HbListWidgetItem()));
		label->setText(pair.first);
		listSecurityDetails->addItem(label.get());
		label.release();
		
		std::auto_ptr<HbListWidgetItem> description(q_check_ptr(new HbListWidgetItem()));
		description->setText(pair.second);
		listSecurityDetails->addItem(description.get());
		description.release();
		}
  	    
	layout->addItem(listSecurityDetails);		
	setLayout(layout.get());
	layout.release();
	}	

CpSecmoduleInfoView::~CpSecmoduleInfoView()
	 {}

// End of file

