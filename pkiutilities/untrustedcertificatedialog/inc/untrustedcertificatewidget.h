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
* Description:  Device dialog plugin that shows untrusted certificate
*               dialog for TLS server authentication failure errors.
*
*/

#ifndef UNTRUSTEDCERTIFICATEWIDGET_H
#define UNTRUSTEDCERTIFICATEWIDGET_H

#include <hbwidget.h>

class UntrustedCertificateInfoBase;
class QGraphicsLinearLayout;
class HbLabel;
class HbCheckBox;
class HbGroupBox;
class HbTextEdit;


/**
 * Untrusted certificate widget.
 * Content widget for untrusted certificate dialog.
 * See UntrustedCertificateDialog.
 */
class UntrustedCertificateWidget : public HbWidget
{
    Q_OBJECT

public:     // constructor and destructor
    UntrustedCertificateWidget(QGraphicsItem *parent=0, Qt::WindowFlags flags=0);
    virtual ~UntrustedCertificateWidget();

    void constructFromParameters(const QVariantMap &parameters);
    void updateFromParameters(const QVariantMap &parameters);
    bool isPermanentAcceptAllowed() const;
    bool isPermanentAcceptChecked() const;
    QString serverName() const;

public:     // new platform specific functions
    bool isCertificateValid();

private:    // new platform specific functions
    void processEncodedCertificate(const QByteArray &encodedCert);
    QString descriptionText();

private:    // new functions
    Q_DISABLE_COPY(UntrustedCertificateWidget)
    void processParameters(const QVariantMap &parameters);

private:    // data
    QGraphicsLinearLayout *mMainLayout;
    HbLabel *mProblemDescription;
    HbCheckBox *mAcceptPermanently;
    HbGroupBox *mCertificateDetailsGroupBox;
    HbTextEdit *mCertificateDetailsText;
    UntrustedCertificateInfoBase *mCertificateInfo;
    QString mServerName;
    int mValidationError;
    bool mIsSavingServerNamePossible;
};


#endif // UNTRUSTEDCERTIFICATEWIDGET_H

