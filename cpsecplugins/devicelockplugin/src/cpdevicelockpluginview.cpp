/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "cpdevicelockpluginview.h"
#include <QStringList>
#include <hblineedit.h>
#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>
#include "cpremotelockdataformviewitem.h"
#include <hblabel.h>
#include <hbpushbutton.h>
#include <HbMessageBox>
#include <hbdataform.h>
#include <QGraphicsLinearLayout>
#include <secuisecuritysettings.h>
#include <secuisecurityhandler.h>
#include <etelmm.h>
#include <rmmcustomapi.h>
#include <hbinputdialog.h>
#include <secui.h>
#include <RemoteLockSettings.h>
#include <e32cmn.h>
#include <centralrepository.h>
#include <settingsinternalcrkeys.h>
#include <hbcombobox.h>
#include<hbaction.h>
#include <RemoteLockSettings.h>
#include "debug.h"
#include <qapplication.h>
#include <qtranslator.h>
#include <../../inc/cpsecplugins.h>

   
        
/*
 *****************************************************************
 * Name        : CpDeviceLockPluginView()
 * Parameters  : QGraphicsItem*
 * Return value: None
 * Description : constructor
 *****************************************************************
 */
CpDeviceLockPluginView::CpDeviceLockPluginView(QGraphicsItem *parent /*= 0*/)
: CpBaseSettingView(0,parent)
    {
    QTranslator *translator = new QTranslator();
    QString lang = QLocale::system().name();
    QString path = "Z:/resource/qt/translations/";
    bool fine = translator->load("devicelocking_en.qm", path);
    if (fine)
        qApp->installTranslator(translator);

    QTranslator *commontranslator = new QTranslator();

    fine = commontranslator->load("common_" + lang, path);
    if (fine)
        qApp->installTranslator(commontranslator);

    TSecUi::InitializeLibL();
    mUiSecuSettings = CSecuritySettings::NewL();
    mRemoteLockSettings = CRemoteLockSettings::NewL();
    iALPeriodRep = CRepository::NewL(KCRUidSecuritySettings);

    mRemoteLockSettingClicked = false;
    mHack = 0;
    HbDataForm *form = qobject_cast<HbDataForm*> (widget());
    if (form)
        {
        QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(
                Qt::Vertical);
        QList<HbAbstractViewItem *> protoTypeList = form->itemPrototypes();
        protoTypeList.append(new CpRemoteLockDataFormViewItem());
        form->setItemPrototypes(protoTypeList);
        form->setHeading(hbTrId("txt_cp_dblist_device_lock"));

        //DataFormModel
        formModel = new HbDataFormModel();

        //lockcode
        HbDataFormModelItem
                *lockCodeItem =
                        new HbDataFormModelItem(
                                static_cast<HbDataFormModelItem::DataItemType> (CpRemoteLockDataFormViewItem::CpCodeEditItem),
                                hbTrId("txt_devicelocking_dialog_lock_code"));
        lockCodeItem->setContentWidgetData("echomode", HbLineEdit::Password);
        lockCodeItem->setContentWidgetData("text", "1234");
        lockCodeItem->setContentWidgetData("readonly", true);
        form->addConnection(lockCodeItem, SIGNAL(clicked()), this,
                SLOT(onLockCodeClicked()));
        formModel->appendDataFormItem(lockCodeItem);

        //Autolock period
        mAutolockPeriodItem = new HbDataFormModelItem(
                HbDataFormModelItem::ComboBoxItem, hbTrId(
                        "txt_devicelocking_formlabel_automatic_locking"));
        TInt autoLockVal;
        TInt err = iALPeriodRep->Get(KSettingsAutoLockTime, autoLockVal);
        TInt index = GetAutoLockIndex(autoLockVal);
        Dprint((_L("Current AL period value %d"),autoLockVal));
        RDEBUG("err", err);
        RDEBUG("index", index);
        RDEBUG("autoLockVal", autoLockVal);
        //TODO: need to set autoLockVal in editor
        QStringList autolockPeriodList;
        autolockPeriodList << hbTrId("txt_devicelocking_button_off")
                << hbTrId("txt_devicelocking_setlabel_val_when_keys_screen")
                << hbTrId("txt_devicelocking_setlabel_val_5_minutes")
                << hbTrId("txt_devicelocking_setlabel_val_30_minutes")
                << hbTrId("txt_devicelocking_setlabel_val_60_minutes");
        mAutolockPeriodItem->setContentWidgetData(QString("items"),
                autolockPeriodList);
        mAutolockPeriodItem->setContentWidgetData(QString("currentIndex"),
                index);
        mAutolockPeriodItem->setContentWidgetData(QString("editable"), true);
        QVariant themeComboData = mAutolockPeriodItem->contentWidgetData(
                QString("currentIndex"));
        mThemeComboPrevIndex = themeComboData.toInt();

        form->addConnection(mAutolockPeriodItem,
                SIGNAL(currentIndexChanged(int)), this,
                SLOT(onAutoLockChanged(int)));
        //form->addConnection(mAutolockPeriodItem,SIGNAL(editTextChanged(const QString&)), this, SLOT(onAutoTextChanged(const QString&)));
        formModel->appendDataFormItem(mAutolockPeriodItem);

        //LockWhenSimChange
        mLockWhenSimChangeItem = new HbDataFormModelItem(
                HbDataFormModelItem::ToggleValueItem, hbTrId(
                        "txt_devicelocking_formlabel_lock_when_sim_changed"));
        TBool lockVal = mUiSecuSettings->IsLockEnabledL(
                RMobilePhone::ELockPhoneToICC);
        Dprint((_L("LockWhenSimChange enabled %d"),lockVal));
        RDEBUG("lockVal", lockVal);
        if (lockVal)
            {
            mLockWhenSimChangeItem->setContentWidgetData("text", hbTrId(
                    "txt_remotelocking_button_sim_changed_on"));
            mLockWhenSimChangeItem->setContentWidgetData("additionalText",
                    hbTrId("txt_devicelocking_button_sim_changed_off"));
            }
        else
            {
            mLockWhenSimChangeItem->setContentWidgetData("text", hbTrId(
                    "txt_devicelocking_button_sim_changed_off"));
            mLockWhenSimChangeItem->setContentWidgetData("additionalText",
                    hbTrId("txt_remotelocking_button_sim_changed_on"));
            }
        mLockWhenSimChangeItem->setContentWidgetData("readonly", true);
        mPrevSIMLockData = mLockWhenSimChangeItem->contentWidgetData(QString(
                "text"));

        connect(formModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
                this, SLOT(onSIMLockDataChanged(QModelIndex,QModelIndex)));
        formModel->appendDataFormItem(mLockWhenSimChangeItem);

        //Remote Lock Setting
        TBool enabled = true;
        TBool val = mRemoteLockSettings->GetEnabled(enabled);
        Dprint((_L("Remote Lock Setting enabled %d"),val));
        RDEBUG("val", val);
        mDeviceRemoteLockItem = new HbDataFormModelItem(
                HbDataFormModelItem::ToggleValueItem, hbTrId(
                        "txt_devicelocking_subhead_remote_locking"));

        if (val && enabled)
            {
            mDeviceRemoteLockItem->setContentWidgetData("text", hbTrId(
                    "txt_devicelocking_button_remote_on"));
            mDeviceRemoteLockItem->setContentWidgetData("additionalText",
                    hbTrId("txt_devicelocking_button_remote_off"));
            }
        else
            {
            mDeviceRemoteLockItem->setContentWidgetData("text", hbTrId(
                    "txt_devicelocking_button_remote_off"));
            mDeviceRemoteLockItem->setContentWidgetData("additionalText",
                    hbTrId("txt_devicelocking_button_remote_on"));
            }

        mPrevRemLockData = mDeviceRemoteLockItem->contentWidgetData(QString(
                "text"));
        QString s = mPrevRemLockData.toString();
        connect(formModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
                this, SLOT(onRemoteLockDataChanged(QModelIndex,QModelIndex)));
        formModel->appendDataFormItem(mDeviceRemoteLockItem);

        //Remote lock message
        mRemoteLockMessageItem
                = new HbDataFormModelItem(
                        static_cast<HbDataFormModelItem::DataItemType> (CpRemoteLockDataFormViewItem::CpCodeEditItem),
                        hbTrId("txt_devicelocking_formlabel_locking_message"));
        mRemoteLockMessageItem->setContentWidgetData("echoMode",
                HbLineEdit::Password);
        mRemoteLockMessageItem->setContentWidgetData("text", "1111");
        mRemoteLockMessageItem->setContentWidgetData("readOnly", true);

        if (mPrevRemLockData.toString() == hbTrId(
                "txt_devicelocking_button_remote_on"))
            mRemoteLockMessageItem->setEnabled(true);
        else
            mRemoteLockMessageItem->setEnabled(false);

        form->addConnection(mRemoteLockMessageItem, SIGNAL(clicked()), this,
                SLOT(onLockMessageClicked()));
        formModel->appendDataFormItem(mRemoteLockMessageItem);

        form->setModel(formModel);
        layout->addItem(form);
        setLayout(layout);
        }
    }


/*
 *****************************************************************
 * Name        : ~CpDeviceLockPluginView()
 * Parameters  : None
 * Return value: None
 * Description : destructor
 *****************************************************************
 */
CpDeviceLockPluginView::~CpDeviceLockPluginView()
    {
    TSecUi::UnInitializeLib();
}


/*
 *****************************************************************
 * Name        : onLockCodeClicked()
 * Parameters  : None
 * Return value: None
 * Description : Enables user to change the lock code
 *****************************************************************
 */
void CpDeviceLockPluginView::onLockCodeClicked()
    {
    Dprint(_L("CpDeviceLockPluginView::onLockCodeClicked()..Enter"));
    RDEBUG("0", 0);
    mUiSecuSettings->ChangeSecCodeL();
    Dprint(_L("CpDeviceLockPluginView::onLockCodeClicked()..Exit"));
}


/*
 ************************************************************************
 * Name        : onAutoLockChanged()
 * Parameters  : int
 * Return value: None
 * Description : handles the data when automatic lock timings are changed
 ************************************************************************
 */
void CpDeviceLockPluginView::onAutoLockChanged(int index)
    {
    Dprint(_L("CpDeviceLockPluginView::onAutoLockChanged()..Enter"));
    RDEBUG("index", index);
    if (index != mThemeComboPrevIndex)
        {
        //TODO: need to set user entered/selected value
        TInt lockValue = GetValueAtIndex(index);
        TInt newAutoLockVal = mUiSecuSettings->ChangeAutoLockPeriodL(
                lockValue);
        RDEBUG("newAutoLockVal", newAutoLockVal);
        RDEBUG("lockValue", lockValue);
        if (newAutoLockVal == lockValue)
            {
            Dprint(_L("onAutoLockChanged().AL setting success !!"));
            RDEBUG("success", 1);
            TInt err = iALPeriodRep->Set(KSettingsAutoLockTime, lockValue);
            RDEBUG("err", err);
            mThemeComboPrevIndex = index;
            }
        else
            {
            Dprint(_L("onAutoLockChanged()..Setting to previous value"));
            RDEBUG("previous", 0);
            QVariant data(mThemeComboPrevIndex);
            mAutolockPeriodItem->setContentWidgetData(
                    QString("currentIndex"), data);
            }

        }
    Dprint(_L("CpDeviceLockPluginView::onAutoLockChanged()..Exit"));
    RDEBUG("0", 0);
}


/*
 *************************************************************************
 * Name        : onLockMessageClicked()
 * Parameters  : None
 * Return value: None
 * Description : verifies security code and enables user to change the lock 
 *               message
 **************************************************************************
 */
void CpDeviceLockPluginView::onLockMessageClicked()
    {
    Dprint(_L("CpDeviceLockPluginView::onLockMessageClicked()..Exit"));
    RDEBUG("0", 0);
    TBuf<KRLockMaxLockCodeLength> remoteLockCode;
    TBool remoteLockStatus(EFalse);
    TInt retVal = KErrNone;
    TInt autoLockVal = -1;
    retVal = mUiSecuSettings->ChangeRemoteLockStatusL(remoteLockStatus,
            remoteLockCode, autoLockVal);
    RDEBUG("retVal", retVal);
    RDEBUG("autoLockVal", autoLockVal);
    RDEBUG("remoteLockStatus", remoteLockStatus);
    if (retVal)
        {
        Dprint(_L("CpDeviceLockPluginView::onLockMessageClicked()..ChangeRemoteLockStatusL sucess"));
        RDEBUG("sucess", 1);
        }
    else
        {
        Dprint(_L("CpDeviceLockPluginView::onLockMessageClicked()..ChangeRemoteLockStatusL failed"));
        RDEBUG("failed", 0);
        }
    Dprint(_L("CpDeviceLockPluginView::onLockMessageClicked()..Exit"));
    RDEBUG("0", 0);
}


/*
 *************************************************************************
 * Name        : onAutoTextChanged()
 * Parameters  : QString&
 * Return value: None
 * Description :  
  **************************************************************************
 */
/*
 *This slot can be enabled once fix from obit team for this siganl is available 
 */

#if 0
void CpDeviceLockPluginView::onAutoTextChanged(const QString& aText)
    {
    Dprint(_L("CpDeviceLockPluginView::onAutoTextChanged()..Enter"));
    RDEBUG("0", 0);
    //TBool ret = DisplaySecurityDialog(); 
    Dprint(_L("CpDeviceLockPluginView::onAutoTextChanged()..Exit"));
    RDEBUG("0", 0);
    }
#endif

/*
 *************************************************************************
 * Name        : onRemoteLockDataChanged()
 * Parameters  : QModelIndex
 * Return value: None
 * Description : handles the data when remote lock settings is changed
 **************************************************************************
 */
void CpDeviceLockPluginView::onRemoteLockDataChanged(QModelIndex aStartIn,QModelIndex aEndIn)
    {
    Q_UNUSED(aEndIn);
    Dprint(_L("CpDeviceLockPluginView::onRemoteLockDataChanged..Enter"));
    RDEBUG("0", 0);
    HbDataFormModelItem *item = formModel->itemFromIndex(aStartIn);

    if ((item->type() == HbDataFormModelItem::ToggleValueItem)
            && (item->data(HbDataFormModelItem::LabelRole).toString()
                    == hbTrId("txt_devicelocking_subhead_remote_locking")))
        {
				RDEBUG("mHack", mHack);
        //The following If-Else condition should be removed once orbit team fix the issue with datachanged() signal
        /****************************************************************************************************************/
        if ((mHack % 2) == 0) //need to capture second datachanged() signal , not first one.
            {
            mHack++;
            return;
            }
        else
            {
            mHack++;
            }
        /****************************************************************************************************************/
        TInt autoLockVal;
        TInt retVal = KErrNone;
        TBuf<KRLockMaxLockCodeLength> remoteLockCode;
        TBool remoteLockStatus(EFalse);
        CRemoteLockSettings *remoteLockSetting = CRemoteLockSettings::NewL();
        TInt err = iALPeriodRep->Get(KSettingsAutoLockTime, autoLockVal);
        RDEBUG("err", err);
        RDEBUG("autoLockVal", autoLockVal);
        RDEBUG("retVal", retVal);
        QVariant remLockData = mDeviceRemoteLockItem->contentWidgetData(
                QString("text"));
        if (remLockData.toString() == hbTrId(
                "txt_devicelocking_button_remote_on"))
            {
            remoteLockStatus = ETrue;
            retVal = mUiSecuSettings->ChangeRemoteLockStatusL(
                    remoteLockStatus, remoteLockCode, autoLockVal);
            }
        else
            {
            remoteLockStatus = EFalse;
            retVal = mUiSecuSettings->ChangeRemoteLockStatusL(
                    remoteLockStatus, remoteLockCode, autoLockVal);
            }
        RDEBUG("retVal", retVal);
        if (retVal == KErrNone)
            {
            RDEBUG("remoteLockStatus", remoteLockStatus);
            if (remoteLockStatus)
                {
                if (remoteLockSetting->SetEnabledL(remoteLockCode))
                    {
                    Dprint(_L("CpDeviceLockPluginView::onRemoteLockDataChanged..remoteLockSetting->SetEnabledL success"));
                    RDEBUG("success", 1);
                    mPrevRemLockData
                            = mDeviceRemoteLockItem->contentWidgetData(
                                    QString("text"));
                    }
                else
                    {
                    RollbackRemoteLockSettingState();
                    Dprint(_L("CpDeviceLockPluginView::onRemoteLockDataChanged..remoteLockSetting->SetEnabledL failed"));
                    RDEBUG("failed", 0);
                    }
                }
            else
                {
                if (mRemoteLockSettings->SetDisabled())
                    {
                    Dprint(_L("CpDeviceLockPluginView::onRemoteLockDataChanged..remoteLockSetting->SetDisabled success"));
                    RDEBUG("success", 1);
                    mPrevRemLockData
                            = mDeviceRemoteLockItem->contentWidgetData(
                                    QString("text"));
                    }
                else
                    {
                    RollbackRemoteLockSettingState();
                    Dprint(_L("CpDeviceLockPluginView::onRemoteLockDataChanged..remoteLockSetting->SetDisabled failed"));
                    RDEBUG("failed", 0);
                    }
                }
            }
        else
            {
            Dprint(_L("CpDeviceLockPluginView::onRemoteLockDataChanged..RollbackRemoteLockSettingState"));
            RDEBUG("Rollback", 0);
            RollbackRemoteLockSettingState();
            }
        delete remoteLockSetting;
        }
    Dprint(_L("CpDeviceLockPluginView::onRemoteLockDataChanged..Exit"));
    RDEBUG("0", 0);
}


/*
 *************************************************************************
 * Name        : onSIMLockDataChanged()
 * Parameters  : QModelIndex
 * Return value: None
 * Description : handles the data afer Lock when SIM changed settings is 
 *               changed
 **************************************************************************
 */
void CpDeviceLockPluginView::onSIMLockDataChanged(QModelIndex aStartIn, QModelIndex aEndIn)
    {
    Q_UNUSED(aEndIn);
    HbDataFormModelItem *item = formModel->itemFromIndex(aStartIn);

    if ((item->type() == HbDataFormModelItem::ToggleValueItem)
            && (item->data(HbDataFormModelItem::LabelRole).toString()
                    == hbTrId(
                            "txt_devicelocking_formlabel_lock_when_sim_changed")))
        {
        RDEBUG("mHack", mHack);
        //The following If-Else condition should be removed once orbit team fix the issue with datachanged() signal
        /****************************************************************************************************************/
        if ((mHack % 2) == 0) //need to capture second datachanged() signal , not first one.
            {
            mHack++;
            return;
            }
        else
            {
            mHack++;
            }
        /****************************************************************************************************************/

        TBool ret = mUiSecuSettings->ChangeSimSecurityL();
        RDEBUG("ret", ret);
        if (!ret)
            {
            /* 
             * disconnect to datachanged() signal as we are not interested in this signal
             * generated as a part of setContentWidgetData() API call below
             */
            disconnect(formModel,
                    SIGNAL(dataChanged(QModelIndex, QModelIndex)), this,
                    SLOT(onSIMLockDataChanged(QModelIndex,QModelIndex)));
            QVariant txt = mLockWhenSimChangeItem->contentWidgetData(QString(
                    "text"));
            QVariant additionalText =
                    mLockWhenSimChangeItem->contentWidgetData(QString(
                            "additionalText"));

            mLockWhenSimChangeItem->setContentWidgetData(QString("text"),
                    additionalText);
            mLockWhenSimChangeItem->setContentWidgetData(QString(
                    "additionalText"), txt);
            /*
             * Now connect back to datachanged() signal .
             */
            connect(formModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
                    this, SLOT(onSIMLockDataChanged(QModelIndex,QModelIndex)));
            }
        }
}


/*
 *************************************************************************
 * Name        : GetAutoLockIndex()
 * Parameters  : TInt
 * Return value: TInt
 * Description : returns corresponding index for the given autolock value
 **************************************************************************
 */
TInt CpDeviceLockPluginView::GetAutoLockIndex(TInt aValue)
    {
    TInt index = 0;
    RDEBUG("ret", aValue);
    switch (aValue)
        {
        case 0:
            index = 0;
            break;
        case 65535:
            index = 1;
            break;
        case 5:
            index = 2;
            break;
        case 30:
            index = 3;
            break;
        case 60:
            index = 4;
            break;
        default:
            break;
        }
		RDEBUG("index", index);
    return index;
    }


/*
 *************************************************************************
 * Name        : GetValueAtIndex()
 * Parameters  : TInt
 * Return value: TInt
 * Description : returns the corresponding autolock value for the given 
 *               index.
 **************************************************************************
 */
TInt CpDeviceLockPluginView::GetValueAtIndex(TInt aIndex)
    {
    TInt value = 0;
		RDEBUG("aIndex", aIndex);
    switch (aIndex)
        {
        case 0:
            value = 0;
            break;
        case 1:
            value = 65535;
            break;
        case 2:
            value = 5;
            break;
        case 3:
            value = 30;
            break;
        case 4:
            value = 60;
            break;
        default:
            break;
        }
		RDEBUG("value", value);
    return value;
}


/*
 *************************************************************************
 * Name        : RollbackRemoteLockSettingState()
 * Parameters  : None
 * Return value: None
 * Description : If Remote Lock Settings fails or user cancels the settings
 *               then this function resets to previous value.
 **************************************************************************
 */
void CpDeviceLockPluginView::RollbackRemoteLockSettingState()
    {
    RDEBUG("0", 0);
    /* 
     * disconnect to datachanged() signal as we are not interested in this signal
     * generated as a part of setContentWidgetData() API call below
     */
    disconnect(formModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this, SLOT(onRemoteLockDataChanged(QModelIndex,QModelIndex)));

    QVariant txt = mDeviceRemoteLockItem->contentWidgetData(QString("text"));
    QVariant additionaltxt = mDeviceRemoteLockItem->contentWidgetData(
            QString("additionalText"));
    mDeviceRemoteLockItem->setContentWidgetData(QString("text"),
            additionaltxt);
    mDeviceRemoteLockItem->setContentWidgetData(QString("additionalText"),
            txt);
    /*
     * Now connect back to datachanged() signal .
     */
    connect(formModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this,
            SLOT(onRemoteLockDataChanged(QModelIndex,QModelIndex)));

    if (mPrevRemLockData.toString() == hbTrId(
            "txt_devicelocking_button_remote_on"))
        mRemoteLockMessageItem->setEnabled(true);
    else
        mRemoteLockMessageItem->setEnabled(false);
		RDEBUG("0", 0);
    }
