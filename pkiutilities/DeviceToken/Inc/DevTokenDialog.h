/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   The header file of DevTokenDialog
*
*/



#ifndef __DEVTOKENDIALOG_H__
#define __DEVTOKENDIALOG_H__


class MSecurityDialog;

/**
 *  Lauch the dialog for asking password from user
 *
 *  @lib 
 *  @since S60 v3.2
 */
class DevTokenDialog
	  {
    public:
	      inline static MSecurityDialog* Dialog();
	      static void InitialiseL();
	      static void Cleanup();
	      
    private:
	      static MSecurityDialog* iDialog;
	  };

inline MSecurityDialog* DevTokenDialog::Dialog()
	  {
	  return iDialog;
	  }

#endif //__DEVTOKENDIALOG_H__
//EOF
