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
* Description:  Advanced security settings main view
*
*/

#include "advsecsettingsmainview.h"
#include "advsecsettingscertificatelistview.h"
#include "advsecsettingssecuritymodulemodel.h"
#include "advsecsettingssecuritymoduleview.h"
#include <QGraphicsLinearLayout>
#include <HbGroupBox>
#include <HbListWidget>
#include <HbListWidgetItem>
#include <QDebug>

const int KAuthorityCertsRow = 0;
const int KTrustedSiteCertsRow = 1;
const int KPersonalCertsRow = 2;
const int KDeviceCertsRow = 3;


// ======== MEMBER FUNCTIONS ========

// ---------------------------------------------------------------------------
// AdvSecSettingsMainView::AdvSecSettingsMainView()
// ---------------------------------------------------------------------------
//
AdvSecSettingsMainView::AdvSecSettingsMainView(
    QGraphicsItem *parent) : AdvSecSettingsViewBase(0, parent),
    mSecModModel(0), mCertListView(0), mSecModView(0),
    mSecModGroupBox(0), mSecModList(0), mContextMenu(0)
{
    Q_ASSERT(mSecModModel == 0);
    mSecModModel = new AdvSecSettingsSecurityModuleModel(this);
    connect(mSecModModel, SIGNAL(initializeCompleted()), this, SLOT(securityModuleInitialized()));
    connect(mSecModModel, SIGNAL(errorOccurred(int)), this, SLOT(displayError(int)));
    mSecModModel->initialize();

	QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);
    HbGroupBox *groupBox = 0;

    // View title
    groupBox = new HbGroupBox;
    groupBox->setHeading(hbTrId("txt_certificate_manager_setlabel_advanced_security"));
    layout->addItem(groupBox);

    // Certificates group
    groupBox = new HbGroupBox;
    groupBox->setHeading(hbTrId("txt_certificate_manager_setlabel_certificates"));
    groupBox->setContentWidget(createCertificatesTopLevel());
    groupBox->setCollapsed(true);
    layout->addItem(groupBox);

    // Protected content group
    groupBox = new HbGroupBox;
    // TODO: localized UI string needed
    groupBox->setHeading("Protected Content");
    groupBox->setContentWidget(createProtectedContentTopLevel());
    groupBox->setCollapsed(true);
    layout->addItem(groupBox);

    // Security module group
    Q_ASSERT(mSecModGroupBox == 0);
    mSecModGroupBox = new HbGroupBox;
    // TODO: localized UI string needed
    mSecModGroupBox->setHeading("Security Module");
    mSecModGroupBox->setVisible(false); // set in securityModuleInitialized()
    layout->addItem(mSecModGroupBox);

    setLayout(layout);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsMainView::~AdvSecSettingsMainView()
// ---------------------------------------------------------------------------
//
AdvSecSettingsMainView::~AdvSecSettingsMainView()
	{
	}

// ---------------------------------------------------------------------------
// AdvSecSettingsMainView::displayCertListView()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsMainView::displayCertListView(const QModelIndex& modelIndex)
{
    if (!mCertListView) {
        mCertListView = new AdvSecSettingsCertificateListView(this);
    }

    AdvSecSettingsCertificate::CertificateType type =
        AdvSecSettingsCertificate::AuthorityCertificate;
    switch (modelIndex.row()) {
    case KAuthorityCertsRow:
        type = AdvSecSettingsCertificate::AuthorityCertificate;
        break;
    case KTrustedSiteCertsRow:
        type = AdvSecSettingsCertificate::TrustedSiteCertificate;
        break;
    case KPersonalCertsRow:
        type = AdvSecSettingsCertificate::PersonalCertificate;
        break;
    case KDeviceCertsRow:
        type = AdvSecSettingsCertificate::DeviceCertificate;
        break;
    default:
        break;
    }

    mCertListView->displayCertificates(type);
    displayView(mCertListView);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsMainView::displaySecurityModuleView()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsMainView::displaySecurityModuleView(const QModelIndex& modelIndex)
{
    if (!mSecModView) {
        mSecModView = new AdvSecSettingsSecurityModuleView(*mSecModModel, this);
    }

    int row = modelIndex.row();
    mSecModView->setSecurityModule(mSecModList->item(row)->text(), row);
    displayView(mSecModView);
}

// ---------------------------------------------------------------------------
// AdvSecSettingsMainView::securityModuleInitialized()
// ---------------------------------------------------------------------------
//
void AdvSecSettingsMainView::securityModuleInitialized()
{
    if (isSecurityModulesAvailable()) {
        mSecModGroupBox->setContentWidget(createSecurityModuleTopLevel());
        mSecModGroupBox->setCollapsed(true);
        mSecModGroupBox->setVisible(true);
    }
}

// ---------------------------------------------------------------------------
// AdvSecSettingsMainView::
// ---------------------------------------------------------------------------
//
HbWidget *AdvSecSettingsMainView::createCertificatesTopLevel()
{
    HbListWidget *list = new HbListWidget;
    HbListWidgetItem *item = 0;

    // KAuthorityCertsRow
    item = new HbListWidgetItem;
    item->setText(hbTrId("txt_certificate_manager_list_authority_certificate"));
    list->addItem(item);

    // KTrustedSiteCertsRow
    item = new HbListWidgetItem;
    item->setText(hbTrId("txt_certificate_manager_list_trusted_site_certific"));
    list->addItem(item);

    // KPersonalCertsRow
    item = new HbListWidgetItem;
    item->setText(hbTrId("txt_certificate_manager_list_personal_certificates"));
    list->addItem(item);

    // KDeviceCertsRow
    item = new HbListWidgetItem;
    item->setText(hbTrId("txt_certificate_manager_list_device_certificates"));
    list->addItem(item);

    connect(list, SIGNAL(released(QModelIndex)), this, SLOT(displayCertListView(QModelIndex)));

    return list;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsMainView::
// ---------------------------------------------------------------------------
//
HbWidget *AdvSecSettingsMainView::createProtectedContentTopLevel()
{
    HbListWidget* list = new HbListWidget;

    // TODO: implement

    return list;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsMainView::
// ---------------------------------------------------------------------------
//
HbWidget *AdvSecSettingsMainView::createSecurityModuleTopLevel()
{
    Q_ASSERT(mSecModList == 0);
    mSecModList = new HbListWidget;

    QMap<QString, QString> labelAndLocation = mSecModModel->moduleLabelsAndLocations();
    QMapIterator<QString, QString> iter(labelAndLocation);
    while (iter.hasNext()) {
        iter.next();
        HbListWidgetItem *item = new HbListWidgetItem;
        item->setText(iter.key());
        item->setSecondaryText(iter.value());
        mSecModList->addItem(item);
    }

    connect(mSecModList, SIGNAL(activated(QModelIndex)),
        this, SLOT(displaySecurityModuleView(QModelIndex)));
    connect(mSecModList, SIGNAL(longPressed(HbAbstractViewItem*, QPointF)),
        this, SLOT(indicateLongPress(HbAbstractViewItem*, QPointF)));   // TODO: implement

    return mSecModList;
}

// ---------------------------------------------------------------------------
// AdvSecSettingsMainView::isSecurityModulesAvailable()
// ---------------------------------------------------------------------------
//
bool AdvSecSettingsMainView::isSecurityModulesAvailable()
{
    return (mSecModModel && mSecModModel->moduleCount());
}

