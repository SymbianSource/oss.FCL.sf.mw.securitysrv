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
* Description:  class RGbaServerSession definition
*
*/


#ifndef GBASERVERSESSION_H_
#define GBASERVERSESSION_H_

#include <e32base.h>
#include <GbaUtility.h>

class RGbaServerSession : public RSessionBase
    {
    public:
        RGbaServerSession();
        TInt Connect();
        TVersion Version() const;
        TInt CancelBootstrap() const;
        TInt RequestOption(TUid optionUid, TDes8& aValue) const;
        TInt WriteOption(TUid optionUid, const TDesC8& aValue) const;
        TInt IsGBAUSupported( TBool& aIsGBAUSupported );
        void RequestBootstrap(TGBABootstrapInputParams& aInput, TGBABootstrapOutputParams& aOutput, TRequestStatus& aStatus);
    private:
        TPtr8   iGbaInputBuffer;
        TPtr8   iGbaOutputBuffer;
    };

#endif // GBASERVERSESSION_H



//EOF
