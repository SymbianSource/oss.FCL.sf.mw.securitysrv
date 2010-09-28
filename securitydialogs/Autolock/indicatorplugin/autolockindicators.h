/*
* ============================================================================
*  Name        : autolockindicators.h
*  Part of     : hb /
*  Description :
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

#ifndef AUTOLOCKINDICATORS_H
#define AUTOLOCKINDICATORS_H

#include <hbindicatorinterface.h>
#include <QMetaType>
#include <QString>

static const char *IndicatorNameTemplate = "com.nokia.hb.indicator.autolock.autolock_%1/1.0";

enum Interaction
{
    InteractionNone,
    ChangeContent,
    Deactivate,
    ChangeOrientation
};

inline QString indicatorName(int indicatorType) {
    return QString(IndicatorNameTemplate).arg(indicatorType);
}

struct IndicatorInfo
{
    const char *icon;
    const char *iconMono;
    const char *primaryText;
    const char *secondaryText;
    HbIndicatorInterface::Category category;
    Interaction interaction;
};

Q_DECLARE_METATYPE(IndicatorInfo)

static const int IndicatorCount = 10;
static const IndicatorInfo IndicatorInfos[IndicatorCount] = {
    {"navi_left.svg", "qtg_mono_settings.svg", "Notification indicator 1", "no interaction", HbIndicatorInterface::NotificationCategory, InteractionNone},
    {"qt_prog_bar_play.svg","qtg_status_new_email.svg", "Notification indicator 2", "deactivates, when clicked (additional to test the text truncation.)", HbIndicatorInterface::NotificationCategory, Deactivate},
    {"hb_vol_slider_increment_pressed.svg", "qtg_status_new_email.svg", "Notification indicator 3", "changes content", HbIndicatorInterface::NotificationCategory, ChangeContent},
    {"hb_vol_slider_muted.svg", "qtg_status_new_email.svg", "Notification indicator 4", "changes content, when clicked", HbIndicatorInterface::NotificationCategory, ChangeContent},
    {"qgn_menu_pinb.svg", "qgn_menu_pinb.svg", "Progress indicator 1", "no interaction", HbIndicatorInterface::ProgressCategory, InteractionNone},
    {"qgn_menu_note.svg", "qgn_menu_note.svg", "Progress indicator 2", "changes content, when clicked", HbIndicatorInterface::ProgressCategory, ChangeContent},
    {"note_error.svg", "note_error.svg", "Progress indicator 3", "changes orientation, when clicked", HbIndicatorInterface::ProgressCategory, ChangeOrientation},
    {"note_info.svg", "note_info.svg", "Progress indicator 4", "changes content, when clicked", HbIndicatorInterface::ProgressCategory, ChangeContent},
    {"qtg_large_device_lock.svg", "qtg_large_device_lock.svg", "Keyguard Enabled", "Keyguard Enabled", HbIndicatorInterface::SettingCategory, InteractionNone},
    {"qgn_menu_phob.svg", "qgn_menu_phob.svg", "Devicelock Enabled", "Devicelock Enabled", HbIndicatorInterface::SettingCategory, InteractionNone}
};


#endif // AUTOLOCKINDICATORS_H
