#include "parser.h"

typedef enum TokenType {
	TOKEN_TYPE_OPERATION,
	TOKEN_TYPE_NAME,
	TOKEN_TYPE_OPEN,
	TOKEN_TYPE_CLOSE,
	TOKEN_TYPE_ASSIGNMENT,
	TOKEN_TYPE_NUMBER,
	TOKEN_TYPE_END,
	TOKEN_TYPE_CONST,
	TOKEN_TYPE_REFERENCE,
	TOKEN_TYPE_COLOR,
	TOKEN_TYPE_ACCESS,
	TOKEN_TYPE_TEXT,
} TokenType;

typedef struct Token {
	TokenType type;
	const char* str;
	uint len;
	Token* next;
}Token;

typedef enum TokenizerState {
	TOKEN_STATE_NONE,
	TOKEN_STATE_NAME,
	TOKEN_STATE_TEXT,
	TOKEN_STATE_COLOR,
	TOKEN_STATE_NUMBER,
} TokenizerState;

Token* appendToken(Token* currentToken) {
	Token* newToken = avAllocate(sizeof(Token), 1, "allocating new token");
	currentToken->next = newToken;
	return newToken;
}

bool isNumber(char chr) {
	if (chr >= '0' && chr <= '9') {
		return true;
	}
	return false;
}

bool isHexNumber(char chr) {
	if (isNumber(chr)) {
		return true;
	}
	if (chr >= 'a' && chr <= 'f') {
		return true;
	}
	if (chr >= 'A' && chr <= 'F') {
		return true;
	}
	return false;
}


bool isLowerCaseLetter(char chr) {
	if (chr >= 'a' && chr <= 'z') {
		return true;
	}
	return false;
}

bool isUpperCaseLetter(char chr) {
	if (chr >= 'A' && chr <= 'Z') {
		return true;
	}
	return false;
}

bool isLetter(char chr) {
	if (isUpperCaseLetter(chr)) {
		return true;
	}
	if (isLowerCaseLetter(chr)) {
		return true;
	}
	return false;
}

bool isNameCharacter(char chr, bool firstChar) {
	if (isUpperCaseLetter(chr)) {
		return true;
	}
	if (isLowerCaseLetter(chr)) {
		return true;
	}
	if (chr == '_') {
		return true;
	}
	if (firstChar) {
		return false;
	}
	if (isNumber(chr)) {
		return true;
	}
	return false;
}

AvResult tokenize(const char* buffer, uint64 size, Token** tokens, uint* tokenCount) {

	const char* cPtr = buffer;
	char c;
	
	TokenizerState state = TOKEN_STATE_NONE;

	Token startToken = { 0 };
	Token* currentToken = &startToken;

	for (uint i = 0; i < size; i++) {
		c = buffer[i];

		switch (c) {
		case '\n':
		case '\t':
		case ' ':
		case '\r':
			continue;
			break;
		case '/':
			if (buffer[i + 1] == '/') {
				// skip to next line
				while((c = buffer[i++]) != '/n'){
					;;
				};
				continue;
			}
			break;
		case '#':
			currentToken->str = buffer + i;
			currentToken->len = 1;
			currentToken->type = TOKEN_TYPE_OPERATION;
			currentToken = appendToken(currentToken);

			currentToken->str = buffer + ++i;
			currentToken->type = TOKEN_TYPE_NAME;

			break;
		case '(':
			break;
			
	
		}



	}

}

AvResult parseFile(const char* buffer, uint64 size) {
	Token* tokens;
	uint tokenCount;
	avAssert(tokenize(buffer, size, &tokens, &tokenCount), AV_SUCCESS, "tokenizing");




	return AV_SUCCESS;
}

AvResult avInterfaceLoadFromFile(AvInterfaceLoadFileInfo info, AvInterface* interface, const char* fileName) {
	FILE* file = fopen(fileName, "r");
	if (!file) {
		avAssert(AV_IO_ERROR, 0, "failed to open file");
	}

	fseek(file, SEEK_END, 0);
	uint64 size = ftell(file);
	fseek(file, SEEK_SET, 0);

	char* buffer = avAllocate(sizeof(char), size + 1, "allocating space for file parsing");
	uint64 readSize = fread(buffer, 1, size, file);
	if (readSize != size) {
		avAssert(AV_IO_ERROR, 0, "failed to read data");
	}

	if (fclose(file)) {
		avAssert(AV_IO_ERROR, 0, "failed to close file");
	}
	;
	avAssert(parseFile(buffer, size), AV_SUCCESS, "file parsed succesfully");
	avFree(buffer);
	return AV_SUCCESS;
}