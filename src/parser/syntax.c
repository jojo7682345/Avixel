#include "syntax.h"
#include <stdarg.h>
#include <stdio.h>
#include <memory.h>

typedef struct SyntaxCheckEntry {
	TokenType type;
	Token* ptr;
}SyntaxCheckEntry;


void syntaxError(TokenType expectedType, Token token) {

	char msgOut[512] = {};

	sprintf(
		msgOut, 
		"syntax error at line %i, %s expected but %s provided", 
		token.location.lineNumber, 
		tokenTypeAsString(expectedType), 
		tokenTypeAsString(token.type)
	);

	avAssert_(AV_INVALID_SYNTAX, 0, token.location.lineNumber, token.location.file, "parsing", AV_LOG_CATEGORY, msgOut);
}

bool getSyntax_(uint tokenCount, Token* tokens, uint* index, ...) {

	DynamicArray parameters;
	dynamicArrayCreate(sizeof(SyntaxCheckEntry), &parameters);

	{
		va_list format;
		va_start(format, index);

		TokenType tokenType;
		while ((tokenType = va_arg(format, TokenType)) != TOKEN_TYPE_UNKNOWN) {
			if (tokenType != TOKEN_TYPE_UNKNOWN) {

				Token* ptr = va_arg(format, Token*);

				SyntaxCheckEntry entry = { 0 };
				entry.type = tokenType;
				entry.ptr = ptr;

				dynamicArrayAdd(&entry, parameters);
			}
		}
		va_end(format);
	}

	uint entryCount = dynamicArrayGetSize(parameters);
	for (uint i = 0; i < entryCount; i++) {
		SyntaxCheckEntry entry;
		dynamicArrayGet(&entry, i, parameters);

		TokenType type = entry.type;
		Token* ptr = entry.ptr;

		if (*index >= tokenCount) {
			avAssert(AV_INVALID_SYNTAX, 0, "unexpected end of file");
			return false;
		}

		Token currentToken = tokens[*index];

		if (!(currentToken.type & type)) {
			syntaxError(type, currentToken);
			return false;
		}

		if (ptr != nullptr) {
			memcpy(ptr, &currentToken, sizeof(Token));
		}

		(*index)++;
	}

	dynamicArrayDestroy(parameters);
	return true;
}

#define getSyntax(tokenCount, tokens, index, ...) getSyntax_(tokenCount, tokens, index, __VA_ARGS__, TOKEN_TYPE_UNKNOWN)

AvResult buildIncludeSyntax(uint tokenCount, Token* tokens, DynamicArray rootNodes, uint* index) {

	Token text;
	bool valid = getSyntax(tokenCount, tokens, index,
		TOKEN_TYPE_INCLUDE, nullptr,
		TOKEN_TYPE_TEXT, &text
	);
	if (!valid) {
		return AV_INVALID_SYNTAX;
	}

	// TODO: handle include

	return AV_SUCCESS;
}

AvResult buildParameterSyntax(uint tokenCount, Token* tokens, DynamicArray rootNodes, uint* index) {
	Token param;
	Token name;
	Token value;

	bool valid = getSyntax(tokenCount, tokens, index,
		TOKEN_TYPE_PARAMETER, &param,
		TOKEN_TYPE_NAME, &name,
		TOKEN_TYPE_ASSIGNMENT, nullptr,
		TOKEN_TYPE_TEXT | TOKEN_TYPE_COLOR | TOKEN_TYPE_BOOL | TOKEN_TYPE_NUMBER | TOKEN_TYPE_NAME, &value,
		TOKEN_TYPE_END, nullptr
	);

	// TODO: add case for no argument specified

	if (!valid) {
		return AV_INVALID_SYNTAX;
	}

	// TODO: handle parameter

	return AV_SUCCESS;
}

AvResult buildComponentSyntax(uint tokenCount, Token* tokens, DynamicArray rootNodes, uint* index) {

	// TODO: implement
	return AV_INVALID_SYNTAX;


	return AV_SUCCESS;
}

AvResult buildPrototypeSyntax(uint tokenCount, Token* tokens, DynamicArray rootNodes, uint* index) {

	// TODO: implement
	return AV_INVALID_SYNTAX;

	return AV_SUCCESS;
}

AvResult buildSyntaxTree(uint tokenCount, Token* tokens, DynamicArray rootNodes) {

	for (uint index = 0; index < tokenCount;) {
		AvResult result;
		switch (tokens[index].type) {
		case TOKEN_TYPE_INCLUDE:
			result = buildIncludeSyntax(tokenCount, tokens, rootNodes, &index);
			if (result) {
				avAssert(
					result,
					AV_SUCCESS,
					"invalid operation"
				);
			}
			break;
		case TOKEN_TYPE_PARAMETER:
			result = buildParameterSyntax(tokenCount, tokens, rootNodes, &index);
			if (result) {
				avAssert(
					result,
					AV_SUCCESS,
					"invalid parameter"
				);
			}
			break;
		case TOKEN_TYPE_NAME:
			result = buildComponentSyntax(tokenCount, tokens, rootNodes, &index);
			if (result) {
				avAssert(
					result,
					AV_SUCCESS,
					"invalid component"
				);
			}
			break;
		case TOKEN_TYPE_PROTOTYPE:
			result = buildPrototypeSyntax(tokenCount, tokens, rootNodes, &index);
			if (result) {
				avAssert(
					result,
					AV_SUCCESS,
					"invalid prototype"
				);
			}
			break;
		default:
			avAssert(AV_UNABLE_TO_PARSE, 0, "Invalid top level token found while building syntax tree");
			return AV_UNABLE_TO_PARSE;
		}
		if (result != AV_SUCCESS) {
			return AV_UNABLE_TO_PARSE;
		}

	}

	return AV_SUCCESS;
}
