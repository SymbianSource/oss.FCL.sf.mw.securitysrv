/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
*
* Description: SecUi notification content widget.
*
*/

#include "secuinotificationcontentwidget.h"
#include "secuinotificationdialogpluginkeys.h"
#include <QGraphicsLinearLayout>
#include <QToolButton>
#include <hblabel.h>
#include <hbpushbutton.h>
#include <hbiconitem.h>
#include <hbcombobox.h>
#include <hblineedit.h>
#include <hbinputeditorinterface.h>
#include <QDebug>

#include <HbEmailAddressFilter>

#define ESecUiCancelSupported  0x1000000
#define ESecUiCancelNotSupported  0x0000000

#define ESecUiEmergencySupported  0x2000000
#define ESecUiEmergencyNotSupported  0x0000000

#define ESecUiAlphaSupported  0x4000000
#define ESecUiAlphaNotSupported  0x0000000

#define ESecUiSecretSupported  0x8000000
#define ESecUiSecretNotSupported  0x0000000

#define ESecUiMaskFlags  0xFF000000
#define ESecUiMaskType   0x00FFFFFF

#define ESecUiTypeDeviceLock		0x00100000
#define ESecUiTypeKeyguard			0x00200000
#define ESecUiTypeClock  				0x00300000
#define ESecUiTypeScreensaver		0x00400000

#define ESecUiTypeMaskLock			0x00F00000


// ----------------------------------------------------------------------------
// SecUiNotificationContentWidget::SecUiNotificationContentWidget()
// ----------------------------------------------------------------------------
//
SecUiNotificationContentWidget::SecUiNotificationContentWidget(
        QGraphicsItem *parent, Qt::WindowFlags flags) : HbWidget(parent, flags)
{
		qDebug() << "SecUiNotificationContentWidget::SecUiNotificationContentWidget";
}

// ----------------------------------------------------------------------------
// SecUiNotificationContentWidget::~SecUiNotificationContentWidget()
// ----------------------------------------------------------------------------
//
SecUiNotificationContentWidget::~SecUiNotificationContentWidget()
{
}

// ----------------------------------------------------------------------------
// SecUiNotificationContentWidget::constructFromParameters()
// ----------------------------------------------------------------------------
//
void SecUiNotificationContentWidget::constructFromParameters(const QVariantMap &parameters)
{
		qDebug() << "SecUiNotificationContentWidget::constructFromParameters 1";
		qDebug() << parameters;
    QGraphicsLinearLayout *mainLayout = new QGraphicsLinearLayout(Qt::Vertical);

    lMinLength = 4;	// might be replaced later
    lMaxLength = 8;	// might be replaced later
 		queryDual=0;
 		isEmergency=0;

    // KApplicationSize
    if (parameters.contains(KQueryType)) {
				qDebug() << "SecUiNotificationContentWidget::KQueryType";
        queryType = parameters.value(KQueryType).toUInt();
				qDebug() << queryType;
				if( (queryType & ESecUiTypeMaskLock) )
					{
					qDebug() << "SecUiNotificationContentWidget::KQueryType=ESecUiTypeLock";
					// showing "Lock" icon. All other params are irrelevant. codeTop is not even created

					
        	HbLabel *iconLabel = new HbLabel("Locked");
        	HbIcon *icon = new HbIcon("qtg_large_device_lock");
        	// iconLabel->setAspectRatioMode(Qt::IgnoreAspectRatio);
    			// iconLabel->setGeometry(QRectF(QPointF(10,10),QSizeF(300,300)));
        	iconLabel->setIcon(*icon);
        	if( (queryType & ESecUiTypeMaskLock)==ESecUiTypeDeviceLock )
        		{	// really big icon for the devicelock
	        	iconLabel->setPreferredHeight(500);
  	      	iconLabel->setPreferredWidth(500);
  	      	}
        	else if( (queryType & ESecUiTypeMaskLock)==ESecUiTypeKeyguard )
        		{	// smaller icon for the keyguard
	        	iconLabel->setPreferredHeight(100);
  	      	iconLabel->setPreferredWidth(100);
  	      	}

        	// icon->setWidth(300);
        	// icon->setHeight(350);
        	// icon->setGeometry(QRectF(QPointF(10,10),QSizeF(500,300)));
        	// icon->setSize(QSizeF(300,300));
        	
        	mainLayout->addItem(iconLabel);
        	mainLayout->setAlignment(iconLabel, Qt::AlignCenter );
        	// mainLayout->setGeometry(QRectF(QPointF(10,10),QSizeF(300,300)));
        	
					/*
					QToolButton* mLabelIcon = new QToolButton;
					mLabelIcon->setIcon(QIcon(":/AutolockSrv_hbicon/qtg_large_device_lock.svg"));
					mLabelIcon->setIconSize(QSize(300,300));
					HbLabel *iconLabel = new HbLabel("Locked");
					iconLabel->setIcon(*mLabelIcon);
					mainLayout->addItem(iconLabel);
					*/

        	// mainLayout->setContentsMargins(10,10,300,500);	// this makes the dialog really big
					setLayout(mainLayout);	// same as at the end
					return;
					}
				// not ESecUiTypeMaskLock
				lEmergencySupported = ESecUiEmergencyNotSupported;
				if((queryType & ESecUiEmergencySupported)==ESecUiEmergencySupported)
					{
					lEmergencySupported = ESecUiEmergencySupported;
					}
				qDebug() << "SecUiNotificationContentWidget::lEmergencySupported =" << lEmergencySupported;
    }

    if (parameters.contains(KQueryMinLength)) {
				qDebug() << "SecUiNotificationContentWidget::KQueryMinLength";
        lMinLength = parameters.value(KQueryMinLength).toUInt();
				qDebug() << lMinLength;
    }
    if (parameters.contains(KQueryMaxLength)) {
				qDebug() << "SecUiNotificationContentWidget::KQueryMaxLength";
        lMaxLength = parameters.value(KQueryMaxLength).toUInt();
				qDebug() << lMaxLength;
    }

    if (parameters.contains(KEmergency)) {
				qDebug() << "SecUiNotificationContentWidget::KEmergency";
        QString emergencyText = parameters.value(KEmergency).toString();
        qDebug() << emergencyText;
        if(!emergencyText.compare("emergencyYes"))
        	{
        	qDebug() << "SecUiNotificationContentWidget::KEmergency emergencyYes";
        	isEmergency = 1;
        	}
        if(!emergencyText.compare("emergencyNo"))
        	{
        	qDebug() << "SecUiNotificationContentWidget::KEmergency emergencyNo";
        	isEmergency = 0;
        	}
    }


    // KCodeTop
    if (parameters.contains(KCodeTop)) {
				qDebug() << "SecUiNotificationContentWidget::KCodeTop 1";
        codeTop = new HbLineEdit("");	// no default value
        qDebug() << "SecUiNotificationContentWidget::KCodeTop lMaxLength=";
        qDebug() << lMaxLength;
        if(lMaxLength>2)
	        codeTop->setMaxLength(lMaxLength);
        // HbLineEdit *codeTop2 = new HbLineEdit;
				qDebug() << "SecUiNotificationContentWidget::KCodeTop 2";
				qDebug() << "SecUiNotificationContentWidget::KCodeTop queryType=";
				qDebug() << queryType;
				codeTop->setInputMethodHints(Qt::ImhDigitsOnly);	// default
 		    if (queryType & ESecUiAlphaSupported)
		    	{
		    	qDebug() << "SecUiNotificationContentWidget::KCodeTop setUpAsLatinAlphabetOnlyEditor";
 	    		codeTop->setInputMethodHints(Qt::ImhNone);
		  		}
 		    if (queryType & ESecUiSecretSupported)
		    	{
		    	qDebug() << "SecUiNotificationContentWidget::KCodeTop ESecUiSecretSupported";
 	    		codeTop->setEchoMode(HbLineEdit::PasswordEchoOnEdit);
 	    		// note that codeButtom is never in secret mode. This nevertheless is restricted by the caller.
		  		}
				qDebug() << "SecUiNotificationContentWidget::KCodeTop 3";
				codeTop->setMaxLength(lMaxLength);
				
				if (parameters.contains(KDefaultCode)) {
					qDebug() << "SecUiNotificationContentWidget::KDefaultCode";
  	      QString defaultCode = parameters.value(KDefaultCode).toString();
    	    qDebug() << defaultCode;
					codeTop->setText(defaultCode);
					}
				qDebug() << "SecUiNotificationContentWidget::KCodeTop 4";

        connect(codeTop, SIGNAL(textChanged(const QString &)), this, SIGNAL(codeTopChanged(const QString &)));
        connect(codeTop, SIGNAL(contentsChanged()), this, SIGNAL(codeTopContentChanged()));

				if (parameters.contains(KDefaultCode)) {	// this is done in this step so that the OK becomes valid (if rules are fulfilled)
					qDebug() << "SecUiNotificationContentWidget::KDefaultCode";
  	      QString defaultCode = parameters.value(KDefaultCode).toString();
    	    qDebug() << defaultCode;
					codeTop->setText(defaultCode);
					}
				qDebug() << "SecUiNotificationContentWidget::KCodeTop 4";

    		mainLayout->addItem(codeTop);
    		// double-query
    		if (parameters.contains(KCodeBottom))
    			{
    			queryDual=1;
    			QString titleText = parameters.value(KDialogTitle).toString();
    			if(titleText.indexOf('|')>0)
    				{	// if no separator, don't create label
    				QString titleBottomStr = titleText.right(titleText.length()-titleText.indexOf('|')-1);
    				HbLabel *titleBottom = new HbLabel(titleBottomStr);
    				mainLayout->addItem(titleBottom);
    				}
    			
        	codeBottom = new HbLineEdit("");	// no default value
	        if(lMaxLength>2)
		        codeBottom->setMaxLength(lMaxLength);
	    		codeBottom->setInputMethodHints(Qt::ImhDigitsOnly);	// default
	 		    if (queryType & ESecUiAlphaSupported)
			    	{
			    	qDebug() << "SecUiNotificationContentWidget::KCodeBottom setUpAsLatinAlphabetOnlyEditor";
	 	    		codeBottom->setInputMethodHints(Qt::ImhNone);
			  		}
					qDebug() << "SecUiNotificationContentWidget::KCodeBottom 3";
	        connect(codeBottom, SIGNAL(textChanged(const QString &)), this, SIGNAL(codeBottomChanged(const QString &)));
        	connect(codeBottom, SIGNAL(contentsChanged(const QString &)), this, SIGNAL(codeBottomChanged(const QString &)));
	    		mainLayout->addItem(codeBottom);
	    		}

    		QGraphicsLinearLayout *mainLayoutButtons = new QGraphicsLinearLayout(Qt::Horizontal);
        HbPushButton *but1 = new HbPushButton("1234");
        HbPushButton *but2 = new HbPushButton("+1");
        HbPushButton *but3 = new HbPushButton("+5");
        connect(but1, SIGNAL(clicked()), this, SIGNAL(but1Changed()));
        connect(but2, SIGNAL(clicked()), this, SIGNAL(but2Changed()));
        connect(but3, SIGNAL(clicked()), this, SIGNAL(but3Changed()));
        #if defined(_DEBUG)
        mainLayoutButtons->addItem(but1);
        mainLayoutButtons->addItem(but2);
        mainLayoutButtons->addItem(but3);
        #endif

        mainLayout->addItem(mainLayoutButtons);
        
        codeTop->setFocus();	// this should open the VKB

    }

    setLayout(mainLayout);
    }

