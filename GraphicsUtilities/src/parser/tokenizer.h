#pragma once
#include "../core/core.h"

typedef enum TokenType {
	TOKEN_TYPE_END_OF_FILE,
	TOKEN_TYPE_OPERATION, //
	TOKEN_TYPE_NAME, // 
	TOKEN_TYPE_OPEN, //
	TOKEN_TYPE_CLOSE, //
	TOKEN_TYPE_ASSIGNMENT, //
	TOKEN_TYPE_NUMBER, //
	TOKEN_TYPE_BOOL,
	TOKEN_TYPE_END, //
	TOKEN_TYPE_CONST, //
	TOKEN_TYPE_REFERENCE, //
	TOKEN_TYPE_COLOR, //
	TOKEN_TYPE_ACCESS, //
	TOKEN_TYPE_TEXT, //
	TOKEN_TYPE_PROTOTYPE, //
	TOKEN_TYPE_POOL_OPEN,
	TOKEN_TYPE_POOL_CLOSE,
	TOKEN_TYPE_PARAMETER,
} TokenType;

typedef struct Token {
	TokenType type;
	const char* str;
	uint len;
} Token;

void printTokens(Token* tokens, uint tokenCount);

AvResult tokenize(const char* buffer, uint64 size, Token** tokens, uint* tokenCount);
