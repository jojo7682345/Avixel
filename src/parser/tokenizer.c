#include "tokenizer.h"
#include "../core/core.h"

#include <memory.h>

typedef struct TokenLL {
	Token token;
	void* next;
}TokenLL;

TokenLL* appendToken(TokenLL* currentToken, uint lineNumber, const char* fileName) {
	currentToken->token.location.lineNumber = lineNumber;
	currentToken->token.location.file = fileName;

	TokenLL* newToken = avAllocate(sizeof(TokenLL), 1, "allocating new token");
	currentToken->next = newToken;
	return newToken;
}

void freeTokensLL(TokenLL* token) {
	TokenLL* nextToken = token->next;
	while (nextToken != nullptr) {
		avFree(token);
		token = nextToken;
		nextToken = token->next;
	}
	avFree(token);
}

void convertTokenLLtoTokenArr(TokenLL* tokenList, Token** tokens, uint* tokenCount) {

	// get the lenght of the linked list of tokens
	uint size = 0;
	TokenLL* current = tokenList;

	while (current != nullptr) {
		current = current->next;
		size += 1;
	}

	// allocate data for array
	*tokens = avAllocate(sizeof(Token), size, "allocating data for token array");

	// copy the tokens into the array
	current = tokenList;
	uint index = 0;
	while (current != nullptr) {
		memcpy((*tokens) + index, current, sizeof(Token));
		current = current->next;
		index++;
	}

	*tokenCount = size;
}

const char* tokenTypeAsString(TokenType token) {
	const char* type;
	switch (token) {
	case TOKEN_TYPE_INCLUDE:
		type = "INCLUDE";
		break;
	case TOKEN_TYPE_OPEN:
		type = "OPEN";
		break;
	case TOKEN_TYPE_CLOSE:
		type = "CLOSE";
		break;
	case TOKEN_TYPE_ASSIGNMENT:
		type = "ASSIGNMENT";
		break;
	case TOKEN_TYPE_CONST:
		type = "CONST";
		break;
	case TOKEN_TYPE_REFERENCE:
		type = "REFERENCE";
		break;
	case TOKEN_TYPE_TEXT:
		type = "TEXT";
		break;
	case TOKEN_TYPE_END:
		type = "END";
		break;
	case TOKEN_TYPE_COLOR:
		type = "COLOR";
		break;
	case TOKEN_TYPE_PROTOTYPE:
		type = "PROTOTYPE";
		break;
	case TOKEN_TYPE_NAME:
		type = "NAME";
		break;
	case TOKEN_TYPE_NUMBER:
		type = "NUMBER";
		break;
	case TOKEN_TYPE_END_OF_FILE:
		type = "END_OF_FILE";
		break;
	case TOKEN_TYPE_ACCESS:
		type = "ACCESS";
		break;
	case TOKEN_TYPE_POOL_OPEN:
		type = "POOL_OPEN";
		break;
	case TOKEN_TYPE_POOL_CLOSE:
		type = "POOL_CLOSE";
		break;
	case TOKEN_TYPE_BOOL:
		type = "BOOLEAN";
		break;
	case TOKEN_TYPE_PARAMETER:
		type = "PARAM";
		break;
	default:
		type = "UNKNOWN";
		break;
	}
	return type;
}

void printTokens(Token* tokens, uint tokenCount) {
	for (uint i = 0; i < tokenCount; i++) {
		Token token = tokens[i];
		const char* type = tokenTypeAsString(token.type);
		printf("%s: %.*s line#:%i file:%s\n", type, token.len, token.str, token.location.lineNumber, token.location.file);
	}
}

AvResult tokenize(const char* buffer, uint64 size, Token** tokens, uint* tokenCount, const char* fileName) {

	const char* cPtr = buffer;
	char c;

	TokenLocationDetails locationDetails = {};
	locationDetails.file = fileName;

	TokenLL* currentToken = avAllocate(sizeof(TokenLL), 1, "allocationg start token");
	TokenLL* tokenList = currentToken;

	uint lineNumber = 1;

	for (uint i = 0; i < size; i++) {
		c = buffer[i];

		switch (c) {
		case '\n':
		{
			lineNumber++;
		}
		case '\t':
		case ' ':
		case '\r':
			continue;
			break;
		case '/':
			if (buffer[i + 1] == '/') {
				// skip to next line
				skipToNextLine(buffer, size, &i);
				lineNumber++;
			}
			break;
		case '#':
		{
			bool isColor = false;

			for (uint j = 0; j < 6; j++) {
				if (!isHexNumber(buffer[i + j + 1])) {
					isColor = false;
					break;
				}
				isColor = true;
			}
			if (isColor) {
				uint length = 0;
				i++;
				currentToken->token.str = buffer + i;
				while (isHexNumber(buffer[i++]) && i <= size) {
					length++;
				}
				if (length == 6 || length == 8) {
					currentToken->token.len = length;
					currentToken->token.type = TOKEN_TYPE_COLOR;
					currentToken = appendToken(currentToken, lineNumber, fileName);
				} else {

					avAssert(AV_UNABLE_TO_PARSE, AV_SUCCESS, "invalid color format");
					return AV_UNABLE_TO_PARSE;
				}
				i -= 2;
				break;
			}

			currentToken->token.str = buffer + ++i;
			currentToken->token.len = 0;
			currentToken->token.type = TOKEN_TYPE_INCLUDE;
			uint length = 0;
			while (i <= size) {
				c = buffer[i++];
				if (!isLetter(c)) {
					break;
				}
				length++;
			}
			currentToken->token.len = length;
			currentToken = appendToken(currentToken, lineNumber, fileName);
			i--;
			break;
		}
		case '(':
			currentToken->token.str = buffer + i;
			currentToken->token.len = 1;
			currentToken->token.type = TOKEN_TYPE_OPEN;
			currentToken = appendToken(currentToken, lineNumber, fileName);
			break;
		case ')':
			currentToken->token.str = buffer + i;
			currentToken->token.len = 1;
			currentToken->token.type = TOKEN_TYPE_CLOSE;
			currentToken = appendToken(currentToken, lineNumber, fileName);
			break;
		case '=':
			currentToken->token.str = buffer + i;
			currentToken->token.len = 1;
			currentToken->token.type = TOKEN_TYPE_ASSIGNMENT;
			currentToken = appendToken(currentToken, lineNumber, fileName);
			break;
		case '*':
			currentToken->token.str = buffer + i;
			currentToken->token.len = 1;
			currentToken->token.type = TOKEN_TYPE_CONST;
			currentToken = appendToken(currentToken, lineNumber, fileName);
			break;
		case '$':
			currentToken->token.str = buffer + i;
			currentToken->token.len = 1;
			currentToken->token.type = TOKEN_TYPE_REFERENCE;
			currentToken = appendToken(currentToken, lineNumber, fileName);
			break;
		case '"':
			currentToken->token.str = buffer + ++i;
			currentToken->token.len = 0;
			currentToken->token.type = TOKEN_TYPE_TEXT;
			uint length = 0;
			while (i <= size) {
				c = buffer[i++];
				if (c == '"') {
					break;
				}
				if (!isTextCharacter(c)) {
					avAssert(AV_UNABLE_TO_PARSE, AV_SUCCESS, "invalid string delimiter");
				}
				length++;
			}
			currentToken->token.len = length;
			currentToken = appendToken(currentToken, lineNumber, fileName);
			i--;
			break;
		case ';':
			currentToken->token.str = buffer + i;
			currentToken->token.len = 1;
			currentToken->token.type = TOKEN_TYPE_END;
			currentToken = appendToken(currentToken, lineNumber, fileName);
			break;
		case '@':
		{
			currentToken->token.str = buffer + ++i;
			currentToken->token.len = 0;
			currentToken->token.type = TOKEN_TYPE_PROTOTYPE;
			uint length = 0;
			while (i <= size) {
				c = buffer[i++];
				if (!isNameCharacter(c)) {
					break;
				}
				length++;
			}
			currentToken->token.len = length;
			currentToken = appendToken(currentToken, lineNumber, fileName);
			i--;
			break;
		}
		case '.':
			currentToken->token.str = buffer + i;
			currentToken->token.len = 1;
			currentToken->token.type = TOKEN_TYPE_ACCESS;
			currentToken = appendToken(currentToken, lineNumber, fileName);
			break;
		case '[':
			currentToken->token.str = buffer + i;
			currentToken->token.len = 1;
			currentToken->token.type = TOKEN_TYPE_POOL_OPEN;
			currentToken = appendToken(currentToken, lineNumber, fileName);
			break;
		case ']':
			currentToken->token.str = buffer + i;
			currentToken->token.len = 1;
			currentToken->token.type = TOKEN_TYPE_POOL_CLOSE;
			currentToken = appendToken(currentToken, lineNumber, fileName);
			break;
		default:
			if (isDecNumber(c)) {
				currentToken->token.str = buffer + i;
				currentToken->token.len = 0;
				currentToken->token.type = TOKEN_TYPE_NUMBER;
				while (isDecNumber(buffer[i++]) && i <= size) {
					currentToken->token.len++;
				}
				i -= 2;
				currentToken = appendToken(currentToken, lineNumber, fileName);
			} else if (isNameCharacter(c)) {
				currentToken->token.str = buffer + i;
				currentToken->token.len = 0;
				currentToken->token.type = TOKEN_TYPE_NAME;
				while (isNameCharacter(buffer[i++]) && i <= size) {
					currentToken->token.len++;
				}
				i -= 2;

				//check if token is boolean
				if (isBool(currentToken->token.str, currentToken->token.len)) {
					currentToken->token.type = TOKEN_TYPE_BOOL;
				}
				if (isParam(currentToken->token.str, currentToken->token.len)) {
					currentToken->token.type = TOKEN_TYPE_PARAMETER;
				}
				currentToken = appendToken(currentToken, lineNumber, fileName);
			} else {
				char errorMessage[64];
				sprintf(errorMessage, "invalid character '%c'", c);
				avAssert(AV_UNABLE_TO_PARSE, AV_SUCCESS, errorMessage);
				return AV_UNABLE_TO_PARSE;
			}
			break;

		}

	}

	convertTokenLLtoTokenArr(tokenList, tokens, tokenCount);
	freeTokensLL(tokenList);


	return AV_SUCCESS;
};