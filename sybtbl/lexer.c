#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "lexer.h"


// YOU CAN ADD YOUR OWN FUNCTIONS, DECLARATIONS AND VARIABLES HERE
#define NumKeywords 21
FILE *input;
int LineCount;
bool TokenListReady, TokenReady;
Token t;
const char* keywords[NumKeywords] = {"class", "constructor", "method", "function", "int", "boolean", "char", "void", "var", "static", "field", "let", "do", "if", "else", "while", "return", "true", "false", "null", "this"};
char current_file[100];
Token TokenList[10000];
int CurrentToken = 0;

// IMPLEMENT THE FOLLOWING functions
//***********************************

// Initialise the lexer to read from source file
// file_name is the name of the source file
// This requires opening the file and making any necessary initialisations of the lexer
// If an error occurs, the function should return 0
// if everything goes well the function should return 1

bool IsKeyWord(char* str){
  for(int i = 0; i < NumKeywords; i++){
    if(strcmp(str, keywords[i]) == 0){
      return true;
    }
  }
  return false;
}

int EatWC(){
  //Consume white space and comments then return char
  int c = getc(input);
  while (c != -1){
    if (c == '\n'){
      LineCount++;
    }
    if (c == '/'){
      c = getc(input);
      if (c == '/'){
        while (c != '\n'){
          if (c == EOF){
            return 0;
          }
          //Eat Comment
          c = getc(input);
        }
        LineCount++;
      } else if (c == '*'){
        while (1==1){
          c = getc(input);
          if (c == EOF){
            return 0;
          } else if (c == '*'){
            c = getc(input);
            if (c == '/'){
              break;
            }
          } else if (c == '\n'){
            LineCount++;
          }
        }
      } else {
        // If not a comment, return the char
        ungetc(c, input);
        return '/';
      }
    } else if (!isspace(c)){
      return c;
    }
    c = getc(input);
  }
  return EOF;
}

char* ResetBuffer(char* buffer){
  // Reset the buffer to empty
  for (int i = 0; i < 100; i++){
    buffer[i] = '\0';
  }
  return buffer;
}
void BuildToken(){
  // Gather tokens from the source file and store them in the token stream
  // This function should be called after the lexer has been initialised and the source file opened
  
  char buffer[1000];
  int chr = 0;

  char c = EatWC();
  //Loop through the file until EOF or a token is found
  if (c == EOF || c == -1){
    t.tp = EOFile;
    strcpy(t.lx, "End of file");
    t.ln = LineCount;
    strncpy(t.fl, current_file, sizeof(t.fl) - 1);
    TokenReady = true;
    return;
  }
  if (c == 0){
    //EOF in comment
    // Set error code and message in token
    t.tp = ERR;
    t.ec = EofInCom;
    t.ln = LineCount;
    strcpy(t.lx, "Error: unexpected eof in comment");
    strncpy(t.fl, current_file, sizeof(t.fl) - 1);
    TokenReady = true;
    return;
  }
  while(c != EOF){
    if (c == '"'){
      //String Literal
      c = getc(input);
      while(c != '"'){
        buffer[chr++] = c;
        c = getc(input);
        if (c == EOF){
          // End of file in string literal
          // Set error code and message in token
          t.tp = ERR;
          t.ec = EofInStr;
          t.ln = LineCount;
          strcpy(t.lx, "Error: unexpected eof in string constant");
          strncpy(t.fl, current_file, sizeof(t.fl) - 1);
          TokenReady = true;
          return;
        }
        if (c == '\n'){
          // New line in string literal
          // Set error code and message in token
          t.tp = ERR;
          t.ec = NewLnInStr;
          t.ln = LineCount;
          strcpy(t.lx, "Error: new line in string constant");
          strncpy(t.fl, current_file, sizeof(t.fl) - 1);
          TokenReady = true;
          return;
        }
      }
      buffer[chr] = '\0';
      t.tp = STRING;
      strncpy(t.lx, buffer, sizeof(t.lx) - 1);
      strcpy(buffer, ResetBuffer(buffer)); chr = 0;
      t.ln = LineCount;
      strncpy(t.fl, current_file, sizeof(t.fl) - 1);
      TokenReady = true;
      return;
      
    }
    else if (isalpha(c) || c == '_'){
      while(isalnum(c) || c == '_'){
        buffer[chr++] = c;
        c = getc(input);
      }
      buffer[chr] = '\0';
      if (IsKeyWord(buffer)){
        t.tp = RESWORD;
      } else {
        t.tp = ID;
      }
      strncpy(t.lx, buffer, sizeof(t.lx) - 1);
      t.ln = LineCount;
      strncpy(t.fl, current_file, sizeof(t.fl) - 1);
      strcpy(buffer, ResetBuffer(buffer)); chr = 0;
      ungetc(c, input); // Put back the last character
      TokenReady = true;
      return;
    }
    else if(isspace(c)){
      c = EatWC();
    }
    else if(isdigit(c)){
      while(isdigit(c)){
        buffer[chr++] = c;
        c = getc(input);
      }
      buffer[chr] = '\0';
      t.tp = INT;
      strncpy(t.lx, buffer, sizeof(t.lx) - 1);
      t.ln = LineCount;
      strncpy(t.fl, current_file, sizeof(t.fl) - 1);
      strcpy(buffer, ResetBuffer(buffer)); chr = 0;
      ungetc(c, input); // Put back the last character
      TokenReady = true;
      return;
    }
    else{
      //Must Be a symbol or illegal character
      if (c == '{' || c == '}' || c == '(' || c == ')' || c == '[' || c == ']' || c == '.' || c == ',' || c == ';' || c == '+' || c == '-' || c == '*' || c == '/' || c == '&' || c == '|' || c == '<' || c == '>' || c == '=' || c == '~'){
        t.tp = SYMBOL;
        t.lx[0] = c;
        t.lx[1] = '\0';
        t.ln = LineCount;
        strncpy(t.fl, current_file, sizeof(t.fl) - 1);
        TokenReady = true;
        return;
      } else {
        // Illegal symbol in source file
        // Set error code and message in token
        t.tp = ERR;
        t.ec = IllSym;
        t.ln = LineCount;
        strcpy(t.lx, "Error: illegal symbol in source file");
        TokenReady = true;
        return;
      }
    }

  }
}

Token FetchToken ()
{
	Token t;
  TokenListReady = false;
  t.tp = ERR;
  BuildToken();
  return t;
}

void StoreTokens(){
  //Build the token stream and store it in the token list
  // This function should be called after the lexer has been initialised and the source file opened
  int tok = 0;
  while (1){
    if (TokenReady){
      TokenList[tok] = t;
      tok++;
      if (t.tp == EOFile){
        break;
      }
    }
    FetchToken();
  }
  TokenListReady = true;
  //print tokken list
  
}

Token GetNextToken ()
{ 
  // Get the next token from the token list
  if (TokenListReady){
	Token current = TokenList[CurrentToken];
  CurrentToken++;
  return current;
}
  Token t;
  t.tp = ERR;
  strcpy(t.lx, "Error: Lexer not initialised or token list not ready");
  return t;
}

// peek (look) at the next token in the source file without removing it from the stream
Token PeekNextToken ()
{
  if (TokenListReady){
    Token current = TokenList[CurrentToken];
    return current;
  }
  t.tp = ERR;
  return t;
}

int InitLexer (char* file_name)
{
  strcpy(current_file, file_name);
  // printf("Current file: %s\n", current_file);
  input = fopen(file_name, "r");
  // printf("input: %p\n", input);
  if (input == NULL){
    // printf("Error: File not found\n");
    return 0;
  }
  TokenListReady = false;
  LineCount = 1;
  TokenReady = false;
  StoreTokens();
  return 1;
}
// clean out at end, e.g. close files, free memory, ... etc
int StopLexer ()
{
  if (input != NULL){
    fclose(input);
  }
  TokenListReady = false;
  t.tp = ERR;
  t.ln = -1;
  strcpy(t.lx, "");
  strcpy(t.fl, "");
  strcpy(current_file, "");
  LineCount = 0;
  CurrentToken = 0;
  TokenReady = false;
  //clear token list
  for (int i = 0; i < 10000; i++){
    TokenList[i].tp = ERR;
    TokenList[i].ln = -1;
    strcpy(TokenList[i].lx, "");
    strcpy(TokenList[i].fl, "");
  }
  //clear token
  
  return 1;
}

// do not remove the next line
#ifndef TEST
// int main()
// {
// 	// implement your main function here
//   // NOTE: the autograder will not use your main function
//   if (InitLexer("Main.jack")){
//     printf("File opened successfully\n");
//   } else {
//     printf("File not opened successfully\n");
//   }
//   // for (int i = 0; i < 1000; i++){
//   //   if (t.tp == EOFile){
//   //     printf("\n!!End of file reached!!\n");
//   //     printf("Number of Tokens: %d\n", i);
//   //     break;
//   //   }
//   //   Token lst[2];
//   //   lst[0] = t;
//   //   GetNextToken();
//   //   lst[1] = t;
//   //   printf("\nToken: %s\n", t.lx);
//   //   printf("Token Type: %d\n", t.tp);
//   //   printf("Line Number: %d\n", t.ln);
//   //}
//   StoreTokens();
//   StopLexer();
// 	return 0;
// }
// do not remove the next line
#endif
