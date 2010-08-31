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
#ifndef CPDEVICELOCKPLUGINVIEW_H
#define CPDEVICELOCKPLUGINVIEW_H

#include <cpbasesettingview.h>
#include <hbinputdialog.h>

class HbDataFormModel;
class HbDataFormModelItem;
class CSecurityHandler;
class CSecuritySettings;
class SecCodeSettings;
class CRemoteLockSettings;
class CRepository;
class RMobilePhone;

class CpDeviceLockPluginView : public CpBaseSettingView
{
    Q_OBJECT
public:
    explicit CpDeviceLockPluginView(QGraphicsItem *parent = 0);
    virtual ~CpDeviceLockPluginView();
private:
    /* Sets the remote lock setting state to previous value */
    void RollbackRemoteLockSettingState();
private slots:
    /* Displays security code dialog and enables user to change the lock code */      
    void onLockCodeClicked();
    /* Aske for Security code dialog when Automatic Lock timings are changed*/
    void onAutoLockChanged(int);
    /* Displays security Code dialog and then a Input dialog when Remote Lock 
     * settings is clicked
     */
    void onLockMessageClicked();
    /* When Automatic Lock timings text is changed.*/
    /*
     * This slot can be enabled once fix from obit team for this siganl is available
     */
   // void onAutoTextChanged(const QString& aText);
    /* Displays Security code dialog and handles the changed value of the Remote Lock*/
    void onRemoteLockDataChanged(QModelIndex,QModelIndex);
    /* Displayes Security code dialog for Lock when SIM changed settings*/
    void onSIMLockDataChanged(QModelIndex,QModelIndex);
    /* Returns the index of automatic lock timings for the given autoLock value*/
    TInt GetAutoLockIndex(TInt);
    /* Returns the auto lcok value for the given automatic lock timings index*/
    TInt GetValueAtIndex(TInt);
private:
    /*Handler for remote lock class*/
    CRemoteLockSettings* mRemoteLockSettings;
    /* Handler for Securit Settings class*/
    CSecuritySettings *mUiSecuSettings;
    /*Handler to Repository Class*/
    CRepository* iALPeriodRep;
        
    /* Data Model for DataForm*/
    HbDataFormModel *formModel;
    /* Data Item for RemoteLock message*/
    HbDataFormModelItem *mRemoteLockMessageItem;
    /* Data Item Remote Lock*/
    HbDataFormModelItem *mDeviceRemoteLockItem;
    /* Data item for automatic lock timings*/
    HbDataFormModelItem *mAutolockPeriodItem;
    /* Data item for Lock when SIM changed settings*/
    HbDataFormModelItem *mLockWhenSimChangeItem;
    /* Input dalog for user to enter Lock message*/
    HbInputDialog* ipDialog;
    
    /* holds Lock when SIM changed data*/
    QVariant mPrevSIMLockData;
    /* holds Remote Lock Value*/
    QVariant mPrevRemLockData;
    /* holds the previous Automatic Lock timings index*/
    int mThemeComboPrevIndex;
    /* holds the user defined Lock Message*/
    QString mLockMsg;
    /* flag set on Remote Lock data changed*/
    TBool mRemoteLockSettingClicked;
    /*
     * Need to remove this variabl once fix available from orbit team
     */
    TInt mHack;
};

#endif
