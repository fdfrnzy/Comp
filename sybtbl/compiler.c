#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include "symbols.h"
#include "parser.h"
#include "compiler.h"
#define COMPILER_C

int InitCompiler ()
{
	InitSymbolTable(&ProgramScope);
	InitIdStack(&IdStack);
	initSpIdStack(&SpIdStack);
	return 1;
}

IdentifierStrct* compareId(SymbolTable* st) {
	//for id in id stack check if it exists in current scope or parent scope
	IdentifierStrct* id[3];
	int IdxParent, IdxTable, IdxProgram, IdxSpId;
	if (st == NULL) {
		return NULL;
	}
	for (int i = 0; i <= IdStack.topIndex; i++) {
		//define id[0] as the first identifier and id[1] as the second identifier
		id[0] = IdStack.data[i][0]; //identifier if id[1] null, or class
		id[1] = IdStack.data[i][1]; //if id[0] is a class, id[1] is the identifier
		//Ensire id[0] exists in the symbol table
		IdxTable = IndexTable(id[0]->name, id[0]->scope);
		IdxParent = IndexParents(id[0]->name, id[0]->scope);
		IdxProgram = IndexTable(id[0]->name, &ProgramScope);
		//check if id is a pair
		if (id[1] == NULL) {
			//check if id exists in current scope or parent scope
			// printf("Id: %s, IdxTable: %d, IdxParent: %d, scope:%p, ProgScope:%d\n", id[0]->name, IdxTable, IdxParent, id[0]->scope, IdxProgram);
			if (IdxTable == -1 && IdxParent == -1) {
				//printf("First hurdle\n");
				return id[0];
			}
			
			continue;
		//If id pair
		} else {
			//printf("Id0: %s, line:%d, file:%s, Id1: %s, line:%d, IdxTable: %d, IdxParent: %d\n", id[0]->name, id[0]->token.ln, id[0]->token.fl, id[1]->name, id[1]->token.ln, IdxTable, IdxParent);
			//check if id exists in current scope or parent scope
			if (IdxTable == -1 && IdxParent == -1) {
				return id[0];
			}
			if (IdxProgram != -1) {
				//the class is being accessed directly
				//check if id[1] exists in the child scope
				if (IndexTable(id[1]->name, ProgramScope.children[IdxProgram]) != -1) {
					//If it exists reutrn null
					continue;
				} else {
					//if not, return id[1]
					return id[1];
				}
			}
			//If the class is not being accessed directly
			//check if id[0] exists in the special id stack
			else{
				//printf("Right here\n");
				IdxSpId = indexSpIdStack(&SpIdStack, id[0]->name, id[0]->scope);
				SymbolTable* temp = id[0]->scope;
				while(IdxSpId == -1 && temp->parent != NULL) {
					//check if id[0] exists in the special id stack
					IdxSpId = indexSpIdStack(&SpIdStack, id[0]->name, temp->parent);
					temp = temp->parent;
				}
				// printf("IdxSpId: %d\n", IdxSpId);
				// //print special id stack
				// for (int j = 0; j <= SpIdStack.topIndex; j++) {
				// 	printf("SpIdStack[%d]: id:%s type:%s\n", j, SpIdStack.data[j][0]->name, SpIdStack.data[j][1]->name);
				// }
				if (IdxSpId != -1) {
					//check if the type is defined in program scope
					IdxProgram = IndexTable(SpIdStack.data[IdxSpId][1]->name, &ProgramScope);
					//if it is, check that id[1] exists in the child scope
					if (IdxProgram != -1) {
						//check if id[1] exists in the child scope
						if (IndexTable(id[1]->name, ProgramScope.children[IdxProgram]) != -1) {
							//If it exists return null
							continue;
						} else {
							//if not, return id[1]
							return id[1];
						}
					} else {
						//if not, return id[0]
						return SpIdStack.data[IdxSpId][1];
					}
				} else {
					return id[1];
				}
				
			}
		}
	}
	return NULL;
}

ParserInfo compile (char* dir_name)
{
	ParserInfo p;
	// write your code below
	// open the directory
	DIR *dir, *predir;
	dir = opendir(dir_name);
	char working_dir[100];
	strcpy(working_dir, dir_name);
	strcat(working_dir, "/..");
	predir = opendir(working_dir);
	// check if the directory is opened successfully
	if (dir == NULL || predir == NULL) {
		perror("Unable to open directory");
		p.er = lexerErr;
		return p;
	}
	struct dirent *entry;
	struct dirent *preentry;
	// Print all files in parent directory
	//printf("first file in parent directory: %s\n", preentry->d_name);
	while((preentry = readdir(predir)) != NULL) {
		// check if it's a jack file
			char file_name[1000];
			sprintf(file_name, "%s/%s", working_dir, preentry->d_name);
			if (strstr(file_name, ".jack") == NULL) {
				//printf("File: %s is not a jack file\n", file_name);
				continue;
			}
			else {
				//printf("File: %s is a jack file\n", file_name);
			}
			InitParser(file_name);
			p = Parse();
			StopParser();
			if (p.er != none) {
				// printf("Error in file: %s\n", file_name);
				// printf("Error type: %d, line: %i,token: %s\n", p.er, p.tk.ln, p.tk.lx);
				break;
			}

			
	}
	//compille all files in the same directory as the directory given
	if (dir == NULL || predir == NULL) {
		perror("Unable to open directory");
		p.er = lexerErr;
		return p;
	}
	while ((entry = readdir(dir)) != NULL) {
		// check if it's a jack file
			char file_name[1000];
			sprintf(file_name, "%s/%s", dir_name, entry->d_name);
			if (strstr(file_name, ".jack") == NULL) {
				// printf("File: %s is not a jack file\n", file_name);
				continue;
			}
			else {
				//printf("File: %s is a jack file\n", file_name);
			}
			InitParser(file_name);
			// printf("Current error: %d\n", p.er);
			p = Parse();
			StopParser();
			if (p.er != none) {
				// printf("Error in file: %s\n", file_name);
				// printf("Error type: %d, line: %i,token: %s\n", p.er, p.tk.ln, p.tk.lx);
				break;
			}

			
	}
	// close the directory
	closedir(dir);
	// print all symbols in all scopes

	// print all identufiers in all scopes
	// printf("Identifiers:\n");
	// for (int i = 0; i <= IdStack.topIndex; i++) {
	// 	printf("Identifier: %s, Scope: %s\n", IdStack.data[i][0]->name, IdStack.data[i][0]->scope->table[0]->name);
	// 	if (IdStack.data[i][1] != NULL){
	// 		printf("Identifier: %s, Scope: %s\n", IdStack.data[i][1]->name, IdStack.data[i][1]->scope->table[0]->name);
	// 	}
	// }
	IdentifierStrct* err = compareId(&ProgramScope);
	if (err == NULL){
		//printf("No undeclared identifiers\n");
	} else {
		//printf("Undeclared identifier: %s, Scope: %s\n", err->name, err->scope->table[0]->name);
		p.er = undecIdentifier;
		p.tk = err->token;
	}
	//print error
	//printf("Error type: %d, line: %i,token: %s\n", p.er, p.tk.ln, p.tk.lx);
	// printf("Symbol Table:\n");
	// 		for (int i = 0; i < ProgramScope.len; i++) {
	// 			printf("Name: %s, Type: %d, Kind: %d\n", ProgramScope.table[i]->name, ProgramScope.table[i]->type, ProgramScope.table[i]->kind);
	// 		}
	return p;
}

int StopCompiler ()
{
	ProgramScope.childCount = 0;
	ProgramScope.parent = NULL;
	for (int i = 0; i < 127; i++) {
		free(ProgramScope.children[i]);
	}
	for (int i = 0; i < ProgramScope.len; i++) {
		free(ProgramScope.table[i]);
	}
	for (int i = 0; i < IdStack.topIndex; i++) {
		free(IdStack.data[i][0]);
		free(IdStack.data[i][1]);
	}
	IdStack.topIndex = -1;
	for (int i = 0; i < SymbolStack.topIndex; i++) {
		free(SymbolStack.data[i]);
	}
	SymbolStack.topIndex = -1;

	return 1;
}


#ifndef TEST_COMPILER
// int main ()
// {
// 	InitCompiler ();
// 	ParserInfo p = compile ("Pong");
// 	#ifdef PrintError
// 	PrintError (p);
// 	#endif
// 	StopCompiler ();
// 	return 1;
// }
#endif
