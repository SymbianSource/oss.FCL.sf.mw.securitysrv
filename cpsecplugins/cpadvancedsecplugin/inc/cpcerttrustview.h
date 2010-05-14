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
* Description:   Declaration of the CCertManUIContainerTrust class
*                Maintains correct list of trusted clients depending
*                on the certificate that was in focus in Authority
*                certificates view when Trust Settings view was entered.
*                Shows and changes correct Yes/No text for the Trusted client.
*
*/


#ifndef  CPCERTTRUSTVIEW_H
#define  CPCERTTRUSTVIEW_H

// INCLUDES
#include <cpbasesettingview.h>
#include <QList>

// CONSTANTS
const TInt KTrustSettingsResourceIndexWAP = 0;
const TInt KTrustSettingsResourceIndexMailAndImageConn = 1;
const TInt KTrustSettingsResourceIndexAppCtrl = 2;
const TInt KTrustSettingsResourceIndexJavaInstall = 3;
const TInt KTrustSettingsResourceIndexOCSPCheck = 4;
const TInt KTrustSettingsResourceIndexVPN = 5;
const TInt KTrustSettingsResourceIndexWidget = 6;
const TInt KTrustSettingsResourceIndexValueYes = 0;
const TInt KTrustSettingsResourceIndexValueNo = 1;

// FORWARD DECLARATIONS
class CCertificateAppInfoManager;
class CpCertDataContainer;
class TCertificateAppInfo;
class CCTCertInfo;

class QString;

class HbDataFormModel;

// CLASS DECLARATION
class CpCertTrustView : public CpBaseSettingView
    {
	Q_OBJECT
	
	public:
	    explicit CpCertTrustView( 	TInt certificateIndex,	
									CpCertDataContainer& certDataContainer,
									QGraphicsItem *parent = 0 );
	    virtual ~CpCertTrustView();
	    void saveTrustSettings();
	    
	private:
	    void viewTrustSettings();
	    void updateListBoxL();
	    TInt trustIdIndex( TUid aTrusterUid ) const;
	    TBool checkCertificateClientTrustL( const TUid clientUid, const CCTCertInfo& entry ) const;
	    TUid trusterId(const QString& clientDescription) const;
	    
	private:
	    CpCertDataContainer& mCertDataContainer;
	    /**
		* Application manager class, used to get applications in system.
		*/
		CCertificateAppInfoManager*   mAppInfoManager;
        
		/**
        * Client Uids of the certstore
        */
        RArray<TUid> mClientUids;

        /**
		* To get the names of the trusted clients from the resources
		*/
		QList<QString>   mTrustedClients;

		/**
		* To get the yes/no trust texts from the resources
		*/
		QList<QString>   mTrustValues;
		
		TInt mCertificateIndex;
		
		HbDataFormModel* mFormModel; 

    };

#endif // CPCERTTRUSTVIEW_H

// End of File
