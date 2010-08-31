/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
 * Description:  
 *
 */

#ifndef SECCODEUIGLOBAL_H
#define SECCODEUIGLOBAL_H

// System includes
#include <QtGlobal>

namespace SecCode
{
    enum SecCodeErr
    {
        ErrNone = 0,
        ErrGsm0707IncorrectPassword,
        ErrAccessDenied,
        ErrGsmSSPasswordAttemptsViolation,
        ErrLocked,
        ErrGsm0707OperationNotAllowed,
        ErrGsm0707SIMPuk2Required,
        ErrAbort,
    };
}

#endif  //SECCODEUIGLOBAL_H
