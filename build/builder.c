#define NOBUILD_IMPLEMENTATION
#include "builder.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define COMPILER "gcc"
#define ARCHIVER "ar"
#define LIB_FLAGS "rcs"

#ifdef _WIN32
#define DLL_EXTENSION ".dll"
#define LIB_PREFIX ""
#define LIB_EXTENSION ".a"
#define EXE_EXTENSION ".exe"
#define COPY_COMMAND(src, dst) "xcopy",src,dst,"/E /I /y"
#else
#define DLL_EXTENSION ".so"
#define LIB_PREFIX "lib"
#define LIB_EXTENSION ".a"
#define EXE_EXTENSION ""
#define COPY_COMMAND(src, dst) "cp","-r",src,dst
#endif

#define PROJECT_FILE ".project"

typedef enum ProjectType {
	PROJECT_TYPE_LIB,
	PROJECT_TYPE_EXE,
	PROJECT_TYPE_DLL,
	PROJECT_TYPE_CUSTOM,
	PROJECT_TYPE_UNKNOWN,
}ProjectType;

typedef enum OperationType {
	OPERATION_TYPE_BUILD,
	OPERATION_TYPE_CLEAN,
	OPERATION_TYPE_REBUILD,
	OPERATION_TYPE_CONFIGURE,
	OPERATION_TYPE_INSTALL,
	OPERATION_TYPE_UNKNOWN,
}OperationType;

const char* toUppercase(const char* str) {
	size_t len = strlen(str);
	char* buffer = malloc(len + 1);
	memcpy(buffer, str, len);
	buffer[len] = '\0';

	const int offset = 'A' - 'a';

	for (size_t i = 0; i < len; i++) {
		char c = str[i];
		if (c >= 'a' && c <= 'z') {
			buffer[i] = c + offset;
		} else {
			buffer[i] = c;
		}
	}

	buffer[len] = '\0';

	return buffer;
}

ProjectType getProjectType(const char* type) {
	const char* str = toUppercase(type);
	if (strcmp(str, "LIB") == 0) {
		return PROJECT_TYPE_LIB;
	}
	if (strcmp(str, "EXE") == 0) {
		return PROJECT_TYPE_EXE;
	}
	if (strcmp(str, "DLL") == 0) {
		return PROJECT_TYPE_DLL;
	}
	if (strcmp(str, "CUSTOM") == 0) {
		return PROJECT_TYPE_CUSTOM;
	}
	return PROJECT_TYPE_UNKNOWN;
}

OperationType getOperationType(const char* type) {
	const char* str = toUppercase(type);
	if (strcmp(str, "BUILD") == 0) {
		return OPERATION_TYPE_BUILD;
	}
	if (strcmp(str, "CLEAN") == 0) {
		return OPERATION_TYPE_CLEAN;
	}
	if (strcmp(str, "REBUILD") == 0) {
		return OPERATION_TYPE_REBUILD;
	}
	if (strcmp(str, "CONFIGURE") == 0) {
		return OPERATION_TYPE_CONFIGURE;
	}
	if(strcmp(str,"INSTALL")==0){
		return OPERATION_TYPE_INSTALL;	
	}
	return OPERATION_TYPE_UNKNOWN;
}

void printUse(const char* program) {
	printf("usage: %s [PROJECT] [CONFIGURATION] [...]\n", NOEXT(program));
}

int isValidDir(const char* dir) {
	if (strcmp(dir, ".") == 0) {
		return 0;
	}
	if (strcmp(dir, "..") == 0) {
		return 0;
	}
	return 1;
}

typedef struct Project {
	char name[64];
	ProjectType type;
	int sourceCount;
	char** sources;
	char flags[512];
	int s_libCount;
	char** s_libs;
	int includeCount;
	char** include;
	int libdirCount;
	char** libdir;
	char compiler[64];
	int exportCount;
	char** exports;

	char outDir[512];
	char outType[64];
	int fileTypeCount;
	char** fileTypes;

	void* next;
} Project;


const char* subsitute(const char* str, const char* var, int begin, int end) {
	int length = end - begin;
	int varLength = strlen(var);
	int orgLength = strlen(str);
	int newLength = varLength + orgLength - length;

	char* buffer = (char*)malloc(newLength + 1);
	memcpy(buffer, str, begin);
	memcpy(buffer + begin, var, varLength);
	memcpy(buffer + begin + varLength, str + end + 1, orgLength - end);

	return buffer;
}

const char* getCustomEnv(const char* env, Project project) {

	if (strcmp(env, "OUTPUT") == 0) {
		const char* output;
		switch (project.type) {
		case PROJECT_TYPE_LIB:
			output = PATH("lib", CONCAT(LIB_PREFIX, project.name, LIB_EXTENSION));
			break;
		case PROJECT_TYPE_DLL:
			output = PATH("lib", CONCAT(LIB_PREFIX, project.name, DLL_EXTENSION));
			break;
		case PROJECT_TYPE_EXE:
			output = PATH("bin", CONCAT(project.name, EXE_EXTENSION));
			break;
		case PROJECT_TYPE_UNKNOWN:
			PANIC("invalid project type");
			return NULL;
		}
		return output;
	}


	return getenv(env);

}

const char* extractEnv(const char* str, Project project) {
	for (int i = 0; i < strlen(str); i++) {
		char c = str[i];
		if (c != '$') {
			continue;
		}
		if (i + 2 >= strlen(str)) {
			continue;
		}
		if (str[i + 1] != '(') {
			continue;
		}
		int preStart = i;
		int start = i + 2;
		i += 2;
		while ((c = str[i] != ')') && i++ < strlen(str)) {
		}
		int end = i;
		int length = end - start;
		char* env = (char*)malloc(length + 1);
		memset(env, 0, length + 1);
		memcpy(env, str + start, length);
		const char* envVar = getCustomEnv(env, project);
		free(env);
		if (!envVar) {
			envVar = "";
		}
		const char* sub = subsitute(str, envVar, preStart, end);
		const char* extracted = extractEnv(sub,project);
		return extracted;
	}
	return str;
}

Cstr_Array splitString(const char* str, char del){
	int len = strlen(str);
	int start = 0;
	int end = 0;

	Cstr_Array array = { 0 };
	
	for(int i = 0; i < len; i++){
		char c = str[i];
		end = i;
		if(c==del){
			int length = end - start;
			char* buffer= malloc(length+1);
			memcpy(buffer,str+start,length);
			buffer[length] = '\0';
			array = cstr_array_append(array,buffer);
			start = i+1;
		}
		
	}
	end++;
	int length = end - start;
	if(length){
		char* buffer= malloc(length+1);
		memcpy(buffer,str+start,length);
		buffer[length] = '\0';
		array = cstr_array_append(array,buffer);
	}

	return array;
}

int filesEqual(const char* newFile, const char* original) {

	FILE* nf = fopen(newFile, "rb");
	FILE* org = fopen(original, "rb");
	if (!nf) {
		PANIC("Unable to open file %s\n", newFile);
	}
	if (!original) {
		return 0;
	}
	fseek(nf, 0, SEEK_END);
	fseek(org, 0, SEEK_END);
	size_t size1 = ftell(nf);
	size_t size2 = ftell(org);
	fseek(nf, 0, SEEK_SET);
	fseek(org, 0, SEEK_SET);
	if (size1 != size2) {
		fclose(nf);
		fclose(org);
		return 0;
	}

	char* buffer1 = (char*)malloc(size1);
	char* buffer2 = (char*)malloc(size2);

	if (fread(buffer1, 1, size1, nf) != size1 || 
		fread(buffer2, 1, size1, org) != size1) {
		fclose(nf);
		fclose(org);
		PANIC("unable to read both files entirly");
		return 0;
	}
	fclose(nf);
	fclose(org);

	int equals =  memcmp(buffer1, buffer2, size1) == 0;
	
	free(buffer1);
	free(buffer2);

	return equals;

}

int copyFile(const char* source, const char* destination) {
	char ch = '\0';
	FILE* fs = NULL;
	FILE* ft = NULL;
	fs = fopen(source, "rb");
	if (fs == NULL) {
		PANIC("Error in Opening the file, %s\n", source);
		return 0;
	}

	fseek(fs, 0, SEEK_END);
	size_t size = ftell(fs);
	fseek(fs, 0, SEEK_SET);

	unsigned char* buffer = malloc(size);
	fread(buffer, 1, size, fs);

	ft = fopen(destination, "wb");
	if (ft == NULL) {
		PANIC("Error in Opening the file, %s\n", destination);
		return 0;
	}
	fwrite(buffer, 1, size, ft);
	free(buffer);
	printf("File copied successfully.\n");
	fclose(fs);
	fclose(ft);

#ifndef _WIN32
	struct stat statRes;
	if (stat(source, &statRes) < 0) {
		PANIC("Failed to get the privileges of the source file");
	}
	chmod(destination, statRes.st_mode);
#endif


	return 0;
}

void compileCodeFile(const char* source, char* file, const char* imBuild, const char*** compiledFiles, size_t* compiledCount, int includeCount, const char** include, const char* compiler, const char* flags, const char* outType) {

	const char* outFile = PATH(imBuild, CONCAT(NOEXT(file), outType));
	
	Cstr_Array args = { 0 };
	args = cstr_array_append(args, compiler);
	if (flags[0] != '\0') {
		args = cstr_array_concat(args, splitString(flags, ' '));
	}
	Cstr_Array includes = { .count = includeCount,.elems = include };
	args = cstr_array_concat(args, includes);
	args = cstr_array_append(args, "-MD");
	args = cstr_array_append(args, "-c");
	args = cstr_array_append(args, "-o");
	args = cstr_array_append(args, outFile);
	args = cstr_array_append(args, PATH(source, file));
	CMD_ARR(args);

	void* data = realloc(*compiledFiles, sizeof(const char*) * (*compiledCount + 1));
	if (!data) {
		PANIC("mem");
	}
	*compiledFiles = data;
	(*compiledFiles)[*compiledCount] = outFile;
	*compiledCount += 1;
}

void compile(const char* source, const char* immediate, const char*** compiledFiles, size_t* compiledCount,int includeCount, const char** include, const char* compiler, const char* flags, int fileTypeCount, const char** fileTypes, const char* outType) {
	Cstr imBuild = PATH(immediate);
	if (!PATH_EXISTS(imBuild)) {
		mkdir_p(imBuild);
	}

	FOREACH_FILE_IN_DIR(file, source, {
		int isValidType = 0;
		for (int i = 0; i < fileTypeCount; i++) {
			if (ENDS_WITH(file, fileTypes[i])) {
				isValidType = 1;
				break;
			}
		}
	
		if (isValidType) {

			compileCodeFile(source,(char*)file,imBuild,compiledFiles,compiledCount,includeCount, include,compiler,flags, outType);
		} else {
			if (IS_DIR(PATH(source,file))) {
				if (isValidDir(file)) {
					compile(PATH(source, file), PATH(immediate, file), compiledFiles, compiledCount, includeCount, include, compiler,flags, fileTypeCount, fileTypes, outType);
				}
			}
		}
	});
}

const char* linker(const char** compiledFiles, size_t compiledCount, Project project) {

	switch (project.type) {
	case PROJECT_TYPE_EXE:
	{
		

		if (!PATH_EXISTS("bin")) {
			MKDIRS("bin");
		}

		const char* output = CONCAT(project.name, EXE_EXTENSION);

		Cstr_Array args = {0};
		args = cstr_array_append(args, project.compiler);
		if (project.flags[0] != '\0') {
			args = cstr_array_concat(args, splitString(project.flags, ' '));
		}
		args = cstr_array_append(args, "-o");
		args = cstr_array_append(args, PATH("bin", output));
		Cstr_Array objectFiles = {.count=compiledCount,.elems=compiledFiles};
		args = cstr_array_concat(args, objectFiles);
		Cstr_Array libDirs = {.count=project.libdirCount,.elems=(const char**)project.libdir};
		args = cstr_array_concat(args,libDirs);
		Cstr_Array libs = {.count=project.s_libCount,.elems=(const char**)project.s_libs};
		args = cstr_array_concat(args,libs);
		CMD_ARR(args);
		return output;
	}
	case PROJECT_TYPE_LIB:
	{

		if (!PATH_EXISTS("lib")) {
			MKDIRS("lib");
		}
		const char* output = CONCAT("lib", project.name, LIB_EXTENSION);

		Cstr_Array args = {0};
		args = cstr_array_append(args, ARCHIVER);
		args = cstr_array_concat(args, splitString(LIB_FLAGS,' '));
		args = cstr_array_append(args, "-o");
		args = cstr_array_append(args, PATH("lib", output));
		Cstr_Array objectFiles = {.count=compiledCount,.elems=compiledFiles};
		args = cstr_array_concat(args, objectFiles);
		CMD_ARR(args);
		//CMD(ARCHIVER, LIB_FLAGS, "-o", PATH("lib", output), objectFiles);
		return output;
	}
	case PROJECT_TYPE_DLL:
	{
		if (!PATH_EXISTS("lib")) {
			MKDIRS("lib");
		}

		const char* output = CONCAT(LIB_PREFIX, project.name, DLL_EXTENSION);

		Cstr_Array args = {0};
		args = cstr_array_append(args, project.compiler);
		if (project.flags[0] != '\0') {
			args = cstr_array_concat(args, splitString(project.flags, ' '));
		}
		args = cstr_array_append(args, "-shared");
		args = cstr_array_append(args, "-o");
		args = cstr_array_append(args, PATH("lib", output));
		Cstr_Array objectFiles = {.count=compiledCount,.elems=(const char**)compiledFiles};
		args = cstr_array_concat(args, objectFiles);
		Cstr_Array libDirs = {.count=project.libdirCount,.elems=(const char**)project.libdir};
		args = cstr_array_concat(args,libDirs);
		Cstr_Array libs = {.count=project.s_libCount,.elems=(const char**)project.s_libs};
		args = cstr_array_concat(args,libs);
		CMD_ARR(args);
		return output;
	}
	case PROJECT_TYPE_CUSTOM:
	{
		if (project.outDir[0]) {
			for (int i = 0; i < strlen(project.outDir); i++) {
				if (project.outDir[i] == '/') {
					project.outDir[i] = PATH_SEP[0];
				}
			}
			if (!PATH_EXISTS(project.outDir)) {
				MKDIRS(project.outDir);
			}
			for (int i = 0; i < compiledCount; i++) {

				const char* outFile = ((char*)compiledFiles[i]) + (strlen("build") + strlen(PATH_SEP) + strlen("tmp") + strlen(PATH_SEP) + strlen(project.name) + strlen(PATH_SEP));


				char* nextPath = project.outDir;
				char* curPath = (char*)outFile;
				char* othPath = project.outDir;
				while (1) {
					char* nextSep = (char*)memchr(nextPath, PATH_SEP[0], strlen(othPath));
					nextPath = nextSep;

					if (nextPath == 0) {
						break;
					}
					if (memcmp(curPath, othPath, nextPath - othPath) == 0) {
						curPath += (nextPath - othPath) + 1;
						othPath += (nextPath - othPath) + 1;
						nextPath += 1;
					} else {
						break;
					}

				}
				const char* out = PATH(project.outDir, curPath);
				nextPath = (char*) out;
				while (1) {
					char* nextSep = (char*)memchr(nextPath, PATH_SEP[0], strlen(out));
					nextPath = nextSep;

					if (nextPath == 0) {
						break;
					}
					char imDir[512] = { 0 };
					memcpy(imDir, out, nextPath - out);
					nextPath += 1;
					imDir[511] = '\0';
					if (!PATH_EXISTS(imDir) && project.outType[0] ? (!ENDS_WITH(imDir,project.outType)) : 1) {
						MKDIRS(imDir);
					}
					else {
						//printf("not a dir %s\n", imDir);
					}
				}

				copyFile(compiledFiles[i], out);
			}
		}

		return "";
	}

		default:
			break;
	}
	PANIC("should not reach here, invalid project type");
	return NULL;
}



typedef enum ParseState {
	STATE_NONE,
	STATE_NAME,
	STATE_PROP,
	STATE_VALUE,
	STATE_LIST,
} ParseState;


int parseType(char c, int* index, char* value, Project* project) {
	if (c == '\r' || c == '\n' || c == ' ') {
		value[*index] = '\0';
		if (strcmp(toUppercase(value), "LIB") == 0) {
			project->type = PROJECT_TYPE_LIB;
		} else if (strcmp(toUppercase(value), "EXE") == 0) {
			project->type = PROJECT_TYPE_EXE;
		} else if (strcmp(toUppercase(value), "DLL") == 0) {
			project->type = PROJECT_TYPE_DLL;
		}else if (strcmp(toUppercase(value), "CUSTOM") == 0){
			project->type = PROJECT_TYPE_CUSTOM;
		} else {
			project->type = PROJECT_TYPE_UNKNOWN;
		}
		return 1;
	}

	value[*index] = c;
	(*index)++;
	return 0;
}

int parseCompiler(char c, int* index, char* value, Project* project) {
	if (c == '\r' || c == '\n' || c == ' ') {
		value[*index] = '\0';
		memcpy(project->compiler, value, 64);
		return 1;
	}

	value[*index] = c;
	(*index)++;
	return 0;
}

int parseOut(char c, int* index, char* value, Project* project) {
	if (c == '\r' || c == '\n' || c == ' ') {
		value[*index] = '\0';
		memcpy(project->outDir, value, 512);
		return 1;
	}

	value[*index] = c;
	(*index)++;
	return 0;
}

int parseOutType(char c, int* index, char* value, Project* project) {
	if (c == '\r' || c == '\n' || c == ' ') {
		value[*index] = '\0';
		memcpy(project->outType, value, 64);
		return 1;
	}

	value[*index] = c;
	(*index)++;
	return 0;
}


int parseExport(char c, int* index, char* value, Project* project) {
	if (c == '\r' || c == '\n' || c == ' ') {
		if(value[0]=='#'){
#ifndef _WIN32
			return 1;
#else
			value++;
			(*index)--;
#endif
		}
		if (value[0] == '^') {
#ifndef _WIN32
			value++;
			(*index)--;
#else
			return 1;
#endif
		}
		value[*index] = '\0';
		char** data = realloc(project->exports, sizeof(char*) * (project->exportCount + 1));
		if (!data) {
			PANIC("out of mem");
		}
		project->exports = data;

		char* str = malloc(*index + 1);
		memcpy(str, value, *index + 1);
		project->exports[project->exportCount] = str;
		project->exportCount++;
		return 1;
	}

	if (c == '/' || c == '\\') {
		for (int i = 0; i < strlen(PATH_SEP); i++) {
			value[*index] = PATH_SEP[i];
			(*index)++;
		}
	} else {
		value[*index] = c;
		(*index)++;
	}
	return 0;
}

int parseFlags(char c, int* index, char* value, Project* project) {
	if (c == '\r' || c == '\n') {
		value[*index] = '\0';
		memcpy(project->flags, value, 512);
		return 1;
	}
	value[*index] = c;
	(*index)++;
	return 0;
}

int parseSource(char c, int* index, char* value, Project* project) {
	if (c == '\r' || c == '\n' || c == ' ') {
		if(value[0]=='#'){
#ifndef _WIN32
			return 1;
#else
			value++;
			(*index)--;
#endif
		}
		if (value[0] == '^') {
#ifndef _WIN32
			value++;
			(*index)--;
#else
			return 1;
#endif
		}
		value[*index] = '\0';
		char** data = realloc(project->sources, sizeof(char*) * (project->sourceCount + 1));
		if (!data) {
			PANIC("out of mem");
		}
		project->sources = data;

		char* str = malloc(*index + 1);
		memcpy(str, value, *index + 1);
		project->sources[project->sourceCount] = str;
		project->sourceCount++;
		return 1;
	}

	if (c == '/' || c == '\\') {
		for (int i = 0; i < strlen(PATH_SEP); i++) {
			value[*index] = PATH_SEP[i];
			(*index)++;
		}
	} else {
		value[*index] = c;
		(*index)++;
	}
	return 0;
}

int parseFileType(char c, int* index, char* value, Project* project) {
	if (c == '\r' || c == '\n' || c == ' ') {
		if (value[0] == '#') {
#ifndef _WIN32
			return 1;
#else
			value++;
			(*index)--;
#endif
		}
		if (value[0] == '^') {
#ifndef _WIN32
			value++;
			(*index)--;
#else
			return 1;
#endif
		}
		value[*index] = '\0';
		char** data = realloc(project->fileTypes, sizeof(char*) * (project->fileTypeCount + 1));
		if (!data) {
			PANIC("out of mem");
		}
		project->fileTypes = data;

		char* str = malloc(*index + 1);
		memcpy(str, value, *index + 1);
		project->fileTypes[project->fileTypeCount] = str;
		project->fileTypeCount++;
		return 1;
	}

	value[*index] = c;
	(*index)++;

	return 0;
}

int parseLib(char c, int* index, char* value, Project* project) {
	if (c == '\r' || c == '\n' || c == ' ') {
		if(value[0]=='#'){
#ifndef _WIN32
			return 1;
#else
			value++;
			(*index)--;
#endif
		}
		if (value[0] == '^') {
#ifndef _WIN32
			value++;
			(*index)--;
#else
			return 1;
#endif
		}
		value[*index] = '\0';
		char** data = realloc(project->s_libs, sizeof(char*) * (project->s_libCount + 1));
		if (!data) {
			PANIC("out of mem");
		}
		project->s_libs = data;

		char* str = malloc(*index + 1);
		memcpy(str, value, *index + 1);
		project->s_libs[project->s_libCount] = str;
		project->s_libCount++;
		return 1;
	}

	value[*index] = c;
	(*index)++;
	
	return 0;
}

int parseDll(char c, int* index, char* value, Project* project) {
	PANIC("DEPRECATED");
	return 1;
}

int parseInclude(char c, int* index, char* value, Project* project) {
	if (c == '\r' || c == '\n' || c == ' ') {
		if(value[0]=='#'){
#ifndef _WIN32
			return 1;
#else
			value++;
			(*index)--;
#endif
		}
		if (value[0] == '^') {
#ifndef _WIN32
			value++;
			(*index)--;
#else
			return 1;
#endif
		}
		value[*index] = '\0';
		char** data = realloc(project->include, sizeof(char*) * (project->includeCount + 1));
		if (!data) {
			PANIC("out of mem");
		}
		project->include = data;

		char* str = malloc(*index + 1);
		memcpy(str, value, *index + 1);
		project->include[project->includeCount] = str;
		project->includeCount++;
		return 1;
	}

	if (c == '/' || c == '\\') {
		for (int i = 0; i < strlen(PATH_SEP); i++) {
			value[*index] = PATH_SEP[i];
			(*index)++;
		}
	} else {
		value[*index] = c;
		(*index)++;
	}
	return 0;
}

int parseLibDir(char c, int* index, char* value, Project* project) {
	if (c == '\r' || c == '\n' || c == ' ') {
		if(value[0]=='#'){
#ifndef _WIN32
			return 1;
#else
			value++;
			(*index)--;
#endif
		}
		if (value[0] == '^') {
#ifndef _WIN32
			value++;
			(*index)--;
#else
			return 1;
#endif
		}
		value[*index] = '\0';
		char** data = realloc(project->libdir, sizeof(char*) * (project->libdirCount + 1));
		if (!data) {
			PANIC("out of mem");
		}
		project->libdir = data;

		char* str = malloc(*index + 1);
		memcpy(str, value, *index + 1);
		project->libdir[project->libdirCount] = str;
		project->libdirCount++;
		return 1;
	}

	if (c == '/' || c == '\\') {
		for (int i = 0; i < strlen(PATH_SEP); i++) {
			value[*index] = PATH_SEP[i];
			(*index)++;
		}
	} else {
		value[*index] = c;
		(*index)++;
	}
	return 0;
}

int parseProjectFile(char* buffer, Project* project) {
	char c;

	char projName[64] = { 0 };
	char property[64] = { 0 };
	char value[512] = { 0 };

	int index = 0;
	ParseState state = STATE_NAME;
	int done = 0;
	int list = 0;
	int readIndex = 0;
	Project* oldProject = 0;

	memcpy(project->compiler, "gcc", 4);

	while ((c = buffer[readIndex++]) != '\0') {
		if (c == '}') {
			state = STATE_NAME;
			memset(property, 0, 64);
			memset(value, 0, 512);
			memset(projName, 0, 64);

			Project* newProject = malloc(sizeof(Project));
			if (!newProject) {
				PANIC("mem error");
			}
			memset(newProject, 0, sizeof(Project));
			memcpy(newProject->compiler, "gcc", 4);
			project->next = newProject;
			oldProject = project;

			project = newProject;
			index = 0;
			list = 0;
			done = 0;
			continue;;
		}
		if (state == STATE_NAME) {
			if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
				continue;
			}
			if (c == '{') {
				state = STATE_PROP;
				index = 0;
				memcpy(project->name, projName, 64);
				project->name[63] = '\0';
				memset(projName, 0, 64);
				continue;
			}
			if (index >= 64) {
				return -1;
			}
			projName[index++] = c;
			continue;
		}
		if (state == STATE_PROP) {
			if (c == ' ' || c == '\t') {
				continue;
			}
			if (c == '\r' || c == '\n') {
				continue;
			}
			if (c == ':') {
				state = STATE_VALUE;
				index = 0;
				continue;
			}
				if (index >= 64) {
				return -1;
			}
			property[index++] = c;
			continue;
		}
		if (state == STATE_VALUE) {
			if (c == '\t') {
				c = ' ';
			}
			if (index == 0 && c == ' ') {
				continue;
			}
			if (c == '[') {
				state = STATE_LIST;
				continue;
			}

			if (list == 1) {
				if (c == ']') {
					memset(property, 0, 64);
					memset(value, 0, 512);
					state = STATE_PROP;
					index = 0;
					list = 0;
					done = 0;
					continue;;
				}
			}

			if (strcmp(toUppercase(property), "TYPE") == 0) {
				done = parseType(c, &index, value, project);
			}
			if (strcmp(toUppercase(property), "COMPILER") == 0) {
				done = parseCompiler(c, &index, value, project);
			}
			if (strcmp(toUppercase(property), "OUTDIR") == 0) {
				done = parseOut(c, &index, value, project);
			}
			if (strcmp(toUppercase(property), "OUTTYPE") == 0) {
				done = parseOutType(c, &index, value, project);
			}
			if(strcmp(toUppercase(property), "FILES") == 0) {
				done = parseFileType(c, &index, value, project);
			}
			if (strcmp(toUppercase(property), "FLAGS") == 0) {
				done = parseFlags(c, &index, value, project);
			}
			if (strcmp(toUppercase(property), "SOURCE") == 0) {
				done = parseSource(c, &index, value, project);
			}
			if (strcmp(toUppercase(property), "EXPORT") == 0) {
				done = parseExport(c, &index, value, project);
			}
			if (strcmp(toUppercase(property), "LIB") == 0) {
				done = parseLib(c, &index, value, project);
			}
			if (strcmp(toUppercase(property), "INCLUDE") == 0) {
				done = parseInclude(c, &index, value, project);
			}
			if (strcmp(toUppercase(property), "LIBDIR") == 0) {
				done = parseLibDir(c, &index, value, project);
			}



			if (done && list == 1) {
				memset(value, 0, 512);
				state = STATE_LIST;
				list = 0;
				done = 0;
				index = 0;
				readIndex--;
				continue;

			}
			if (done && list == 0) {
				memset(property, 0, 64);
				memset(value, 0, 512);
				state = STATE_PROP;
				index = 0;
				done = 0;
				continue;;
			}
		}
		if (state == STATE_LIST) {
			if (c == '\n') {
				state = STATE_VALUE;
				list = 1;
			}
			continue;
		}


	}

	// fix the emtpy project at the end
	// TODO: find better solution
	if (oldProject) {
		oldProject->next = 0;
	}

	return 0;
}

int loadProjectFile(const char* file, Project* project) {
	FILE* projectFile = fopen(file, "rb");
	if (!projectFile) {
		printf("no project file found for %s", file);
		return -1;
	}

	fseek(projectFile, 0, SEEK_END);
	size_t size = ftell(projectFile);
	fseek(projectFile, 0, SEEK_SET);

	char* buffer = malloc(size + 1);
	buffer[size] = '\0';
	size_t readSize = fread(buffer, 1, size, projectFile);
	if (readSize != size) {
		printf("error reading project file");
		return -1;
	}

	int ret = parseProjectFile(buffer, project);

	fclose(projectFile);
	return ret;
}

#ifndef _WIN32
#define _POSIX_SOURCE
#include <sys/stat.h>
#endif

int userIsRoot(){
#ifndef _WIN32

	return getuid()==0;

#endif
	return 0;
}

void installProject(Project project, const char* projectName){
#ifdef _WIN32
	
	INFO("Instaling is not required on windows");
	return;

#endif
	if(!userIsRoot()){
		WARN("You must run install with root privileges");
		return;
	}

	if(project.type!=PROJECT_TYPE_DLL && project.type!=PROJECT_TYPE_LIB){
		return;
	}

	const char* projectFile = CONCAT(LIB_PREFIX,project.name,project.type==PROJECT_TYPE_DLL?DLL_EXTENSION:LIB_EXTENSION); 
	const char* outDir = CONCAT(PATH_SEP,PATH("usr","local","lib"));
	copyFile(
		PATH("lib",projectFile), 
		PATH(outDir,projectFile)
	);
	char mode[] = "0755";
	int modeNum = strtol(mode,0,8);
	if(chmod(PATH(outDir,projectFile),modeNum)<0){
		PANIC("failed to give the library the propper permissions");
	}
}

const char* defaultFileTypes[] = {
	".c",
	".cpp"
};

int buildProject(Project project, const char* projectName) {
	const char* tmp = PATH("build", "tmp");
	if (!PATH_EXISTS(tmp)) {
		MKDIRS(tmp);
	}

	const char** compiledFiles = NULL;
	size_t compiledCount = 0;
	for (int i = 0; i < project.sourceCount; i++) {
		const char* source = extractEnv(project.sources[i],project);

		Cstr tempBuild = PATH(tmp, project.name);
		if (!PATH_EXISTS(tempBuild)) {
			MKDIRS(tempBuild);
		}
		
		for (int i = 0; i < project.includeCount; i++) {
			project.include[i] = (char*) CONCAT("-I", extractEnv(project.include[i],project));
		}
		for(int i = 0; i < project.s_libCount; i++){
			project.s_libs[i] = (char*) CONCAT("-l",project.s_libs[i]);
		}
		for(int i = 0; i < project.libdirCount; i++){
			project.libdir[i] = (char*) CONCAT("-L",extractEnv(project.libdir[i],project));
		}

		if (project.fileTypeCount == 0) {
			
			project.fileTypes = (char**) defaultFileTypes;
			project.fileTypeCount = sizeof(defaultFileTypes) / sizeof(const char*);
		}

		if (project.outType[0] == 0) {
			project.outType[0] = '.';
			project.outType[1] = 'o';
			project.outType[2] = '\0';
		}

		const char* outFolder = project.sourceCount == 1 ? tempBuild : PATH(tempBuild, source);

		compile(source, outFolder, &compiledFiles, &compiledCount,project.includeCount, (const char**)project.include, project.compiler, project.flags, project.fileTypeCount,(const char**) project.fileTypes, project.outType);
		const char* output = linker(compiledFiles, compiledCount, project);

		if (!PATH_EXISTS(projectName)) {
			MKDIRS(projectName);
		}
		for (int i = 0; i < project.exportCount; i++) {
			const char* export = extractEnv(project.exports[i], project);
			if (IS_DIR(export)) {
				CMD(COPY_COMMAND( export, PATH(projectName,FOLDERNAME(export))));
			} else {
				copyFile(export, PATH(projectName, output));
			}
		}
		

	}
	free(compiledFiles);

	return 0;
}

void cleanProject(Project project, const char* projectName) {
	if (PATH_EXISTS(PATH("build", "tmp", project.name))) {
		RM(PATH("build", "tmp", project.name));
	}
	if (PATH_EXISTS(projectName)) {
		RM(projectName);
	}
	
	switch (project.type) {
	case PROJECT_TYPE_EXE:
		if (PATH_EXISTS(PATH("bin", CONCAT(project.name, EXE_EXTENSION)))) {
			RM(PATH("bin", CONCAT(project.name, EXE_EXTENSION)));
		}
		break;
	case PROJECT_TYPE_LIB:
		if (PATH_EXISTS(PATH("lib",CONCAT(LIB_PREFIX, project.name, LIB_EXTENSION)))) {
			RM(PATH("lib", CONCAT(LIB_PREFIX, project.name, LIB_EXTENSION)));
		}
		break;
	case PROJECT_TYPE_DLL:
		if (PATH_EXISTS(PATH("lib",CONCAT(LIB_PREFIX, project.name, DLL_EXTENSION)))) {
			RM(PATH("lib", CONCAT(LIB_PREFIX, project.name, DLL_EXTENSION)));
		}
		break;
	case PROJECT_TYPE_CUSTOM:
		for (int i = 0; i < strlen(project.outDir); i++) {
			if (project.outDir[i] == '/') {
				project.outDir[i] = PATH_SEP[0];
			}
		}
		if (PATH_EXISTS(project.outDir)) {
			RM(project.outDir);
		}
		
		default:
		break;
	}
}

void cleanWorkspace() {
	if (PATH_EXISTS(PATH("build", "tmp"))) {
		FOREACH_FILE_IN_DIR(file, PATH("build","tmp"), {
			if (isValidDir(file)) {
				RM(PATH("build","tmp", file));
			}
			});

		RM(PATH("build", "tmp"));
	}
	FOREACH_FILE_IN_DIR(file, "bin", {
		if (isValidDir(file)) {
			RM(PATH("bin", file));
		}
		});
	if (PATH_EXISTS("bin")) {
		RM("bin");
	}
	FOREACH_FILE_IN_DIR(file, "lib", {
		if (isValidDir(file)) {
			RM(PATH("lib", file));
		}

		});
	if (PATH_EXISTS("lib")) {
		RM("lib");
	}
}

int processProject(const char* projectName, OperationType op) {
	Project project = { 0 };

	char buffer[512] = { 0 };
	strcpy(buffer, NOEXT(projectName));
	strcat(buffer, ".project");
	buffer[127] = '\0';
	loadProjectFile(buffer, &project);
	Project* p = &project;
	while (p) {
		switch (op) {
		case OPERATION_TYPE_BUILD:
			if (buildProject(*p, NOEXT(projectName)) != 0) {
				return -1;
			}
			break;
		case OPERATION_TYPE_CLEAN:
			cleanProject(*p, NOEXT(projectName));
			break;
		case OPERATION_TYPE_REBUILD:
			cleanProject(*p, NOEXT(projectName));
			if (buildProject(*p, NOEXT(projectName)) != 0) {
				return -1;
			}
			break;
		case OPERATION_TYPE_CONFIGURE:
			// TODO: implement
			break;
		case OPERATION_TYPE_INSTALL:
			installProject(*p, NOEXT(projectName));
			break;
		}
		p = p->next;
	}

}

void here(int line) {
	printf("HERE %i\n", line);
}
#define HERE here(__LINE__);

int main(int argC, char* argV[]) {
	const char* program = shift_args(&argC, &argV);

	OperationType op = OPERATION_TYPE_BUILD;
	if (argC > 0) {
		const char* operation = shift_args(&argC, &argV);
		op = getOperationType(operation);
		if (op == OPERATION_TYPE_UNKNOWN) {
			printUse(program);
			return -1;
		}
	}

	if (argC == 0) {
		if (op == OPERATION_TYPE_CLEAN) {
			cleanWorkspace();
		}
		if (op == OPERATION_TYPE_REBUILD) {
			cleanWorkspace();
		}
		FOREACH_FILE_IN_DIR(project, "./", {
			if (ENDS_WITH(project, PROJECT_FILE)) {
				printf("processing project %s\n", project);
				if (processProject(project, op) != 0) {
					return -1;
				}
			}
		});


		return 0;
	} else {
		const char* project = shift_args(&argC, &argV);
		printf("processing project %s\n", project);
		return processProject(project, op);
	}

}
