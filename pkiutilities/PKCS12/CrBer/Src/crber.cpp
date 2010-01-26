/*
* Copyright (c) 2000, 2004 Nokia Corporation and/or its subsidiary(-ies). 
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
* Description:   This module contains the implementation of CCrBer class. 
*
*/



//  INCLUDE FILES
#include "crber.h"
#include <e32math.h>    // Pow


// -----------------------------------------------------------------------------
// CCrBer
// Constructor 
// This function constructs CCrBer object
// -----------------------------------------------------------------------------
CCrBer::CCrBer(TInt aLevel /* = 0 */)
    {
    TReal size = 0;
    
    Math::Pow(size, 256, sizeof(TUint));
    iMaxUint = STATIC_CAST(TUint, size - 1);
    
    Math::Pow(size, 128, sizeof(TInt));
    iMaxInt = STATIC_CAST(TInt, size - 1);
    
    
    iEOCBytes[0] = 0x00;
    iEOCBytes[1] = 0x00;

    iType = KBerUnknown;
    
    iIndefinite = EFalse;

    iObjectBegin = 0;
    iContentBegin = 0;
    iContentLen = 0;
    iObjectLen = 0;

    iLevel = aLevel;

    iData = NULL;
    }

// -----------------------------------------------------------------------------
// CCrBer
// Destructor 
// This function destructs CCrBer object
// -----------------------------------------------------------------------------
EXPORT_C CCrBer::~CCrBer()
    {
    iData = NULL;
	iInt.Close();
    }

// -----------------------------------------------------------------------------
// CCrBer::ConstructL
// This function initializes CCrBer object's member objects.
// -----------------------------------------------------------------------------
TAny CCrBer::ConstructL()
    {
    }
// -----------------------------------------------------------------------------
// CCrBer::NewLC
// -----------------------------------------------------------------------------
EXPORT_C CCrBer* CCrBer::NewLC(TInt aLevel /* = 0 */)
    {
    CCrBer* self = new (ELeave) CCrBer(aLevel);
    CleanupStack::PushL(self);

    self->ConstructL();

    return self; 
    }

// -----------------------------------------------------------------------------
// CCrBer::NewL
// -----------------------------------------------------------------------------
EXPORT_C CCrBer* CCrBer::NewL(TInt aLevel /* = 0 */)
    {
    CCrBer* self = NewLC(aLevel);
    CleanupStack::Pop();

    return self; 
    }

// -----------------------------------------------------------------------------
// CCrBer::Read
// Read next BER encoded object from the current position of
// given CCrData to this object. Return the type of the BER
// object, or KBerUnknown, if not of any known type. Only pointer
// to aData is stored into this object, so Get* functions are
// meaningful only if original data object is still existing,
// when these functions are used.
// -----------------------------------------------------------------------------
EXPORT_C TUint8 CCrBer::Read(CCrData* aData)
    {
    if ((iType = ReadType(aData)) != KBerUnknown)
        {
        TInt  move;
        TInt  pos = 0;
        TBool conLenKnown;


        // If type is known, this object begins right here,
        // well, to be exact, one byte before this point.
        if ((aData->Seek(ESeekCurrent, pos)) == KErrNone)
            {
            conLenKnown = ETrue;

            // Now we have at least something sensible. Store pointer to
            // the data object (and hope that the caller doesn't delete it :)
            iData = aData;

            iObjectBegin = pos - 1;

            // Read length.
            iContentLen = ReadLen(aData, iIndefinite, &iLenLen);

            // Content begins right after type tag and length bytes.
            iContentBegin = pos + iLenLen;

            // If length is indefinite and type isn't set, sequence,
            // or explicit/implicit constructed, find next end-of-content
            // tag to define exact length.
            if (iIndefinite)
                {
                if ((iType != KBerSeq) &&
                    (iType != KBerSet) &&
                    (iType != KBerImplicitConstructed) &&
                    (iType != KBerExplicitConstructed))
                    {
                    // Give special treatment to constructed encoding.
                    if ((iType & KBerConstructedBit) ||
                        (iType & KBerImplicitConstructed))
                        {
                        iContentLen = OpenConstructedEncodingWithTagL(
                                        aData, *this);
                        
                        }
                    else
                        {
                        iContentLen = FindEndOfContent(aData);
                        }
                    }
                else
                    {
                    // We really can't know what the length is,
                    // until we open up the whole inner content.
                    conLenKnown = EFalse;
                    }
                }

            // Now, if we know content length, then we can calculate whole
            // object's length; it is tag + length bytes + content length.
            if (conLenKnown)
                {
                SetObjectLen();

                // Also move data pointer at the end of this object, except
                // if object was set or sequence, in which case don't move,
                // because we still have to open inner items.
                // At indefinite case this is already done (other than
                // sequence or set) or this can't be done (sequence or
                // set). So, if not indefinite and not sequence or set,
                // move data pointer the amount of content length.
                if (!(iIndefinite) && !(IsSeqOrSet(iType)))
                    {
                    move = iContentLen;

                    aData->Seek(ESeekCurrent, move);
                    }
                }

            // If tag was end-of-content tag, check that also length
            // was 0 and not indefinite, this guarantees that we really
            // have an end-of-content tag (00 00).
            if (iType == KBerEndOfContent)
                {
                if ((iContentLen != 0) || (iIndefinite))
                    {
                    iType = KBerUnknown;
                    }
                }
            }
        
	
	 
		}
    return iType;
    }



// -----------------------------------------------------------------------------
// CCrBer::ReadType
// Read type tag from current position in given CCrData.
// Returns type tag or KBerUnknown, if not of any known type.
// -----------------------------------------------------------------------------
EXPORT_C TUint8 CCrBer::ReadType(CCrData* aData)
    {
    TBuf8<1> byte;
    TUint8   type = KBerUnknown;   // By default we are pessimists.

    // Read type.
    if ((aData != 0) &&
        (aData->Read(byte, 1) == KErrNone) &&
        (byte.Length() > 0))
        {
        type = byte[0];

        if (!IsKnownType(type))
            {
            // Don't let type to be whatever byte happened to be
            // at that place. Set it to unknown, which we all know.
            type = KBerUnknown;
            }
        }

    return type;
    }

// -----------------------------------------------------------------------------
// CCrBer::ReadLen
// Read length tags from current position in given CCrData.
// Returns length. If length is indefinite, aIndefinite is set
// to true, otherwise to false. In indefinite case length is 0.
// Also sets amount of length bytes in aLenLen, if given.
// -----------------------------------------------------------------------------
EXPORT_C TUint CCrBer::ReadLen(
    CCrData* aData,
    TBool&   aIndefinite,
    TUint8*  aLenLen/* = 0 */)
    {
    TBuf8<LEN_BYTES_MAX> bytes;
    TUint8               byte = 0;
    TUint                len = 0;

    // Originally assume that length is definite,
    aIndefinite = EFalse;

    // First byte tells if the length bytes are in short or in long form.
    if ((aData != 0) && (aData->Read(bytes, 1) == KErrNone))
        {
        if (bytes.Length() <= 0)        
            {
            User::Leave(KErrArgument);
            }
        byte = bytes[0];

        // We have at least one length byte.
        if (aLenLen != 0)
            {
            *aLenLen = 1;
            }

        if (byte & KBerLongLengthBit)
            {
            // We have length bytes, but how many?
            TUint8 lenBytes = (TUint8)(byte & ~KBerLongLengthBit);

            // If length is in long form, but there are zero
            // length bytes, then length is indefinite.
            if (lenBytes == 0)
                {
                aIndefinite = ETrue;
                }
            else
                {
                if (aLenLen != 0)
                    {
                    // Add amount of actual length bytes to one,
                    // which was for long length form byte.
                    for (TInt i = 0; i < lenBytes; i++)
                        {
                        (*aLenLen)++;
                        }
					}

                // Otherwise we have to interpret length bytes and
                // move the result into len variable. First check
                // that there aren't more bytes than fits to
                // unsigned integer, we don't want any troubles here.
                if (lenBytes > sizeof(TUint))
                    {
                    // Return max uint, caller can then decide
                    // what to do with this huge pile of...
                    return iMaxUint;
                    }

                // Read length bytes
                if (aData->Read(bytes, lenBytes) == KErrNone)
                    {
                    TUint8 i;

                    // and move them to len.
                    for (i = 0; i < lenBytes; i++)
                        {
                        len = len << 8;

                        len += bytes[i];
                        }
                    }
                }
            }
        else
            {
            // Otherwise length was in short form and length
            // byte alone tells us the length of the contens.
            len = byte;
            }
        }

    return len;
    }


// -----------------------------------------------------------------------------
// CCrBer::IsKnownType
// Returns true if given BER tag is identified one.
// Returns:  ETrue of EFalse.
// -----------------------------------------------------------------------------
EXPORT_C TBool CCrBer::IsKnownType(TBerTag aTag)
    {
    TBool isKnown = EFalse;

    // Remove constructed bit, if it is on without other upper byte tags.
    if ((aTag & 0xF0) == KBerConstructedBit)
        {
        aTag &= ~KBerConstructedBit;
        }

    // Remove implicit/explicit constructed bit, 
    // if it is on without other upper byte tags.
    if ((aTag & 0xF0) == KBerImplicitConstructed)
        {
        aTag &= ~KBerImplicitConstructed;
        }

    switch (aTag)
        {
        case KBerEndOfContent:
        case KBerBoolean:
        case KBerInteger:
        case KBerBitString:
        case KBerOctetString:
        case KBerNull:
        case KBerOid:
		case KBerNumS:
        case KBerPrS:
        case KBerT61S:
		case KBerVideoS:
        case KBerIA5S:
        case KBerUtc:
		case KBerGenTime:                                
		case KBerGraphS: 
		case KBerVisibleS:
		case KBerGeneralS:
        case KBerBmpS:
        case KBerSeq:
        case KBerSet:
        case KBerImplicitConstructed:
        // Same as KBerExplicitConstructed
        case KBerImplicit:
        // Same as KBerExplicit
            {
            isKnown = ETrue;
            break;
            }
        default:
            {
            isKnown = EFalse;
            break;
            }
        }

    return isKnown;
    }



// -----------------------------------------------------------------------------
// CCrBer::IsSeqOrSet
// Returns true if given BER tag is set, sequence,
// implicit contructed, or explicit constructed tag.
// -----------------------------------------------------------------------------
EXPORT_C TBool CCrBer::IsSeqOrSet(TBerTag aTag)
    {
    TBool isSeqOrSet = EFalse;

    if ((aTag == KBerSeq) ||
        (aTag == KBerSet) ||
        (aTag == KBerImplicit) ||
        (aTag == KBerExplicit) ||
        (aTag == KBerImplicitConstructed) ||
        (aTag == KBerExplicitConstructed))

        {
        isSeqOrSet = ETrue;
        }

    return isSeqOrSet;
    }

// -----------------------------------------------------------------------------
// CCrBer::FindEndOfContent
// Finds next end-of-content (00 00) tag from given data.
// Returns the distance between current place and the tag,
// or 0, if not found. Moves data pointer to the next byte
// _after_ the tag, i.e. two bytes longer than you might
// except from the return value.
// -----------------------------------------------------------------------------
EXPORT_C TUint CCrBer::FindEndOfContent(CCrData* aData)
    {
    TBool go = ETrue;
    TInt bufferOffset = 0;
    TInt dataOffset = 0;
    TInt move = 0;

    TBuf8<KReadBufMax> bytes;

    if (aData == 0)
        {
        return dataOffset;
        }

    // Read buffer of data. Go on, if there is something to go with.
    while ((aData->Read(bytes, KReadBufMax) == KErrNone) &&
           (bytes.Length() != 0) && go)
        {
        TBuf8<2> EOCBytes;

        EOCBytes.Append(iEOCBytes[0]);
        EOCBytes.Append(iEOCBytes[1]);

        // Try to find end-of-content tag.
        bufferOffset = bytes.Find(EOCBytes);

        // If found, move data pointer right _after_
        // the place of founded tag, and break.
        if (bufferOffset != KErrNotFound)
            {
            move = bufferOffset -
                       bytes.Length() +
                       EOCBytes.Length();

            aData->Seek(ESeekCurrent, move);

            dataOffset += bufferOffset;

            go = EFalse;                    //break;
            }
        else
            {
            // Otherwise keep searching. First check that the last
            // character of the buffer isn't 00. If it is, move the
            // data pointer one byte backwards to prevent us from
            // missing end-of-content tag, if it is divided between
            // buffers.
            if (bytes[bytes.Length() - 1] == KBerEndOfContent)
                {
                move = -1;

                aData->Seek(ESeekCurrent, move);

                dataOffset--;
                }

            dataOffset += bytes.Length();
            }
        }

    // If nothing was found, move data pointer back
    // to where it was and return 0.
    if (bytes.Length() == 0)
        {
        dataOffset = -dataOffset;

        aData->Seek(ESeekCurrent, dataOffset);

        dataOffset = 0;
        }

    return dataOffset;
    }

// -----------------------------------------------------------------------------
// CCrBer::OpenConstructedEncodingL
// Open constructed encoding from this object.  
// Parameters: aTarget; target for contentbytes.
// Return Values:  Number of objects in this contructed object.
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrBer::OpenConstructedEncodingL(CCrData& aTarget)
    {      
    TInt err = KErrNone;
    TInt count = 0;
    TInt offset = iContentBegin;
    TInt objectNum = 0;
    TInt type = Type();
    CCrBer* object = NULL;

    if (type <= KBerConstructed)        // not valid type
        {
        User::Leave(KErrArgument);
        }
    else  
        {
        type -= KBerConstructed;        // type what we are looking for
        }

    CCrBerSet* newSet = CCrBerSet::NewL(1);
    CleanupStack::PushL(newSet);
    iData->Seek(ESeekStart, offset);    // seek right place; content begin
    newSet->OpenL(iData, KOpenAllLevels);
    count = newSet->Count();            

    HBufC8* obj = NULL;                 // buffer for CCrBer-objects

    for (objectNum = 0; (objectNum < count); objectNum++)
        {
        object = newSet->At(objectNum); // select object
        
        if (object->Type() == type)
            {
            obj = HBufC8::NewLC(object->ContentLen());
            TPtr8 pObj  = obj->Des();
            
            err = iData->Read(object->ContentBegin(), pObj, object->ContentLen());
            
            if (err < KErrNone)         // if error -> leave
                {
                CleanupStack::PopAndDestroy(2);  // delete newSet, obj;
                User::Leave(err);
                }    
            err = aTarget.Write(pObj);  // write content to CCrData  
            
            CleanupStack::PopAndDestroy();  // delete obj;
            obj = NULL;
            
            if (err < KErrNone)         // if error -> leave
                {
                CleanupStack::PopAndDestroy();  // delete newSet;
                User::Leave(err);
                }
            
            }
        else 
            {
            count = objectNum;
            }
        }
    CleanupStack::PopAndDestroy();      // delete newSet;

    return count;
    }

// -----------------------------------------------------------------------------
// CCrBer::OpenConstructedEncodingWithTagL
// Open constructed encoding with given tag from given data.
// Add all founded octets into the parameter string, if given.
// Return amount of bytes read.
// -----------------------------------------------------------------------------
EXPORT_C TUint CCrBer::OpenConstructedEncodingWithTagL(
    CCrData* aData,
    CCrBer&  parentObj,
    HBufC8*  aBuf /* = 0 */)
    {
    TInt    read = 0;
    CCrBer  temp;
    TBerTag parentTag = KBerUnknown;
    TBerTag tempTag   = KBerUnknown;
    HBufC8* tempBuf   = NULL;

    parentTag = parentObj.Type();

    // Check given object. It should have constructed tag.
    if (parentTag & KBerConstructedBit)
        {
        TUint len = 0;
        TUint totalLen = iMaxUint;
        TBool isIndefinite = parentObj.Indefinite();

        TBufC8<4> buf = _L8("Test");
        TPtr8     ptr = buf.Des();

        if (aBuf != 0)
            {
            ptr = aBuf->Des();
            }

        tempBuf = HBufC8::NewLC(KReadBufMax);

        // After that we know that object either contains length or
        // it is indefinite. Set max int to len even in the case of
        // indefinite length to prevent this loop from going on forever.
        if (!isIndefinite)
            {
            totalLen = parentObj.ContentLen();
            }

        while (len < totalLen)
            {
            // Read objects one by one, calculate content lengths together,
            // add contents to target buffer, if given, and calculate also
            // total object lengths together to know when stop. In indefinite
            // case stop when end-of-content tag found.
            temp.Read(aData);

            tempTag = temp.Type();

            if ((tempTag == KBerEndOfContent) ||
                (tempTag == KBerUnknown) ||
                (!(tempTag & parentTag)))
                {
                CleanupStack::PopAndDestroy();  // delete tempBuf;
                return read;                // break
                }

            len += temp.ObjectLen();
            read += temp.ContentLen();

            if (aBuf != 0)
                {
                // Resize buffer, if needed, and append data to it.
                if (aBuf->Length() < read)
                    {
                    // It is impossible to know how long buffer should be,
                    // but add some extra space, so reallocation won't occur
                    // each time.
                    aBuf->ReAllocL(read + (read / 5));
                    ptr = aBuf->Des();
                    }

                temp.Content(tempBuf);
                ptr.Append(*tempBuf);
                }
            }

        CleanupStack::PopAndDestroy();      // delete tempBuf;
        }

    return read;
    }



// -----------------------------------------------------------------------------
// CCrBer::Type
// Get type of the object.
// -----------------------------------------------------------------------------
EXPORT_C TBerTag CCrBer::Type()
    {
	return iType;
    }

// -----------------------------------------------------------------------------
// CCrBer::Value
// Get value of the object. Used in encoding.
// -----------------------------------------------------------------------------
TInt CCrBer::Value()
    {
    return iValue;
    }
// -----------------------------------------------------------------------------
// CCrBer::GetBigInt
// -----------------------------------------------------------------------------
RInteger CCrBer::GetBigInt()
    {
    return iInt;
    }

// -----------------------------------------------------------------------------
// CCrBer::SetValue
// Set value of the object.
// -----------------------------------------------------------------------------
TAny CCrBer::SetValue(TInt aInt)
    {
    iValue = aInt;
    }

// -----------------------------------------------------------------------------
// CCrBer::ValuePtr
// Get valuePtr of the object. (encoding)
// -----------------------------------------------------------------------------
TDesC8* CCrBer::ValuePtr()
    {
    return iValuePtr;
    }

// -----------------------------------------------------------------------------
// CCrBer::LenLen
// Get amount of length bytes.
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrBer::LenLen()
    {
    return iLenLen;
    }

// -----------------------------------------------------------------------------
// CCrBer::ObjectBegin
// Return begin of the whole object.
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrBer::ObjectBegin()
    {
    return iObjectBegin;
    }

// -----------------------------------------------------------------------------
// CCrBer::ContentBegin
// Return begin of the content.
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrBer::ContentBegin()
    {
    return iContentBegin;
    }

// -----------------------------------------------------------------------------
// CCrBer::ObjectLen
// Get length of the whole object.
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrBer::ObjectLen()
    {
	return iObjectLen;
	}

// -----------------------------------------------------------------------------
// CCrBer::ContentLen
// Get length of the content.
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrBer::ContentLen()
    {
    return iContentLen;
    }

// -----------------------------------------------------------------------------
// CCrBer::Indefinite
// Returns true if this object is indefinite length.
// -----------------------------------------------------------------------------
EXPORT_C TBool CCrBer::Indefinite()
    {
    return iIndefinite;
    }

// -----------------------------------------------------------------------------
// CCrBer::Data
// Return pointer to data object.
// -----------------------------------------------------------------------------
EXPORT_C CCrData* CCrBer::Data()
    {
    return iData;
    }

// -----------------------------------------------------------------------------
// CCrBer::Object
// Return whole BER object with tag and stuff. For implementation
// reasons ObjectL returns only max 255 bytes of data.
// -----------------------------------------------------------------------------

EXPORT_C TInt CCrBer::Object(HBufC8* aBuf)
    {
    return BufferL(aBuf, iData, iObjectBegin, iObjectLen);
    }

// -----------------------------------------------------------------------------
// CCrBer::Object
// Return whole BER object with tag and stuff. For implementation
// reasons ObjectL returns only max 255 bytes of data.
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrBer::ObjectL(HBufC* aBuf)
    {
    TUint8 len = 255;


    if (iObjectLen < 255)
        {
        len = (TUint8)iObjectLen;
        }
	
    return BufferL(aBuf, iData, iObjectBegin, len);
    }

// -----------------------------------------------------------------------------
// CCrBer::Content
// Return content of this object. For implementation reasons
// ContentL returns only max 255 bytes of data.
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrBer::Content(HBufC8* aBuf)
    {
    return BufferL(aBuf, iData, iContentBegin, iContentLen);
    }

// -----------------------------------------------------------------------------
// CCrBer::ContentL
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrBer::ContentL(HBufC* aBuf)
    {
    TUint8 len = 255;

    if (iContentLen < 255)
        {
        len = (TUint8)iContentLen;
        }

    return BufferL(aBuf, iData, iContentBegin, len);
    }

// -----------------------------------------------------------------------------
// CCrBer::BufferL
// Read aAmount of data starting from begin to the buffer.
// Return amount of data read. For implementation reasons
// BufferL can be used only to read max 256 bytes of data.
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrBer::BufferL(
    HBufC8*   aBuf,
    CCrData*  aData,
    TUint     aBegin  /* = 0 */,
    TUint     aAmount /* = KReadBufMax */)
    {
    TInt pos = 0;
    TInt begin = aBegin;
    TInt amount = aAmount;
    TInt read = 0;

    if ((aBuf == 0) || (aData == 0))
        {
        // Don't bother to continue.
        return read;
        }

    // Store current position,
    if ((aData->Seek(ESeekCurrent, pos)) != KErrNone)
        {
        // Don't bother to continue.
        return read;
        }

    // Set data pointer to wanted position.
    if (aData->Seek(ESeekStart, begin) == KErrNone)
        {
        // Resize buffer, if needed, and read data.
        if (aBuf->Length() < amount)
            {
            aBuf->ReAllocL(amount);
            }

        TPtr8 ptr = aBuf->Des();

        if (aData->Read(ptr, amount) == KErrNone)
            {
            read = aBuf->Length();
            }
        }

    // Put data pointer back to previous position.
    aData->Seek(ESeekStart, pos);

    return read;
    }

EXPORT_C TInt CCrBer::BufferL(
    HBufC*   aBuf,
    CCrData* aData,
    TUint    aBegin  /* = 0 */,
    TUint8   aAmount /* = 255 */)
    {
    TInt      index = 0;
    TInt      len = 0;
    TInt      amount = NULL;
    TBuf<255> buf;
    HBufC8*   heapBuf = NULL;

    heapBuf = HBufC8::NewLC(255);

    amount = BufferL(heapBuf, aData, aBegin, aAmount);

    len = heapBuf->Length();

    for (index = 0; index < len; index++)
        {
        buf.Append((*heapBuf)[index]);
        }

    *aBuf = buf;

    CleanupStack::PopAndDestroy();      // delete heapBuf;
    // heapBuf = NULL;

    return amount;
    }


// -----------------------------------------------------------------------------
// CCrBer::Level
// Return nesting level of this object.
// -----------------------------------------------------------------------------
EXPORT_C TUint CCrBer::Level()
    {
    return iLevel;
    }

// -----------------------------------------------------------------------------
// CCrBer::SetLevel
// Set nesting level of this object.
// -----------------------------------------------------------------------------
EXPORT_C TUint CCrBer::SetLevel(TUint aLevel)
    {
    iLevel = aLevel;

    return iLevel;
    }

// -----------------------------------------------------------------------------
// CCrBer::AddToContentLen
// Add given integer to content length of this object.
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBer::AddToContentLen(TInt iLen)
    {
    iContentLen += iLen;
    }

// -----------------------------------------------------------------------------
// CCrBer::SetObjectLen
// Calculate object length from tag, length's length, and
// content's length. Used to 'close' indefinite objects, meaning
// that their end-of-contents tag is finally found, so their
// length can be calculated.
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBer::SetObjectLen()
    {
    // Objects length is tag len (always 1) + length byte
    // amount + content length.
    iObjectLen = 1 + iLenLen + iContentLen;

    // If object was of indefinite length, there are
    // two additional end-of-contents bytes at the end.
    if (iIndefinite)
        {
        iObjectLen += 2;
        }
    }

// -----------------------------------------------------------------------------
// CCrBer::SetObjLenWithOutContent
// Parameters:     Content length.
// -----------------------------------------------------------------------------
TAny CCrBer::SetObjLenWithOutContent(TUint aContentLen)
    {
    iObjectLen  = KBerShortLen;             // Tag + len
    if (aContentLen >= KBerLongLengthBit)
        {
        while (aContentLen > 0)                    
            {
            aContentLen >>= 8;              // Next byte
            iObjectLen++;                 
            }
        }
    }

// -----------------------------------------------------------------------------
// Get content functions.
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// CCrBer::GetBooleanL
// Returns value of Boolean object.
// -----------------------------------------------------------------------------
EXPORT_C TBool CCrBer::GetBooleanL()
    {		
    TInt err = KErrNone;
    TASN1DecBoolean dec;                                // ASN1 lib (Symbian)
    TInt pos    = NULL;
    TBool ret   = EFalse;

    HBufC8* buf = HBufC8::NewLC(iObjectLen);            // buffer to object

    TPtr8 ptr   = buf->Des();
   
    err = iData->Read(iObjectBegin,ptr,iObjectLen);     // Read object

    if (err == KErrNone)
        {
        ret = dec.DecodeDERL(ptr, pos); 	        // decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        User::Leave(err);
        }

    CleanupStack::PopAndDestroy();                      // delete buf;
 
    return ret;
    }	

// -----------------------------------------------------------------------------
// CCrBer::GetIntegerL
// Returns value of Integer object.
// -----------------------------------------------------------------------------		
EXPORT_C TInt CCrBer::GetIntegerL()
    {
    TInt err = KErrNone;
    TASN1DecInteger dec;                                // ASN1 lib (Symbian)
    TInt pos    = NULL;
    TInt ret    = NULL;

    HBufC8* buf = HBufC8::NewLC(iObjectLen);             // buffer to object

    TPtr8 ptr   = buf->Des();
   
    err = iData->Read(iObjectBegin,ptr,iObjectLen);     // Read object

    if (err == KErrNone)
        {
        ret = dec.DecodeDERShortL(ptr, pos);        	// decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        User::Leave(err);
        }

    CleanupStack::PopAndDestroy();                      // delete buf;

    return ret;
    }

// -----------------------------------------------------------------------------
// CCrBer::GetIntegerL
// Returns value of Long Integer object.
// -----------------------------------------------------------------------------		
EXPORT_C RInteger CCrBer::GetLongIntegerL()
    {	
    TInt err    = KErrNone;
    TInt pos    = NULL;
    TASN1DecInteger dec;                                // ASN1 lib (Symbian)

    HBufC8* buf = HBufC8::NewLC(iObjectLen);             // buffer to object

    TPtr8 ptr   = buf->Des();
    RInteger value;

    // Read object to buf
    err = iData->Read(iObjectBegin,ptr,iObjectLen);

    if (err == KErrNone)
        {
        value = dec.DecodeDERLongL(ptr, pos);           // decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        User::Leave(err);
        }
    
    CleanupStack::PopAndDestroy();                      // delete buf;

    return value;
    }
  
// -----------------------------------------------------------------------------
// HBufC* CCrBer::GetOidL
// Returns value of object identifier.
// -----------------------------------------------------------------------------
EXPORT_C HBufC* CCrBer::GetOidL()
    {	
    TInt err    = KErrNone;
    TInt pos    = NULL;
    HBufC* ret  = NULL;
    TASN1DecObjectIdentifier dec;                       // ASN1 lib (Symbian)

    HBufC8* buf = HBufC8::NewLC(iObjectLen);            // buffer to object

    TPtr8 ptr   = buf->Des();

    // Read object to buf
    err = iData->Read(iObjectBegin,ptr,iObjectLen);    

    if (err == KErrNone)
        {
        ret = dec.DecodeDERL(ptr, pos);                 // decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        User::Leave(err);
        }

    CleanupStack::PopAndDestroy();                      // delete buf;

    return ret;
    }

// -----------------------------------------------------------------------------
// HBufC8* CCrBer::GetOctetStringL
// Returns value of octet string.
// -----------------------------------------------------------------------------
EXPORT_C HBufC8* CCrBer::GetOctetStringL()
    {	
    TInt pos = NULL;
    HBufC8* ret = NULL;
    TInt err = KErrNone;
    TASN1DecOctetString dec;                            // ASN1 lib (Symbian)

    HBufC8* buf = HBufC8::NewLC(iObjectLen);             // buffer to object

    TPtr8 ptr   = buf->Des();

    // Read object to buf
    err = iData->Read(iObjectBegin,ptr,iObjectLen);    

    if (err == KErrNone)
        {
        ret = dec.DecodeDERL(ptr, pos);                 // decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy(buf);                  // delete buf;
        User::Leave(err);
        }

    CleanupStack::PopAndDestroy(buf);                      // delete buf;

    return ret;
    }

// -----------------------------------------------------------------------------
// HBufC8* CCrBer::GetOctetStringL
// Returns value of octet string.
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBer::GetOctetStringL(CCrData& Trg)
    {	
    TUint pos = iContentBegin;
    TUint end = iContentBegin + iContentLen;
    TInt err = KErrNone;
    TInt len = KBufSize;
    HBufC8* buf = HBufC8::NewL(KBufSize);              // buffer to object
    CleanupStack::PushL(buf);

    TPtr8 ptr   = buf->Des();
    // Read object to buf
    while (pos < end)
        {
        len = end - pos;

        if (len > KBufSize)
            {
            len = KBufSize;
            }

        err = iData->Read(pos, ptr, len);    
        
        if (err == KErrNone)
            {
            Trg.Write(ptr);
            }
        if (err < KErrNone)
            {        
            CleanupStack::PopAndDestroy();
            // buf = NULL;
            User::Leave(err);
            }
        pos += KBufSize;
        }
    CleanupStack::PopAndDestroy(); // buf
    }

// -----------------------------------------------------------------------------
// HBufC8* CCrBer::GetContentStringLC
// Note buf is left to CleanupStack.
// Returns value of content string.
// -----------------------------------------------------------------------------
EXPORT_C HBufC8* CCrBer::GetContentStringLC()
    {	       
    HBufC8* buf = HBufC8::NewL(iContentLen);             // buffer to Content
    CleanupStack::PushL(buf);

    TPtr8 ptr   = buf->Des();

    // Read object to buf
    iData->Read(iContentBegin, ptr, iContentLen);    

    return buf;
    }

// -----------------------------------------------------------------------------
// HBufC* CCrBer::GetIA5StringL
// Returns value of IA5 string object.
// -----------------------------------------------------------------------------
EXPORT_C HBufC* CCrBer::GetIA5StringL()
    {		
    TInt err    = KErrNone;
    TInt pos    = NULL;
    HBufC* ret  = NULL;
    TASN1DecIA5String dec;                              // ASN1 lib (Symbian)

    HBufC8* buf = HBufC8::NewLC(iObjectLen);             // buffer to object
    TPtr8 ptr   = buf->Des();

    // Read object to buf
    err = iData->Read(iObjectBegin,ptr,iObjectLen);    

    if (err == KErrNone)
        {
        ret = dec.DecodeDERL(ptr, pos);                 // decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        User::Leave(err);
        }
     
    CleanupStack::PopAndDestroy();                      // delete buf;
    
    return ret;
    }

// -----------------------------------------------------------------------------
// TTime CCrBer::GetUTCTimeL
// Returns value of UTC time object.
// -----------------------------------------------------------------------------
EXPORT_C TTime CCrBer::GetUTCTimeL()
    {
    TInt err = KErrNone;
    TInt pos = NULL;
    TTime ret = TTime(0);
    TASN1DecUTCTime dec;                                // ASN1 lib (Symbian)

    HBufC8* buf = HBufC8::NewLC(iObjectLen);             // buffer to object
    TPtr8 ptr   = buf->Des();

    // Read object to buf
    err = iData->Read(iObjectBegin,ptr,iObjectLen);    

    if (err == KErrNone)
        {
        ret = dec.DecodeDERL(ptr, pos);                 // decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        User::Leave(err);
        }  

    CleanupStack::PopAndDestroy();                      // delete buf;

    return ret;
    }



// -----------------------------------------------------------------------------
// TTime CCrBer::GetGeneralizedTimeL
// Returns value of generalised time object.
// -----------------------------------------------------------------------------
EXPORT_C TTime CCrBer::GetGeneralizedTimeL()
    {
    TInt err = KErrNone;
    TInt pos = NULL;
    TTime ret = TTime(0);
    TASN1DecGeneralizedTime dec;                        // ASN1 lib (Symbian)

    HBufC8* buf = HBufC8::NewLC(iObjectLen);            // buffer to object
    TPtr8 ptr   = buf->Des();

    // Read object to buf
    err = iData->Read(iObjectBegin,ptr,iObjectLen);    

    if (err == KErrNone)
        {
        ret = dec.DecodeDERL(ptr, pos);                 // decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        User::Leave(err);
        }  

    CleanupStack::PopAndDestroy();                      // delete buf;

    return ret;
    }

// -----------------------------------------------------------------------------
// HBufC* CCrBer::GetPrintStringL
// Returns value of printable string object.
// -----------------------------------------------------------------------------
EXPORT_C HBufC* CCrBer::GetPrintStringL()
    {		
    HBufC* ret = NULL;
    TInt err = KErrNone;
    TInt pos = NULL;
    TASN1DecPrintableString dec;                        // ASN1 lib (Symbian)

    HBufC8* buf = HBufC8::NewLC(iObjectLen);             // buffer to object
    TPtr8 ptr   = buf->Des();

    // Read object to buf
    err = iData->Read(iObjectBegin,ptr,iObjectLen);    

    if (err == KErrNone)
        {
        ret = dec.DecodeDERL(ptr, pos);                 // decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        User::Leave(err);
        }
     
    CleanupStack::PopAndDestroy();                      // delete buf;

    return ret;
    }

// -----------------------------------------------------------------------------
// HBufC* CCrBer::GetTeletextStringL
// Returns value of teletext string object.
// -----------------------------------------------------------------------------
EXPORT_C HBufC* CCrBer::GetTeletextStringL()
    {																			
    TInt err = KErrNone;
    TInt pos = NULL;
    HBufC* ret = NULL;
    TASN1DecTeletexString dec;                          // ASN1 lib (Symbian)

    HBufC8* buf = HBufC8::NewLC(iObjectLen);             // buffer to object
    TPtr8 ptr   = buf->Des();

    // Read object to buf
    err = iData->Read(iObjectBegin,ptr,iObjectLen);    

    if (err == KErrNone)
        {
        ret = dec.DecodeDERL(ptr, pos);                 // decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        User::Leave(err);
        }
    
    CleanupStack::PopAndDestroy();                      // delete buf;
    
    return ret;
    }



// -----------------------------------------------------------------------------
// HBufC8* CCrBer::GetSequence
// Returns value of segunce object.
// -----------------------------------------------------------------------------
EXPORT_C HBufC8* CCrBer::GetSequenceL()
    {
    TInt err = KErrNone;
    HBufC8* buf = HBufC8::NewLC(iObjectLen);             // buffer to object
    TPtr8 ptr   = buf->Des();

    // Read object to buf
    err = iData->Read(ContentBegin(),ptr,ContentLen());
	
    if (err < KErrNone)
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        // buf = NULL;
        User::Leave(err);
        }
    CleanupStack::PopAndDestroy();                      // delete buf;
    // buf = NULL;

    return ptr.AllocL();
    }

// -----------------------------------------------------------------------------
// HBufC* CCrBer::GetVideoString
// Returns value of video string object.
// -----------------------------------------------------------------------------
EXPORT_C HBufC* CCrBer::GetVideoStringL()
    {
    TInt err = KErrNone;
    HBufC* ret = NULL;
    TInt pos   = NULL;
    TASN1DecVideotexString dec;                         // ASN1 lib (Symbian)

    HBufC8* buf = HBufC8::NewLC(iObjectLen);             // buffer to object
    TPtr8 ptr   = buf->Des();

    // Read object to buf
    err = iData->Read(iObjectBegin,ptr,iObjectLen);    

    if (err == KErrNone)
        {
        ret = dec.DecodeDERL(ptr, pos);                 // decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        User::Leave(err);
        }
    
    CleanupStack::PopAndDestroy();                      // delete buf;

    return ret;
    }

// -----------------------------------------------------------------------------
// HBufC* CCrBer::GetGeneralString
// Returns value of general string object.
// -----------------------------------------------------------------------------
EXPORT_C HBufC* CCrBer::GetGeneralStringL()
    {
    TInt err    = KErrNone;
    TInt pos    = NULL;
    HBufC* ret  = NULL;
    TASN1DecGeneralString dec;                          // ASN1 lib (Symbian)

    HBufC8* buf = HBufC8::NewLC(iObjectLen);             // buffer to object
    TPtr8 ptr   = buf->Des();

    // Read object to buf
    err = iData->Read(iObjectBegin,ptr,iObjectLen);    

    if (err == KErrNone)
        {
        ret = dec.DecodeDERL(ptr, pos);                 // decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        User::Leave(err);
        }
    
    CleanupStack::PopAndDestroy();                      // delete buf;

    return ret;
    }

// -----------------------------------------------------------------------------
// HBufC* CCrBer::GetVisibleStringL
// Returns value of visible string object.
// -----------------------------------------------------------------------------
EXPORT_C HBufC* CCrBer::GetVisibleStringL()
    {
    TInt err    = KErrNone;
    HBufC* ret  = NULL;
    TInt pos    = NULL;
    TASN1DecVisibleString dec;                          // ASN1 lib (Symbian)

    HBufC8* buf = HBufC8::NewLC(iObjectLen);             // buffer to object
    TPtr8 ptr   = buf->Des();

    // Read object to buf
    err = iData->Read(iObjectBegin,ptr,iObjectLen);    

    if (err == KErrNone)
        {
        ret = dec.DecodeDERL(ptr, pos);                 // decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        User::Leave(err);
        }
    
    CleanupStack::PopAndDestroy();                      // delete buf;

    return ret;
    }

// -----------------------------------------------------------------------------
// HBufC* CCrBer::GetGraphicsStringL
// Returns value of graphical string object.
// -----------------------------------------------------------------------------
EXPORT_C HBufC* CCrBer::GetGraphicsStringL()
    {
    TInt err    = KErrNone;
    HBufC* ret  = NULL;
    TInt pos    = NULL;
    TASN1DecGraphicString dec;                          // ASN1 lib (Symbian)

    HBufC8* buf = HBufC8::NewLC(iObjectLen);             // buffer to object
    TPtr8 ptr   = buf->Des();

    // Read object to buf
    err = iData->Read(iObjectBegin,ptr,iObjectLen);    

    if (err == KErrNone)
        {
        ret = dec.DecodeDERL(ptr, pos);                 // decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        User::Leave(err);
        }
   
    CleanupStack::PopAndDestroy();                      // delete buf;
    
    return ret;
    }



// -----------------------------------------------------------------------------
// HBufC* CCrBer::GetNumericStringL
// Returns value of numeric string object.
// -----------------------------------------------------------------------------
EXPORT_C HBufC* CCrBer::GetNumericStringL()
    {
    TInt err    = KErrNone;
    HBufC* ret  = NULL;
    TInt pos    = NULL;
    TASN1DecNumericString dec;                          // ASN1 lib (Symbian)

    HBufC8* buf = HBufC8::NewLC(iObjectLen);             // buffer to object
    TPtr8 ptr   = buf->Des();

    // Read object to buf
    err = iData->Read(iObjectBegin,ptr,iObjectLen);    

    if (err == KErrNone)
        {
        ret = dec.DecodeDERL(ptr, pos);                 // decode object
        }
    else
        {        
        CleanupStack::PopAndDestroy();                  // delete buf;
        User::Leave(err);
        }
   
    CleanupStack::PopAndDestroy();                      // delete buf;
    
    return ret;
    }

// -----------------------------------------------------------------------------
//  Encoding
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// CCrBer::CreateBool
// Paremeters:     Value of the boolean object. True or False.
// -----------------------------------------------------------------------------
TAny CCrBer::CreateBool(TBool aBool)
    {
    iType       = KBerBoolean;
    iObjectLen  = KBerBooleanLen;           // Boolean object len is always 3
    iValue      = aBool; 
    }                                       

// -----------------------------------------------------------------------------
// CCrBer::CreateInt
// Paremeters:     Value of the object (positive, null or negative)
//                 e.g. 234 or 0xFFF or -45676.
// -----------------------------------------------------------------------------
TAny CCrBer::CreateInt(TInt aInt)
    {
    iType       = KBerInteger;
    iValue      = aInt; 

    // Find out object len. Tag + len + value.
    if (aInt == 0 || (aInt > (-0x81) &&  aInt < 0x80 ))
        iObjectLen  = 0x03;                 
    
    else if (aInt > (-0x8001) && aInt < 0x8000)                            
        iObjectLen  = 0x04;

    else if (aInt > (-0x800001) && aInt < 0x800000)
        iObjectLen  = 0x05;

    else
        iObjectLen  = 0x06;
    }

// -----------------------------------------------------------------------------
// CCrBer::CreateInt
// Paremeters:     Value of the object (positive, null or negative)
//                 e.g. 234 or 0xFFF or -45676.
// -----------------------------------------------------------------------------
TAny CCrBer::CreateLongInt(RInteger& aData)
    {
    TInt len    = aData.ByteCount();
    iType       = KBerInteger;
    iValue      = NULL;
    iInt        = RInteger::NewL(aData);

    iObjectLen  = KBerShortLen + len;       // Tag + len(lenlen) + string 

    if (len >= KBerLongLengthBit)
        {         
        while (len > NULL)                  // add number of lenbytes 
            {
            len >>= KOctetWidth;            // Next byte
            iObjectLen++;   
            }
        }
    }

// -----------------------------------------------------------------------------
// CCrBer::CreateNull
// -----------------------------------------------------------------------------
TAny CCrBer::CreateNull()
    {
    iType       = KBerNull;                 // Tag
    iObjectLen  = KBerNullLen;              // 05 00
    }

// -----------------------------------------------------------------------------
// CCrBer::CreateOId
// Paremeters:     Pointer to string. e.g.(1.2.43.4335.242)
// -----------------------------------------------------------------------------
TAny CCrBer::CreateOIdL(TDesC8* aString)
    {
    CCrBerSet* set = CCrBerSet::NewLC(1);
    TInt len    = set->AppendObjectIdL(*aString, ETrue);// ask Oid content len
    iType       = KBerOid;
    iValuePtr   = aString;
        
    iObjectLen  = (len > NULL) ? KBerShortLen + len : 0; // tag + len + content
   
    CleanupStack::PopAndDestroy();  // delete set;
    // set = NULL;    
    }



// -----------------------------------------------------------------------------
// CCrBer::CreateString
// Paremeters:     Type of object and pointer to string.
// -----------------------------------------------------------------------------
TAny CCrBer::CreateString(TBerTag aTag, TDesC8* aString)
    {
    TInt len    = aString->Length();
    iType       = aTag;
    iValuePtr   = aString;  
    iObjectLen  = KBerShortLen + len;       // Tag + len(lenlen) + string 

    if (len >= KBerLongLengthBit)
        {         
        while (len > NULL)                  // add number of lenbytes 
            {
            len >>= KOctetWidth;            // Next byte
            iObjectLen++;   
            }
        }
    }

// -----------------------------------------------------------------------------
// CCrBer::CreateString
// -----------------------------------------------------------------------------
TAny CCrBer::CreateString(TBerTag aTag, CCrData* aData)
    {
    TInt len    =  0;
    aData->Size(len);
    iType       = aTag;
    iData   = aData;  
    iObjectLen  = KBerShortLen + len;       // Tag + len(lenlen) + string 

    if (len >= KBerLongLengthBit)
        {         
        while (len > NULL)                  // add number of lenbytes 
            {
            len >>= KOctetWidth;            // Next byte
            iObjectLen++;   
            }
        }
    }

// -----------------------------------------------------------------------------
// CCrBer::CreateStart
// Paremeters:     First parameter is tag type (e.g. seq). Second param is
//                 True if definite length is wanted.
//                 False if indefinite (xx 80) .
// -----------------------------------------------------------------------------
TAny CCrBer::CreateStart(TBerTag aTag, TBool aDefinite)
    {
    iType = aTag;

    // Definite object length = 0 (set rigth length later).
    // Indefinite object length is always 4 (Tag + 80 + 00 + 00)
    if (!aDefinite)
        {
        iObjectLen = KBerIndefiniteLen;
        }
    }                                                   
                                            
// -----------------------------------------------------------------------------
// CCrBer::CreateEnd
// -----------------------------------------------------------------------------
TAny CCrBer::CreateEnd()
    {
    iType   = KBerEndOfContent;                   // =0x00
    iValue  = KBerEndOfContent;
    }                                       

// -----------------------------------------------------------------------------
// CCrBer::CreateBEREncodedObject
// This fuction can be used to create a BER object
// from buffer, which already contains a full BER
// encoded object.
// Paremeters:     Buffer containing BER encoded object.
// -----------------------------------------------------------------------------
TAny CCrBer::CreateBEREncodedObject(TDesC8* aBuffer)
    {
    iType      = KBerEncodedObject;
    iValuePtr  = aBuffer;  
    iObjectLen = aBuffer->Length();
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================
                                                                                
// ---------------------------------------------------------
// E32Dll
// DLL entry point
// ---------------------------------------------------------
//
#ifndef EKA2
GLDEF_C TInt E32Dll(TDllReason /*aReason*/)
    {
    return KErrNone;
    }
#endif
