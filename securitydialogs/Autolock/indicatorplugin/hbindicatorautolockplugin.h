/*
* ============================================================================
*  Name        : hbindicatorautolockplugin.h
*  Part of     : hb / hbcore
*  Description : indicator autolock plugin header
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

#ifndef HBINDICATORAUTOLOCKPLUGIN_H
#define HBINDICATORAUTOLOCKPLUGIN_H

#include <QObject>
#include <QStringList>
#include <QVariant>

#include <hbindicatorplugininterface.h>
#include <hbindicatorinterface.h>
#include "autolockindicators.h"

class HbIndicatorAutolockPlugin : public QObject, public HbIndicatorPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(HbIndicatorPluginInterface)

public:
    HbIndicatorAutolockPlugin();
    ~HbIndicatorAutolockPlugin();

    QStringList indicatorTypes() const;
    bool accessAllowed(const QString &indicatorType,
        const QVariantMap &securityInfo) const;
    HbIndicatorInterface* createIndicator(const QString &indicatorType);
    int error() const;
private:
    int typeIndex(const QString &indicatorType) const;
private:
    Q_DISABLE_COPY(HbIndicatorAutolockPlugin)
    int mError;
    QStringList mIndicatorTypes;
};

class HbAutolockIndicator : public HbIndicatorInterface
{
public:
    HbAutolockIndicator(const QString &indicatorType,
                    int typeIndex,
                    Interaction interaction);
    ~HbAutolockIndicator();
    bool handleInteraction(InteractionType type);
    QVariant indicatorData(int role) const;
protected:
    bool handleClientRequest(RequestType type, const QVariant &parameter);
private:
    QString mPrimaryText;
    QString mSecondaryText;
    QString mIcon;
    QString mIconMono;
    int mTypeIndex;
    Interaction mInteraction;
    QVariant mParameter;
};

#endif // HBINDICATORAUTOLOCKPLUGIN_H

