#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <string.h>

//---------------------------------------------------------------------------

const int TabSize    = 4;
const int MaxStrSize = 255;

//---------------------------------------------------------------------------

extern const char* MainFileName;
extern const char* MainFuncName;
extern const char* MainStackName;

extern const char* CurFileName;
extern const char* CurFuncName;
extern int         CurLine;

//---------------------------------------------------------------------------

enum StackErrors 
{
    NULL_DATA_PTR    = (1 << 0),
    INVALID_SIZE     = (1 << 1),
    INVALID_CAPACITY = (1 << 2),
};

static const char* ErrorLines[] = {"Data null ptr",
                                   "Invalid size",
                                   "Invalid capacity"};

struct Stack_t
{
    Elem_t* data;

    double stepResizeUp;
    double stepResizeDown;
    
    size_t size;
    size_t capacity;

    unsigned long long int errStatus;
};

//---------------------------------------------------------------------------

int  _StackCtor      (Stack_t* stack);
void _StackDump      (Stack_t* stack);

int  StackErrHandler (Stack_t* stack);
int  StackErrPrint   (Stack_t* stack, size_t indent = 0);

void StackResize     (Stack_t* stack, size_t num);

int  StackPush       (Stack_t* stack, Elem_t value);    

//---------------------------------------------------------------------------

void PutSpaces (int numSpaces, FILE* file = stdout);

void PrintfDividers (char divideSym, int numDividers, FILE* file);

//---------------------------------------------------------------------------

void* Recalloc (void* arr, size_t num, size_t size);

//---------------------------------------------------------------------------

#ifndef NDUMP

#define StackCtor(...) { MainFileName  = __FILE__;           \
                         MainFuncName  = __PRETTY_FUNCTION__; \
                         MainStackName = #__VA_ARGS__;       \
                         _StackCtor (__VA_ARGS__); }

#define StackDump(...) { CurFileName = __FILE__;            \
                         CurFuncName = __PRETTY_FUNCTION__; \
                         CurLine     = __LINE__;            \
                         _StackDump (__VA_ARGS__); }
#else 

#define StackCtor(...) _StackCtor (__VA_ARGS__);
#define StackDump(...) ;

#endif
//---------------------------------------------------------------------------

#endif