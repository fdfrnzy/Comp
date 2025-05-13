#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "lexer.h"

typedef enum {STATIC, FIELD, ARG, VAR, METHOD, FUNCTION, CONSTRUCTOR, CLASS} Kind; // Define the kinds of symbols
typedef enum {INTEGER, CHAR, BOOLEAN, ARRAY, IDENTIFIER} Type; // Define the types of symbols


typedef struct{
    char name[128];
    Type type;
    Kind kind;
    char ID[128]; // Identifier name
    int calls;
} Symbol;

typedef struct SymbolTable{
    Symbol* table[1280]; // Array of symbols
    int len;            // Number of symbols in the table
    struct SymbolTable* parent; // Pointer to the parent symbol table (if any)
    struct SymbolTable* children[128]; // Array of child symbol tables
    int childCount;     // Number of child symbol tables
} SymbolTable; // Define the SymbolTable type

typedef struct{
    char data[1280][128]; // Array of strings to store symbols
    int topIndex; // Index of the top of the stack
} Stack; // Define the Stack type


typedef struct{
    char name[128];
    SymbolTable* scope;
    Token token;
} IdentifierStrct; // Define the Identifier type

typedef struct{
    IdentifierStrct* data[1280][3]; // Array of identifiers
    int topIndex; // Index of the top of the stack
} IdentifierStack; // Define the IdentifierStack type

typedef struct{
	IdentifierStrct* data[1280][3]; // Array of identifiers
	int topIndex; // Index of the top of the stack
} SpecialIdStack;

void pushId(IdentifierStack* s, char* str, SymbolTable* st, Token t, int ArrIdx, int check);
IdentifierStrct* popId(IdentifierStack* s, int idx);
void pushSpId(SpecialIdStack* s, char* str, SymbolTable* st, Token t, int ArrIdx);
void initSpIdStack(SpecialIdStack* s);
void InitIdStack(IdentifierStack* s);
int indexIdStack(IdentifierStack* s, char* str, SymbolTable* st);
int indexSpIdStack(SpecialIdStack* s, char* str, SymbolTable* st);

Type GetType(char* type);
Kind GetKind(char* kind);
void InitSymbolTable(SymbolTable* st);
void InsertSymbol(char* name, Type type, Kind kind, SymbolTable* st);
int LocateSymbol(char* name, SymbolTable* st);
Symbol* GetSymbol(char* name, SymbolTable* st);
Symbol* GetSymbolGlobal(char* name, SymbolTable* st);
SymbolTable* GetSymbolTable(Symbol* s, SymbolTable* st);
int IndexTable(char* name, SymbolTable* st);
void InsertChildTable(SymbolTable* parent, SymbolTable* child);
int IndexParents(char* name, SymbolTable* st);
int IndexChildren(char* name, SymbolTable* st);
char* pop(Stack* s);
void push(Stack* s, char* str);

int indexStack(Stack* s, char* str);

void InitStack(Stack* s);
void printTable(SymbolTable* st);

#endif
