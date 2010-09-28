/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Observer definition for CSecuiDialog operations
*
*/

#ifndef SECUIDIALOGOPERATIONOBSERVER_H
#define SECUIDIALOGOPERATIONOBSERVER_H


/**
 * CSecuiDialogOperations inform observer when they are complete.
 * Completed operations can be deleted safely. Deleting an incomplete
 * operation cancels the ongoing request.
 */
class MSecuiDialogOperationObserver
    {
    public:
        virtual void OperationComplete() = 0;
    };

#endif  // SECUIDIALOGOPERATIONOBSERVER_H

