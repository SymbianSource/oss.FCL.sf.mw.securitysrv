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
* Description: autolockuseractivityservice_p.cpp
*
*/

#include "autolockuseractivityservice.h"
#include "autolockuseractivityservice_p.h"
#include <e32debug.h>

#include <activitymanager.h>

const int lDefaultInactivityPeriod(5);

// ======== LOCAL FUNCTIONS ========

/*!
    Called as callback to activity event.
    \param pointer to activity callback object.
    \retval error code.    
*/
TInt activityCallback(TAny *ptr)
{
	RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );

    static_cast<AutolockUserActivityServicePrivate *>(ptr)->emitActive();
    // activity manager doesn't care about the return value,
    // we return a value indicating 'true' to be elegant (see CIdle)
    return 1;
}

/*!
    Called as callback to inactivity event.
    \param pointer to inactivity callback object.
    \retval error code.    
*/
TInt inactivityCallback(TAny *ptr)
{
	RDebug::Printf( "%s %s (%u) value=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, 0 );

    static_cast<AutolockUserActivityServicePrivate *>(ptr)->emitNotActive();
    // activity manager doesn't care about the return value,
    // we return a value indicating 'true' to be elegant (see CIdle)
    return 1;
}

// ======== MEMBER FUNCTIONS ========

/*!
    Constructor.
    \param servicePublic public implementation.
*/
AutolockUserActivityServicePrivate::AutolockUserActivityServicePrivate(AutolockUserActivityService *servicePublic) :
    m_q(servicePublic), mActivityManager(0), mInactivityPeriod(lDefaultInactivityPeriod)
{
    mActivityManager = CUserActivityManager::NewL(CActive::EPriorityStandard);
}

/*!
    Destructor.
*/
AutolockUserActivityServicePrivate::~AutolockUserActivityServicePrivate()
{
    delete mActivityManager;
}

/*!
    Sets the inactivity period after which to emit signal of inactivity.
    \param seconds after which inactivity is detected.
*/
void AutolockUserActivityServicePrivate::setInactivityPeriod(int seconds)
{
	RDebug::Printf( "%s %s (%u) seconds=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, seconds );

    mInactivityPeriod = seconds;
    // activity manager panics if timeout set before start
    if (isWatching())
    {
        mActivityManager->SetInactivityTimeout(TTimeIntervalSeconds(mInactivityPeriod));   
    }
}

/*!
    Retrives the current inactivity period setting.
    \retval inactivity period set.
*/
int AutolockUserActivityServicePrivate::inactivityPeriod() const
{
    return mInactivityPeriod;
}

/*!
    Starts or stops activity manager user activity watching.
    \param shouldWatch determines if we shoul start watching or stop watching.
*/
void AutolockUserActivityServicePrivate::watch(bool shouldWatch)
{
	RDebug::Printf( "%s %s (%u) shouldWatch=%x", __FILE__, __PRETTY_FUNCTION__, __LINE__, shouldWatch );
    if (shouldWatch && !isWatching()) {
        mActivityManager->Start(
                TTimeIntervalSeconds(mInactivityPeriod),
                TCallBack(inactivityCallback, this),
                TCallBack(activityCallback, this));
    } else if (!shouldWatch && isWatching()) {
        mActivityManager->Cancel();
    }
}

/*!
    Checks if activity service is currently watching for user activity.
    \retval true if user acitivity service is active.
*/
bool AutolockUserActivityServicePrivate::isWatching() const
{
    return mActivityManager && mActivityManager->IsActive();
}

/*!
    Emits signal that user is active.
*/
void AutolockUserActivityServicePrivate::emitActive() const
{
    emit m_q->active();
}

/*!
    Emits signal that user is not active.
*/
void AutolockUserActivityServicePrivate::emitNotActive() const
{
    emit m_q->notActive();
}
