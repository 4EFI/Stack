
//#define NDUMP

#include "Config.h"

#include <stdio.h>
#include <stdlib.h>

#include "Stack.h"
#include "Assert.h"

//---------------------------------------------------------------------------

const char* MainFileName  = NULL;
const char* MainFuncName  = NULL;
const char* MainStackName = NULL;

const char* CurFileName   = NULL;
const char* CurFuncName   = NULL;
int         CurLine       = 0;

//---------------------------------------------------------------------------

FILE* StackFileOut = stderr;

//---------------------------------------------------------------------------

Elem_t StackDataPoisonValue = 0xBAAAAAAD;

//---------------------------------------------------------------------------

int _StackCtor (Stack_t* stack)
{ 
    Assert (stack != NULL, 0);
    
    stack->data           = NULL;
    stack->stepResizeUp   = 1.1;
    stack->stepResizeDown = 1.5;
    stack->size           = 0;
    stack->capacity       = 0;

    StackResize (stack, 1);

    stack->errStatus  = 0;

    StackDump (stack);

    return 1;
}

//---------------------------------------------------------------------------

int StackErrHandler (Stack_t* stack)
{
    Assert (stack != NULL, 0);
    
    stack->errStatus = 0;    
    
    if (!stack->data) 
    {       
        stack->errStatus |= StackErrors::NULL_DATA_PTR;
    }
    
    if (stack->size < 0 || stack->size >= stack->capacity)
    {
        stack->errStatus |= StackErrors::INVALID_SIZE;
    }

    if (stack->capacity < 0)
    {
        stack->errStatus |= StackErrors::INVALID_CAPACITY;
    }

    return 1;
}

//---------------------------------------------------------------------------

int StackErrPrint (Stack_t* stack, size_t indent)
{
    Assert (stack != NULL, 0);

    if (!stack->errStatus) return 1;
    
    unsigned long long int errStatCopy = stack->errStatus;

    int numLines = sizeof (ErrorLines) / sizeof (char*);
    
    for (int i = 0; i < numLines; i++)
    {
        if (errStatCopy & 1) fprintf (StackFileOut, "%*s%s\n", indent, "", ErrorLines[i]);

        errStatCopy >>= 1;
    }

    return 1;
}

//---------------------------------------------------------------------------

void _StackDump (Stack_t* stack)
{
    const char divideSym        = '=';
    const int  numDividsersSyms = 70;
    
    StackErrHandler (stack);

    PrintfDividers (divideSym, numDividsersSyms, StackFileOut);

    fprintf (StackFileOut, "%s at %s(%d)\n", 
                            CurFuncName, 
                            CurFileName, 
                            CurLine);
    
    fprintf (StackFileOut, "Stack[%p] (%s) \"%s\" at %s at %s\n", 
                            stack, 
                            stack->errStatus == 0 ? "ok" : "ERROR",
                            MainStackName + 1, // Remove & in name
                            MainFileName,
                            MainFuncName);

    if (stack->errStatus)
    {
        fprintf (StackFileOut, "ERRORS:\n");
        fprintf (StackFileOut, "{\n");

        StackErrPrint (stack, TabSize);

        fprintf (StackFileOut, "}\n");    
    }

    fputs ("{\n", StackFileOut);
    {
    
        fprintf (StackFileOut, "%*ssize     = %lu;\n",   TabSize, "", stack->size);
        fprintf (StackFileOut, "%*scapacity = %lu;\n\n", TabSize, "", stack->capacity);

        fprintf (StackFileOut, "%*sdata[%p]\n", TabSize, "", stack->data);

        fprintf (StackFileOut, "%*s{\n", TabSize, "");
        {
            for (int i = 0; i < stack->capacity; i++)
            {
                bool isEmpty = (i >= stack->size || stack->data[i] == StackDataPoisonValue);
            
                fprintf (StackFileOut, "%*s%s[%d] = ", 
                                        TabSize * 2, "",  
                                        isEmpty ? " " : "*", i);

                // Print value                        
                if(isEmpty) fprintf (StackFileOut, "%s\n", (stack->data[i] == 0) ? "0" : "NAN (POISON)");
                else        fprintf (StackFileOut, "%d\n",  stack->data[i]);
            }
        }
        fprintf (StackFileOut, "%*s}\n", TabSize, "");

    }
    fputs ("}\n", StackFileOut);

    PrintfDividers (divideSym, numDividsersSyms, StackFileOut);
}

//---------------------------------------------------------------------------

void StackResize (Stack_t* stack, size_t num)
{
    Assert (stack != NULL);

    StackErrHandler (stack);
    StackDump       (stack);

    if (num <= 0) return;
    
    stack->data = (Elem_t*)Recalloc (stack->data, num, sizeof (Elem_t));

    // Fill data with poison
    #ifndef NDUMP

    if (stack->capacity < num)
    {
        for (int i = stack->capacity; i < num; i++)
        {
            stack->data[i] = StackDataPoisonValue;
        }
    }

    #endif

    stack->capacity = num;

    StackErrHandler (stack);
    StackDump       (stack);
}

//---------------------------------------------------------------------------

int StackPush (Stack_t* stack, Elem_t value)
{
    Assert (stack != NULL, 0);

    StackErrHandler (stack);
    StackDump       (stack);

    if(stack->size >= stack->capacity) 
    {
        StackResize (stack, size_t((stack->capacity) * (stack->stepResizeUp)));
    }

    stack->data[stack->size] = value;

    stack->size++;

    StackDump (stack);
}

//---------------------------------------------------------------------------

Elem_t StackPop (Stack_t* stack)
{

}

//---------------------------------------------------------------------------

void PutSpaces (int numSpaces, FILE* file)
{
    Assert (file != NULL);

    fprintf (file, "%*s", numSpaces, "");
}

//---------------------------------------------------------------------------

void PrintfDividers (char divideSym, int numDividers, FILE* file)
{
    Assert (file != NULL);
    
    for (int i = 0; i < numDividers; i++)
    {
        fputc (divideSym, file);
    }

    fputc ('\n', file);
}

//---------------------------------------------------------------------------

void* Recalloc (void* arr, size_t num, size_t size)
{
    void* arrNew = NULL;
    
    if (num > 0) arrNew = calloc (num, size);

    if (arrNew == NULL) return NULL;

    if (arr != NULL && arrNew != NULL) 
    {  
        memcpy (arrNew, arr, num);
        free   (arr);
    }

    return arrNew;
}

//---------------------------------------------------------------------------