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
* Description: autolockuseractivityserviceinterface.h
*
*/

#ifndef AUTOLOCKUSERACTIVITYSERVICEINTERFACE_H
#define AUTOLOCKUSERACTIVITYSERVICEINTERFACE_H

#include <QObject>

class AutolockUserActivityServiceInterface : public QObject
{
    Q_OBJECT

public:

    virtual void setInactivityPeriod(int seconds) = 0;
    virtual int inactivityPeriod() const = 0;

signals:

    void notActive();
    void active();

};

#endif // AUTOLOCKUSERACTIVITYSERVICEINTERFACE_H
