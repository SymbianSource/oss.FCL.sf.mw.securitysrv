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

#ifndef AUTOLOCK_H
#define AUTOLOCK_H

#ifdef _DEBUG
		#define RDEBUG( x, y ) RDebug::Printf( "%s %s (%u) %s=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, x, y );
#else
    #define RDEBUG( x, y )
#endif

#include <QWidget>
#include <QEvent>
#include <QModelIndex>
#include <QMap>
#include <QStringList>
#include <xqserviceprovider.h>
#include <QToolButton>
#include <qmobilityglobal.h>

#include "autolockuseractivityservice.h"
#include "../../SecUi/Inc/SecQueryUi.h"

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

class QTimer;

class AutolockService;

class Autolock : public QWidget
{
    Q_OBJECT

public:
    Autolock( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~Autolock();

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
    void quit();
    void handleAnswerDelivered();
    void subscriberKSettingsAutolockStatusChanged();
    void subscriberKSettingsAutoLockTimeChanged();
    void subscriberKSettingsAutomaticKeyguardTimeChanged();
    void subscriberKDisplayLightsTimeoutChanged();
    void subscriberKProEngActiveProfileChanged();
    void subscriberKAknKeyguardStatusChanged();
    void subscriberKCoreAppUIsAutolockStatusChanged();
    void subscriberKHWRMGripStatusChanged();
    void subscriberKSecurityUIsDismissDialogChanged();

private slots:
    void activeKeyguard();
    void notActiveKeyguard();
    void activeDevicelock();
    void notActiveDevicelock();
    void switchScreensaverToActiveMode();
    void switchScreensaverToPowerSaveMode();
    void handleMessageFromScreensaver( const QVariantMap &data );
    void handleLockSwitch();

private:
    AutolockService* mService;
    // int mKeyCode;
    // int mKeyCaptureHandle;
    QValueSpaceSubscriber *subscriberKSettingsAutolockStatus;
    QValueSpaceSubscriber *subscriberKSettingsAutoLockTime;
    QValueSpaceSubscriber *subscriberKSettingsAutomaticKeyguardTime;
    QValueSpaceSubscriber *subscriberKDisplayLightsTimeout;
    QValueSpaceSubscriber *subscriberKProEngActiveProfile;
    QValueSpaceSubscriber *subscriberKAknKeyguardStatus;
    QValueSpaceSubscriber *subscriberKCoreAppUIsAutolockStatus;
    QValueSpaceSubscriber *subscriberKHWRMGripStatus;
    QValueSpaceSubscriber *subscriberKSecurityUIsDismissDialog;

		AutolockUserActivityService* serviceKeyguard;
		AutolockUserActivityService* serviceDevicelock;
		
		int iLockStatus;
		int iLockStatusPrev;
		CSecQueryUi *iSecQueryUi;
		int iSecQueryUiCreated;
		HbDeviceDialog *iDeviceDialog;
		int iDeviceDialogCreated;
    TInt32 mPowerKeyCaptureHandle;
    TInt32 mApplicationKeyCaptureHandle;
    TInt32 mApplicationLongKeyCaptureHandle;
    
    QTimer *mScreensaverModeTimer;
};

class AutolockService : public XQServiceProvider
{
    Q_OBJECT
public:
    AutolockService( Autolock *parent = 0 );
    ~AutolockService();
    
    void complete(QString number);
    bool asyncAnswer() {return mAsyncAnswer;}
public slots:
    int service(const QString& number, const QString& aParam1, const QString& aParam2 );

private slots:
   void handleClientDisconnect();

private:
    Autolock* mAutolock;
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


#endif // AUTOLOCK_H
