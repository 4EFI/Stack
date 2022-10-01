
//#define NDUMP
//#define NHASH
#define NCANARY

#include "Config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>

#include "Stack.h"
#include "Assert.h"
#include "LOG.h"

//---------------------------------------------------------------------------

FILE* StackFileOut = NULL;

//---------------------------------------------------------------------------

const Elem_t   StackDataPoisonValue  = 0x5EEDEAD;

const uint64_t StackLeftCanaryValue  = 0xBAADF00D32DEAD32;
const uint64_t StackRightCanaryValue = 0xD0E0D76BB013927C; 

const uint64_t DataLeftCanaryValue   = 0x32BAADF0032DDEAD;
const uint64_t DataRightCanaryValue  = 0x39A3EE7561D89D5E; 

//---------------------------------------------------------------------------

const char* CurFileName = NULL;
const char* CurFuncName = NULL;
int         CurLine     = 0;

//---------------------------------------------------------------------------

const int ResizeUp   =  1;
const int ResizeNum  =  0;
const int ResizeDown = -1;

//---------------------------------------------------------------------------

void StackRecalloc (Stack_t* stack, size_t size, uint64_t leftCanary, uint64_t rightCanary);

//---------------------------------------------------------------------------

enum StackErrors 
{
    NULL_DATA_PTR             = (1 << 0),
    INVALID_SIZE              = (1 << 1),
    INVALID_CAPACITY          = (1 << 2),
    LEFT_CANARY_INVALID       = (1 << 3),
    RIGHT_CANARY_INVALID      = (1 << 4),
    INVALID_HASH_VALUE        = (1 << 5),
    LEFT_DATA_CANARY_INVALID  = (1 << 6),
    RIGHT_DATA_CANARY_INVALID = (1 << 7),
};

static const char* ErrorLines[] = {"Data null ptr",
                                   "Invalid size",
                                   "Invalid capacity",
                                   "Left canary was changed",
                                   "Right canary was changed",
                                   "Hash was changed", 
                                   "Left data canary was changed",
                                   "Right data canary was changed",};

int _StackCtor (Stack_t* stack, int dataSize, const char* mainFileName, 
                                              const char* mainFuncName, 
                                              const char* mainStackName)
{   
    Assert (stack != NULL, 0);

    if (dataSize <= 0) dataSize = 1;

    StackFileOut = LogFile;

    if (StackFileOut == NULL) StackFileOut = stderr;

    ON_CANARY_PROTECTION 
    (
        // Canary left protection 
        stack->canaryLeft = StackLeftCanaryValue;
        // Canary right protection
        stack->canaryRight = StackRightCanaryValue;
    )

    stack->info.mainFileName  = mainFileName;
    stack->info.mainFuncName  = mainFuncName;
    stack->info.mainStackName = mainStackName;
    stack->info.errStatus     = 0;
    stack->info.isStackValid  = true;
    
    ON_HASH_PROTECTION
    (
        stack->info.numHashIgnore = 4;

        static HashIgnore arrHashIgnore[4] = 
        { 
            (char*)(&stack->info.hashValue) - (char*)stack, sizeof (uint64_t),
            (char*)(&stack->info.errStatus) - (char*)stack, sizeof (uint64_t)

            ON_CANARY_PROTECTION 
            (,
            (char*)(&stack->canaryLeft)     - (char*)stack, sizeof (uint64_t),
            (char*)(&stack->canaryRight)    - (char*)stack, sizeof (uint64_t)
            )
        };
    
        stack->info.arrHashIgnorePtr = arrHashIgnore;
    )

    stack->data            = NULL;
    stack->info.stepResize = 2;
    stack->size            = 0;
    stack->capacity        = 0;

    StackRecalloc (stack, dataSize * sizeof (Elem_t), DataLeftCanaryValue, DataRightCanaryValue);

    LOG ("%llu", *((uint64_t*)((unsigned int)stack->data - sizeof (uint64_t))));
    LOG ("%d", stack->data[0]);

    ON_HASH_PROTECTION ( stack->info.hashValue = StackHashProtection (stack); )

    StackDump (stack);

    return 1;
}

//---------------------------------------------------------------------------

int StackDtor (Stack_t* stack)
{
    Assert (stack != NULL, 0);

    ON_CANARY_PROTECTION
    (
        // Canary left protection 
        stack->canaryLeft  = 0;
        // Canary right protection
        stack->canaryRight = 0;
    )

    stack->info.mainFileName  = NULL;
    stack->info.mainFuncName  = NULL;
    stack->info.mainStackName = NULL;
    stack->info.isStackValid  = false;

    
    ON_HASH_PROTECTION ( stack->info.numHashIgnore = 0; )

    stack->data            = NULL;
    stack->info.stepResize = 0;
    stack->size            = 0;
    stack->capacity        = 0;
    
    stack->info.errStatus = 0;

    free (stack->data);

    stack->data = NULL;
}

//---------------------------------------------------------------------------

uint64_t StackHashProtection (Stack_t* stack)
{   
    #ifndef NHASH
        return HashProtection (stack,       sizeof (Stack_t), stack->info.arrHashIgnorePtr, stack->info.numHashIgnore) +
               HashProtection (stack->data, sizeof (Elem_t) * stack->capacity);
    #else
        return 0;
    #endif
}

//---------------------------------------------------------------------------

int StackErrHandler (Stack_t* stack)
{
    Assert (stack != NULL, 0);
    
    stack->info.errStatus = 0;    
    
    if (!stack->data) 
    {       
        stack->info.errStatus |= StackErrors::NULL_DATA_PTR;

        stack->info.isStackValid = false;
    }
    
    if (stack->size < 0 || stack->size > stack->capacity)
    {
        stack->info.errStatus |= StackErrors::INVALID_SIZE;
    }

    if (stack->capacity < 0)
    {
        stack->info.errStatus |= StackErrors::INVALID_CAPACITY;
    }

    ON_CANARY_PROTECTION 
    (
        if (stack->canaryLeft != StackLeftCanaryValue)
        {
            stack->info.errStatus |= StackErrors::LEFT_CANARY_INVALID;
        }

        if (stack->canaryRight != StackRightCanaryValue)
        {
            stack->info.errStatus |= StackErrors::RIGHT_CANARY_INVALID;
        }
    )

    ON_HASH_PROTECTION
    (
        if (stack->info.hashValue != StackHashProtection (stack))
        {
            stack->info.errStatus |= StackErrors::INVALID_HASH_VALUE;

            stack->info.isStackValid = false;
        }
    )

    return 1;
}

//---------------------------------------------------------------------------

int StackErrPrint (Stack_t* stack, int indent)
{
    Assert (stack != NULL, -1);

    if (!stack->info.errStatus) return 1;
    
    uint64_t errStatCopy = stack->info.errStatus;

    int numLines = sizeof (ErrorLines) / sizeof (char*);
    
    for (int i = 0; i < numLines; i++)
    {
        if (errStatCopy & 1) fprintf (StackFileOut, "%*s%s\n", indent, "", ErrorLines[i]);

        errStatCopy >>= 1;
    }

    return 0;
}

//---------------------------------------------------------------------------

void _StackDump (Stack_t* stack)
{
    StackErrHandler (stack);

    PrintSyms ('-', 25, StackFileOut);
    fprintf (StackFileOut, "Start StackDump");
    PrintSyms ('-', 25, StackFileOut);
    fprintf (StackFileOut, "\n");

    fprintf (StackFileOut, "%s at %s(%d)\n", 
                            CurFuncName, 
                            CurFileName, 
                            CurLine);
    
    fprintf (StackFileOut, "Stack[%p] (%s) \"%s\" at %s at %s\n", 
                            stack, 
                            stack->info.errStatus == 0 ? "ok" : "ERROR",
                            stack->info.mainStackName + 1, // Remove & in name
                            stack->info.mainFileName,
                            stack->info.mainFuncName);

    if (stack->info.errStatus)
    {
        fprintf (StackFileOut, "ERRORS:\n");
        fprintf (StackFileOut, "{\n");

        StackErrPrint (stack, TabSize);

        fprintf (StackFileOut, "}\n");    
    }

    fputs ("{\n", StackFileOut);
    {
        ON_HASH_PROTECTION ( fprintf (StackFileOut, "%*shashValue = %lu;\n",   TabSize, "", stack->info.hashValue); )

        fprintf (StackFileOut, "%*ssize      = %lu;\n",   TabSize, "", stack->size);
        fprintf (StackFileOut, "%*scapacity  = %lu;\n\n", TabSize, "", stack->capacity);

        fprintf (StackFileOut, "%*sdata[%p]\n", TabSize, "", stack->data);

        fprintf (StackFileOut, "%*s{\n", TabSize, "");
        {
            for (size_t i = 0; i < stack->capacity; i++)
            {
                if (stack->info.errStatus & StackErrors::NULL_DATA_PTR) break;
                
                bool isEmpty = (i >= stack->size || stack->data[i] == StackDataPoisonValue);
            
                fprintf (StackFileOut, "%*s%s[%lu] = ", 
                                        TabSize * 2, "",  
                                        isEmpty ? " " : "*", i);

                // Print value                        
                if(stack->data[i] == StackDataPoisonValue) 
                {
                    fprintf (StackFileOut, "%s\n", "NAN (POISON)");
                }
                else     
                {   
                    fprintf (StackFileOut, "%d\n",  stack->data[i]);
                }
            }
        }
        fprintf (StackFileOut, "%*s}\n", TabSize, "");

    }
    fputs ("}\n", StackFileOut);

    PrintSyms ('-', 65, StackFileOut);
    fprintf (StackFileOut, "\n");
}

//---------------------------------------------------------------------------

int StackResize (Stack_t* stack, int numResize, bool sideResize)
{
    Assert          (stack != NULL, 0); 
    StackErrHandler (stack); 

    switch (sideResize)
    {
    case ResizeNum:
        if (numResize <= 0) return 0;
        break;
    
    case ResizeUp:
        numResize = size_t(stack->capacity * stack->info.stepResize);
        break;

    case ResizeDown:
        numResize = size_t(stack->capacity / stack->info.stepResize);
        break;

    default:
        return -1;
        break;
    }
     
    StackRecalloc (stack, numResize * sizeof (Elem_t), DataLeftCanaryValue, DataRightCanaryValue);

    //LOG ("%llu", *((uint64_t*)((unsigned int)stack->data - sizeof (uint64_t))));
    //LOG ("%llu", *((uint64_t*)((unsigned int)stack->data + sizeof (Elem_t) * numResize)));

    stack->capacity = numResize;
    
    ON_HASH_PROTECTION ( stack->info.hashValue = StackHashProtection (stack); )

    StackDump (stack);
}

//---------------------------------------------------------------------------

void StackRecalloc (Stack_t* stack, size_t size, uint64_t leftCanary, uint64_t rightCanary)
{
    #ifndef NCANARY
        stack->data = (Elem_t*)CanaryRecalloc (stack->data, size, leftCanary, rightCanary); 
    #else
        stack->data = (Elem_t*)Recalloc       (stack->data, size);
    #endif

    // Fill data with poison (increase data)
    #ifndef NDUMP

    #endif
}

//---------------------------------------------------------------------------

void StackPush (Stack_t* stack, Elem_t value)
{
    Assert          (stack != NULL); 
    StackErrHandler (stack);

    if (!stack->info.isStackValid) return;

    if(stack->size >= stack->capacity) StackResize (stack, 0, ResizeUp);

    stack->data[stack->size] = value;

    stack->size++;

    ON_HASH_PROTECTION ( stack->info.hashValue = StackHashProtection (stack); )
    
    StackDump (stack);
}

//---------------------------------------------------------------------------

Elem_t StackPop (Stack_t* stack)
{
    Assert          (stack != NULL, StackDataPoisonValue); 
    StackErrHandler (stack);

    if (!stack->info.isStackValid) return 0;

    if (stack->size > 0) 
    {
        stack->size--;   

        if (stack->size <= size_t(stack->capacity / (2*stack->info.stepResize)))   
        {
            StackResize (stack, 0, ResizeDown);
        }  

        ON_HASH_PROTECTION ( stack->info.hashValue = StackHashProtection (stack); )
        
        StackDump (stack);

        return stack->data[stack->size];
    }

    ON_HASH_PROTECTION ( stack->info.hashValue = StackHashProtection (stack); )

    StackDump (stack);
    
    return StackDataPoisonValue;
}

//---------------------------------------------------------------------------

int PrintSyms (char sym, int num, FILE* file)
{   
    if (file == NULL) return 0;
    
    for (int i = 0; i < num; i++)
    {
        fputc (sym, file);
    }
}

//---------------------------------------------------------------------------

void* Recalloc (void* arr, size_t size)
{   
    size_t curNum = _msize (arr);
    
    if (curNum == size) return arr;

    arr = (void*)realloc (arr, size);

    if (curNum < size)
    {
        for (int i = curNum; i < size; i++)
        {
            ((char*)arr)[i] = 0;
        }
    }

    return arr;
}

//---------------------------------------------------------------------------

size_t NumBytesHashIgnore (void* arrToComp, void* arr, HashIgnore* arrHashIgnorePtr, size_t numHashIgnore)
{
    if (arr == NULL || arrHashIgnorePtr == NULL) return 0;
    
    // Check Hash Ignored
    for (size_t i = 0; i < numHashIgnore; i++)
    {
        if (arrToComp == (arr + arrHashIgnorePtr[i].pos))
        {
            return arrHashIgnorePtr[i].size;  
        }
    }

    return 0;
}

//---------------------------------------------------------------------------

uint64_t HashProtection (void*       arr, 
                         size_t      size, 
                         HashIgnore* arrHashIgnorePtr,
                         size_t      numHashIgnore)
{
    if (arr == NULL) return 0;

    uint64_t hashValue = 0;

    for (size_t i = 0; i < size; i++) 
    {
        size_t numBytesHashIgnore = 0;
        
        if (arrHashIgnorePtr != NULL)
        {
            numBytesHashIgnore = NumBytesHashIgnore ((char*)arr + i, (char*)arr, arrHashIgnorePtr, numHashIgnore);
        }

        if (numBytesHashIgnore > 0) 
        {
            i += (numBytesHashIgnore - 1);

            continue;
        }

        hashValue += ((*((char*)arr + i))*i);
    }

    return hashValue;
}

//---------------------------------------------------------------------------

int CanaryDataSet (void* data, size_t size, uint64_t leftCanary, uint64_t rightCanary)
{   
    Assert (data != NULL, -1);

    memcpy ((char*)data - sizeof (uint64_t), &leftCanary,  sizeof (uint64_t));
    memcpy ((char*)data + size,              &rightCanary, sizeof (uint64_t)); 
}

//---------------------------------------------------------------------------

void* CanaryRecalloc (void* data, size_t size, uint64_t leftCanary, uint64_t rightCanary)
{
    if ((size_t)data <= 0) data = 0;

    if ((unsigned int)data > sizeof (uint64_t)) 
    { 
       data = (void*)((unsigned int)data - sizeof (uint64_t));
    } 
    
    data = Recalloc (data, size + 2 * sizeof (uint64_t));

    data = (void*)((unsigned int)data + sizeof (uint64_t));

    // Set canaries
    CanaryDataSet (data, size, leftCanary, rightCanary);

    LOG ("asdfasfsfasf");

    return data;
}

//---------------------------------------------------------------------------

int FillArray (void* arr, size_t num, size_t size, void* value, size_t sizeVal)
{
    /*
    Assert (arr   != NULL, 0);
    Assert (value != NULL, 0);
    
    for (size_t i = 0; i < num; i++)
    {
        memcpy (arr + i * size, value, sizeVal);
    }
    */

    return 1;
}

//---------------------------------------------------------------------------