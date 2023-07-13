#include "parser.h"
#include "tokenizer.h"
#include "syntax.h"
#include <stdio.h>


AvResult parseFile(const char* buffer, uint64 size, const char* fileName) {
	Token* tokens;
	uint tokenCount;
	DynamicArray syntaxTree;
	dynamicArrayCreate(sizeof(SyntaxTreeNode), &syntaxTree);


	avAssert(tokenize(buffer, size, &tokens, &tokenCount, fileName), AV_SUCCESS, "tokenizing");
	avAssert(buildSyntaxTree(tokenCount, tokens, syntaxTree), AV_SUCCESS, "generating syntax tree");
	

	// TODO: preprocessor


<<<<<<< HEAD
	dynamicArrayDestroy(syntaxTree);
	avFree(tokens);
=======
AvResult parseFile(const char* buffer, uint64 size) {
	Token* tokens = nullptr;
	uint tokenCount = 0;
	avAssert(tokenize(buffer, size, &tokens, &tokenCount), AV_SUCCESS, "tokenizing");
	//printTokens(tokens);
	testTokens(buffer, size, tokens);
	// TODO: parse syntax
	// TODO: preprocessor
	freeTokens(tokens);
>>>>>>> c9aa9d7bd7700d3ffa37d96e13c9d8b74b491b16

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
	avAssert(parseFile(buffer, size, fileName), AV_SUCCESS, "file parsed succesfully");
	avFree(buffer);
	return AV_SUCCESS;
}

