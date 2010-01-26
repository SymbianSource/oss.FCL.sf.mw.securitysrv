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
* Description:   This module contains the implementation of CCrBerSet class. 
*
*/



//  INCLUDE FILES

#include "crber.h"


// -----------------------------------------------------------------------------
//CCrBerSet
//Constructor
// -----------------------------------------------------------------------------
CCrBerSet::CCrBerSet(TInt aGranularity)
    : CArrayPtrSeg<CCrBer>(aGranularity), iLevel(0)
    {
    }

// -----------------------------------------------------------------------------
// CCrBerSet
// Destructor
// This function destructs CCrBerSet object
// -----------------------------------------------------------------------------
CCrBerSet::~CCrBerSet()
    {
    ResetAndDestroy();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::ConstructL
// This function initializes CCrBerSet object's member objects.
// -----------------------------------------------------------------------------
TAny CCrBerSet::ConstructL()
    {
    }

// -----------------------------------------------------------------------------
// CCrBerSet::NewLC
// -----------------------------------------------------------------------------
EXPORT_C CCrBerSet* CCrBerSet::NewLC(TInt aGranularity)
    {
    CCrBerSet* self = new (ELeave) CCrBerSet(aGranularity);
    CleanupStack::PushL(self);

    self->ConstructL();

    return self; 
    }

// -----------------------------------------------------------------------------
// CCrBerSet::NewL
// -----------------------------------------------------------------------------
EXPORT_C CCrBerSet* CCrBerSet::NewL(TInt aGranularity)
    {
    CCrBerSet* self = NewLC(aGranularity);
    CleanupStack::Pop();

    return self; 
    }

// -----------------------------------------------------------------------------
// CCrBerSet::OpenL
// 
// Opens next BER encoded object from the current position of
// given CCrData to this object set. Opens also all nested BER
// objects as deep as aRecursionLevel tells. If it is zero, one
// object is read, if it is one, all objects at first level are
// read, and if it is 0xff, all objects at 255 first levels are
// extracted. This function cannot be used to extract deeper
// structures than that, if you want to go deeper, implement
// a function of your own. Returns the amount of extracted
// objects. The data pointer of CCrData points right after the
// opened BER object after this function is finished.
// Parameters: CCrData* aData    Data object containing the BER
//                               object to be opened.
// Returns:    TUint             Amount of BER objects extracted.
// -----------------------------------------------------------------------------
EXPORT_C TUint CCrBerSet::OpenL(
    CCrData* aData,
    TUint8   aRecursionLevel /* = 1 */)
    {
    TInt    i       = 0;
    TUint   level   = 0;
    TUint8  amount  = 0;
    TInt definiteObjectEnd [255]= {0};
    TBerTag  type    = KBerEndOfContent;
    CCrBer* newObject = 0;

    while (type != KBerUnknown)
        {
        // Don't create new object if old is to be recycled.
        if (newObject == 0)
            {
            newObject = CCrBer::NewLC(level);
            }

        // Read next BER object.
        if ((type = newObject->Read(aData)) != KBerUnknown)
            {                   
            i = 0;
            // check definite seq(e.g) end, and if so, take away 1 level. 
            while (definiteObjectEnd[i] != 0)
                {
                if ((definiteObjectEnd[i]  <= newObject->ObjectLen() 
                    + newObject->ObjectBegin()) &&
                    (definiteObjectEnd[i]) &&
                   !(newObject->IsSeqOrSet(type)))                                    
                    {                
                    level--;                
                    definiteObjectEnd[i] = 0;
                    }
                i++;
                }
            // If type is explicit or implicit constructed or sequence
            // or set with indefinite length, we are one level deeper.
            if (CCrBer::IsSeqOrSet(type))
                 {
                if (newObject->Indefinite())
                    {
                    level++;
                    }
                else 
                    {               
                    level++;
                    i = 0;
                    while (definiteObjectEnd[i] != 0)
                        {
                        i++;
                        }
                    definiteObjectEnd[i] = newObject->ObjectLen() 
                        + newObject->ObjectBegin();                                      
                    }
				// Changed by junuutin on 3.5.2004:				
				if ((type == KBerImplicit) || (type == KBerExplicit))
					{
						// Don't try to parse the content of context-specified stuff
			            // Jump to next object.
						TInt currentPos = 0;
						aData->Seek(ESeekCurrent, currentPos);
                        TInt pos = currentPos + newObject->ContentLen(); 
                        aData->Seek(ESeekStart, pos);
                        level--;
					} 
				else if ((type == KBerImplicitConstructed) || (type == KBerExplicitConstructed))
                    {
                     TInt currentPos = 0;
                     aData->Seek(ESeekCurrent, currentPos);
                     CCrBer* tempObject = CCrBer::NewL();
                     
                     // Read next BER object.
                     TBerTag tempType = tempObject->Read(aData);
                     if (!(CCrBer::IsKnownType(tempType)))
                        {
                        // Jump to next object.
                        TInt pos = currentPos + newObject->ContentLen(); 
                        aData->Seek(ESeekStart, pos);
                        level--;

                        }
                     else // return current position.
                         {
                         aData->Seek(ESeekStart, currentPos);
                         }

                    delete tempObject;
                    tempObject = NULL;
                    } 
                } 

            // If type is end-of-content tag, last constructed object,
            // sequence or set with indefinite length has ended, we
            // are one level lower.
            if (type == KBerEndOfContent)
                {
                level--;

                // Also set new object's level to current,
                // because that's where it actually belongs.
                newObject->SetLevel(level);
            
            
                // Close last sequence or set.
                CloseLastSeqOrSet(level);
                }

            // If it is of known type, append object to this set,
            // null the pointer and continue to look for next objects.
            // Append only if wanted recursion level not exceeded.
            if ((level <= aRecursionLevel) || (aRecursionLevel == 255))
                {
                AppendAndUpdateL(newObject);
                CleanupStack::Pop(); //newObject
                newObject = NULL;
                }
            else
                {
                // Anyway previous open objects have to be
                // updated with the length of this object.
                Update(newObject);
                }

            // Anyway we have one item more.
            
            amount++;
            }
        else
            {
            // Otherwise delete object and stop the whole thing.
            // Don't return those bloody errors, because we cannot
            // know what the caller was looking for, let him evaluate
            // the results and decide whether this was a failure or not.
            //delete newObject;
            CleanupStack::PopAndDestroy();

            // go to return 
            type = KBerUnknown;         
            }
        }

    return amount;
    }

// -----------------------------------------------------------------------------
// CCrBerSet::AppendAndUpdateL
// Appends given new object into this set. Updates all previous
// items in this set, which have indefinite length, with the
// length of the new object.
// Parameters: CCrBer* aBerObject    New BER object to be added to this set
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::AppendAndUpdateL(CCrBer* aBerObject)
    {
    Update(aBerObject);

    AppendL(aBerObject);
    }

// -----------------------------------------------------------------------------
// CCrBerSet::Update
// Updates all previous items in this set, which have
// indefinite length, with the length of the new object.
// Parameters: CCrBer* aBerObject    New BER object with whom
//                                   this set is updated.
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::Update(CCrBer* aBerObject)
    {
    TInt  add = 0;
    TInt  index = 0;
    TInt  size = 0;
    TUint berObjectLevel = 0; 

    CCrBer* obj;

    if (aBerObject == 0)
        {
        return;
        }

    // Loop backwards through set and append length of the new object
    // to the lengths of each found upper constructed, sequence or set
    // objects with indefinite lengths, until first 'closed' from upper
    // level found.

    // Use object length, unless this is an object, whose total length is
    // not yet known. In that case use tag length (always 1) combined with
    // the amount of the length bytes.
    if ((add = aBerObject->ObjectLen()) == 0)
        {
        add = 1 + aBerObject->LenLen();

        // If new object is of indefinite length, still
        // add two more bytes for the end-of-content tag.
        if (aBerObject->Indefinite())
            {
            add += 2;
            }
        }

    berObjectLevel = aBerObject->Level();

    size = Count();

    for (index = size - 1; index >= 0; index--)
        {
        obj = At(index);

        if ((obj != 0) &&
            (obj->Level() < berObjectLevel) &&
            (obj->Indefinite()))
            {
            if ((obj->Type() == KBerSeq) ||
                (obj->Type() == KBerSet) ||
                (obj->Type() == KBerImplicitConstructed) ||
                (obj->Type() == KBerExplicitConstructed))
                {
                if (obj->ObjectLen() == 0)
                    {
                    obj->AddToContentLen(add);
                    }
                else
                    {
                    index = -1;                 // break;
                    }
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CloseLastSeqOrSet
// Finds last open indefinite length sequence
// or set from this set at given level and closes it.
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CloseLastSeqOrSet(TUint aLevel)
    {
    TInt    index = 0;
    TInt    size  = 0;
    TBerTag type  = KBerUnknown;

    CCrBer* obj = NULL;

    // Loop backwards through set and find first sequence or set with
    // indefinite length at same level. 'Close' it by setting its object
    // length.
    size = Count();

    for (index = size - 1; index >= 0; index--)
        {
        obj = At(index);

        if ((obj != 0) &&
            (obj->Level() == aLevel) &&
            (obj->Indefinite()))
            {
            type = obj->Type();

            if ((type == KBerSeq) ||
                (type == KBerSet) ||
                (type == KBerImplicitConstructed) ||
                (type == KBerExplicitConstructed))
                {
                obj->SetObjectLen();

                // Update upper objects with closed object.
                Update(obj);

                index = -1;             // break;
                }
            }
        }
    }

// -----------------------------------------------------------------------------
//  Encoding
// -----------------------------------------------------------------------------
    
// -----------------------------------------------------------------------------
// CCrBerSet::AppendBool
// Write boolean object to CCrData.
// Parameters:     Boolean value. ETrue or EFalse ( 0 / 1 ).
// Return Values:  Error code
// -----------------------------------------------------------------------------
TInt CCrBerSet::AppendBoolL(TBool aBool)
    {
    TInt err = KErrNone;
    TBuf8<1> value;

    if (aBool)           
        {
        value.Append(KBerBooleanTrue);            
        }
    else                  
        {
        value.Append(KBerBooleanFalse);
        }

    err = AppendStringL(KBerBoolean, value);                       

    return err;
    }

// -----------------------------------------------------------------------------
// CCrBerSet::AppendInt
// Write integer (0x02) object to CCrData. Parameter is either
// a positive whole number, or a negative whole number, or zero.
// value : from - 0xFFFFFFF to +0xFFF FFFF
// Parameters:     Integer value e.g. 1024 or 0xABC. 
// Return Values:  Error code
// -----------------------------------------------------------------------------
TInt CCrBerSet::AppendIntL(TInt aData)
    {
    TInt err = KErrNone;
    TBuf8<8> indexBuf;
    TUint len   = 0;
 
    HBufC8* buf = NULL;
    
    buf = HBufC8::NewLC(KIntBufSize);
    TPtr8 data  = buf->Des();
              
    if (aData >= 0)
        { 
        if (aData >= KBerLongLengthBit)
            {      
            while (aData > 0)                    
                {
                TInt8 byte;
                byte = TInt8(aData);        // Take byte
                indexBuf.Append(byte);      // and add it to indexBuf   
                aData >>= KOctetWidth;      // Next byte
                len++;                 
                }       
            if (indexBuf[len-1] >= KBerLongLengthBit)   // if high order bit=1
                {
                indexBuf.Append(0);         // e.g. content 00 FF no FF 
                len++;                      // (FF = neg num)
                }
            }
        else                                // only one byte
            {
            indexBuf.Append(aData);         // so add directly
            len++;
            }
        }
   
    else
        {
        aData = -(aData + 1);               // convert to positive

        while ((aData > 0) || (len < 1))    // (len < 1) because -1 -> 0 ->FF            
            {
            TInt8 byte;
            byte = TInt8(aData);            // Take byte
            indexBuf.Append(~byte);         // and add complement to buf
            aData >>= KOctetWidth;          // Next byte
            len++;                 
            }
        if (indexBuf[len-1] < KBerLongLengthBit) // if high order bit=0
                {
                indexBuf.Append(0xFF);  
                len++;                  
                }
        }

    for (TInt i=len ; i > 0 ; i--)
        {
        data.Append(indexBuf[i - 1]);       // add data to buffer
        }

    err = AppendStringL(KBerInteger, data);        // write data to file
    
    CleanupStack::PopAndDestroy();
    buf = NULL;

    return err;
    }



TInt CCrBerSet::AppendLongIntL(RInteger& aData)
    {
    TInt err = KErrNone;
    
    err = AppendStringL(KBerInteger, *aData.BufferLC());
    
    CleanupStack::PopAndDestroy(); 

    return err;
    }


// -----------------------------------------------------------------------------
// CCrBerSet::AppendNull
// Write null (05 00) object to CCrData.
// Parameters:     None
// Return Values:  Error code
// -----------------------------------------------------------------------------
TInt CCrBerSet::AppendNull()
    {
    TInt err = KErrNone;
    TBuf8<2> data;
     
    data.Append(KBerNull);                      // Add Null tag 05
    data.Append(KBerNullContent);               // and null;    00
    
    err = iTarget->Write(data);                 // write data-buffer to CCrData 
    
    return err;
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CloseIndefinite
// Write end of indefinite length (add 00 00) to CCrData.
// Return Values:  Error code
// -----------------------------------------------------------------------------
TInt CCrBerSet::CloseIndefinite()
    {   
    TInt err = KErrNone;
    TBuf8<2> data;                  
    data.Append(KBerEndOfContent);              // add end of indefinite length 
    data.Append(KBerEndOfContent);              // ( 00 00 )
    err = iTarget->Write(data);                 // write data-buffer to file
    
    return err;
    }

// -----------------------------------------------------------------------------
// CCrBerSet::AppendConstructed
// Write constructed type to CCrData.
// Parameters:     First param is type of constructed tag
//                 e.g. 04 for octetS.  Second parameter is length or 0 for
//                 indefinite length. e.g. if params are 4 and 0 --> 24 00
// Return Values:  Error code
// -----------------------------------------------------------------------------
TInt CCrBerSet::AppendConstructedL(TBerTag aTag, TUint aLength)
    {
    TInt err    = KErrNone;
    HBufC8* buf = NULL; 
    
    buf = HBufC8::NewLC(8);
    TPtr8 data  = buf->Des();

    if (aTag < KBerConstructedBit)      // if tag not consist ConstructedBit
        {
        aTag += KBerConstructedBit;     // add ConstructedBit
        }

    if (aLength == KBerEndOfContent)            // indefinite length 
        {
        data.Append(aTag);                      // add Tag
        data.Append(KBerLongLengthBit);         // add indefinite length
        }

    else if (aLength < KBerLongLengthBit)       // if short len
        {
        data.Append(aTag);                      // add Tag    
        data.Append(aLength);                   // add len
        }

    else                                        // else we have long len
        {
        TBuf8<6> indexBuf;              
        TInt lenlen = 0;                        // number of length bytes

        while (aLength > 0)
            {
            TInt8 byte;           
            byte = TInt8(aLength);                
            indexBuf.Append(byte);
            aLength >>= KOctetWidth;
            lenlen++;
            }

        data.Append(aTag);                          // Add Tag 
        data.Append(lenlen + KBerLongLengthBit);    // Lenlen byte
            
        for (TInt i = lenlen ; i > 0 ; i--)
            {
            data.Append(indexBuf[i - 1]);       // length bytes
            }
        }                                           
        
    err = iTarget->Write(data);                 // write to CCrData
                  
    CleanupStack::PopAndDestroy();
    buf = NULL;

    return err;
    }

// -----------------------------------------------------------------------------
// CCrBerSet::AppendObjectId
// Write object id object to CCrData.
// Parameters:     Object ID. e.g. _L8("1.2.3.4.5"). Second parameter is 
//                 true if you don't want write object to CCrData.
// Return Values:  Error code (or length of object if aOnlyLen is true).
// -----------------------------------------------------------------------------
TInt CCrBerSet::AppendObjectIdL(const TDesC8& aString, TBool aOnlyLen)
    {
    TChar ch   = NULL;
    TUint num  = 0;
    TUint sum  = 0; 
    TInt index = 0;
    TInt len   = 0;
    TInt i     = 0;
    TInt start = 0;
    TInt err   = KErrNone;
    TBuf8<16> outbuf;

    HBufC8* fileBuf  = NULL;
    HBufC8* str = NULL;

    fileBuf = HBufC8::NewLC(aString.Length() + 1);
    str = aString.AllocLC();
  
    TPtr8 ptrFileBuf = fileBuf->Des();
    TPtr8 strptr = str->Des();
    
    // Check valid ( string must start "x.z.*", x < 10 and z < 10)
    if ((len = strptr.Locate('.')) == KObjectIDDot)            
        { 
        ch  = strptr[i];                            // take first number
        num = ch.GetNumericValue();                 // convert ascii to num                     

        // First byte = (first number * 40) + second number  
        sum = num * KObjectIDFirstFactor;           // num * 4                              
       
        strptr.Delete(start, len + 1);              // delete 'x.'

        // also second num must be smaller than 10
        if (!((strptr.Locate('.')) == KObjectIDDot))                   
            {
            err = KErrArgument;
            }

        while (strptr.Length() > 0)          
            {
            len = strptr.Locate('.');               // Find next dot
            
            if (len == KErrNotFound)                // if not found: take all 
                {
                len = strptr.Length();              // string len
                } 

            for (i = 0; i < len; i++)               
                {
                ch  = strptr[i];
                num = ch.GetNumericValue();        
                sum = sum * KObjectIDSecondFactor + num;    // sum * 10 + num           
                num = 0;
                }
       
            strptr.Delete(start, len + 1);          // delete also dot

            for (index = 0; (sum >= KBerLongLengthBit); index++)
                {   
                outbuf.Append(sum % KBerLongLengthBit);        // add remainder 
                sum = (sum - outbuf[index]) / KBerLongLengthBit;// take new 
                }

            outbuf.Append(sum);

            for (; index >= 0 ; index--)
                {
                if (!index)                      
                    {
                    ptrFileBuf.Append(outbuf[index]);
                    }
                else                        
                    {
                    ptrFileBuf.Append(outbuf[index] + KBerLongLengthBit);
                    }   
                }
            
            sum = 0;
            outbuf.Delete(start, outbuf.Length());       
            }
        
        if ((!aOnlyLen && ptrFileBuf.Length()) && (err == KErrNone))
            {
            err = AppendStringL(KBerOid, ptrFileBuf);   // write object to CCrData
            }
        else if (err == KErrNone)
            {
            len = ptrFileBuf.Length();              // return object length 
            err = len;
            }                                       // else return error
        }
    else                                            
        {                                           
        err = KErrArgument;                         // return argument error
        }

    CleanupStack::PopAndDestroy(2);                  // delete fileBuf, str;
    return err;                                     // return error code
    }

// -----------------------------------------------------------------------------
// CCrBerSet::AppendString
// Write string object to CCrData.
// Parameters:     Object type (tag) and string. e.g. (KBerIA5S, _L("Hello"))      
// Return Values:  Error code
// -----------------------------------------------------------------------------
TInt CCrBerSet::AppendStringL(TBerTag aTag, const TDesC8& aString)
    {
    TInt err = KErrNone;
    TInt len = aString.Length();

    HBufC8* buf  = NULL;
    
    buf = HBufC8::NewLC(KBerShortLen + LEN_BYTES_MAX);
    TPtr8   data = buf->Des();
 
    if (len < KBerLongLengthBit)                    // If short len:
        {
        data.Append(aTag);                          // Add Tag
        data.Append(len);                           // length 
        }                                                  
    else                                            // Else long
        {
        TBuf8<8> indexBuf;
        TUint lenlen = 0;                           // number of len bytes
              
        while (len > 0)
            {
            TInt8 byte;                             
            byte = TInt8(len);                      // Take byte
            indexBuf.Append(byte);                  // append it to buf
            len >>= KOctetWidth;                    // next byte
            lenlen++;                                                        
            }

        data.Append(aTag);                          // Add tag
        data.Append(lenlen + KBerLongLengthBit);    // now add lenlen byte
                                                    // 0x80 + lenlen

        for (TInt i=lenlen ; i > 0 ; i--)
            {
            data.Append(indexBuf[i - 1]);           // add len bytes
            }

        len = aString.Length();
        }

    err = iTarget->Write(data);                     // write buffer to file
    err = iTarget->Write(aString);                     // write buffer to file
                  
    CleanupStack::PopAndDestroy();                  // delete buf;                         
    buf = NULL;    

    return err;
    }
 
// -----------------------------------------------------------------------------
// CCrBerSet::AppendString
// Write string object to CCrData.
// Parameters:     Object type (tag) and CCrData      
// Return Values:  Error code
// -----------------------------------------------------------------------------
TInt CCrBerSet::AppendStringL(TBerTag aTag, CCrData* const aData)
    {
    TInt err = KErrNone;
    TInt len = 0;
    aData->Size(len);

    HBufC8* buf  = NULL;
    buf = HBufC8::NewLC(KBerShortLen + LEN_BYTES_MAX);
    TPtr8 data = buf->Des();
 
    if (len < KBerLongLengthBit)                    // If short len:
        {
        data.Append(aTag);                          // Add Tag
        data.Append(len);                           // length 

        }                                                  
    else                                            // Else long
        {
        TBuf8<8> indexBuf;
        TUint lenlen = 0;                           // number of len bytes
              
        while (len > 0)
            {
            TInt8 byte;                             
            byte = TInt8(len);                      // Take byte
            indexBuf.Append(byte);                  // append it to buf
            len >>= KOctetWidth;                    // next byte
            lenlen++;                                                        
            }

        data.Append(aTag);                          // Add tag
        data.Append(lenlen + KBerLongLengthBit);    // now add lenlen byte
                                                    // 0x80 + lenlen

        for (TInt i=lenlen ; i > 0 ; i--)
            {
            data.Append(indexBuf[i - 1]);           // add len bytes
            }

        aData->Size(len);

        }

    err = iTarget->Write(data);                     // write buffer to file
    
    // Write aData to iTarget
    HBufC8* tempBuf = tempBuf = HBufC8::NewLC(KBufSize);
    
    // write aData to iTarget in loop.
    TPtr8 ptr = tempBuf->Des();
    TInt size = 0;
    aData->Size(size);
    TInt left = size;
    TInt handleThisTime = 0;
    TInt handled = 0;

    while (left > 0)
        {
        if (left > KBufSize)
            {
            handleThisTime = KBufSize;
            }
        else
            {
            handleThisTime = left;
            }
        aData->Read(handled, ptr, handleThisTime);
        iTarget->Write(ptr);
        ptr.Zero();

        handled = handled + handleThisTime;
        left = size - handled;
        }

    CleanupStack::PopAndDestroy(2);
    return err;
    }
 
// -----------------------------------------------------------------------------
// CCrBerSet::AppendBerEncodedObject
// Write BER encoded object to CCrData.
// Parameters:     Buffer containing BER encoded object.      
// Return Values:  Error code
// -----------------------------------------------------------------------------
TInt CCrBerSet::AppendBerEncodedObject(const TDesC8& aString)
    {
    TInt err = KErrNone;

    err = iTarget->Write(aString); // write buffer to file
                  
    return err;
    }
 

// -----------------------------------------------------------------------------
// CCrBerSet::CreateBoolL
// Function creates Ber encoded boolean object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// Parameters:     Value of the object
// Return Values:  None 
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateBoolL(TBool aValue)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateBool(aValue);
    
    AppendL(newObject);              // add to set
    
    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateIntL
// Function creates Ber encoded integer object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// Parameters:     Value of the object
// Return Values:  None 
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateIntL(TInt aValue)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateInt(aValue);
    
    AppendL(newObject);              // add to set
    
    CleanupStack::Pop();
    }


EXPORT_C TAny CCrBerSet::CreateLongIntL(RInteger& aValue)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    
    newObject->CreateLongInt(aValue);
    
    AppendL(newObject);              // add to set
    
    CleanupStack::Pop();
    }


// -----------------------------------------------------------------------------        
// CCrBerSet::CreateNullL
// Function creates Ber encoded null object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateNullL()
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateNull();                
    
    AppendL(newObject);              // add to set
    
    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateIA5L
// Function creates Ber encoded IA5 string object (CCrBer object).
// Write a object to CCrData  by Flush(..) 
// Parameters:     IA5 string
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateIA5L(TDesC8& aString)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateString(KBerIA5S, &aString);
    
    AppendL(newObject);              // add to set

    CleanupStack::Pop();
    }



// -----------------------------------------------------------------------------
// CCrBerSet::CreateNumericL
// Function creates Ber encoded Numeric string object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// Parameters:     Numeric string
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateNumericL(TDesC8& aString)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateString(KBerNumS, &aString);
        
    AppendL(newObject);              // add to set

    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreatePrintableL
// Function creates Ber encoded Printable string object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// Parameters:     Printable string
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreatePrintableL(TDesC8& aString)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateString(KBerPrS, &aString);
    AppendL(newObject);              // add to set

    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateT61L
// Function creates Ber encoded T61 string object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// Parameters:     T61 string (= a teletext string)
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateT61L(TDesC8& aString)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateString(KBerT61S, &aString);
    AppendL(newObject);              // add to set

    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateVideoTexL
// Function creates Ber encoded Video text object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// Parameters:     VideoTex string
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateVideoTexL(TDesC8& aString)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateString(KBerVideoS, &aString);
    AppendL(newObject);              // add to set

    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateUTCL
// Function creates Ber encoded UTC time object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// Parameters:     UTC time (string)
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateUTCL(TDesC8& aString)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateString(KBerUtc, &aString);
    AppendL(newObject);              // add to set

    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateGenTimeL
// Function creates Ber encoded generalised time object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// Parameters:     Generalised time (string)
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateGenTimeL(TDesC8& aString)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateString(KBerGenTime, &aString);
    AppendL(newObject);              // add to set

    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateGraphicalL
// Function creates Ber encoded graphical string object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// Parameters:     Graphical string
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateGraphicalL(TDesC8& aString)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateString(KBerGraphS, &aString);
    AppendL(newObject);              // add to set

    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateVisibleL
// Function creates Ber encoded visible string object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// Parameters:     Visible string
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateVisibleL(TDesC8& aString)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateString(KBerVisibleS, &aString);    // save type & pointer
    AppendL(newObject);              // add to set

    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateGeneralL
// Function creates Ber encoded general string object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// Parameters:     Generalised string
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateGeneralL(TDesC8& aString)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateString(KBerGeneralS, &aString);    // save type & pointer
    AppendL(newObject);              // add to set
    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateOctetL
// Function creates Ber encoded octet string object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// Parameters:     Octet string
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateOctetL(TDesC8& aString)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateString(KBerOctetString, &aString); // save type & pointer
    AppendL(newObject);              // add to set

    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateOctetL
// Function creates Ber encoded octet string object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// Parameters:     CCrData* 
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateOctetL(CCrData* aData)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateString(KBerOctetString, aData); // save type & pointer
    AppendL(newObject);              // add to set

    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateOIdL
// Function creates Ber encoded object identifier object (CCrBer object).
// Write a object to CCrData (file) by Flush(..) 
// Parameters:     object identifier content e.g.(_L8("1.2.3.4445"))
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateOIdL(TDesC8& aString)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateOIdL(&aString);         // save type and pointer to data

    AppendL(newObject);              // add to set

    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateSeqStartL
// Function creates start of sequence (0x30). End this by EndL().
// Parameters:     True for definite length sequence, false for indefinite
//                 length 
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateSeqStartL(TBool aDefinite)
    {
    CCrBer* newObject = CCrBer::NewLC(++iLevel);
    newObject->CreateStart(KBerSeq, aDefinite); 
        
    if (iLevel > iMaxLevel) 
        {
        iMaxLevel = iLevel;                 // iMaxLevel is used in Flush(..)
        }

    AppendL(newObject);              // add to set

    CleanupStack::Pop();
    }



// -----------------------------------------------------------------------------
// CCrBerSet::CreateSetStartL
// Start set (31 xx) object. End this set by EndL(). 
// Parameters:     Parameter is true for definite length, false for
//                 indefinite length. 
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateSetStartL(TBool aDefinite)
    {
    CCrBer* newObject = CCrBer::NewLC(++iLevel);
    newObject->CreateStart(KBerSet, aDefinite);   
        
    if (iLevel > iMaxLevel) 
        {
        iMaxLevel = iLevel;                 // iMaxLevel is used in Flush(..)
        }

    AppendL(newObject);                     // add to set

    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateConstructedStartL
// Function creates start of constructed type object. End this by EndL(). 
// Parameters:     First parameter is object type (tag), Second parameter is 
//                 true for definite length, false for indefinite length. 
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateConstructedStartL(TBerTag aTag, TBool aDefinite)
    {
    CCrBer* newObject = CCrBer::NewLC(++iLevel);
    if (aTag < KBerConstructed)
        {
        aTag += KBerConstructedBit;         // e.g. 0x04 --> 0x24
        }

    newObject->CreateStart(aTag, aDefinite);   
        
    if (iLevel > iMaxLevel) 
        {
        iMaxLevel = iLevel;                 // iMaxLevel is used in Flush(..)
        }
    AppendL(newObject);              // add to set
    CleanupStack::Pop();
    }



// -----------------------------------------------------------------------------
// CCrBerSet::CreateEndL
// Function close last constructed (e.g. seq) object. 
// (definite or indefinite). 
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateEndL()
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel--);
    newObject->CreateEnd();

    AppendL(newObject);              // add to set
    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CCrBerSet::CreateBEREncodedObjectL
// This fuction can be used to create a BER object
// from buffer, which already contains a full BER
// encoded object.
// Parameters:     Buffer containing BER encoded object.
// -----------------------------------------------------------------------------
EXPORT_C TAny CCrBerSet::CreateBEREncodedObjectL(TDesC8& aString)
    {
    CCrBer* newObject = CCrBer::NewLC(iLevel);
    newObject->CreateBEREncodedObject(&aString); // save pointer
    AppendL(newObject);                          // add to set
    CleanupStack::Pop();                         // newObject
    }

// -----------------------------------------------------------------------------
// CCrBerSet::Flush
// Function writes all created crber objects (which are AppendL to set)
// to file. Function also set length for definite lengths.
// Call this function then crberSet is ready (all berObjects are given).
// Parameters:     Reference to target CCrData (file)
// Return Values:  Number of written objects or error code. 
// -----------------------------------------------------------------------------
EXPORT_C TInt CCrBerSet::FlushL(CCrData* aTarget)
    {
    TInt index = 0;
    TInt i = 0;
    TUint level = 0;
    TUint8 type = 0;
    TInt err = KErrNone;
    CCrBer* berObject = NULL;
    CCrBer* newObject = NULL;
    TInt size = Count();                    // number of the ber-objects

    iTarget = aTarget;

    if (iLevel || !size)                    // end count != start count
        {                                   // or if no CrBer objects 
        return KErrArgument;
        }
    
    // Set definite lengths  
    for (level = iMaxLevel; level > 0; level--)     // check levels (except 0)
        {                                           
        for (index = 0 ; index < size ; index++)    // all objects
            {
            berObject = At(index);                  
        
            if ((berObject->Type() >= KBerConstructed) &&   // Constructed 
                (!berObject->ObjectLen())              &&   // definite len
                (berObject->Level() == level))              // same level
                {
                for (i = index + 1; i < size; i++)
                    {
                    newObject = At(i);
 
                    if ((newObject->Type() == KBerEndOfContent) &&
                        (newObject->Level() == berObject->Level()) &&
                        (newObject->Value() >= 0))
                        {
                        berObject->SetObjLenWithOutContent(
                            berObject->ContentLen());
                        newObject->SetValue(-1);    // this end is now used
                        i = size;                   // end found, go away
                        }
                    else                                
                        {
                        berObject->AddToContentLen(newObject->ObjectLen());
                        }                       
                    }
                }
            }
        }


    // Write objects to file
    for (index = 0; index < size; index++)
        {
        berObject = At(index);
        type = berObject->Type();

        if (type > KBerConstructed && type != KBerEncodedObject )
            {
            type = KBerConstructed;
            }
        
        switch (type)
            {
            case KBerBoolean:
                {
                err += AppendBoolL(berObject->Value());
                break;
                }
            case KBerInteger:
                {
                if (!berObject->Value() && (berObject->GetBigInt() != NULL))
                    {
					RInteger integer = berObject->GetBigInt();
                    err += AppendLongIntL(integer);
                    }
                else
                    {
                    err += AppendIntL(berObject->Value());
                    }
                break;
                }  
            case KBerNull:
                {
                err += AppendNull();
                break;
                }
            case KBerOid:
                {
                err += AppendObjectIdL(*berObject->ValuePtr());
                break;
                }
            case KBerBitString:
            case KBerOctetString:             
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
                {
                CCrData* tempData = berObject->Data();
                if (tempData != NULL)
                    {
                    err += AppendStringL((berObject->Type()),
                                    (berObject->Data()));
                    }
                else
                    {
                    err += AppendStringL((berObject->Type()),
                                    (*berObject->ValuePtr()));
                    }
                break;
                }
            case KBerSeq:
            case KBerSet:
            case KBerConstructed:
                {
                err += AppendConstructedL((berObject->Type()), 
                                         (berObject->ContentLen()));
                break;
                }
            case KBerEncodedObject:
                {
                err += AppendBerEncodedObject(
                    (*berObject->ValuePtr()));
                break;
                }
            case KBerEndOfContent:
                {
                if (!berObject->Value())            // Indefinite len
                    {   
                    err += CloseIndefinite();
                    }
                break;
                }
            }
        }
    if (err < KErrNone)
        {
        return KErrGeneral;
        }
    else    
        {
        return size;
        }
    }
