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

    // Works
    //stack.info.mainFileName = "I glupoe jivotnoe";
    
    //stack.data[1] = 100; // Works

    //stack.data = NULL; // Works

    //stack.data = (Elem_t*)1; // Cringeeeeee

    StackDump (&stack);

    //StackPush (&stack, StackDataPoisonValue); // Cringeee Dump

    StackPush (&stack, 8);
    StackPush (&stack, 6);
    StackPush (&stack, 5);
    StackPush (&stack, 6);
    StackPush (&stack, 100);

    /*
    StackPop  (&stack);
    StackPush (&stack, 6);
    StackPop  (&stack);
    StackPush (&stack, 6);
    StackPop  (&stack);
    StackPush (&stack, 6);
    */

    Elem_t a = StackPop (&stack);

    StackDtor (&stack);
}

//---------------------------------------------------------------------------
