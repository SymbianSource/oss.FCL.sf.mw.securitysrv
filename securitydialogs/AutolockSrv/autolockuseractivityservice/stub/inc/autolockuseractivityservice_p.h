/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: autolockuseractivityservice_p.h
*
*/

#ifndef AUTOLOCKUSERACTIVITYSERVICEPRIVATE_H
#define AUTOLOCKUSERACTIVITYSERVICEPRIVATE_H

#include "autolocktest_global.h"

class QTimer;

class AutolockUserActivityService;

class AutolockUserActivityServicePrivate
{

public:

    explicit AutolockUserActivityServicePrivate(AutolockUserActivityService *servicePublic);
    ~AutolockUserActivityServicePrivate();

    void setInactivityPeriod(int seconds);
    int inactivityPeriod() const;

    void watch(bool shouldWatch);

private:

    bool isWatching() const;

private:

    AutolockUserActivityService *m_q;

    QTimer *mActivityTimer;
    QTimer *mInactivityTimer;

    int mInactivityPeriod;
    SCREENSAVER_TEST_FRIEND_CLASS(T_AutolockUserActivityService)
};

#endif // AUTOLOCKUSERACTIVITYSERVICEPRIVATE_H
