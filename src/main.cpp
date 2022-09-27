#include "Config.h"

#include <stdio.h>

#include "Assert.h"
#include "Stack.h"

//---------------------------------------------------------------------------

int main()
{
    Stack_t     stack = {0};
    StackCtor (&stack);

    StackPush (&stack, 5);
    StackPush (&stack, 6);
    StackPush (&stack, 5);
    StackPush (&stack, 6);

    StackDump (&stack);
}

//---------------------------------------------------------------------------
