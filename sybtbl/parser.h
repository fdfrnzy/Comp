#ifndef PARSER_HEADER_DEFINED
#define PARSER_HEADER_DEFINED
#include "lexer.h"
#include "symbols.h"
typedef enum {
	none,					// no errors
	lexerErr,				// lexer error
	classExpected,			// keyword class expected
	idExpected,				// identifier expected
	openBraceExpected,		// { expected
	closeBraceExpected,		// } expected
	memberDeclarErr,		// class member declaration must begin with static, field, constructor , function , or method
	classVarErr,			// class variables must begin with field or static
	illegalType,			// a type must be int, char, boolean, or identifier
	semicolonExpected,		// ; expected
	subroutineDeclarErr,	// subrouting declaration must begin with constructor, function, or method
	openParenExpected,		// ( expected
	closeParenExpected,		// ) expected
	closeBracketExpected,	// ] expected
	equalExpected,			// = expected
	syntaxError,			// any other kind of syntax error
	// extend this list to include two types of semantic errors
	undecIdentifier,		// undeclared identifier (e.g. class, subroutine, or variable)
	redecIdentifier,		// redeclaration of identifier in the same scope
	NoLexErr
} SyntaxErrors;


// every parsing function should return this struct
// the struct contains an error type field (er)
// and a token field (tk) that is set to the token at or near which the error was encountered
// if no errors are encountered er should be set to none
typedef struct
{
	SyntaxErrors er;
	Token tk;
} ParserInfo;

int InitParser (char* file_name); // initialise the parser to parse source code in file_name
ParserInfo Parse (); // parse the input file (the one passed to InitParser)
int StopParser (); // stop the parser and do any necessary clean up
char* ErrorString (SyntaxErrors e);
void error (SyntaxErrors err, Token t); // print an error message and exit the program
void classDeclar(); // parse a class declaration
void memberDeclar(SymbolTable* cs/*class scope*/); // parse a class member declaration
void classVarDeclar(SymbolTable* cs/*class scope*/); // parse a class variable declaration
void type(SymbolTable* CurrentScope); // parse a type (int, char, boolean, or identifier)
void subroutineDeclar(SymbolTable* cs/*class scope*/); // parse a subroutine declaration (constructor, function, or method)
void paramList(SymbolTable* ss /*subroutine scope*/); // parse a parameter list (a list of types and identifiers)
void subroutineBody(SymbolTable* ss /*subroutine scope*/); // parse a subroutine body (a list of statements)
void statement(SymbolTable* ss); // parse a statement (let, if, while, do, return)
void varDeclarStatement(SymbolTable* s/*scope*/); // parse a variable declaration statement (var)
void letStatement(SymbolTable* s); // parse a let statement (let)
void ifStatement(SymbolTable* s/*scope*/); // parse an if statement (if)
void whileStatement(SymbolTable* s/*scope*/); // parse a while statement (while)
void doStatement(SymbolTable* s/*scope*/); // parse a do statement (do)
void subroutineCall(SymbolTable* s/*scope*/); // parse a subroutine call (a method or function call)
void expressionList(SymbolTable* s/*scope*/); // parse an expression list (a list of expressions)
void returnStatemnt(SymbolTable* s/*scope*/); // parse a return statement (return)
void expression(SymbolTable* s/*scope*/); // parse an expression (a term or a series of terms)
void relationalExpression(SymbolTable* s/*scope*/); // parse a relational expression (a term or a series of terms)
void ArithmeticExpression(SymbolTable* s/*scope*/); // parse an arithmetic expression (a term or a series of terms)
void term(SymbolTable* s/*scope*/); // parse a term (a variable, a constant, or an expression in parentheses)
void factor(SymbolTable* s/*scope*/); // parse a factor (a variable, a constant, or a string literal)
void Operand(SymbolTable* s/*scope*/); // parse an operand (a variable, a constant, or a string literal)
#endif
