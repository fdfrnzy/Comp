#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "lexer.h"
#include "symbols.h"
#include "parser.h"
#include "compiler.h"



// you can declare prototypes of parser functions below
//type pushes the type onto the stack

ParserInfo pi;
int ErrorFlag = 1;

void error (SyntaxErrors err, Token t){
	if (ErrorFlag == 0){
		return;
	}
	pi.er = err;
	pi.tk = t;
	ErrorFlag = 0;
}

int InitParser (char* file_name)
{
	Token temp; temp.tp = ERR;
	InitStack(&SymbolStack);
	InitLexer(file_name);
	pi.er = none;
	pi.tk = temp;
	return 1;
}

void classDeclar(){
	Token t = PeekNextToken();
	if (strcmp(t.lx, "class") == 0){
		GetNextToken(); // consume the token
	} else {
		error(classExpected, t);
		return;
	}
	t = PeekNextToken();
	if (t.tp != ID){
		error(idExpected, t);
		return;
	}
	if (IndexTable(t.lx, &ProgramScope) != -1){
		error(redecIdentifier, t);
		return;
	}
	else{
		pushId(&IdStack, t.lx, &ProgramScope, t, 0, 1); // Push the class name onto the stack
	}
	push(&SymbolStack, t.lx); // Push the class name onto the stack
	GetNextToken(); // consume the token
	t = PeekNextToken();
	if (strcmp(t.lx, "{") != 0){
		error(openBraceExpected, t);
		return;
	}
	GetNextToken(); // consume the token
	//Insert the class name into the symbol table
	char* id = pop(&SymbolStack); // Pop the class name from the stack
	//printf("Class name: %s\n", id);
	InsertSymbol(id, IDENTIFIER, CLASS, &ProgramScope);
	SymbolTable* ClassScope = (SymbolTable*)malloc(sizeof(SymbolTable));
	InitSymbolTable(ClassScope); // Initialize the class scope
	InsertChildTable(&ProgramScope, ClassScope); // Insert the class scope into the parent scope
	while (1){
		if (ErrorFlag == 0){
			break;
		}
		t = PeekNextToken();
		////printf("classDeclar current token: %s\n", t.lx);
		if (strcmp(t.lx, "}") == 0){
			GetNextToken(); // consume the token
			break;
		}
		if (t.tp == EOFile){
			error(closeBraceExpected, t);
			return;
		}
		memberDeclar(ClassScope);
	}
	return;
}

void memberDeclar(SymbolTable* cs/*class scope*/){
	Token t = PeekNextToken();
	if (strcmp(t.lx, "static") == 0 || strcmp(t.lx, "field") == 0){
		classVarDeclar(cs);// Process class variable declaration
	} else if(strcmp(t.lx, "constructor") == 0 || strcmp(t.lx, "function") == 0 || strcmp(t.lx, "method") == 0){
		subroutineDeclar(cs);// Process subroutine declaration
	} else {
		error(memberDeclarErr, t);
		return;
	}
	//printf("memberDeclar returns\n");
	return;
}

void classVarDeclar(SymbolTable* cs/*class scope*/){
	Token t = PeekNextToken();
	if (strcmp(t.lx, "static") == 0 || strcmp(t.lx, "field") == 0){
		push(&SymbolStack, t.lx); // Push the kind onto the stack
		GetNextToken(); // consume the token
	} else {
		error(memberDeclarErr, t);
		return;
	}
	type(cs);
	//print the stack
	char* type = pop(&SymbolStack); // Pop the kind from the stack
	char* kind = pop(&SymbolStack); // Pop the type from the stack
	while(1){
		if (ErrorFlag == 0){
			break;
		}
		t = PeekNextToken();
		//printf("classVarDeclar current token: %s\n", t.lx);
		if (t.tp != ID){
			error(idExpected, t);
			return;
		}
		GetNextToken(); // consume the token
		if (IndexTable(t.lx, cs) != -1){
			error(redecIdentifier, t);
			return;
		}
		else{
			pushId(&IdStack, t.lx, cs, t, 0, 1); // Push the identifier name onto the stack
		}
		InsertSymbol(t.lx, GetType(type), GetKind(kind), cs); // Insert the symbol into the class scope
		t = PeekNextToken();
		if (strcmp(t.lx, ",") == 0){
			GetNextToken(); // consume the token
		} else {
			break;
		}
	}
	//print the current symbol table
	// printf("Symbol Table:\n");
	// for (int i = 0; i < cs->len; i++){
	// 	printf("Name: %s, Type: %d, Kind: %d\n", cs->table[i]->name, cs->table[i]->type, cs->table[i]->kind);
	// }
	t = PeekNextToken();
	if (strcmp(t.lx, ";") != 0){
		error(semicolonExpected, t);
		GetNextToken(); // consume the token
		return;
	}
	GetNextToken(); // consume the token
	return;
}

void type(SymbolTable* CurrentScope){
	Token t = PeekNextToken();
	Token idtk;
	if (strcmp(t.lx, "int") == 0 || strcmp(t.lx, "char") == 0 || strcmp(t.lx, "boolean") == 0){
		push(&SymbolStack, t.lx); // Push the type onto the stack
		GetNextToken(); // consume the token
	} else if (t.tp == ID){
		//Impelemnt symbols with type ID
		//
		pushId(&IdStack, t.lx, CurrentScope, t, 0, 1); // Push the identifier name onto the stack
		push(&SymbolStack, "identifier"); // Push the type onto the stack
		GetNextToken(); // consume the token
		idtk = t; // Store the identifier token
		t = PeekNextToken();
		if (t.tp == ID){
			pushSpId(&SpIdStack, t.lx, CurrentScope, t, 0); // Push the identifier name onto the stack}
			pushSpId(&SpIdStack, idtk.lx, CurrentScope, idtk, 1); // Push the identifier name onto the stack
		}
		} else {
		error(illegalType, t);
		return;
	}
	return;
}

void subroutineDeclar(SymbolTable* cs/*class scope*/){
	Token t = PeekNextToken();
	Token idtk;
	if (strcmp(t.lx, "constructor") == 0 || strcmp(t.lx, "function") == 0 || strcmp(t.lx, "method") == 0){
		push(&SymbolStack, t.lx); // Push the kind onto the stack
		GetNextToken(); // consume the token
	} else {
		error(subroutineDeclarErr, t);
		return;
	}
	t = PeekNextToken();
	if (strcmp(t.lx, "void") == 0){
		push(&SymbolStack, t.lx); // Push the type onto the stack
		GetNextToken(); // consume the token
	} else {
		type(cs);
	}
	t = PeekNextToken();
	if (t.tp != ID){
		error(idExpected, t);
		return;
	}
	idtk = t; // Store the identifier token
	GetNextToken(); // consume the token
	push(&SymbolStack, t.lx); // Push the subroutine name onto the stack
	t = PeekNextToken();
	if (strcmp(t.lx, "(") != 0){
		error(openParenExpected, t);
		return;
	}
	GetNextToken(); // consume the token
	//new scope for the subroutine
	SymbolTable* SubroutineScope = (SymbolTable*)malloc(sizeof(SymbolTable));
	InitSymbolTable(SubroutineScope); // Initialize the subroutine scope
	InsertChildTable(cs, SubroutineScope); // Insert the subroutine scope into the class scope
	//Added parameter list to the symbol table
	paramList(SubroutineScope);
	t = PeekNextToken();
	if (strcmp(t.lx, ")") != 0){
		error(closeParenExpected, t);
		return;
	}
	//insert the subroutine name into the symbol table
	char* id = pop(&SymbolStack); // Pop the subroutine name from the stack
	char* type = pop(&SymbolStack); // Pop the type from the stack
	char* kind = pop(&SymbolStack); // Pop the kind from the stack
	if (IndexTable(id, SubroutineScope) != -1){
		error(redecIdentifier, idtk);
		return;
	}
	pushId(&IdStack, id, cs, idtk, 0, 1); // Push the subroutine name onto the stack
	InsertSymbol(id, GetType(type), GetKind(kind), cs); // Insert the subroutine name into the subroutine scope
	GetNextToken(); // consume the token
	//printf("%p\n", SubroutineScope);
	subroutineBody(SubroutineScope);
	//printf("subroutineDeclar returns\nToken: %s\n", PeekNextToken().lx);
	return;
}

void paramList(SymbolTable* ss /*subroutine scope*/){
	Token t = PeekNextToken();
	if (strcmp(t.lx, ")") == 0){
		// empty parameter list
		return;
	}
	else if (strcmp(t.lx, "{") == 0){
		error(closeParenExpected, t);
		return;
	}
	while(1){
		if (ErrorFlag == 0){
			break;
		}
		type(ss);
		t = PeekNextToken();
		if (t.tp != ID){
			error(idExpected, t);
			return;
		}
		push(&SymbolStack, t.lx); // Push the parameter name onto the stack
		if (IndexTable(t.lx, ss) != -1){
			error(redecIdentifier, t);
			return;
		}
		pushId(&IdStack, t.lx, ss, t, 0, 1); // Push the parameter name onto the stack
		GetNextToken(); // consume the token
		char* id = pop(&SymbolStack); // Pop the parameter name from the stack
		char* type = pop(&SymbolStack); // Pop the type from the stack
		if (IndexTable(id, ss) != -1 && IndexParents(id, ss) != -1){
			error(redecIdentifier, t);
			return;
		}
		InsertSymbol(id, GetType(type), VAR, ss); // Insert the parameter into the subroutine scope
		t = PeekNextToken();
		if (strcmp(t.lx, ",") == 0){
			GetNextToken(); // consume the token
		} else {
			break;
		}
	}
	return;
}

void subroutineBody(SymbolTable* ss /*subroutine scope*/){
	Token t = PeekNextToken();
	if (strcmp(t.lx, "{") == 0){
		GetNextToken(); // consume the token
		while (1){
			if (ErrorFlag == 0){
				break;
			}
			t = PeekNextToken();
			//printf("subroutineBody current token %s\n", t.lx);
			if (strcmp(t.lx, "}") == 0){
				GetNextToken(); // consume the token
				break;
			}
			if (t.tp == EOFile){
				error(closeBraceExpected, t);
				return;
			}
			statement(ss);
		}
	} else {
		error(openBraceExpected, t);
	}
	//printf("subroutineBody returns\nToken: %s\n", PeekNextToken().lx);
	return;
}

void statement(SymbolTable* ss/*scope*/){
	Token t = PeekNextToken();
	//printf("statement current token %s\n", t.lx);
	if (strcmp(t.lx, "let") == 0){
		//printf("let statement\n");
		letStatement(ss);
	} else if (strcmp(t.lx, "if") == 0){
		//printf("if statement\n");
		ifStatement(ss);
	} else if (strcmp(t.lx, "while") == 0){
		//printf("while statement\n");
		whileStatement(ss);
	} else if (strcmp(t.lx, "do") == 0){
		//printf("do statement\n");
		doStatement(ss);
	} else if (strcmp(t.lx, "return") == 0){
		//printf("return statement\n");
		returnStatemnt(ss);
	} else if (strcmp(t.lx, "var") == 0){
		//printf("var statement\n");
		varDeclarStatement(ss);
	} else if (pi.er == none){
		//printf("syntax error in statement\n");
		error(syntaxError, t);
	}
	//printf("statement returns\n");
	return;
}

void varDeclarStatement(SymbolTable* s/*scope*/){
	Token t = PeekNextToken();
	if (strcmp(t.lx, "var") == 0){
		GetNextToken(); // consume the token
		type(s);
		char* type = pop(&SymbolStack); // Pop the type from the stack
		t = PeekNextToken();
		if (t.tp != ID){
			error(idExpected, t);
			return;
		}
		//printf("%p\n", s);
		push(&SymbolStack, t.lx); // Push the identifier name onto the stack
		pushId(&IdStack, t.lx, s, t, 0, 1); // Push the identifier name onto the stack
		//printf("Pushed identifier: %s scope: %p\n", t.lx, s);
		GetNextToken(); // consume the token
		while (1){
			if (ErrorFlag == 0){
				break;
			}
			char* id = pop(&SymbolStack); // Pop the identifier name from the stack
			if (IndexTable(id, s) != -1){
				error(redecIdentifier, t);
				return;
			}
			InsertSymbol(id, GetType(type), VAR, s); // Insert the symbol into the class scope
			t = PeekNextToken();
			if (strcmp(t.lx, ",") == 0){
				GetNextToken(); // consume the token
				t = PeekNextToken();
				if (t.tp != ID){
					error(idExpected, t);
					return;
				}
				push(&SymbolStack, t.lx); // Push the identifier name onto the stack
				pushId(&IdStack, t.lx, s, t, 0, 1); // Push the identifier name onto the stack
				GetNextToken(); // consume the token
			} else {
				break;
			}
		}
		if (strcmp(t.lx, ";") != 0){
			error(semicolonExpected, t);
			GetNextToken(); // consume the token
			return;
		}
		GetNextToken(); // consume the token
	} else {
		error(syntaxError, t);
	}
	return;
}

void letStatement(SymbolTable* s/*scope*/){
	Token t = PeekNextToken();
	//printf("letStatement current token %s\n", t.lx);
	if (strcmp(t.lx, "let") == 0){
		GetNextToken(); // consume the token
		t = PeekNextToken();
		//printf("current token %s\n", t.lx);
		if (t.tp != ID){
			error(idExpected, t);
			return;
		}
		pushId(&IdStack, t.lx, s, t, 0, 1); // Push the identifier name onto the stack
		GetNextToken(); // consume the token
		t = PeekNextToken();
		if (strcmp(t.lx, "[") == 0){
			GetNextToken(); // consume the token
			expression(s);
			t = PeekNextToken();
			if (strcmp(t.lx, "]") != 0){
				error(closeBracketExpected, t);
				return;
			}
			GetNextToken(); // consume the token
			t = PeekNextToken();
		}
		if (strcmp(t.lx, "=") != 0){
			//printf("current token not equal %s\n", t.lx);

			error(equalExpected, t);
			return;
		}
		GetNextToken(); // consume the token
		//printf("preexpression token %s\n", t.lx);
		t = PeekNextToken();
		if(strcmp(t.lx, ";") == 0){
			// Expression Missing
			error(syntaxError, t);
			return;
		}
		expression(s);
		t = PeekNextToken();
		if (strcmp(t.lx, ";") != 0){
			error(semicolonExpected, t);
			GetNextToken(); // consume the token
			return;
		}
		GetNextToken(); // consume the token
	} else {
		error(syntaxError, t);
	} 
	//printf("letStatement returns\nNext token: %s\n", PeekNextToken().lx);
	return;
}

void ifStatement(SymbolTable* s/*scope*/){
	Token t = PeekNextToken();
	if (strcmp(t.lx, "if") == 0){
		GetNextToken(); // consume the token
		t = PeekNextToken();
		if (strcmp(t.lx, "(") != 0){
			error(openParenExpected, t);
			return;
		}
		GetNextToken(); // consume the token
		expression(s);
		t = PeekNextToken();
		if (strcmp(t.lx, ")") != 0){
			error(closeParenExpected, t);
			return;
		}
		GetNextToken(); // consume the token
		t = PeekNextToken();
		if (strcmp(t.lx, "{") != 0){
			error(openBraceExpected, t);
			return;
		}
		GetNextToken(); // consume the token
		while (1){
			if (ErrorFlag == 0){
				break;
			}
			t = PeekNextToken();
			if (strcmp(t.lx, "}") == 0){
				GetNextToken(); // consume the token
				break;
			}
			if (t.tp == EOFile){
				error(closeBraceExpected, t);
				return;
			}
			//printf("if statement current token %s\n", t.lx);
			statement(s);
		}
		t = PeekNextToken();
		if (strcmp(t.lx, "else") == 0){
			GetNextToken(); // consume the token
			t = PeekNextToken();
			if (strcmp(t.lx, "{") != 0){
				error(openBraceExpected, t);
				return;
			}
			GetNextToken(); // consume the token
			while (1){
				if (ErrorFlag == 0){
					break;
				}
				t = PeekNextToken();
				if (strcmp(t.lx, "}") == 0){
					GetNextToken(); // consume the token
					break;
				}
				if (t.tp == EOFile){
					error(closeBraceExpected, t);
					return;
				}
				statement(s);
			}
		}
	} else {
		error(syntaxError, t);
	}
	return;
}

void whileStatement(SymbolTable* s/*scope*/){
	Token t = PeekNextToken();
	int i = 0;
	if (strcmp(t.lx, "while") == 0){
		//printf("In while statement\n");
		GetNextToken(); // consume the token
		t = PeekNextToken();
		if (strcmp(t.lx, "(") != 0){
			error(openParenExpected, t);
			return;
		}
		GetNextToken(); // consume the token
		expression(s);
		t = PeekNextToken();
		if (strcmp(t.lx, ")") != 0){
			error(closeParenExpected, t);
			return;
		}
		GetNextToken(); // consume the token
		t = PeekNextToken();
		//printf("while statement current token %s\n", t.lx);
		if (strcmp(t.lx, "{") != 0){
			error(openBraceExpected, t);
			return;
		}
		GetNextToken(); // consume the token
		t = PeekNextToken();
		while (strcmp(t.lx, "}" ) != 0 && i < 10){
			if (ErrorFlag == 0){
				break;
			}
			//printf("while loop current token %s\n", t.lx);
			//printf("%d\n", i);
			if (t.tp == EOFile){
				error(closeBraceExpected, t);
				return;
			}
			statement(s);
			//printf("while loop current token %s\n", t.lx);
			t = PeekNextToken();
			i++;
		}
		GetNextToken(); // consume the token
		// Process while statement
	} else {
		error(syntaxError, t);
	}
	return;
}

void doStatement(SymbolTable* s/*scope*/){
	Token t = PeekNextToken();
	if (strcmp(t.lx, "do") == 0){
		GetNextToken(); // consume the token
		subroutineCall(s);
		t = PeekNextToken();
		//printf("do statement current token %s\n", t.lx);
		if (strcmp(t.lx, ";") != 0){
			error(semicolonExpected, t);
			return;
		}
		GetNextToken(); // consume the token
		// Process do statement
	} else {
		error(syntaxError, t);
		//printf("do statement error\n");
	}
	//printf("do statement returns\nNext Token: %s\n", PeekNextToken().lx);
	return;
}

void subroutineCall(SymbolTable* s/*scope*/){
	Token t = PeekNextToken();
	if (t.tp == ID){
		pushId(&IdStack, t.lx, s, t, 0, 1); // Push the identifier name onto the stack
		GetNextToken(); // consume the token
		t = PeekNextToken();
		if (strcmp(t.lx, "(") == 0){
			GetNextToken(); // consume the token
			expressionList(s);
			t = GetNextToken();
			if (strcmp(t.lx, ")") != 0){
				error(closeParenExpected, t);
			}
			return;
		} else if (strcmp(t.lx, ".") == 0){
			GetNextToken(); // consume the token
			t = GetNextToken(); // get the next token
			if (t.tp != ID){
				error(idExpected, t);
				return;
			}
			pushId(&IdStack, t.lx, s, t, 1, 1); // Push the identifier name onto the stack
			t = PeekNextToken();
			if (strcmp(t.lx, "(") == 0){
				GetNextToken(); // consume the token
				expressionList(s);
				t = GetNextToken();
				//printf("subroutineCall current token %s\n", t.lx);
				if (strcmp(t.lx, ")") != 0){
					error(closeParenExpected, t);
				}
				//printf("subroutineCall returns\nNext token %s\n", PeekNextToken().lx);
				return;
			}
			else{
				error(openParenExpected, t);
				return;
			}
		}
	} else {
		error(idExpected, t);
	}

	return;
}

void expressionList(SymbolTable* s/*scope*/){
	Token t = PeekNextToken();
	if (strcmp(t.lx, ")") == 0){
		// empty expression list
	}
	else {
		expression(s);
		while (1){
			if (ErrorFlag == 0){
				break;
			}
			t = PeekNextToken();
			if (strcmp(t.lx, ",") == 0){
				GetNextToken(); // consume the token
				expression(s);
			} else {
				break;
			}
		}
	}
	return;
}

void returnStatemnt(SymbolTable* s/*scope*/){
	Token t = PeekNextToken();
	//printf("returnStatemnt current token %s\n", t.lx);
	if (strcmp(t.lx, "return") == 0){
		GetNextToken(); // consume the token
		t = PeekNextToken();
		if (strcmp(t.lx, ";") == 0){
			GetNextToken(); // consume the token
			// Proces return statement with no expression
		} else if(strcmp(t.lx, "}") == 0){
			// Process return statement with no expression
			error(semicolonExpected, t);
			return;
		} else if (t.tp == EOFile){
			error(semicolonExpected, t);
			return;
		} else {
			expression(s);
			t = GetNextToken();
			if (strcmp(t.lx, ";") != 0){
				error(semicolonExpected, t);
				GetNextToken(); // consume the token
				return;
			}
			//printf("token: %s", t.lx); // consume the token
			// Process return statement with expression
		}
	} else {
		error(syntaxError, t);
	}
	return;
}

void expression(SymbolTable* s/*scope*/){
	relationalExpression(s);
	while (1){
		if (ErrorFlag == 0){
			break;
		}
		Token t = PeekNextToken();
		//printf("expression current token %s\n", t.lx);
		if (strcmp(t.lx, "&") == 0 || strcmp(t.lx, "|") == 0){
			GetNextToken(); // consume the token
			relationalExpression(s);
		} else {
			break;
		}
	}
	// Process the expression
	return;
}

void relationalExpression(SymbolTable* s/*scope*/){
	ArithmeticExpression(s);

	while (1){
		if (ErrorFlag == 0){
			break;
		}
		Token t = PeekNextToken();
		//printf("relationalExpression current token %s\n", t.lx);
		if (strcmp(t.lx, "<") == 0 || strcmp(t.lx, ">") == 0 || strcmp(t.lx, "=") == 0){
			GetNextToken(); // consume the token
			ArithmeticExpression(s);

		} else {
			break;
		}
	}
	// Process the relational expression
	return;
}

void ArithmeticExpression(SymbolTable* s/*scope*/){
	//printf("ArithmeticExpression Next Token: %s\n", PeekNextToken().lx);
	term(s);
	while (1){
		if (ErrorFlag == 0){
			break;
		}
		Token t = PeekNextToken();
		if (strcmp(t.lx, "+") == 0 || strcmp(t.lx, "-") == 0){
			GetNextToken(); // consume the token
			term(s);
		} else {
			break;
		}
	}
	// Process the expression
	return;
}

void term(SymbolTable* s/*scope*/){
	factor(s);
	while (1){
		if (ErrorFlag == 0){
			break;
		}
		Token t = PeekNextToken();
		if (strcmp(t.lx, "*") == 0 || strcmp(t.lx, "/") == 0){
			GetNextToken(); // consume the token
			factor(s);
		} else {
			break;
		}
	}
	// Process the term
	return;
}

void factor(SymbolTable* s/*scope*/){
	Token t = PeekNextToken();
	//printf("factor current token: %s\n", t.lx);
	if (strcmp(t.lx, "-") == 0 || strcmp(t.lx, "~") == 0){
		GetNextToken(); // consume the token
		//printf("factor unary operator %s\n", t.lx);
		// Process unary operator
		Operand(s);
	} else if (t.tp == INT || t.tp == ID || t.tp == STRING || t.tp == SYMBOL || t.tp == RESWORD){
		// Process operand
		Operand(s);
	} else {
		error(syntaxError, t);
	}
	return;
}

void Operand(SymbolTable* s/*scope*/){
	Token t = PeekNextToken();
	Token temp;
	//printf("Operand current token %s\n", t.lx);
	// integerConstant
	if (t.tp == INT){
		GetNextToken(); // consume the token
		// process integer
		return;
	// identifier [.identifier] [ [expression] | (expressionList) ]
	} else if (t.tp == ID){
		temp = t;
		GetNextToken(); // consume the token
		//printf("Operand identifier %s\n", t.lx);
		// Can be a variable or a function call or indexing
		t = PeekNextToken();
		if (strcmp(t.lx, ".") == 0){
			pushId(&IdStack, temp.lx, s, temp, 0, 1); // Push the identifier name onto the stack
		}
		else{
			pushId(&IdStack, temp.lx, s, temp, 0, 0); // Push the identifier name onto the stack
		}
		if (t.tp == SYMBOL){
			//printf("Operand Id then symbol %s\n", t.lx);
			// Check for indexing
			if (strcmp(t.lx, "[") == 0){
				GetNextToken(); // consume the token
				expression(s);
				//printf("Operand Id then symbol expression returns %s\n", PeekNextToken().lx);
				t = GetNextToken();
				if (strcmp(t.lx, "]") != 0){
					error(closeBracketExpected, t);
					return;
				}
			// Check for function call
			} else if (strcmp(t.lx, "(") == 0){
				GetNextToken(); // consume the token
				expressionList(s);
				t = GetNextToken();
				if (strcmp(t.lx, ")") != 0){
					error(closeBracketExpected, t);
					return;
				}
			// Check for dot operator (method call)
			} else if (strcmp(t.lx, ".") == 0){
				GetNextToken(); // consume the token
				t = GetNextToken(); // get the next token
				if (t.tp != ID){
					error(idExpected, t);
					return;
				}
				pushId(&IdStack, t.lx, s, t, 1, 0); // Push the identifier name onto the stack
				t = PeekNextToken();
				if (strcmp(t.lx, "(") == 0){
					GetNextToken(); // consume the token
					expressionList(s);
					t = GetNextToken();
					if (strcmp(t.lx, ")") != 0){
						error(closeBracketExpected, t);
						return;
					}
				}
				else if (strcmp(t.lx, "[") == 0){
					GetNextToken(); // consume the token
					expression(s);
					t = GetNextToken();
					if (strcmp(t.lx, "]") != 0){
						error(closeBracketExpected, t);
						return;
					}
				}
			}
		}
		// process variable
		//printf("Operand returns CT: %s\n", t.lx);
		return;
	// (expression)
	} else if (t.tp == SYMBOL){
		GetNextToken(); // consume the token
		// Check for open bracket
		if (strcmp(t.lx, "(") == 0){
			expression(s);
			t = GetNextToken();
			if (strcmp(t.lx, ")") != 0){
				error(closeParenExpected, t);
			}
			return;
		}
	// stringLiteral
	} else if (t.tp == STRING){
		GetNextToken(); // consume the token
		// process string
		return;
	}
	// true | false | null | this
	else if (t.tp == RESWORD){
		if (strcmp(t.lx, "true") == 0 || strcmp(t.lx, "false") == 0 || strcmp(t.lx, "null") == 0 || strcmp(t.lx, "this") == 0){
			//printf("true | false | null | this\n");
			GetNextToken(); // consume the token
			// process boolean or null or this
			return;
		}
	}
	else {
		error(syntaxError, t);
	}
}




ParserInfo Parse ()
{
	// implement the function
	// parse the input file (the one passed to InitParser)
	// and return the ParserInfo struct
	Token t = PeekNextToken();
	//printf("Parse current token %s\n", t.lx);
	if (t.tp == ERR || t.tp == EOFile){
		error(lexerErr, t);
		//printf("Error in lexer\n");
		return pi;
	}
	while (1){
		if (ErrorFlag == 0){
			//printf("ErrorFlagged\n");
			break;
		}
		//printf("Parse current error %d\n", pi.er);
		if (pi.er != none){
			//printf("Error in parser\n");
			break;
		}
		t = PeekNextToken();
		//printf("%s\n", t.lx);
		if (t.tp == EOFile){
			break;
		}
		if (t.tp == ERR){
			error(lexerErr, t);
			break;
		}
		if (strcmp(t.lx, "class") == 0){
			classDeclar();
		}
		else if (strcmp(t.lx, "static") == 0 || strcmp(t.lx, "field") == 0 || strcmp(t.lx, "constructor") == 0 || strcmp(t.lx, "function") == 0 || strcmp(t.lx, "method") == 0){
			memberDeclar(&ProgramScope);
		}
		else if (strcmp(t.lx, "var") == 0 || strcmp(t.lx, "let") == 0 || strcmp(t.lx, "if") == 0 || strcmp(t.lx, "while") == 0 || strcmp(t.lx, "do") == 0 || strcmp(t.lx, "return") == 0){
			statement(&ProgramScope);
			//printf("Here\n");
		}
		else{
			error(classExpected, t);
			break;
		}
	}
	//printf("Parse returns: %d with tkn: %s ln: %d\n", pi.er, pi.tk.lx, pi.tk.ln);
	//print all symbols in all scopes
	// for (int i = 0; i < ProgramScope.len; i++){
	// 	//printf("Name: %s, Type: %d, Kind: %d\n", ProgramScope.table[i]->name, ProgramScope.table[i]->type, ProgramScope.table[i]->kind);
	// }
	SymbolTable* current = &ProgramScope;
	// while (current->childCount > 0){
	// 	printTable(current);
	// 	for (int i = 0; i < current->childCount; i++){
	// 		if (current->childCount > 0){
	// 			current = current->children[i];
	// 		}
	// 		else{
	// 			break;
	// 		}
	// 	}
	// }
	// printTable(&ProgramScope); // Print the symbol table
	//print all identifiers in the stack
	// for (int i = 0; i < IdStack.topIndex; i++){
	// 	//printf("Id: %s\n", IdStack.data[i]->name);
	// }
	return pi;
}


int StopParser ()
{
	StopLexer();
	pi.er = none;
	pi.tk = (Token){ERR, "", 0, 0, ""};
	ErrorFlag = 1;
	return 1;
}

#ifndef TEST_COMPILER
// int main ()
// {
// 	printf("Parser module\n");
// 	InitParser("Output.jack");
// 	ParserInfo pi = Parse();
// 	StopParser();
// 	return 0;
// }
#endif
