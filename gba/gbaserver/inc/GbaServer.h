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
* Description:  GBA Server definitions
*
*/


#ifndef   GBASERVER_H
#define   GBASERVER_H

#include <e32base.h>
#include <f32file.h>
#include "GbaServerPolicy.h"

void PanicClient( const RMessage2& aMessage, TInt aPanic );
void PanicServer( TInt aPanic );

class CShutdown : public CTimer
    {
    enum { EGbaServerShutdownDelay = 0x200000 }; // approx 2s
    public:
        inline CShutdown();
        inline void ConstructL();
        inline void Start();
    private:
        void RunL();
    };



class CGbaServer : public CPolicyServer
    {
    public : 
        static CGbaServer* NewL();
        static CGbaServer* NewLC();
        ~CGbaServer();
        //Increment the count of the active sessions for this server
        void IncrementSessions();

        //Decrement the count of the active sessions for this server. 
        //If no more sessions are in use the server terminates.
        void DecrementSessions();
       //Reads specified option from server.
       TBool ReadOptionL(const TUid& aOptionID, TDes8& aValue) const;
       //write specified option 
       void WriteOptionL(const TUid& aOptionID, const TDesC8& aValue) const;

    private:
        CGbaServer() ;
        void ConstructL() ;
        CSession2* NewSessionL(const TVersion& aVersion, const RMessage2& aMessage) const;
        void MakePrivateFilenameL(RFs& aFs, const TDesC& aLeafName, TDes& aNameOut) const;
        void  EnsurePathL( RFs& aFs, const TDesC& aFile ) const;

    private:
        TInt        iSessionCount;
        CShutdown   iShutdown;    
    };


#endif //GBASERVER_H

//EOF
