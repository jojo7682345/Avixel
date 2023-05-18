typedef struct AvInterfaceLoadFileInfo {
	int a;
} AvInterfaceLoadFileInfo;

typedef struct AvInterfaceLoadDataInfo {
	int a;
}AvInterfaceLoadDataInfo;

AvResult avInterfaceLoadFromFile(AvInterfaceLoadFileInfo info, AvInterface* interface, const char* fileName);
AvResult avInterfaceLoadFromData(AvInterfaceLoadDataInfo info, AvInterface* interface, byte* data, uint64 size);