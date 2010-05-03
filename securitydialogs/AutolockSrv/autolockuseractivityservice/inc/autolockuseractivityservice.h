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
* Description: autolockuseractivityservice.h
*
*/

#ifndef AUTOLOCKAUTOLOCKUSERACTIVITYSERVICE_H
#define AUTOLOCKAUTOLOCKUSERACTIVITYSERVICE_H

#include "autolockuseractivityserviceinterface.h"
#include "autolockuseractivityservice_global.h"

class AutolockUserActivityServicePrivate;

class AUTOLOCKUSERACTIVITYSERVICE_EXPORT AutolockUserActivityService : public AutolockUserActivityServiceInterface
{
    Q_OBJECT

public:

    explicit AutolockUserActivityService(/*QObject *parent = 0*/);
    ~AutolockUserActivityService();

    virtual void setInactivityPeriod(int seconds);
    virtual int inactivityPeriod() const;

private:

    //virtual void connectNotify(const char *signal);
    //virtual void disconnectNotify(const char *signal);

    //bool receiversConnected();

    Q_DISABLE_COPY(AutolockUserActivityService)

private:

    AutolockUserActivityServicePrivate *m_d;
    friend class AutolockUserActivityServicePrivate;

};

#endif // AUTOLOCKAUTOLOCKUSERACTIVITYSERVICE_H
