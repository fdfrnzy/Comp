#include "lexer.h"
#include "symbols.h"
#include "parser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define SEARCHING_PARENT 0
#define SEARCHING_CHILDREN 0

Type GetType(char* type) {
    // Map string representation of type to enum Type
    if (strcmp(type, "int") == 0) return INTEGER; // Corrected INT to INTEGER
    if (strcmp(type, "char") == 0) return CHAR;
    if (strcmp(type, "boolean") == 0) return BOOLEAN;
    if (strcmp(type, "identifier") == 0) return IDENTIFIER;
    return IDENTIFIER; // Default case
}

Kind GetKind(char* kind) {
    // Map string representation of kind to enum Kind
    if (strcmp(kind, "static") == 0) return STATIC;
    if (strcmp(kind, "field") == 0) return FIELD;
    if (strcmp(kind, "arg") == 0) return ARG;
    if (strcmp(kind, "var") == 0) return VAR;
    if (strcmp(kind, "method") == 0) return METHOD;
    if (strcmp(kind, "function") == 0) return FUNCTION;
    if (strcmp(kind, "constructor") == 0) return CONSTRUCTOR;
    if (strcmp(kind, "class") == 0) return CLASS;
    return CLASS; // Default case
}

void InitSymbolTable(SymbolTable* st) {
    // Initialize the symbol table
    st->len = 0; // Set the length of the symbol table to 0
    st->childCount = 0; // Set the child count to 0
    st->parent = NULL; // Set the parent to NULL
    for (int i = 0; i < 128; i++) {
        st->children[i] = NULL; // Initialize all child pointers to NULL
    }
}

// Function to insert a new symbol
void InsertSymbol(char* name, Type type, Kind kind, SymbolTable* st) {
    // Check if the symbol table is full
    if (st->len >= 1280) {
        printf("Symbol table is full. Cannot insert new symbol.\n");
        return;
    }
    Symbol* newSymbol = (Symbol*)malloc(sizeof(Symbol)); // Allocate memory for the new symbol
    if (newSymbol == NULL) {
        printf("Memory allocation failed. Cannot insert new symbol.\n");
        return;
    }


    // Insert the symbol into the symbol table
    strcpy(newSymbol->name, name);
    newSymbol->type = type;
    newSymbol->kind = kind;
    newSymbol->calls = 0; // Initialize calls to 0
    st->table[st->len] = newSymbol; // Add the new symbol to the table
    st->len++;
}

int IndexTable(char* name, SymbolTable* st) {
    // Search for the symbol in the symbol table
    if (st == NULL) {
        return -1; // Return -1 if the symbol table is NULL
    }
    
    for (int i = 0; i < st->len; i++) {
        if (strcmp(st->table[i]->name, name) == 0) {
            return i; // Return the index of the symbol if found
        }
    }
    return -1; // Return -1 if the symbol is not found
}

int IndexParents(char* name, SymbolTable* st){
    //printf("Searching in parent symbol table...\n");
    // Search for the symbol in the parent symbol table
    if (st == NULL) {
        // Return -1 if the symbol table is NULL
        return -1;
    }
    else if (st->parent == NULL) {
        // Return -1 if the parent symbol table is NULL
        return -1;
    }
    int i = IndexTable(name, st->parent);
    if (i != -1) {
        return i; // Return 0 if the symbol is found in the parent table
    }
    i = IndexParents(name, st->parent);
    if (i != -1) {
        return i; // Return 0 if the symbol is found in the parent table 
    }
    return -1; // Search in the parent symbol table
}
int IndexChildren(char* name, SymbolTable* st) {
    //printf("Searching in child symbol tables...\n");
    // Search for the symbol in the child symbol tables
    if (st == NULL) {
        return -1; // Return -1 if the symbol table is NULL
    }
    else if (st->childCount == 0) {
        return -1; // Return -1 if there are no child tables
    }
    else {
        for (int i = 0; i < st->childCount; i++) {
            if (IndexTable(name, st->children[i]) != -1) {
                return i; // Return 0 if the symbol is found in a child table
            }
            else if (IndexChildren(name, st->children[i]) != -1) {
                return i; // Return 0 if the symbol is found in a child table
            }
        }

    }
    return -1; // Return -1 if the symbol is not found in any child table
}

// Function to locate a symbol in the program
// If the symbol is found, return 0; otherwise, return -1
int LocateSymbol(char* name, SymbolTable* st) {
    // Search for the symbol in the symbol table
    if (st == NULL) {
        return -1; // Return -1 if the symbol table is NULL
    }
    else if (IndexTable(name, st) != -1) {
        return 0; // Return the index if the symbol is found in the current table
    }
    else if (IndexParents(name, st) != -1) {
        return 0; // Search in the parent symbol table
    }
    else if (IndexChildren(name, st) != -1) {
        return 0; // Search in the child symbol tables
    }
    return -1; // Return -1 if the symbol is not found
}

Symbol* GetSymbol(char* name, SymbolTable* st) {
    // Search for the symbol in the symbol table
    if (st == NULL) {
        ////printf("Symbol table is NULL\n");
        return NULL; // Return NULL if the symbol table is NULL
    }
    else if (IndexTable(name, st) != -1) {
        //printf("Symbol found in current table\n");
        return st->table[IndexTable(name, st)]; // Return the symbol if found
    }
    return NULL; // Return NULL if the symbol is not found
}

Symbol* GetSymbolGlobal(char* name, SymbolTable* st) {
    // Search for the symbol in the global symbol table
    if (st == NULL) {
        return NULL; // Return NULL if the symbol table is NULL
    }
    int i = IndexTable(name, st);
    if (i != -1) {
        return st->table[i]; // Return the symbol if found
    }
    else if (IndexParents(name, st) != -1) {
        return GetSymbolGlobal(name, st->parent); // Search in the parent symbol table
    }
    int j = IndexChildren(name, st);
    if (j != -1) {
        return GetSymbolGlobal(name, st->children[j]); // Search in the child symbol tables
    }
    return NULL; // Return NULL if the symbol is not found
}
//Get the symbol table a symbol belongs to
SymbolTable* GetSymbolTable(Symbol* s, SymbolTable* st) {
    // Search for the symbol in the symbol table
    if (st == NULL || s == NULL) {
        return NULL; // Return NULL if the symbol table is NULL
    }
    for (int i = 0; i < st->len; i++) {
        if (st->table[i] == s) {
            return st; // Return the symbol table if the symbol is found
        }
    }
    // Search in the parent symbol table
    if (st->parent != NULL) {
        SymbolTable* parentTable = GetSymbolTable(s, st->parent);
        if (parentTable != NULL) {
            return parentTable; // Return the parent symbol table if found
        }
    }
    return NULL; // Return NULL if the symbol is not found
}

void pushSpId(SpecialIdStack* s, char* str, SymbolTable* st, Token t, int ArrIdx) {
    // Push a special identifier onto the special identifier stack
    if (s->topIndex >= 1279) {
        printf("Special identifier stack overflow. Cannot push new identifier.\n");
        return;
    }
    // Check if the identifier already exists in the stack
    //printf("Pushing special identifier: %s\n", str);
    IdentifierStrct* id = (IdentifierStrct*)malloc(sizeof(IdentifierStrct)); // Allocate memory for the special identifier
    strcpy(id->name, str); // Copy the special identifier name
    id->scope = st; // Set the scope of the special identifier
    id->token = t; // Set the token of the special identifier
    if (ArrIdx == 0){
        s->topIndex++;
    }
    s->data[s->topIndex][ArrIdx] = id; // Push the special identifier onto the stack
}

void initSpIdStack(SpecialIdStack* s) {
    // Initialize the special identifier stack
    s->topIndex = -1; // Set the top index to -1 (empty stack)

}

int indexSpIdStack(SpecialIdStack* s, char* str, SymbolTable* st) {
    // Check if the special identifier stack is empty
    if (s->topIndex < 0) {
        return -1; // Return -1 if the stack is empty
    }
    for(int i = 0; i <= s->topIndex; i++) {
        if (strcmp(s->data[i][0]->name, str) == 0 && s->data[i][0]->scope == st) {
            return i; // Return the index if the special identifier is found
        }
    }
    return -1; // Return -1 if the special identifier is not found
}

// Function to insert a child symbol table into a parent symbol table
void InsertChildTable(SymbolTable* parent, SymbolTable* child) {
    // Check if the parent symbol table can accommodate a new child
    if (parent->childCount >= 128) {
        return;
    }

    // Insert the child symbol table into the parent symbol table
    parent->children[parent->childCount] = child;
    parent->childCount++;
    child->parent = parent; // Set the parent of the child symbol table
}

void InitStack(Stack* s) {
    // Initialize the stack
    s->topIndex = -1; // Set the top index to -1 (empty stack)
}

void InitIdStack(IdentifierStack* s) {
    // Initialize the identifier stack
    s->topIndex = -1; // Set the top index to -1 (empty stack)
    for (int i = 0; i < 1280; i++) {
        s->data[i][1] = NULL; // Initialize all identifier pointers to NULL
    }
}

void push(Stack* s, char* str) {
    // Push a string onto the stack
    if (s->topIndex >= 127) {
        printf("Stack overflow. Cannot push new string.\n");
        return;
    }
    s->topIndex++;
    strcpy(s->data[s->topIndex], str); // Copy the string to the top of the stack
}

char* pop(Stack* s) {
    char* temp = (char*)malloc(64 * sizeof(char)); // Allocate memory for the popped string
    // Pop a string from the stack
    if (s->topIndex < 0) {
        printf("Stack underflow. Cannot pop string.\n");
        return '\0'; // Return null character if the stack is empty
    }
    strcpy(temp, s->data[s->topIndex]); // Get the top string
    strcpy(s->data[s->topIndex], ""); // Clear the top string
    s->topIndex--; // Decrease the top index
    return temp; // Return the popped string
}

void pushId(IdentifierStack* s, char* str, SymbolTable* st, Token t, int ArrIdx, int check) {
    // Push an identifier onto the identifier stack
    if (s->topIndex >= 1279) {
        printf("Identifier stack overflow. Cannot push new identifier.\n");
        return;
    }
    // Check if the identifier already exists in the stack
    int idx = indexIdStack(s, str, st);
    if (idx != -1 && check == 0) {
        //printf("%s already exists in the stack.\n", str);
        // Check if the identifier is in the same scope
        //printf("Identifier: %s, Scope: %p, Stack Scope: %p\n", str, st, s->data[idx]->scope);
        return; // Return if the identifier already exists
    }
    //printf("Pushing identifier: %s\n", str);
    IdentifierStrct* id = (IdentifierStrct*)malloc(sizeof(IdentifierStrct)); // Allocate memory for the identifier
    strcpy(id->name, str); // Copy the identifier name
    id->scope = st; // Set the scope of the identifier
    id->token = t; // Set the token of the identifier
    if (ArrIdx == 0){
        s->topIndex++;
    }
    s->data[s->topIndex][ArrIdx] = id; // Push the identifier onto the stack
}
IdentifierStrct* popId(IdentifierStack* s, int idx) {
    // Pop an identifier from the identifier stack
    if (s->topIndex < 0) {
        printf("Identifier stack underflow. Cannot pop identifier.\n");
        return NULL; // Return null character if the stack is empty
    }
    IdentifierStrct* temp = (IdentifierStrct*)malloc(sizeof(IdentifierStrct)); // Allocate memory for the popped identifier
    temp = s->data[s->topIndex][idx]; // Get the top identifier
    s->data[s->topIndex][idx] = NULL; // Clear the top identifier
    s->topIndex--; // Decrease the top index
    return temp; // Return the popped identifier name
}

// Function to check if a string is in the stack
int indexStack(Stack* s, char* str) {
    // Check if the stack is empty
    if (s->topIndex < 0) {
        return -1; // Return -1 if the stack is empty
    }
    for(int i = 0; i <= s->topIndex; i++) {
        if (strcmp(s->data[i], str) == 0) {
            return i; // Return the index if the string is found
        }
    }
    return -1; // Return -1 if the string is not found
}

int indexIdStack(IdentifierStack* s, char* str, SymbolTable* st) {
    // Check if the identifier stack is empty
    if (s->topIndex < 0) {
        return -1; // Return -1 if the stack is empty
    }
    for(int i = 0; i <= s->topIndex; i++) {
        if (strcmp(s->data[i][0]->name, str) == 0 && s->data[i][0]->scope == st) {
            return i; // Return the index if the identifier is found
        }
    }
    return -1; // Return -1 if the identifier is not found
}


void printTable(SymbolTable* st) {
    // Print the symbol table
    for (int i = 0; i < st->len; i++) {
        printf("Name: %s, Type: %d, Kind: %d\n", st->table[i]->name, st->table[i]->type, st->table[i]->kind);
    }
    for (int i = 0; i < st->childCount; i++) {
        SymbolTable* child = st->children[i];
        printf("Child Table %d:\n", i);
        printTable(child); // Recursively print child tables
    }
}

#ifndef COMPILER_H
// void main() {
//     SymbolTable st;
//     SymbolTable childSt;
//     SymbolTable grandChildSt;
//     SymbolTable greatGrandChildSt;

//     // Initialize the symbol tables
//     InitSymbolTable(&st);
//     InitSymbolTable(&childSt);
//     InitSymbolTable(&grandChildSt);
//     InitSymbolTable(&greatGrandChildSt);

//     // Set parent-child relationships
//     InsertChildTable(&st, &childSt);
//     InsertChildTable(&childSt, &grandChildSt);
//     InsertChildTable(&grandChildSt, &greatGrandChildSt);

//     // Insert symbols into the symbol tables
//     InsertSymbol("x", INTEGER, VAR, &st);
//     InsertSymbol("y", CHAR, VAR, &st);
//     InsertSymbol("z", BOOLEAN, VAR, &childSt);
//     InsertSymbol("a", IDENTIFIER, VAR, &grandChildSt);
//     InsertSymbol("b", IDENTIFIER, VAR, &greatGrandChildSt);

//     // Locate symbols and print results
//     int index = LocateSymbol("x", &greatGrandChildSt);
//     if (index != -1) {
//         printf("Symbol 'x' found at index %d\n", index);
//     } else {
//         printf("Symbol 'x' not found\n");
//     }

//     index = LocateSymbol("z", &st);
//     if (index != -1) {
//         printf("Symbol 'z' found at index %d\n", index);
//     } else {
//         printf("Symbol 'z' not found\n");
//     }

//     index = LocateSymbol("a", &greatGrandChildSt);
//     if (index != -1) {
//         printf("Symbol 'a' found at index %d\n", index);
//     } else {
//         printf("Symbol 'a' not found\n");
//     }

//     index = LocateSymbol("b", &st);
//     if (index != -1) {
//         printf("Symbol 'b' found at index %d\n", index);
//     } else {
//         printf("Symbol 'b' not found\n");
//     }

//     index = LocateSymbol("c", &greatGrandChildSt);
//     if (index != -1) {
//         printf("Symbol 'c' found at index %d\n", index);
//     } else {
//         printf("Symbol 'c' not found\n");
//     }

//     // Test the symbol table functions
//     Stack s;
//     InitStack(&s); // Initialize the stack
//     push(&s, "Hello");
//     push(&s, "World"); // Push strings onto the stack
//     printf("Popped string: %s\n", pop(&s)); // Pop a string from the stack    
//     char* poppedStr = pop(&s); // Pop another string from the stack
//     printf("Popped string: %s\n", poppedStr); // Print the popped string
//     push(&s, "Test"); // Push another string onto the stack
//     push(&s, "int"); // Push another string onto the stack
//     push(&s, "method"); // Push another string onto the stack
    
//     // Print stack
//     for (int i = 0; i <= s.topIndex; i++) {
//         printf("Stack[%d]: %s\n", i, s.data[i]); // Print the stack contents
//     }
//     InsertSymbol(pop(&s), GetType(pop(&s)), GetKind(pop(&s)), &st); // Insert a symbol into the symbol table
//     index = LocateSymbol("Test", &st); // Locate the symbol in the symbol table
//     if (index != -1) {
//         printf("Symbol '%s' found at index %d\n", "Test", index);
//     } else {
//         printf("Symbol '%s' not found\n", "Test");
//     }
//     Symbol* symbol = GetSymbolGlobal("b", &st); // Get the symbol from the symbol table
//     if (symbol != NULL) {
//         printf("Symbol found: Name: %s, Type: %d, Kind: %d\n", symbol->name, symbol->type, symbol->kind); // Print the symbol details
//     } else {
//         printf("Symbol not found\n");
//     }
//     // Print symbol table
//     for (int i = 0; i < st.len; i++) {
//         printf("Symbol %d: Name: %s, Type: %d, Kind: %d\n", i, st.table[i].name, st.table[i].type, st.table[i].kind);
//     }
//     printf("Popped string: %s\n", poppedStr); // Print the popped string
// }
#endif