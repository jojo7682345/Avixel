#pragma once
#include "tokenizer.h"
#include "../core/util/dynamicArray.h"

typedef enum NodeType {
	NODE_TYPE_ONCE,
	NODE_TYPE_INCLUDE,
	NODE_TYPE_PARAM,
	NODE_TYPE_PROTOTYPE,
	NODE_TYPE_COMPONENT,
	NODE_TYPE_PROPERTY,
	NODE_TYPE_VALUE,
}NodeType;

typedef struct OnceNode {
	byte padding_;
} OnceNode;

typedef struct IncludeNode {
	const char* file;
} IncludeNode;

typedef struct ParamNode {
	const char* name;
	Token* value; //
} ParamNode;

typedef struct PrototypeNode {
	const char* name;
	const char* type;
	DynamicArray children;
} PrototypeNode;

typedef struct ComponentNode {
	const char* name;
	const char type;
	DynamicArray children;
} ComponentNode;

typedef struct PropertyNode {
	const char* name;
	bool isConst;
	SyntaxTreeNode* value;
} PropertyNode;

typedef struct ValueNode {
	Token* value;
} ValueNode;

typedef struct SyntaxTreeNode {
	NodeType type;

	union {
		OnceNode once;
		IncludeNode include;
		ParamNode param;
		PrototypeNode prototype;
		ComponentNode component;
		PropertyNode property;
		ValueNode value;
	};

}SyntaxTreeNode;

AvResult buildSyntaxTree(uint tokenCount, Token* tokens, DynamicArray rootNodes);