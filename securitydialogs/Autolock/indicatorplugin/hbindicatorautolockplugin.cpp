/*
* ============================================================================
*  Name        : hbindicatorautolockplugin.cpp
*  Part of     : hb / hbcore
*  Description : indicator autolock plugin implementation
*  Version     : %version: 1 %
*
*  Copyright (c) 2009 Nokia.  All rights reserved.
*  This material, including documentation and any related computer
*  programs, is protected by copyright controlled by Nokia.  All
*  rights are reserved.  Copying, including reproducing, storing,
*  adapting or translating, any or all of this material requires the
*  prior written consent of Nokia.  This material also contains
*  confidential information which may not be disclosed to others
*  without the prior written consent of Nokia.
* ============================================================================
*/

#include <QtPlugin>
#include <QVariant>
#include <HbInstance>
#include <HbMainWindow>
#include <QDebug>

#include "hbindicatorautolockplugin.h"

Q_EXPORT_PLUGIN(HbIndicatorAutolockPlugin)

// Constructor
HbIndicatorAutolockPlugin::HbIndicatorAutolockPlugin() : mError(0)
{
    for (int i = 0; i < IndicatorCount; ++i) {
        mIndicatorTypes.append(indicatorName(i));
    }
}

// Destructor
HbIndicatorAutolockPlugin::~HbIndicatorAutolockPlugin()
{
}

// Return notification types this plugin implements
QStringList HbIndicatorAutolockPlugin::indicatorTypes() const
{
    return mIndicatorTypes;
}

// Check if client is allowed to use notification widget
bool HbIndicatorAutolockPlugin::accessAllowed(const QString &indicatorType,
    const QVariantMap &securityInfo) const
{
    Q_UNUSED(indicatorType)
    Q_UNUSED(securityInfo)

    // This plugin doesn't perform operations that may compromise security.
    // All clients are allowed to use.
    return true;
}

HbIndicatorInterface* HbIndicatorAutolockPlugin::createIndicator(
        const QString &indicatorType)
{
    HbIndicatorInterface *indicator = 0;
    int index(typeIndex(indicatorType));
    if (index >= 0) {
        indicator = new HbAutolockIndicator(
                indicatorType, index, IndicatorInfos[index].interaction);
    }
    return indicator;
}

int HbIndicatorAutolockPlugin::error() const
{
    return mError;
}

int HbIndicatorAutolockPlugin::typeIndex(const QString &indicatorType) const
{
    for (int i = 0; i < mIndicatorTypes.count(); ++i) {
        if (mIndicatorTypes.at(i) == indicatorType) {
            return i;
        }
    }
    return -1;
}

HbAutolockIndicator::HbAutolockIndicator(const QString &indicatorType,
                                 int typeIndex,
                                 Interaction interaction) :
    HbIndicatorInterface(indicatorType, IndicatorInfos[typeIndex].category,
    (interaction == InteractionNone) ? NoInteraction : InteractionActivated),
    mPrimaryText(IndicatorInfos[typeIndex].primaryText),
    mSecondaryText(IndicatorInfos[typeIndex].secondaryText),
    mIcon(IndicatorInfos[typeIndex].icon),
    mIconMono(IndicatorInfos[typeIndex].iconMono),
    mTypeIndex(typeIndex), mInteraction(interaction)
{
}

HbAutolockIndicator::~HbAutolockIndicator()
{
}

bool HbAutolockIndicator::handleInteraction(InteractionType type)
{
    if (type == InteractionActivated) {
        switch(mInteraction) {
        case ChangeContent:
            mPrimaryText = "Clicked";
            mSecondaryText = "content changed!";
            mIcon = "qtg_mono_ok.svg";
            break;
        case Deactivate:
            emit deactivate();
            break;
        case ChangeOrientation:
        		qDebug() << "============= HbAutolockIndicator::handleInteraction doesn't react to ChangeOrientation";
            /*
            if (hbInstance->orientation() == Qt::Horizontal) {
                hbInstance->setOrientation(Qt::Vertical);
            } else {
                hbInstance->setOrientation(Qt::Horizontal);
            }
            */
            if (hbInstance->allMainWindows().at(0)->orientation() == Qt::Horizontal) {
                hbInstance->allMainWindows().at(0)->setOrientation(Qt::Vertical);
            } else {
                hbInstance->allMainWindows().at(0)->setOrientation(Qt::Horizontal);
            }
            break;
        default:
            return false;
        }
        emit dataChanged();
    }
    return false;
}

QVariant HbAutolockIndicator::indicatorData(int role) const
{
    if (role == PrimaryTextRole) {
        if (mParameter.isValid()) {
        		qDebug() << "============= HbAutolockIndicator::indicatorData 11=";
						qDebug() << QString("data:").append(mParameter.toString())
                                   .append(" ")
                                   .append(mPrimaryText);
            return QString("data:").append(mParameter.toString())
                                   .append(" ")
                                   .append(mPrimaryText);
        } else {
        		qDebug() << "============= HbAutolockIndicator::indicatorData 12=";
        		qDebug() << mPrimaryText ;
            return mPrimaryText;
        }
    } else if (role == SecondaryTextRole && mTypeIndex != 7) {
        if (mParameter.isValid()) {
        		qDebug() << "============= HbAutolockIndicator::indicatorData 21=";
						qDebug() << QString("data:").append(mParameter.toString())
                                   .append(" ")
                                   .append(mPrimaryText);
            return QString("data:").append(mParameter.toString())
                                   .append(" ")
                                   .append(mSecondaryText);
        } else {
        		qDebug() << "============= HbAutolockIndicator::indicatorData 22=";
        		qDebug() << mSecondaryText ;
            return mSecondaryText;
        }
    } else if (role == MonoDecorationNameRole) {
        if (mParameter.isValid()) {
        		qDebug() << "============= HbAutolockIndicator::indicatorData 31=";
        		qDebug() << "qtg_mono_ok.svg" ;
            return "qtg_mono_ok.svg";
        } else {
        		qDebug() << "============= HbAutolockIndicator::indicatorData 32=";
        		qDebug() << mIcon ;
            return mIcon;
        }
    } else if (role == DecorationNameRole) {
        if (mParameter.isValid()) {
        		qDebug() << "============= HbAutolockIndicator::indicatorData 41=";
        		qDebug() << "qtg_mono_ok.svg" ;
            return "qtg_mono_ok.svg";
        } else {
        		qDebug() << "============= HbAutolockIndicator::indicatorData 42=";
        		qDebug() << mIcon ;
            return mIcon;
        }
    } else if (role == MonoDecorationNameRole) {
        if (mParameter.isValid()) {
        		qDebug() << "============= HbAutolockIndicator::indicatorData 51=";
        		qDebug() << "qtg_mono_ok.svg" ;
            return "qtg_mono_ok.svg";
        } else {
        		qDebug() << "============= HbAutolockIndicator::indicatorData 52=";
        		qDebug() << mIconMono ;
            return mIconMono;
        }
    }
    return QVariant();
}

bool HbAutolockIndicator::handleClientRequest(RequestType type, const QVariant &parameter)
{
    bool handled(false);
    switch (type) {
    case RequestActivate:
        if (mParameter != parameter) {
            mParameter = parameter;
            emit dataChanged();
        }
        handled =  true;
        break;
    default:
        mParameter.clear();
    }

    return handled;
}

