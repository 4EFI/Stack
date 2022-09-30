
//#define NDUMP

#include "Config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "Stack.h"
#include "Assert.h"
#include "LOG.h"

//---------------------------------------------------------------------------

FILE* StackFileOut = NULL;

//---------------------------------------------------------------------------

Elem_t StackDataPoisonValue = 0xBAAAAAAD;

uint64_t StackLeftCanaryValue  = 0xBAADF00D;
uint64_t StackRightCanaryValue = 0xDEADDEAD;

//---------------------------------------------------------------------------

const char* CurFileName = NULL;
const char* CurFuncName = NULL;
int         CurLine     = 0;

//---------------------------------------------------------------------------

enum StackErrors 
{
    NULL_DATA_PTR        = (1 << 0),
    INVALID_SIZE         = (1 << 1),
    INVALID_CAPACITY     = (1 << 2),
    LEFT_CANARY_CHANGED  = (1 << 3),
    RIGHT_CANARY_CHANGED = (1 << 4),
    INVALID_HASH_VALUE   = (1 << 5)
};

static const char* ErrorLines[] = {"Data null ptr",
                                   "Invalid size",
                                   "Invalid capacity",
                                   "Left canary was changed",
                                   "Right canary was changed",
                                   "Hash was changed"};

int _StackCtor (Stack_t* stack, int dataSize, const char* mainFileName, 
                                              const char* mainFuncName, 
                                              const char* mainStackName)
{   
    Assert (stack != NULL, 0);

    StackFileOut = LogFile;

    if (StackFileOut == NULL) StackFileOut = stderr;

    // Canary left protection 
    stack->canaryLeft = StackLeftCanaryValue;
    // Canary right protection
    stack->canaryRight = StackRightCanaryValue;

    stack->info.mainFileName  = mainFileName;
    stack->info.mainFuncName  = mainFuncName;
    stack->info.mainStackName = mainStackName;
    stack->info.isStackValid  = true;

    stack->info.numHashIgnore = 2;

    stack->info.arrHashIgnore = (HashIgnore*)calloc (stack->info.numHashIgnore, sizeof (HashIgnore));

    stack->info.arrHashIgnore[0] = { &stack->info.hashValue, sizeof (uint64_t) };
    stack->info.arrHashIgnore[1] = { &stack->info.errStatus, sizeof (uint64_t) };
    
    stack->data            = NULL;
    stack->info.stepResize = 2;
    stack->size            = 0;
    stack->capacity        = 0;

    stack->info.hashValue = StackHashProtection (stack); 
    
    StackResize (stack, dataSize == 0 ? 1 : dataSize);

    stack->info.errStatus = 0;

    stack->info.hashValue = StackHashProtection (stack); 

    StackDump (stack);

    return 1;
}

//---------------------------------------------------------------------------

uint64_t StackHashProtection (Stack_t* stack)
{  
    return HashProtection (stack, sizeof (Stack_t), stack->info.arrHashIgnore, stack->info.numHashIgnore);
}

//---------------------------------------------------------------------------

int StackErrHandler (Stack_t* stack)
{
    Assert (stack != NULL, 0);
    
    stack->info.errStatus = 0;    
    
    if (!stack->data) 
    {       
        stack->info.errStatus |= StackErrors::NULL_DATA_PTR;
    }
    
    if (stack->size < 0 || stack->size > stack->capacity)
    {
        stack->info.errStatus |= StackErrors::INVALID_SIZE;
    }

    if (stack->capacity < 0)
    {
        stack->info.errStatus |= StackErrors::INVALID_CAPACITY;
    }

    if (stack->canaryLeft != StackLeftCanaryValue)
    {
        stack->info.errStatus |= StackErrors::LEFT_CANARY_CHANGED;
    }

    if (stack->canaryRight != StackRightCanaryValue)
    {
        stack->info.errStatus |= StackErrors::RIGHT_CANARY_CHANGED;
    }

    if (stack->info.hashValue != StackHashProtection (stack))
    {
        stack->info.isStackValid = false;
        
        stack->info.errStatus |= StackErrors::INVALID_HASH_VALUE;
    }

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
    
        fprintf (StackFileOut, "%*shashValue = %llu;\n",  TabSize, "", stack->info.hashValue);
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

void StackResize (Stack_t* stack, size_t num)
{
    Assert (stack != NULL); 

    // No resize
    if (stack->capacity == num) return;

    StackDump (stack);

    if (num <= 0) return;
    
    stack->data = (Elem_t*)Recalloc (stack->data, stack->capacity, num, sizeof (Elem_t));

    // Fill data with poison (increase data)
    #ifndef NDUMP
    
    if (stack->capacity < num)
    {
        for (size_t i = stack->capacity; i < num; i++)
        {
            stack->data[i] = StackDataPoisonValue;
        }
    }

    #endif

    stack->capacity = num;

    stack->info.hashValue = StackHashProtection (stack);

    StackDump (stack);
}

//---------------------------------------------------------------------------

void StackPush (Stack_t* stack, Elem_t value)
{
    Assert (stack != NULL);

    StackDump (stack);

    if(stack->size >= stack->capacity) 
    {
        StackResize (stack, size_t((stack->capacity) * (stack->info.stepResize)));
    }

    stack->data[stack->size] = value;

    stack->size++;

    stack->info.hashValue = StackHashProtection (stack);
    
    StackDump (stack);
}

//---------------------------------------------------------------------------

Elem_t StackPop (Stack_t* stack)
{
    Assert (stack != NULL, 0);

    StackDump (stack);

    if (stack->size > 0) 
    {
        stack->size--;   

        if (stack->size >= size_t(stack->capacity / (2*stack->info.stepResize)))   
        {
            StackResize (stack, size_t(stack->capacity / stack->info.stepResize));
        }  

        stack->info.hashValue = StackHashProtection (stack);
        
        StackDump (stack);

        return stack->data[stack->size];
    }

    stack->info.hashValue = StackHashProtection (stack);

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

void* Recalloc (void* arr, size_t curNum, size_t newNum, size_t size)
{
    if (curNum == newNum) return arr;

    void* arrNew = NULL;

    if (newNum > 0) arrNew = calloc (newNum, size);

    if (arrNew == NULL) return NULL;

    if (arr != NULL) 
    {  
        memcpy (arrNew, arr, (curNum < newNum ? curNum : newNum) * size);
        free   (arr);
    }

    return arrNew;
}

//---------------------------------------------------------------------------

size_t NumBytesHashIgnore (void* arr, HashIgnore* arrHashIgnore, size_t numHashIgnore)
{
    Assert (arr           != NULL, 0);
    Assert (arrHashIgnore != NULL, 0);
    
    // Check Hash Ignored
    for (size_t i = 0; i < numHashIgnore; i++)
    {
        if ((char*)arr == arrHashIgnore[i].ignorePtr)
        {
            return arrHashIgnore[i].size;  
        }
    }

    return 0;
}

//---------------------------------------------------------------------------

uint64_t HashProtection (void*       arr, 
                         size_t      size, 
                         HashIgnore* arrHashIgnore,
                         size_t      numHashIgnore)
{
    Assert (arr != NULL, 0);

    uint64_t hashValue = 0;

    for (size_t i = 0; i < size; i++) 
    {
        size_t numBytesHashIgnore = 0;
        
        if (arrHashIgnore != NULL)
        {
            numBytesHashIgnore = NumBytesHashIgnore ((char*)arr + i, arrHashIgnore, numHashIgnore);
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