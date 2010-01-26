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
* Description:  Key pattern matching component.
 *
*/


#ifndef __LOCKAPPKEYPATTERN__
#define __LOCKAPPKEYPATTERN__

// INCLUDES
#include <w32std.h>
#include <e32base.h>

enum TPatternState
    {
    EPatternNotInitialized = 1,
    EPatternNotEnabled,
    EPatternNoMatch,
    EPatternPrimaryMatch,
    EPatternSecondaryMatch,
    };

/**
 *  CLockAppKeyPattern class implements a 2-key pattern matcher. 
 *  Can be used for both iCodes and iScancodes, as it matches integer numbers. 
 *  The caller's responsability is to make sense what is beeing matched. 
 *  Multiple patterns can be defined.
 *
 *  @lib    lockapp
 *  @since  5.0
 *  @author Tamas Koteles
 */
class CLockAppKeyPattern : public CBase
    {
    public:

        /**
         * Two phased constructor.
         */
        static CLockAppKeyPattern* NewL( );

        /**
         * Destructor.
         */
        ~CLockAppKeyPattern( );

        /**
         * If no patterns have been defined it cannot be used.
         * @return ETrue if has succesfully loaded policy
         */
        TBool HasPatterns( );

        /**
         * Gets the enabled state.
         *
         * @return true if the patterns are enabled, false othewise
         */
        TBool IsEnabled( );

        /**
         * Sets the enabled state.
         */
        void SetEnabled( TBool aEnabled );

        /**
         * Gets the primary key timeout.
         *
         * @return the timeout in miliseconds
         */
        TUint GetKeyTimeOut( );

        /**
         * Sets the primary key timeout.
         *
         */
        void SetKeyTimeOut( TUint aTimeOut );

        /**
         * Adds a key combination.
         *
         * @param aPrimaryKey code for primary key
         * @param aSecondaryKey code for secondary key
         * @return standard Symbian error code
         */
        TInt AddPattern( TUint32 aPrimaryKey, TUint32 aSecondaryKey );

        /**
         * Gets a key combination.
         *
         * @param aIndex index of the key combination
         * @param aPrimaryKey primary key code if found
         * @param aSecondaryKey secondary key code if found
         * @return standard Symbian error code
         */
        TInt GetPattern( TInt aIndex, TUint32& aPrimaryKey, TUint32& aSecondaryKey );

        /**
         * Clears all the defined key combinations.
         *
         * @return standard Symbian error code
         */
        TInt ClearPatterns( );

        /**
         * Handles Key events. Result of the event is returned in the pattern state.
         *
         * @param aKey the actual key event (code or scancode)
         * @return TPatternState caused by the event.
         */
        TPatternState HandleKeyEvent( TUint32 aKey );

    protected:

        /**
         * C++ default constructor (private so cannot be derived).
         */
        CLockAppKeyPattern( );

        /**
         * Second constructor initializes the policy
         */
        void ConstructL( );

    private:

        /**
         * A primary key has been received, set primary-match state.
         */
        void HandlePrimaryKeyEvent( TUint32 aKey );

        /**
         * A secondary key has been received, set secondary-match state.
         */
        void HandleSecondaryKeyEvent( );

        /**
         * Any other key has been received, set no-match state.
         */
        void HandleOtherKeyEvent( );

        /**
         * A call back to the keylock timer
         * param TAny aSelf a pointer to the parent class
         */
        static TInt HandleKeyTimerTimeout( TAny* aSelf );

    private:

        // defined configurations
        RArray<TUint> iPrimaryKeys;
        RArray<TUint> iSecondaryKeys;
        RArray<TUint> iWaitingKeys;

        // defined timeout
        TUint iKeyTimeOut;

        // first key pressed
        TPatternState iState;

        // if the keypatterns are enabled
        TBool iEnabled;

        // if the keylock policy exists
        TBool iHasPatterns;

        // timer used between primary and secondary keys
        CPeriodic* iKeyTimer;

    };

#endif // __LOCKAPPKEYPATTERN__
