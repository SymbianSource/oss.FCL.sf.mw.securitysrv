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
* Description: autolockuseractivityservice.cpp
*
*/

#include "autolockuseractivityservice.h"
#include "autolockuseractivityservice_p.h"

/*!
    \class AutolockUserActivityService
    \brief ?brier_description

    ?Qt_style_documentation 
*/

// ======== MEMBER FUNCTIONS ========

/*!
    Constructor.
*/
AutolockUserActivityService::AutolockUserActivityService() :
    m_d(new AutolockUserActivityServicePrivate(this))
{
    m_d->watch(true);
}

/*!
    Destructor.
*/
AutolockUserActivityService::~AutolockUserActivityService()
{
    delete m_d;
}

/*!
    Sets the inactivity period after which to emit signal of inactivity.
    \param seconds after which inactivity is detected.
*/
void AutolockUserActivityService::setInactivityPeriod(int seconds)
{
    m_d->setInactivityPeriod(seconds);
}

/*!
    Retrives the current inactivity period setting.
    \retval inactivity period set.
*/
int AutolockUserActivityService::inactivityPeriod() const
{
    return m_d->inactivityPeriod();
}
