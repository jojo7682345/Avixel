#pragma once
#include "../core/core.h"

typedef enum TokenType {
	TOKEN_TYPE_END_OF_FILE		= 0,
	TOKEN_TYPE_INCLUDE			= 1 << 0,
	TOKEN_TYPE_NAME				= 1 << 1,
	TOKEN_TYPE_OPEN				= 1 << 2,
	TOKEN_TYPE_CLOSE			= 1 << 3,
	TOKEN_TYPE_ASSIGNMENT		= 1 << 4,
	TOKEN_TYPE_NUMBER			= 1 << 5,
	TOKEN_TYPE_BOOL				= 1 << 6,
	TOKEN_TYPE_END				= 1 << 7,
	TOKEN_TYPE_CONST			= 1 << 8,
	TOKEN_TYPE_REFERENCE		= 1 << 9,
	TOKEN_TYPE_COLOR			= 1 << 10,
	TOKEN_TYPE_ACCESS			= 1 << 11,
	TOKEN_TYPE_TEXT				= 1 << 12,
	TOKEN_TYPE_PROTOTYPE		= 1 << 13,
	TOKEN_TYPE_POOL_OPEN		= 1 << 14,
	TOKEN_TYPE_POOL_CLOSE		= 1 << 15,
	TOKEN_TYPE_PARAMETER		= 1 << 16,
} TokenType;

typedef struct Token {
	TokenType type;
	const char* str;
	uint len;
} Token;

void printTokens(Token* tokens, uint tokenCount);

AvResult tokenize(const char* buffer, uint64 size, Token** tokens, uint* tokenCount);
