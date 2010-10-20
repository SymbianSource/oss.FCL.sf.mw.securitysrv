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

#include "secuinotificationdebug.h"
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

#include <HbCheckBox>  // needed for a checkbox dialog
#include <HbListWidget>  // needed for multicheckbox dialog
#include <HbListWidgetItem>
#include <HbAbstractItemView>
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
		RDEBUG("0", 0);
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
		RDEBUG("0", 0);
		RDEBUGQT("parameters", parameters);
    QGraphicsLinearLayout *mainLayout = new QGraphicsLinearLayout(Qt::Vertical);

    lMinLength = 4;	// might be replaced later
    lMaxLength = 8;	// might be replaced later
 		queryDual=0;
 		isEmergency=0;
       codeTop=0;

    // KApplicationSize
    if (parameters.contains(KQueryType)) {
				RDEBUG("0", 0);
        queryType = parameters.value(KQueryType).toUInt();
				RDEBUG("queryType", queryType);
				if( (queryType & ESecUiTypeMaskLock) )
					{
					RDEBUG("KQueryType=ESecUiTypeMaskLock", queryType);
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
				RDEBUG("lEmergencySupported", lEmergencySupported);
    }

    if (parameters.contains(KQueryMinLength)) {
				RDEBUG("KQueryMinLength", 0);
        lMinLength = parameters.value(KQueryMinLength).toUInt();
				RDEBUG("lMinLength", lMinLength);
    }
    if (parameters.contains(KQueryMaxLength)) {
				RDEBUG("KQueryMaxLength", 0);
        lMaxLength = parameters.value(KQueryMaxLength).toUInt();
				RDEBUG("lMaxLength", lMaxLength);
    }

    if (parameters.contains(KEmergency)) {
				RDEBUG("KEmergency", 0);
        QString emergencyText = parameters.value(KEmergency).toString();
        RDEBUGQT("emergencyText", emergencyText);
        if(!emergencyText.compare("emergencyYes"))
        	{
					RDEBUG("emergencyYes", 1);
        	isEmergency = 1;
        	}
        if(!emergencyText.compare("emergencyNo"))
        	{
					RDEBUG("emergencyNo", 0);
        	isEmergency = 0;
        	}
    }


    // KCodeTop
    if (parameters.contains(KCodeTop)) {
				RDEBUG("KCodeTop", 0);
        codeTop = new HbLineEdit("");	// no default value
				RDEBUG("queryType", queryType);
				codeTop->setInputMethodHints(Qt::ImhDigitsOnly);	// default
 		    if (queryType & ESecUiAlphaSupported)
		    	{
					RDEBUG("ESecUiAlphaSupported", ESecUiAlphaSupported);
 	    		codeTop->setInputMethodHints(Qt::ImhPreferNumbers);	// all queries start with numbers, even if allow alpha. (old value was ImhNone)
		  		}
 		    if (queryType & ESecUiSecretSupported)
		    	{
					RDEBUG("ESecUiSecretSupported", ESecUiSecretSupported);
 	    		codeTop->setEchoMode(HbLineEdit::Password);	// instead of PasswordEchoOnEdit
		  		}
				RDEBUG("lMaxLength", lMaxLength);
        if(lMaxLength>2)
	        codeTop->setMaxLength(lMaxLength);
				
				if (parameters.contains(KDefaultCode)) {
					RDEBUG("KDefaultCode", 0);
  	      QString defaultCode = parameters.value(KDefaultCode).toString();
    	    RDEBUGQT("defaultCode", defaultCode);
					codeTop->setText(defaultCode);
					}
				RDEBUG("1", 1);

        connect(codeTop, SIGNAL(textChanged(const QString &)), this, SIGNAL(codeTopChanged(const QString &)));
        connect(codeTop, SIGNAL(contentsChanged()), this, SIGNAL(codeTopContentChanged()));

				if (parameters.contains(KDefaultCode)) {	// this is done in this step so that the OK becomes valid (if rules are fulfilled)
					RDEBUG("KDefaultCode", 0);
  	      QString defaultCode = parameters.value(KDefaultCode).toString();
    	    RDEBUGQT("defaultCode", defaultCode);
					codeTop->setText(defaultCode);
					}
				RDEBUG("2", 2);

		    if (parameters.contains(KDialogTitle)) {
		        QString titleText = parameters.value(KDialogTitle).toString();
		        QString titleAttempts = "";
		   			if(titleText.indexOf('|')>0)
		    				{	// if separator, take only first part
		    				titleText = titleText.left(titleText.indexOf('|'));
		    				}
		   			if(titleText.indexOf('$')>0)
		    				{	// if separator, take only first part
		    					// it comes translated alredy, from the client
		    				titleAttempts = titleText.right(titleText.length()-titleText.indexOf('$')-1);
		    				RDEBUGQT("titleAttempts", titleAttempts);
		    				int nAttempts = titleAttempts.toInt();
		    				RDEBUG("nAttempts", nAttempts);
		    				titleText = titleText.left(titleText.indexOf('$'));	// this comes translated already
								if(nAttempts==1)	// last attempt
		    					{
		    					// This must be in a new line, to avoid problems with left-to-right writing
		    					titleAttempts = hbTrId("txt_pin_code_dpophead_last_attempt");
			    				}
			    			else if(nAttempts>0)
		    					{
		    					titleAttempts = hbTrId("attempts") + "=" + QString::number(nAttempts);
		    					// TODO this requires %L1 localization using txt_pin_code_dpopinfo_ln_attempts_remaining
			    				}
			    			else if(nAttempts==0)
		    					{
		    					titleAttempts = "";	// something special for the last attempt ?
			    				}
				    		else
				    			{
				    			titleAttempts = "";
				    			}
				    		}
		        HbLabel *titleTop = new HbLabel(titleText);
		        mainLayout->addItem(titleTop);
		        if (titleAttempts.length()>0 )
		        	{
		        	HbLabel *titleTopAttemps = new HbLabel(titleAttempts);
		        	mainLayout->addItem(titleTopAttemps);
		        	}
		        // in the dialog, it was setHeadingWidget(title);
		    }
				RDEBUG("3", 3);
    		mainLayout->addItem(codeTop);
    		// double-query
    		if (parameters.contains(KCodeBottom))
    			{
    			queryDual=1;
    			QString titleText = parameters.value(KDialogTitle).toString();
    			if(titleText.indexOf('|')>0)
    				{	// if no separator, don't create label
    				QString titleBottomStr = titleText.right(titleText.length()-titleText.indexOf('|')-1);
    				// no need to translate
    				HbLabel *titleBottom = new HbLabel(titleBottomStr);
    				mainLayout->addItem(titleBottom);
    				}
    			
        	codeBottom = new HbLineEdit("");	// no default value
	        if(lMaxLength>2)
		        codeBottom->setMaxLength(lMaxLength);
	    		codeBottom->setInputMethodHints(Qt::ImhDigitsOnly);	// default
	 		    if (queryType & ESecUiAlphaSupported)
			    	{
			    	RDEBUG("setInputMethodHints", 0);
	 	    		codeBottom->setInputMethodHints(Qt::ImhPreferNumbers);	// all queries start with numbers, even if allow alpha. (old value was ImhNone)
			  		}
	 		    if (queryType & ESecUiSecretSupported)
			    	{
			    	RDEBUG("ESecUiSecretSupported", 0);
	 	    		codeBottom->setEchoMode(HbLineEdit::Password);	// this will also remove predictive, which is what we want 	(don't use PasswordEchoOnEdit)
			  		}
		    	RDEBUG("connecting", 0);
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
        
        // remove setFocus from PIN query because Orbit can't handle it
        // codeTop->setFocus();	// this should open the VKB

    }

    if (parameters.contains(KChecboxType)) {
	    	RDEBUG("KChecboxType", 0);
        if (parameters.contains(KDialogTitle)) {
            //TODO position of the label is not centered
            QString tmpText=parameters.value(KDialogTitle).toString();
            if (tmpText.endsWith("\n"))  tmpText=tmpText.left(tmpText.length()-1);
            DialogText= new HbLabel(tmpText);   
            DialogText->setTextWrapping(Hb::TextWordWrap);
            DialogText->setAlignment(Qt::AlignVCenter);
            mainLayout->addItem(DialogText);                   
        }

        checkbox = new HbCheckBox("Caption");   
        if (parameters.contains(KDefaultCode)) {
	    			RDEBUG("KDefaultCode", 0);
            QStringList list1 = parameters.value(KDefaultCode).toString().split("|");
            if (!list1.isEmpty() && list1.count()==2) {
                if (!list1[0].isNull() && !list1[0].isEmpty()) checkbox->setText(list1[0]);
                if (!list1[1].isNull() && !list1[1].isEmpty()) checkbox->setChecked(list1[1].toInt());
            }
            // TODO this needs localization
        }
        mainLayout->addItem(checkbox);
    }

    if (parameters.contains(KMultiChecboxType) && parameters.contains(KDefaultCode)) 
    	{
   			RDEBUG("KMultiChecboxType", 0);
        QStringList list1 = parameters.value(KDefaultCode).toString().split("1\t");
        if (!list1.isEmpty()) {
            listWidget = new HbListWidget();
            for (int i = 0; (i < list1.count()); i++)
                if (!list1[i].isEmpty() && !list1[i].isNull()) {
                    HbListWidgetItem* modelItem = new HbListWidgetItem();
                    modelItem->setData(QVariant(list1[i]), Qt::DisplayRole);
            				// TODO this needs localization
                    listWidget->addItem(modelItem);  
                }
            listWidget->setCurrentRow(0);
            listWidget->setSelectionMode(HbAbstractItemView::MultiSelection);
            //listWidget->setClampingStyle(HbScrollArea::BounceBackClamping);
            listWidget->setVerticalScrollBarPolicy(HbScrollArea::ScrollBarAsNeeded);
            listWidget->setMaximumHeight(150);
            mainLayout->addItem(listWidget); 
            //     delete listWidget;
            }
        }

    setLayout(mainLayout);
    }

