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

#ifndef CPCERTDETAILVIEW_H
#define CPCERTDETAILVIEW_H

#include <cpbasesettingview.h>
#include <cpcertview.h>
#include <pkixcertchain.h>
#include <cctcertinfo.h>

class CpCertDataContainer;
class CX509Certificate;
class CCertificate;
class HbListWidget;
class HbAbstractViewItem;
class CCTCertInfo;

class QString;
class QSizeF;

class CpCertDetailView : public CpBaseSettingView
{
    Q_OBJECT
public:
    explicit CpCertDetailView(	CpCertView::TCertificateViews currentView, TInt pos, 
										CpCertDataContainer& certDataContainer, 
										QGraphicsItem *parent = 0 );
    virtual ~CpCertDetailView();
    
private:
    void viewCertificateDetailsL( TInt index, CpCertView::TCertificateViews type );
    void validateCertificateL(TInt index, const CCertificate& certDetails, const CCTCertInfo& certInfo, CpCertView::TCertificateViews type );
    CArrayFixFlat<TValidationError>* validateX509CertificateL( const CX509Certificate& certDetails );
    TKeyUsagePKCS15 keyUsageAndLocationL( const CCTCertInfo& certEntry, TUid* keyLocation );
    void setLocationInfo( TBool certificate, TUid locUid, HbListWidget& certDisplayDetails );
    void divideIntoBlocks( const TDesC8& input, TPtr& output );

    void addToListWidget(	HbListWidget& certDisplayDetails, const QString& data );
    void addToListWidget(	HbListWidget& certDisplayDetails, 
							const QString& displayString, const QString& displayDetails );
        
    const CCTCertInfo* certInfo( TInt index, CpCertView::TCertificateViews certView );
    void addTrustedSitesL( const CCertificate& certDetails, HbListWidget& certDisplayDetails);
    void addLabel( const CCTCertInfo& certInfo, HbListWidget& certDisplayDetails);
    void addIssuerL( const CCertificate& certDetails, HbListWidget& certDisplayDetails );
    void addSubjectL( const CCertificate& certDetails, HbListWidget& certDisplayDetails );
    void addCertUsage( TKeyUsagePKCS15 usage, HbListWidget& certDisplayDetails );
    void addValidityPeriod( const CCertificate& certDetails, HbListWidget& certDisplayDetails);
    void setURLLocation( HBufC8& urlDetails, HbListWidget& certDisplayDetails );
    void addCertFormat( TCertificateFormat certFormat, HbListWidget& certDisplayDetails );
    void addAlgoDetails( const CCertificate& certificate, HbListWidget& certDisplayDetails );
    void addSerialNo( const CCertificate& certificate,	HbListWidget& certDisplayDetails );
    void addFingerprint( const CCertificate& certificate, HbListWidget& certDisplayDetails );
    void addPublicKeyDetailsL(	const CCertificate& certificate, HbListWidget& certDisplayDetails );
    QString asymmetricAlgoId( const CCertificate& certificate );
    
private:
   CpCertDataContainer& mCertDataContainer;
  
};

#endif // CPCERTDETAILVIEW_H 
