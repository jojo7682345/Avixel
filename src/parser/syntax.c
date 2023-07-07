#include "syntax.h"
#include <stdarg.h>

bool getSyntax_(uint tokenCount, Token* tokens, uint* index, ...) {

	va_list format;
	va_start(format, index);
	TokenType token;
	while ((token = va_arg(format, TokenType)) != TOKEN_TYPE_UNKNOWN) {




	}

	va_end(format);
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
		return AV_SYNTAX_ERROR;
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

	if (!valid) {
		return AV_SYNTAX_ERROR;
	}

	// TODO: handle parameter

	return AV_SUCCESS;
}

AvResult buildComponentSyntax(uint tokenCount, Token* tokens, DynamicArray rootNodes, uint* index) {

	// TODO: implement

	return AV_SUCCESS;
}

AvResult buildPrototypeSyntax(uint tokenCount, Token* tokens, DynamicArray rootNodes, uint* index) {

	// TODO: implement

	return AV_SUCCESS;
}

AvResult buildSyntaxTree(uint tokenCount, Token* tokens, DynamicArray rootNodes) {

	for (uint index = 0; index < tokenCount; index++) {

		switch (tokens[index].type) {
		case TOKEN_TYPE_INCLUDE:
			avAssert(
				buildIncludeSyntax(tokenCount, tokens, rootNodes, &index),
				AV_SUCCESS,
				"invalid operation"
			);
			break;
		case TOKEN_TYPE_PARAMETER:
			avAssert(
				buildParameterSyntax(tokenCount, tokens, rootNodes, &index),
				AV_SUCCESS,
				"invalid parameter"
			);
			break;
		case TOKEN_TYPE_NAME:
			avAssert(
				buildComponentSyntax(tokenCount, tokens, rootNodes, &index),
				AV_SUCCESS,
				"invalid component"
			);
			break;
		case TOKEN_TYPE_PROTOTYPE:
			avAssert(
				buildPrototypeSyntax(tokenCount, tokens, rootNodes, &index),
				AV_SUCCESS,
				"invalid prototype"
			);
			break;
		default:
			avAssert(AV_PARSE_ERROR, 0, "Invalid top level token found while building syntax tree");
			return AV_PARSE_ERROR;
		}

	}

	return AV_SUCCESS;
}
