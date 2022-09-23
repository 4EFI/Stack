
#include <stdio.h>
#include <assert.h>
#include "Assert.h"

//---------------------------------------------------------------------------

typedef unsigned long long int Uint64_t;

const int TabSize = 4;

//---------------------------------------------------------------------------

enum StackErrors 
{
    ERR_NULL_PTR_DATA,
    ERR_
};

typedef int Elem_t;

struct Stack_t
{
    Elem_t* data;
    
    size_t size;
    size_t capacity;

    Uint64_t errStatus;
};

//---------------------------------------------------------------------------

int  StackCtor       (Stack_t* stack);
int  StackErrHandler (Stack_t* stack);
void StackDump       (Stack_t* stack);

int StackPush (Stack_t* stack);    

//---------------------------------------------------------------------------

void PutSpaces (int numSpaces, FILE* file = stdout);

//---------------------------------------------------------------------------

int main()
{
    Stack_t stack = {};

    StackCtor (&stack);

    StackDump (&stack);

    //printf ();
}

//---------------------------------------------------------------------------

int StackCtor (Stack_t* stack)
{ 
    Assert (stack != NULL, 0);
    
    stack->data      = NULL;
    stack->size      = 0;
    stack->capacity  = 0; 
    stack->errStatus = 0;

    return 1;
}

//---------------------------------------------------------------------------

int StackErrHandler (Stack_t* stack)
{
    int curErr = 0;

    if (!stack->errStatus) return 0;

    return 1;
}

//---------------------------------------------------------------------------

void StackDump (Stack_t* stack)
{
    FILE* file = stderr;
    
    fputs ("{\n", file);

    PutSpaces (TabSize, file);
        fprintf        (file, "size     = %d;\n", stack->size);
    PutSpaces (TabSize, file);
        fprintf        (file, "capacity = %d;\n", stack->capacity);

    PutSpaces (2*TabSize, file);
        fputs ("{\n",     file);

    for (int i = 0; i < stack->size; i++)
    {
    PutSpaces (2*TabSize, file);
            fprintf      (file, "%s[%d] = %d", 0 ? " " : "*", i, stack->data[i]);
    }

    PutSpaces (2*TabSize, file);
        fputs ("}\n",     file);

    fputs ("}\n", file);
}

//---------------------------------------------------------------------------

void PutSpaces (int numSpaces, FILE* file)
{
    Assert (file != NULL);

    fprintf (file, "%*s", TabSize, "");
}

//---------------------------------------------------------------------------