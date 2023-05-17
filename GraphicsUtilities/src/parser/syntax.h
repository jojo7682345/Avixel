#pragma once
#include "tokenizer.h"

typedef enum NodeType {

}NodeType;

typedef struct SyntaxTreeNode {
	NodeType* type;
}SyntaxTreeNode;

typedef struct OnceNode {
	SyntaxTreeNode node;
} OnceNode;

typedef struct IncludeNode {
	SyntaxTreeNode node;
	const char* file;
} IncludeNode;

typedef struct ParamNode {
	SyntaxTreeNode node;
	const char* name;
	Token* value; //
} ParamNode;

typedef struct PrototypeNode {
	SyntaxTreeNode node;
	const char* name;
	const char* type;
	uint childCount;
	SyntaxTreeNode* children;
} PrototypeNode;

typedef struct ComponentNode {
	SyntaxTreeNode node;
	const char* name;
	const char type;
	uint childCount;
	SyntaxTreeNode* children;
} ComponentNode;

typedef struct PropertyNode {
	SyntaxTreeNode node;
	const char* name;
	bool isConst;
	SyntaxTreeNode* value;
} PropertyNode;

typedef struct ValueNode {
	SyntaxTreeNode node;
	Token* value;
} ValueNode;