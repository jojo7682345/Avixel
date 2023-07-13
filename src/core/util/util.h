#pragma once
#include "../core.h"

bool isDecNumber(char chr);

bool stringEquals(const char* strA, const char* strB, uint size);

bool isBool(const char* str, uint size);

bool isParam(const char* str, uint size);

bool isHexNumber(char chr);

bool isLowerCaseLetter(char chr);

bool isUpperCaseLetter(char chr);

bool isLetter(char chr);

bool isNameCharacter(char chr);

bool isTextCharacter(char chr);

void skipToNextLine(const char* buffer, uint64 size, uint* i);


