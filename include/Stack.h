#ifndef STACK_H
#define STACK_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>

//---------------------------------------------------------------------------

const int TabSize = 4;

//---------------------------------------------------------------------------

extern const char* CurFileName;
extern const char* CurFuncName;
extern int         CurLine;

extern Elem_t StackDataPoisonValue;

//---------------------------------------------------------------------------

struct HashIgnore
{
    size_t pos;
    size_t size;
};

//---------------------------------------------------------------------------

struct StackInfo
{
    const char* mainFileName;
    const char* mainFuncName;
    const char* mainStackName;

    double stepResize;

    uint64_t errStatus;
    
    #ifndef NHASH

    uint64_t hashValue;

    int numHashIgnore; 
    HashIgnore* arrHashIgnorePtr;

    #endif

    bool isStackValid;
};

struct Stack_t
{
    #ifndef NCANARY
    // Canary left protection
    uint64_t canaryLeft;
    #endif
    
    StackInfo info;
    
    Elem_t* data;
    
    size_t size;
    size_t capacity;

    #ifndef NCANARY
    // Canary right protection
    uint64_t canaryRight;
    #endif
};

//---------------------------------------------------------------------------

int  _StackCtor (Stack_t* stack, int dataSize, const char* mainFileName, 
                                               const char* mainFuncName, 
                                               const char* mainStackName);

int   StackDtor (Stack_t* stack);

void _StackDump (Stack_t* stack);

uint64_t StackHashProtection (Stack_t* stack);

int  StackErrHandler (Stack_t* stack);
int  StackErrPrint   (Stack_t* stack, int indent = 0);

void StackResize     (Stack_t* stack, size_t num);

void   StackPush     (Stack_t* stack, Elem_t value);    
Elem_t StackPop      (Stack_t* stack);

//---------------------------------------------------------------------------

int PrintSyms (char sym, int num, FILE* file);

//---------------------------------------------------------------------------

void* Recalloc (void* arr, size_t newNum, size_t size);

size_t NumBytesHashIgnore (void* arrToComp, void* arr, HashIgnore* arrHashIgnorePtr, size_t numHashIgnore);

uint64_t HashProtection (void*       arr, 
                         size_t      size, 
                         HashIgnore* arrHashIgnore = NULL,
                         size_t      numHashIgnore = 0);

//---------------------------------------------------------------------------

#define StackCtor(stack, dataSize) { _StackCtor (stack, dataSize, __FILE__, __PRETTY_FUNCTION__, #stack); }

#ifndef NDUMP

#define StackDump(stack) { CurFileName = __FILE__;            \
                           CurFuncName = __PRETTY_FUNCTION__; \
                           CurLine     = __LINE__;            \
                           _StackDump (stack); }
#else 

#define StackDump(...) ;

#endif

//---------------------------------------------------------------------------

#endif