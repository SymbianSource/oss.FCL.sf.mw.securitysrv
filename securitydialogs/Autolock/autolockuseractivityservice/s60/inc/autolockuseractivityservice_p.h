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

class CUserActivityManager;

class AutolockUserActivityService;

class AutolockUserActivityServicePrivate
{

public:

    explicit AutolockUserActivityServicePrivate(AutolockUserActivityService *servicePublic);
    ~AutolockUserActivityServicePrivate();

    void setInactivityPeriod(int seconds);
    int inactivityPeriod() const;

    void watch(bool shouldWatch);
    
    void emitActive() const;
    void emitNotActive() const;

private:

    bool isWatching() const;

private:

    AutolockUserActivityService *m_q;

    CUserActivityManager *mActivityManager;
    int mInactivityPeriod;

};

#endif // AUTOLOCKUSERACTIVITYSERVICEPRIVATE_H
