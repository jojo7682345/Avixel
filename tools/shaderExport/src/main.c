#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif

char* shift_args(int* argc, char*** argv) {
	if (*argc <= 0) {
		printf("Not enough arguments specified\n");
		exit(-1);
	}
	char* result = **argv;
	*argc -= 1;
	*argv += 1;
	return result;
}

unsigned long long hash(unsigned char* str) {
	unsigned long long hash = 5381;
	int c;

	while (c = *str++)
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

int main(int argC, char* argV[]) {

	const char* program = shift_args(&argC, &argV);

	char* outFile = 0;
	char* inFile = 0;
	while (1) {
		char* argument = shift_args(&argC, &argV);
		if (strcmp(argument, "-c") == 0) {
			continue;
		}
		if (strcmp(argument, "-o") == 0) {
			outFile = shift_args(&argC, &argV);
			continue;
		}
		inFile = argument;
		if (inFile && outFile) {
			break;
		}
	}

	printf("Generating header file for %s\n", inFile);

	FILE* compiled = fopen(inFile, "rb");
	if (!compiled) {
		printf("Could not open file %s!\n", inFile);
		return -1;
	}
	fseek(compiled, 0, SEEK_END);
	size_t size = ftell(compiled);
	fseek(compiled, 0, SEEK_SET);

	unsigned char* buffer = (unsigned char*)malloc(size + 1);
	if (!buffer) {
		printf("out of mem\n");
		return -1;
	}
	memset(buffer, 0, size + 1);
	if (fread(buffer, sizeof(char), size, compiled) != size) {
		printf("unable to read entire file!\n");
		return -1;
	}

	if (fclose(compiled)) {
		printf("failed to close file\n");
		return -1;
	}


	FILE* file = fopen(outFile, "w");

	if (!file) {
		printf("unable to open file %s!\n", outFile);
	}

	char* shaderName;
	int shaderNameLength = 0;

	{
		char* end = outFile + strlen(outFile);
		int isName = 0;
		while (end != outFile) {
			if (isName) {
				shaderNameLength++;
			}
			if (end[0] == PATH_SEP) {
				shaderName = end + 1;
				break;
			}
			if (end[0] == '.') {
				isName = 1;
			}
			end--;
			shaderName = end;
		}
		if (!isName) {
			printf("invalid file\n");
			return -1;
		}
		shaderNameLength--;
	}

	char* nameBuffer = (char*)malloc(shaderNameLength + 1);
	if (!nameBuffer) {
		printf("out of mem\n");
		return -1;
	}
	memset(nameBuffer, 0, shaderNameLength + 1);
	memcpy(nameBuffer, shaderName, shaderNameLength);
	shaderName = nameBuffer;

	unsigned long long fileHash = hash(inFile);
	fileHash = fileHash << 32;
	fileHash += hash(outFile);

	fprintf(file, "#ifndef __%llX_GUARD__\n#define __%llX_GUARD__\n#else\n#ifdef CHECK_HEADER_GUARD_COLLISIONS\n#error \"HEADER_COLLISION_DETECTED\"\n#endif\n#endif\n\n//THIS FILE IS AUTOMATICALLY GENERATED - DO NOT EDIT\n\n", fileHash, fileHash);
	fprintf(file, "const char %s_data[] = {", shaderName);
	for (int i = 0; i < size; i++) {
		if (i % 16 == 0) {
			fprintf(file, "\n\t");
		} else if(i % 8 == 0) {
			fprintf(file, " ");
		}
		fprintf(file, "0x%02x, ", buffer[i]);
	}
	fprintf(file, "\n};\n\n");
	fprintf(file, "const unsigned long long int %s_size = sizeof(%s_data)/sizeof(char);\n", shaderName, shaderName);
	fprintf(file, "\n#endif //__%llX_GUARD__\n", fileHash);
	fclose(file);

	printf("Generation successfull!\n");

	free(buffer);
	return 0;
}