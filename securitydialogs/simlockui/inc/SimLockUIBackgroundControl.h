/*
* ============================================================================
*  Name         : SimLockUIBackgroundControl.h
*  Part of      : Sim Lock UI Application
*  Description  : SimLock UI Background Control Header
*  Version      : 
*  
* Copyright (c) 2005-2010 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:   Build info file for Ado domain appinstall 
* ============================================================================
*/

#ifndef __SIMLOCKUI_BACKGROUNDCONTROL_H__
#define __SIMLOCKUI_BACKGROUNDCONTROL_H__

// System includes
#include <e32base.h>
#include <coecntrl.h>       // CCoeControl

// Forward declarations
class CAknsBasicBackgroundControlContext;
class CGraphicsContext;

/**
 *  CSimLockUIBackgroundControl
 *
 *  A CCoeControl object that represents the application background.
 *
 *  @lib cone.lib
 *  @lib aknskins.lib
 *  @lib aknskinsrv.lib
 *  @lib aknswallpaperutils.lib
 */
class CSimLockUIBackgroundControl : public CCoeControl
    {
public: // Public Constructors

    /**
     * NewL
     * Create a CSimLockUIBackgroundControl object, which will draw itself to aRect
     *
     * @param aRect the rectangle this view will be drawn to
     * @return a pointer to the created instance of CSimLockUIBackgroundControl
     */
    static CSimLockUIBackgroundControl* NewL( const TRect& aRect );

    /**
     * NewLC
     * Create a CSimLockUIBackgroundControl object, which will draw itself to aRect
     *
     * @param aRect the rectangle this view will be drawn to
     * @return a pointer to the created instance of CSimLockUIBackgroundControl
     */
    static CSimLockUIBackgroundControl* NewLC( const TRect& aRect );


    /**
     * ~CSimLockUIBackgroundControl
     * Destroy the object and release all memory objects
     */
     virtual ~CSimLockUIBackgroundControl();

public: // Public API

    /**
     * SetBackgroundText
     * Set the background text
     *
     * @param aText new background text( ownership passed )
     */
    void SetBackgroundText( HBufC* aText );

private: // Private Consructors

    /**
     * ConstructL
     * Perform the second phase construction of a CSimLockUIBackgroundControl object
     *
     * @param aRect the rectangle this view will be drawn to
     */
    void ConstructL( const TRect& aRect );

private: // From CCoeControl

    /**
     * SizeChanged
     * Indicate that the control has been resized
     */
    virtual void SizeChanged();

    /**
     * Draw
     * Draw this CSimLockUIBackgroundControl to the screen
     *
     * @param aRect the rectangle of this view that needs updating
     */
    virtual void Draw( const TRect& aRect ) const;

    /**
     * MopSupplyObject
     * Retrieves an object of the same type as that encapsulated in aId.
     *
     * @param aId type of object to retrieve
     */
    virtual TTypeUid::Ptr MopSupplyObject( TTypeUid aId );

private: // Private Member API

    /**
     * DisplayText
     * Called by Draw() to display the appropriate background text
     *
     * @param aGc graphics context which draws the text
     */
    void DisplayText( CGraphicsContext& aGc ) const;

private:    // Member data

    /**
     * Control which represents the background image
     * owns
     */
    CAknsBasicBackgroundControlContext* iBackgroundSkinContext;

    /**
     * Text to display on background pane
     * owns
     */
    HBufC* iDisplayText;
    };


#endif // __SIMLOCKUI_BACKGROUNDCONTROL_H__

// End of file.

