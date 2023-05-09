#include "parser.h"

typedef enum TokenType {
	TOKEN_TYPE_END_OF_FILE,
	TOKEN_TYPE_OPERATION, //
	TOKEN_TYPE_NAME, // 
	TOKEN_TYPE_OPEN, //
	TOKEN_TYPE_CLOSE, //
	TOKEN_TYPE_ASSIGNMENT, //
	TOKEN_TYPE_NUMBER, //
	TOKEN_TYPE_END, //
	TOKEN_TYPE_CONST, //
	TOKEN_TYPE_REFERENCE, //
	TOKEN_TYPE_COLOR, //
	TOKEN_TYPE_ACCESS, //
	TOKEN_TYPE_TEXT, //
	TOKEN_TYPE_PROTOTYPE, //
	TOKEN_TYPE_POOL_OPEN,
	TOKEN_TYPE_POOL_CLOSE,
} TokenType;

typedef struct Token {
	TokenType type;
	const char* str;
	uint len;
	void* next;
}Token;

Token* appendToken(Token* currentToken) {
	Token* newToken = avAllocate(sizeof(Token), 1, "allocating new token");
	currentToken->next = newToken;
	return newToken;
}

void freeTokens(Token* token) {
	Token* nextToken = token->next;
	while (nextToken != NULL) {
		avFree(token);
		token = nextToken;
		nextToken = token->next;
	}
	avFree(token);
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

	if (chr >= '0' && chr <= '9') {
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

bool isNameCharacter(char chr) {
	if (isUpperCaseLetter(chr)) {
		return true;
	}
	if (isLowerCaseLetter(chr)) {
		return true;
	}
	if (chr == '_') {
		return true;
	}
	if (isNumber(chr)) {
		return true;
	}
	return false;
}

bool isTextCharacter(char chr) {
	if (chr == '\n') {
		return false;
	}
	if (chr == '\r') {
		return false;
	}
	return true;
}

AvResult tokenize(const char* buffer, uint64 size, Token** tokens, uint* tokenCount) {

	const char* cPtr = buffer;
	char c;

	Token* currentToken = avAllocate(sizeof(Token), 1, "allocationg start token");
	*tokens = currentToken;

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
				while ((c = buffer[i++]) != '\n' && i <= size) {
					;;
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
				currentToken->str = buffer + i;
				while (isHexNumber(buffer[i++]) && i <= size) {
					length++;
				}
				if (length == 6 || length == 8) {
					currentToken->len = length;
					currentToken->type = TOKEN_TYPE_COLOR;
					currentToken = appendToken(currentToken);
				} else {
					avAssert(AV_PARSE_ERROR, AV_SUCCESS, "invalid color format");
					return AV_PARSE_ERROR;
				}
				i-=2;
			} else {
				currentToken->str = buffer + i;				// Looks current token
				currentToken->len = 1;						// Sets the length of the token
				currentToken->type = TOKEN_TYPE_OPERATION;	// Sets the type of the token
				currentToken = appendToken(currentToken);   // Appends the token to the list
			}
			currentToken->len = 1;
			currentToken->type = TOKEN_TYPE_PREPROCESSOR;
			currentToken = appendToken(currentToken);

			currentToken->str = buffer + ++i;
			currentToken->type = TOKEN_TYPE_NAME;

			break;
		}
		case '(':
			currentToken->str = buffer + i;
			currentToken->len = 1;
			currentToken->type = TOKEN_TYPE_OPEN;
			currentToken = appendToken(currentToken);
			break;
		case ')':
			currentToken->str = buffer + i;
			currentToken->len = 1;
			currentToken->type = TOKEN_TYPE_CLOSE;
			currentToken = appendToken(currentToken);
			break;
		case '=':
			currentToken->str = buffer + i;
			currentToken->len = 1;
			currentToken->type = TOKEN_TYPE_ASSIGNMENT;
			currentToken = appendToken(currentToken);
			break;
		case '*':
			currentToken->str = buffer + i;
			currentToken->len = 1;
			currentToken->type = TOKEN_TYPE_CONST;
			currentToken = appendToken(currentToken);
			break;
		case '$':
			currentToken->str = buffer + i;
			currentToken->len = 1;
			currentToken->type = TOKEN_TYPE_REFERENCE;
			currentToken = appendToken(currentToken);
			break;
		case '"':
			currentToken->str = buffer + i + 1;
			currentToken->len = 0;
			currentToken->type = TOKEN_TYPE_TEXT;
			uint length = 0;
			i++;
			while (i <= size) {
				c = buffer[i++];
				if (c == '"') {
					break;
				}
				if (!isTextCharacter(c)){
					avAssert(AV_PARSE_ERROR, AV_SUCCESS, "invalid string delimiter");
				}
				length++;
			}
			currentToken->len = length;
			currentToken = appendToken(currentToken);
			i --;
			break;
		case ';':
			currentToken->str = buffer + i;
			currentToken->len = 1;
			currentToken->type = TOKEN_TYPE_END;
			currentToken = appendToken(currentToken);
			break;
		case '@':
			currentToken->str = buffer + i;
			currentToken->len = 1;
			currentToken->type = TOKEN_TYPE_PROTOTYPE;
			currentToken = appendToken(currentToken);
			break;
		case '.':
			currentToken->str = buffer + i;
			currentToken->len = 1;
			currentToken->type = TOKEN_TYPE_ACCESS;
			currentToken = appendToken(currentToken);
			break;
		case '[':
			currentToken->str = buffer + i;
			currentToken->len = 1;
			currentToken->type = TOKEN_TYPE_POOL_OPEN;
			currentToken = appendToken(currentToken);
			break;
		case ']':
			currentToken->str = buffer + i;
			currentToken->len = 1;
			currentToken->type = TOKEN_TYPE_POOL_CLOSE;
			currentToken = appendToken(currentToken);
			break;
		default:
			if (isNumber(c)) {
				currentToken->str = buffer + i;
				currentToken->len = 0;
				currentToken->type = TOKEN_TYPE_NUMBER;
				while (isNumber(buffer[i++]) && i <= size) {
					currentToken->len++;
				}
				i-=2;
				currentToken = appendToken(currentToken);
			} else if (isNameCharacter(c)) {
				currentToken->str = buffer + i;
				currentToken->len = 0;
				currentToken->type = TOKEN_TYPE_NAME;
				while (isNameCharacter(buffer[i++]) && i <= size) {
					currentToken->len++;
				}
				i-=2;
				currentToken = appendToken(currentToken);
			} else {
				char errorMessage[64];
				sprintf(errorMessage, "invalid character '%c'", c);
				avAssert(AV_PARSE_ERROR, AV_SUCCESS, errorMessage);
				return AV_PARSE_ERROR;
			}
			break;

		}

	}
	return AV_SUCCESS;
};

void testTokens(const char* buffer, uint64 size, Token* token) {
	// TODO: write test function for checking if tokanization is correct

}

void printTokens(Token* token) {
	while (token) {
		const char* type;
		switch (token->type) {
			case TOKEN_TYPE_OPERATION:
				type = "OPERATION";
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
			default:
				type = "UNKNOWN";
				break;
		}
		printf("%s: %.*s\n", type, token->len, token->str);
		token = token->next;
	}
}

AvResult parseFile(const char* buffer, uint64 size) {
	Token* tokens;
	uint tokenCount;
	avAssert(tokenize(buffer, size, &tokens, &tokenCount), AV_SUCCESS, "tokenizing");
	printTokens(tokens);
	testTokens(buffer, size, tokens);
	// TODO: parse syntax
	// TODO: preprocessor
	freeTokens(tokens);

	return AV_SUCCESS;
}

AvResult avInterfaceLoadFromFile(AvInterfaceLoadFileInfo info, AvInterface* interface, const char* fileName) {
	FILE* file = fopen(fileName, "rb");
	if (!file) {
		avAssert(AV_IO_ERROR, 0, "failed to open file");
	}

	fseek(file, 0, SEEK_END);
	uint64 size = ftell(file);
	fseek(file, 0, SEEK_SET);

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

