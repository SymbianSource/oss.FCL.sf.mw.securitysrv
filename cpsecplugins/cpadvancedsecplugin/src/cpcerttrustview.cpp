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
* Description:   Implementation of class CCertManUIContainerTrust.
*                Maintains correct list of trusted clients depending
*                on the certificate that was in focus in Authority
*                certificates view when Trust Settings view was entered.
*                Shows and changes correct Yes/No text for the Trusted client.
*
*/

// INCLUDE FILES
#include <certificateapps.h>
#include <mctcertapps.h>
#include <mctcertstore.h>

#include <QGraphicsLinearLayout>
#include <QString>
#include <QList>

#include <hbdataform.h>
#include <hbdataformmodel.h>
#include <hbdataformmodelitem.h>
#include <hblistwidgetitem.h>
#include <hblistwidget.h>

#include <memory>
#include <../../inc/cpsecplugins.h>
#include "cpcerttrustview.h"
#include "cpcertdatacontainer.h"
#include "cpcertmanuisyncwrapper.h"
#include "cpuitrustids.h"

const TInt KMidpCertStoreUid = 0x101F5B71;

// ============================= LOCAL FUNCTIONS ===============================

// ================= MEMBER FUNCTIONS =======================

CpCertTrustView::CpCertTrustView( TInt certificateIndex, CpCertDataContainer& certDataContainer, QGraphicsItem *parent /*= 0*/)
		: CpBaseSettingView(0,parent), mCertDataContainer(certDataContainer), mCertificateIndex(certificateIndex)
		{
		setTitle(tr("Trust Settings"));
		viewTrustSettings();	 
		}

// ---------------------------------------------------------
// CpCertTrustView::~CpCertTrustView()
// Destructor
// ---------------------------------------------------------
//
CpCertTrustView::~CpCertTrustView()
    {
    delete mAppInfoManager;
    mClientUids.Close();
    delete mFormModel;	
    mTrustedClients.erase(mTrustedClients.begin(), mTrustedClients.end());
    mTrustValues.erase(mTrustValues.begin(), mTrustValues.end());
    }


void CpCertTrustView::viewTrustSettings()
	{
	RDEBUG("0", 0);
	mTrustedClients << " \tWAP connection\t\t";
	mTrustedClients << " \tMail and Image conn.\t\t";
	mTrustedClients << " \tNative installing\t\t";
	mTrustedClients << " \tJava installing\t\t";
	mTrustedClients << " \tOCSP validation\t\t";
	mTrustedClients << " \tVPN\t\t";
	mTrustedClients << " \tWidget Installation\t\t";
	
	mTrustValues << "On";
	mTrustValues << "Off";
		
	try
		{
		QT_TRAP_THROWING( mAppInfoManager =
			CCertificateAppInfoManager::NewL( mCertDataContainer.iRfs, EFalse );
			updateListBoxL();)
		}
	catch(std::exception& exception)
		{
		QString error(exception.what());
		QT_TRAP_THROWING(	mCertDataContainer.ShowErrorNoteL( error.toInt()) );
		throw(exception);
		}
	}


// ---------------------------------------------------------------------------
// CpCertTrustView::UpdateListBoxL()
// Puts correct Trust Settings text to listbox and correct value (yes/no)
// to the setting, asks the value from CheckCertificateClientTrust
// For X509 certificates, only one Application is currently supported
// (Application controller), more applications maybe added later however,
// so one additional application has been left in comments as an example.
// WAP connection and Application installer have their names from resources
// and thus localized,  other additional application get their names from
// TCertificateAppInfo's Name() function.
// ---------------------------------------------------------------------------
//

void CpCertTrustView::updateListBoxL()
    {
    RDEBUG("0", 0);
    mClientUids.Reset();

    TInt clientCount = 0;

    // Use certificate index from previous view
    CCTCertInfo* entry = mCertDataContainer.iCALabelEntries[ mCertificateIndex ]->iCAEntry;

    // Here MCTCertApps should be used to get client options, but it is not properly
    // supported in any cert store. It should be possible to get the MCTCertApps
    // through token's GetInterface function.
    if ( entry->Token().TokenType().Type() == TUid::Uid( KTokenTypeFileCertstore ) )
        {
        if ( !mAppInfoManager )
            {
            User::Leave( KErrArgument );
            }
        RArray<TCertificateAppInfo> apps;
        apps = mAppInfoManager->Applications();
        for ( TInt i = 0; i < apps.Count(); i++ )
            {
            if ( apps[i].Id() != KCertManUIViewTrustJavaInstallingId &&
            		apps[i].Id() != KCertManUIViewTrustApplicationControllerId )
                {
                mClientUids.Append( apps[i].Id() );
                }
            }
        }
    else
        {
		mCertDataContainer.iWrapper->GetApplicationsL(
				mCertDataContainer.CertManager(), *entry, mClientUids );
        }

    // This is needed because MIDPCertStore does not support MCTCertApps
    if ( mClientUids.Count() == 0 &&
        entry->Token().TokenType().Type() == TUid::Uid( KMidpCertStoreUid ) )
        {
        mClientUids.Append( KCertManUIViewTrustJavaInstallingId );
        }
    
    if ( EX509Certificate != entry->CertificateFormat() )
        {
		return;
        }
	TInt resIndex = KErrNotFound;
	QList <int> itemArray;
	QList <int> trustValue;
	clientCount = mClientUids.Count();
	if ( clientCount > 0 )
		{
		for ( TInt k = 0; k < clientCount; k++ )
			{
			resIndex = trustIdIndex( mClientUids[k] );
			if ( resIndex == KErrNotFound )
				{
				continue;
				}
			
			// based on the trust list update the array
			TBool trustState = EFalse;
			QString item = mTrustedClients[ resIndex ];
			trustState = checkCertificateClientTrustL( mClientUids[k], *entry );
			// insert the index which would be used to retrieve the value from mTrustedClients
			itemArray.insert(k,resIndex);
			
			if ( trustState )
				{
				trustValue.insert(k,1);
				}
			else
				{
				trustValue.insert(k,0);
				}
			}
		}
    // display the list
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);
    setLayout(layout);
    std::auto_ptr<HbDataForm> form(new HbDataForm());
	mFormModel = new HbDataFormModel();
	
	HbListWidget* mCertLabelList = new HbListWidget(this); 
	std::auto_ptr<HbListWidgetItem> certLabel(q_check_ptr(new HbListWidgetItem()));
	const TDesC& label = entry->Label();
	certLabel->setText(QString( (QChar*)label.Ptr(),label.Length() ) );
	mCertLabelList->addItem(certLabel.get());
	certLabel.release();
	
	RDEBUG("0", 0);
	
	int count = itemArray.size();
	for( int index = 0 ;index < count; ++index)
		{
		std::auto_ptr<HbDataFormModelItem> item (q_check_ptr(new HbDataFormModelItem( 
										HbDataFormModelItem::ToggleValueItem,
										mTrustedClients[itemArray[index]] ) ));
		
		if( trustValue[index] == 1)
			{
			item->setContentWidgetData("text",mTrustValues[0]);
			item->setContentWidgetData("additionalText",mTrustValues[1]);
			}
		else
			{
			item->setContentWidgetData("text",mTrustValues[1]);
			item->setContentWidgetData("additionalText",mTrustValues[0]);
			}
		mFormModel->appendDataFormItem(item.get());
		item.release();
		}
	
	form->setModel(mFormModel);
	layout->addItem(mCertLabelList);
	layout->addItem(form.get());
	form.release();
    }


void CpCertTrustView::saveTrustSettings()
	{
	RDEBUG("0", 0);
	CCTCertInfo& entry = *( mCertDataContainer.iCALabelEntries[ mCertificateIndex ]->iCAEntry );
	if ( entry.IsDeletable() )
		{
		TInt itemCount = mFormModel->rowCount();
		RArray<TUid> newUids;
		QT_TRAP_THROWING
			(	
			CleanupClosePushL( newUids );
			
			for(int index = 0;index<itemCount;++index)
				{
				HbDataFormModelItem *item = mFormModel->item(index);
				QString trustValue = item->contentWidgetData("text").toString();
				if(trustValue == "On")
					{
					newUids.Append(trusterId(mTrustedClients[index]));
					}
				}
			if(newUids.Count() > 0 )
				{
				mCertDataContainer.iWrapper->SetApplicabilityL( mCertDataContainer.CertManager(), entry, newUids );
				}
			CleanupStack::PopAndDestroy(&newUids);
			)
		}
	}

TUid CpCertTrustView::trusterId(const QString& clientDescription) const
	{
	RDEBUG("0", 0);
	TUid retValue = TUid::Uid(0);
	if( clientDescription ==  " \tWAP connection\t\t" )
		retValue = KCertManUIViewTrustWapConnectionId;
	else if( clientDescription ==  " \tMail and Image conn.\t\t" )
		retValue = KCertManUIViewTrustMailAndImageConnId;
	else if( clientDescription ==  " \tNative installing\t\t" )
		retValue = KCertManUIViewTrustApplicationControllerId;
	else if( clientDescription ==  " \tJava installing\t\t" )
		retValue = KCertManUIViewTrustJavaInstallingId;
	else if( clientDescription ==  " \tOCSP validation\t\t" )
		retValue = KCertManUIViewOCSPCheckInstallingId;
	else if( clientDescription ==  " \tVPN\t\t" )
		retValue = KCertManUIViewTrustVPNId;
	else if( clientDescription ==  " \tWidget Installation\t\t" )
		retValue = KCertManUIViewTrustWidgetInstallId;
	return retValue;
	}

// ---------------------------------------------------------------------------
// CpCertTrustView::trustIdIndex( TUid trustUid )
// ---------------------------------------------------------------------------
//
TInt CpCertTrustView::trustIdIndex( TUid trustUid ) const
    {
    RDEBUG("0", 0);
    TInt resIndex = KErrNotFound;
    
    if ( trustUid == KCertManUIViewTrustApplicationControllerId )
        {
        resIndex = KTrustSettingsResourceIndexAppCtrl;
        }
    else if( trustUid == KCertManUIViewTrustWapConnectionId )
    	{
		resIndex = KTrustSettingsResourceIndexWAP;
    	}
    else if ( trustUid == KCertManUIViewTrustMailAndImageConnId )
        {
        resIndex = KTrustSettingsResourceIndexMailAndImageConn;
        }
    else if ( trustUid == KCertManUIViewTrustJavaInstallingId )
        {
        resIndex = KTrustSettingsResourceIndexJavaInstall;
        }
    else if ( trustUid == KCertManUIViewOCSPCheckInstallingId )
        {
        resIndex = KTrustSettingsResourceIndexOCSPCheck;
        }
    else if ( trustUid == KCertManUIViewTrustVPNId )
        {
        resIndex = KTrustSettingsResourceIndexVPN;
        }
    else if ( trustUid == KCertManUIViewTrustWidgetInstallId )
		{
		resIndex = KTrustSettingsResourceIndexWidget;
		}
    return resIndex;
    }


// ---------------------------------------------------------------------------
// CpCertTrustView::CheckCertificateClientTrustL(const TUid aClientTUid,
// CCTCertInfo& aEntry) const
// Checks if a certificate trusts a client
// * Certificate format == ECrX509Certificate ECrX968Certificate ECrCertificateURL
// are not supported if certificate location == ECrCertLocationWIMCard
// * Certificate format == ECrX968Certificate ECrCertificateURL are not supported
// if certificate location == ECrCertLocationCertMan
// * Certificate location == ECrCertLocationWIMURL ECrCertLocationPhoneMemory
// ECrCertLocationPhoneMemoryURL are not supported
// ---------------------------------------------------------------------------
//
TBool CpCertTrustView::checkCertificateClientTrustL(
    const TUid clientUid, const CCTCertInfo& entry ) const
    {
    RDEBUG("0", 0);
    TBool trustSettingTrusted = EFalse;
    TCertificateFormat format = entry.CertificateFormat();

    if ( format == EX509Certificate )
        {

        RArray<TUid> trusterUids;
        CleanupClosePushL( trusterUids );

        //Owned by mCertDataContainer.
        CUnifiedCertStore*& store = mCertDataContainer.CertManager();
        // Get trusting applications' Uids for the one selected
        // certificate entry
        mCertDataContainer.iWrapper->GetApplicationsL( store, entry, trusterUids );

        // get the number of trusting applications for one
        // selected certificate entry
        TInt trusterCount = trusterUids.Count();
        // in this loop, for every trusting application in one
        // selected certificate entry
        for ( TInt i = 0; i < trusterCount; ++i )
            {
            // put the trusting application's Uid to TUid truster
            if ( clientUid == trusterUids[i] )
                {
                trustSettingTrusted = ETrue;
                break;
                }
            }
        CleanupStack::PopAndDestroy(&trusterUids);  
        }
    
    return trustSettingTrusted;
    }

// End of File

