
#include <stdio.h>
#include <assert.h>
#include "Assert.h"

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
};

//---------------------------------------------------------------------------

int StackCtor (Stack_t* stack);

//---------------------------------------------------------------------------

int main()
{
    Stack_t stack = {};

    StackCtor (NULL);

    //printf ();
}

//---------------------------------------------------------------------------

int StackCtor (Stack_t* stack)
{ 
    Assert (stack != NULL, 0);
    
    stack->data     = NULL;
    stack->size     = 0;
    stack->capacity = 0; 

    return 1;
}

//---------------------------------------------------------------------------