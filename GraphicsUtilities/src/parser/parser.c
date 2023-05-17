#include "parser.h"
#include "tokenizer.h"

AvResult parseFile(const char* buffer, uint64 size) {
	Token* tokens;
	uint tokenCount;
	avAssert(tokenize(buffer, size, &tokens, &tokenCount), AV_SUCCESS, "tokenizing");
	printTokens(tokens, tokenCount);
	// TODO: parse syntax
	// TODO: preprocessor
	avFree(tokens);

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

