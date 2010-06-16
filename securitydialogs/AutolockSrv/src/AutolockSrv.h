/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, version 2.1 of the License.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, 
* see "http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html/".
*
* Description:
*
*/

#ifndef AUTOLOCKSRV_H
#define AUTOLOCKSRV_H

#define RDEBUG( x, y ) RDebug::Printf( "%s %s (%u) %s=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, x, y );
// #define RDEBUG( x, y )

#include <QWidget>
#include <QEvent>
#include <QModelIndex>
#include <QMap>
#include <QStringList>
#include <xqserviceprovider.h>
#include <xqsharablefile.h>
#include <QToolButton>
#include <qmobilityglobal.h>

#include "autolockuseractivityservice.h"
#include "../../SecUi/inc/SecQueryUi.h"

QTM_BEGIN_NAMESPACE
    class QValueSpacePublisher;
    class QValueSpaceSubscriber;
QTM_END_NAMESPACE
QTM_USE_NAMESPACE

#include <hbwidget.h>
#include <qapplication.h>
#include <hbdevicedialog.h>

enum TLockStatus
	{
	ELockNotActive = 0,
	EKeyguardActive,
	EDevicelockActive
	};

enum TDevicelockReason
	{
	EDevicelockManual = 1,
	EDevicelockRemote,
	EDevicelockTimer
	};


class QLineEdit;
class QPushButton;
class AutolockSrvService;
class QLabel;
class HbLabel;
class XQSharableFile;

class AutolockSrv : public QWidget
{
    Q_OBJECT

public:
    AutolockSrv( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~AutolockSrv();

    void setLabelNumber(QString label,QString number);
    void setLabelIcon(int value);
    int CheckIfLegal(int value);
    void DebugRequest(int value);
    void DebugError(int value);
    void DebugStatus(int value);
    void adjustInactivityTimers(int aReason);
    int updateIndicator(int aReason);
    int AskValidSecCode(int aReason);
    int publishStatus(int aReason);
    int TryChangeStatus(int aReason);
    int setLockDialog(int aReason, int status);
    int showNoteIfRequested(int aReason);

    bool event(QEvent *event);    
    bool eventFilter(QObject *, QEvent *);

		int callerHasECapabilityWriteDeviceData;
		int iShowKeyguardNote;
    int mParam1;
    int mParam2;

public slots:    
    void endCall();
    void quit();
    void unlockAction();
    void unguardAction();
    void lockAction();
    void test1Action();
    void test2Action();
    void handleAnswerDelivered();
    void subscriberKSettingsAutolockStatusChanged();
    void subscriberKSettingsAutoLockTimeChanged();
    void subscriberKSettingsAutomaticKeyguardTimeChanged();
    void subscriberKDisplayLightsTimeoutChanged();
    void subscriberKProEngActiveProfileChanged();

private slots:
    void activeKeyguard();
    void notActiveKeyguard();
    void activeDevicelock();
    void notActiveDevicelock();

private:
    QLabel *mLabel;
    QLineEdit *mNumberEdit;
    QToolButton *mLabelIcon;
    int mLabelIcon_visible;
    //QPushButton *mEndCallButton;
    AutolockSrvService* mService;
    // int mKeyCode;
    // int mKeyCaptureHandle;
    QValueSpaceSubscriber *subscriberKSettingsAutolockStatus;
    QValueSpaceSubscriber *subscriberKSettingsAutoLockTime;
    QValueSpaceSubscriber *subscriberKSettingsAutomaticKeyguardTime;
    QValueSpaceSubscriber *subscriberKDisplayLightsTimeout;
    QValueSpaceSubscriber *subscriberKProEngActiveProfile;

		AutolockUserActivityService* serviceKeyguard;
		AutolockUserActivityService* serviceDevicelock;
		
		int iLockStatus;
		int iLockStatusPrev;
		CSecQueryUi *iSecQueryUi;
		int iSecQueryUiCreated;
		HbDeviceDialog *iDeviceDialog;
		int iDeviceDialogCreated;
};

class AutolockSrvService : public XQServiceProvider
{
    Q_OBJECT
public:
    AutolockSrvService( AutolockSrv *parent = 0 );
    ~AutolockSrvService();
    
    void complete(QString number);
    bool asyncAnswer() {return mAsyncAnswer;}
public slots:
    int service(const QString& number, const QString& aParam1, const QString& aParam2 );

private slots:
   void handleClientDisconnect();

private:
    AutolockSrv* mAutolockSrv;
    QString mNumber;
    bool mAsyncAnswer;
    int mAsyncReqId;
};

class  CWait : public CActive
    {
     public:
       /**
        * Creates instance of the CWait class.
        *
		* @return Returns the instance just created.
        */
		static CWait* NewL();
		/**
        * Destructor.
        */
        ~CWait();
    public:
        /**
		* Starts waiting for aReqStatus. 
		*/
		TInt WaitForRequestL();
    public:
        /**
        * Sets active request type. 
        */
        void SetRequestType(TInt aRequestType);
        /**
        * Gets active request type. 
        */
        TInt GetRequestType();
	private:
		/**
		* C++ default constructor.
		*/
		CWait();
		/**
		* Symbian OS constructor.
		*/
		void ConstructL();
	private: // from CActive
        /** @see CActive::RunL() */
		void RunL();
		/** @see CActive::DoCancel() */
        void DoCancel();
		RTimer iTimer;
		CActiveSchedulerWait iWait;
		// Used if there is a need to cancel an active request;
        // namely in situations where destructor is called when Wait
        // is active.
        TInt iRequestType;
	};


#endif // AUTOLOCKSRV_H