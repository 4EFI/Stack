#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <string.h>

//---------------------------------------------------------------------------

const int TabSize = 4;

//---------------------------------------------------------------------------

extern const char* CurFileName;
extern const char* CurFuncName;
extern int         CurLine;

//---------------------------------------------------------------------------

struct HashIgnore
{
    void*  ignorePtr;
    size_t size;
};

//---------------------------------------------------------------------------

struct StackInfo
{
    const char* mainFileName;
    const char* mainFuncName;
    const char* mainStackName;

    double stepResizeUp;
    double stepResizeDown;

    unsigned long long int errStatus;
    unsigned long long int hashValue;

    int numHashIgnore; 
    HashIgnore* arrHashIgnore;
};

struct Stack_t
{
    // Canary left protection
    unsigned long long int canaryLeft;
    
    StackInfo info;
    
    Elem_t* data;
    
    size_t size;
    size_t capacity;

    // Canary right protection
    unsigned long long int canaryRight;
};

//---------------------------------------------------------------------------

int  _StackCtor (Stack_t* stack, int dataSize, const char* mainFileName, 
                                               const char* mainFuncName, 
                                               const char* mainStackName);
void _StackDump (Stack_t* stack);

unsigned long long int StackHashProtection (Stack_t* stack);

int  StackErrHandler (Stack_t* stack);
int  StackErrPrint   (Stack_t* stack, int indent = 0);

void StackResize     (Stack_t* stack, size_t num);

void   StackPush     (Stack_t* stack, Elem_t value);    
Elem_t StackPop      (Stack_t* stack);

//---------------------------------------------------------------------------

void PutSpaces (int numSpaces, FILE* file = stdout);

void PrintfDividers (char divideSym, int numDividers, FILE* file);

//---------------------------------------------------------------------------

void* Recalloc (void* arr, size_t curNum, size_t newNum, size_t size);

size_t NumBytesHashIgnore (void* arr, HashIgnore* arrHashIgnore, size_t numHashIgnore);

unsigned long long int HashProtection (void*  arr, 
                                       size_t size, 
                                       HashIgnore* arrHashIgnore,
                                       size_t      numHashIgnore);

//---------------------------------------------------------------------------

#ifndef NDUMP

#define StackCtor(stack, dataSize) { _StackCtor (stack, dataSize, __FILE__, __PRETTY_FUNCTION__, #stack); }

#define StackDump(stack) { CurFileName = __FILE__;            \
                           CurFuncName = __PRETTY_FUNCTION__; \
                           CurLine     = __LINE__;            \
                           _StackDump (stack); }
#else 

#define StackCtor(...) _StackCtor (__VA_ARGS__);
#define StackDump(...) ;

#endif

//---------------------------------------------------------------------------

#endif