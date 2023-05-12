#define NOBUILD_IMPLEMENTATION
#include "builder.h"

#include <stdio.h>
#include <unistd.h>

#define COMPILER "gcc"
#define ARCHIVER "ar"
#define CFLAGS "-Wall"
#define LIB_FLAGS "rcs"

#ifdef _WIN32
#define LIB_EXTENSION ".lib"
#define EXE_EXTENSION ".exe"
#define DLL_EXTENSION ".dll"
#else
#define DLL_EXTENSION ".so"
#define LIB_EXTENSION ".a"
#define EXE_EXTENSION ""
#endif

#define PROJECT_FILE ".project"

typedef enum ProjectType {
	PROJECT_TYPE_LIB,
	PROJECT_TYPE_EXE,
	PROJECT_TYPE_UNKNOWN,
}ProjectType;

typedef enum OperationType {
	OPERATION_TYPE_BUILD,
	OPERATION_TYPE_CLEAN,
	OPERATION_TYPE_REBUILD,
	OPERATION_TYPE_CONFIGURE,
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
	return PROJECT_TYPE_UNKNOWN;
}

OperationType getOperationType(const char* type) {
	const char* str = toUppercase(type);
	printf("%s\n", str);
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
	int d_libCount;
	char** d_libs;
	int includeCount;
	char** include;
	int libdirCount;
	char** libdir;
	char compiler[64];
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

const char* extractEnv(const char* str) {
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
		const char* envVar = getenv(env);
		free(env);
		if (!envVar) {
			envVar = "";
		}
		const char* sub = subsitute(str, envVar, preStart, end);
		const char* extracted = extractEnv(sub);
		return extracted;
	}
	return str;
}

void compile(const char* source, const char* immediate, const char*** compiledFiles, size_t* compiledCount, const char* include, const char* compiler) {
	Cstr imBuild = PATH(immediate);
	if (!IS_DIR(imBuild)) {
		mkdir_p(imBuild);
	}

	FOREACH_FILE_IN_DIR(file, source, {

		if (ENDS_WITH(file,".c") || ENDS_WITH(file,".cpp")) {
			const char* outFile = JOIN("\\", imBuild, CONCAT(NOEXT(file), ".o"));
			CMD(compiler, CFLAGS, include, "-c", "-o", outFile, JOIN("\\", source, file));
			void* data = realloc(*compiledFiles, sizeof(const char*) * (*compiledCount + 1));
			if (!data) {
				PANIC("mem");
			}
			*compiledFiles = data;
			(*compiledFiles)[*compiledCount] = outFile;
			*compiledCount += 1;
		} else {
			if (IS_DIR(PATH(source,file))) {
				if (isValidDir(file)) {
					compile(PATH(source, file), PATH(immediate, file), compiledFiles, compiledCount, include, compiler);
				}
			}
		}
	});
}

void link(const char** compiledFiles, size_t* compiledCount, Project project) {

	const char* objectFiles = "";
	for (size_t i = 0; i < *compiledCount; i++) {
		objectFiles = JOIN(" ", objectFiles, compiledFiles[i]);
	}

	switch (project.type) {
		case PROJECT_TYPE_EXE:
		{
			const char* lib = "";
			for (int i = 0; i < project.s_libCount; i++) {
				const char* inc = extractEnv(project.s_libs[i]);
				lib = JOIN(" -l", lib, CONCAT(inc,""));
			}

			const char* dll = "";
			for (int i = 0; i < project.d_libCount; i++) {
				const char* inc = extractEnv(project.d_libs[i]);
				dll = JOIN(" -l", dll, CONCAT(inc, ""));
			}

			const char* libDir = "";
			for (int i = 0; i < project.libdirCount; i++) {
				const char* inc = extractEnv(project.libdir[i]);
				libDir = JOIN(" -L", libDir, inc);
			}

			if (!IS_DIR("bin")) {
				MKDIRS("bin");
			}

			CMD(project.compiler, CFLAGS, libDir, lib, dll, objectFiles, "-o", PATH("bin", CONCAT(project.name, EXE_EXTENSION)));
			break;
		}
		case PROJECT_TYPE_LIB:
		{


			if (!IS_DIR("lib")) {
				MKDIRS("lib");
			}
			CMD(ARCHIVER, LIB_FLAGS, "-o", PATH("lib", CONCAT(project.name, LIB_EXTENSION)), objectFiles);
			break;
		}
	}

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
		value[*index] = '\0';
		char** data = realloc(project->sources, project->sourceCount + 1);
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
	value[*index] = c;
	(*index)++;
	return 0;
}

int parseLib(char c, int* index, char* value, Project* project) {
	if (c == '\r' || c == '\n' || c == ' ') {
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
	if (c == '\r' || c == '\n' || c == ' ') {
		value[*index] = '\0';
		char** data = realloc(project->d_libs, sizeof(char*) * (project->d_libCount + 1));
		if (!data) {
			PANIC("out of mem");
		}
		project->d_libs = data;

		char* str = malloc(*index + 1);
		memcpy(str, value, *index + 1);
		project->d_libs[project->d_libCount] = str;
		project->d_libCount++;
		return 1;
	}
	value[*index] = c;
	(*index)++;
	return 0;
}

int parseInclude(char c, int* index, char* value, Project* project) {
	if (c == '\r' || c == '\n' || c == ' ') {
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
	value[*index] = c;
	(*index)++;
	return 0;
}

int parseLibDir(char c, int* index, char* value, Project* project) {
	if (c == '\r' || c == '\n' || c == ' ') {
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
	value[*index] = c;
	(*index)++;
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
			if (strcmp(toUppercase(property), "FLAGS") == 0) {
				done = parseFlags(c, &index, value, project);
			}
			if (strcmp(toUppercase(property), "SOURCE") == 0) {
				done = parseSource(c, &index, value, project);
			}
			if (strcmp(toUppercase(property), "LIB") == 0) {
				done = parseLib(c, &index, value, project);
			}
			if (strcmp(toUppercase(property), "DLL") == 0) {
				done = parseDll(c, &index, value, project);
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



int buildProject(Project project) {
	const char* tmp = PATH(".\\build", "tmp");
	if (!IS_DIR(tmp)) {
		MKDIRS(tmp);
	}

	const char** compiledFiles = NULL;
	size_t compiledCount = 0;
	for (int i = 0; i < project.sourceCount; i++) {
		const char* source = extractEnv(project.sources[i]);

		Cstr tempBuild = PATH(tmp, project.name);
		if (!IS_DIR(tempBuild)) {
			MKDIRS(tempBuild);
		}
		const char* include = "";
		for (int i = 0; i < project.includeCount; i++) {
			const char* inc = extractEnv(project.include[i]);
			include = JOIN(" -I", include, inc);
		}

		printf("%s: %s\n", project.name, source);

		compile(source, PATH(tempBuild, source), &compiledFiles, &compiledCount,include,project.compiler);
		link(compiledFiles, &compiledCount, project);

	}
	free(compiledFiles);

	return 0;
}

void cleanProject(Project project) {
	RM(PATH("build", "tmp", project.name));
	switch (project.type) {
		case PROJECT_TYPE_EXE:
			RM(PATH("bin", CONCAT(project.name, EXE_EXTENSION)));
			break;
		case PROJECT_TYPE_LIB:
			RM(PATH("lib", CONCAT(project.name, LIB_EXTENSION)));
			break;
	}
}

void cleanWorkspace() {
	FOREACH_FILE_IN_DIR(file, "build\\tmp", {
		if (isValidDir(file)) {
			RM(PATH("build","tmp", file));
		}
						});
	if (IS_DIR(PATH("build", "tmp")) ){
		RM(PATH("build", "tmp"));
	}
	FOREACH_FILE_IN_DIR(file, "bin", {
		if (isValidDir(file)) {
			RM(PATH("bin", file));
		}
						});
	FOREACH_FILE_IN_DIR(file, "lib", {
		if (isValidDir(file)) {
			RM(PATH("lib", file));
		}

	});
}

int processProject(const char* projectName, OperationType op) {
	Project project = { 0 };

	char buffer[128] = { 0 };
	strcpy_s(buffer, 64, NOEXT(projectName));
	strcat_s(buffer, 128, ".project");
	buffer[127] = '\0';
	loadProjectFile(buffer, &project);
	Project* p = &project;
	while (p) {
		switch (op) {
			case OPERATION_TYPE_BUILD:
				if (buildProject(*p) != 0) {
					return -1;
				}
				break;
			case OPERATION_TYPE_CLEAN:
				cleanProject(*p);
				break;
			case OPERATION_TYPE_REBUILD:
				cleanProject(*p);
				if (buildProject(*p) != 0) {
					return -1;
				}
				break;
			case OPERATION_TYPE_CONFIGURE:
				// TODO: implement
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
	chdir("../");
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
		if (op == OPERATION_TYPE_CLEAN ) {
			cleanWorkspace();
			return 0;
		}
		if (op == OPERATION_TYPE_REBUILD) {
			cleanWorkspace();
		}
		FOREACH_FILE_IN_DIR(project, "./", {
			if (ENDS_WITH(project, PROJECT_FILE)) {
				printf("building project %s\n", project);
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