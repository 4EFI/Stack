#include "Config.h"

#include <stdio.h>

#include "Assert.h"
#include "Stack.h"
#include "LOG.h"

//---------------------------------------------------------------------------

int main()
{   
    Stack_t     stack = {0};
    StackCtor (&stack, 2);

    StackPush (&stack, 5);
    StackPush (&stack, 6);
    StackPush (&stack, 5);
    StackPush (&stack, 6);

    Elem_t a = StackPop (&stack);

    //stack.size = 0;

    FLOG ("Cur stack value = %d", a);

    a = StackPop (&stack);

    StackPush (&stack, 10);

    FLOG ("Cur stack value = %d", a);

    a = StackPop (&stack);

    FLOG ("Cur stack value = %d", a);
}

//---------------------------------------------------------------------------
