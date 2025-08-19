#include "Utils.h"

#include <process.h>
#include <Tlhelp32.h>

struct PropertyInfo {
	BSFixedString		scriptName;		// 00
	BSFixedString		propertyName;	// 08
	UInt64				unk10;			// 10
	void*				unk18;			// 18
	void*				unk20;			// 20
	void*				unk28;			// 28
	SInt32				index;			// 30	-1 if not found
	UInt32				unk34;			// 34
	BSFixedString		unk38;			// 38
};
STATIC_ASSERT(offsetof(PropertyInfo, index) == 0x30);
STATIC_ASSERT(sizeof(PropertyInfo) == 0x40);

typedef bool(*_GetPropertyValueByIndex)(VirtualMachine* vm, VMIdentifier** identifier, int idx, VMValue* outValue);
const int Idx_GetPropertyValueByIndex = 0x25;

typedef void* (*_GetPropertyInfo)(VMObjectTypeInfo* objectTypeInfo, void* outInfo, BSFixedString* propertyName, bool unk4);
RelocAddr <_GetPropertyInfo> GetPropertyInfo_Internal(0x026F3970);

typedef bool(*_IKeywordFormBase_HasKeyword)(IKeywordFormBase* keywordFormBase, BGSKeyword* keyword, UInt32 unk3);

TESForm * GetFormFromIdentifier(const std::string & identifier)
{
	auto delimiter = identifier.find('|');
	if (delimiter != std::string::npos) {
		std::string modName = identifier.substr(0, delimiter);
		std::string modForm = identifier.substr(delimiter + 1);

		const ModInfo* mod = (*g_dataHandler)->LookupModByName(modName.c_str());
		if (mod && mod->modIndex != -1) {
			UInt32 formID = std::stoul(modForm, nullptr, 16) & 0xFFFFFF;
			UInt32 flags = GetOffset<UInt32>(mod, 0x334);
			if (flags & (1 << 9)) {
				// ESL
				formID &= 0xFFF;
				formID |= 0xFE << 24;
				formID |= GetOffset<UInt16>(mod, 0x372) << 12;	// ESL load order
			}
			else {
				formID |= (mod->modIndex) << 24;
			}
			return LookupFormByID(formID);
		}
	}
	return nullptr;
}
void GetPropertyInfo(VMObjectTypeInfo * objectTypeInfo, PropertyInfo * outInfo, BSFixedString * propertyName)
{
	GetPropertyInfo_Internal(objectTypeInfo, outInfo, propertyName, 1);
}
bool GetPropertyValue(const char * formIdentifier, const char * scriptName, const char * propertyName, VMValue * valueOut)
{
	TESForm* targetForm = GetFormFromIdentifier(formIdentifier);
	if (!targetForm) {
		_WARNING("Warning: Cannot retrieve property value %s from a None form. (%s)", propertyName, formIdentifier);
		return false;
	}

	VirtualMachine* vm = (*g_gameVM)->m_virtualMachine;
	VMScript script(targetForm, scriptName);

	if (!script.m_identifier) {
		_WARNING("Warning: Cannot retrieve a property value %s from a form with no scripts attached. (%s)", propertyName, formIdentifier);
		return false;
	}

	// Find the property
	PropertyInfo pInfo = {};
	pInfo.index = -1;
	GetPropertyInfo(script.m_identifier->m_typeInfo, &pInfo, &BSFixedString(propertyName));

	if (pInfo.index != -1) {
		vm->GetPropertyValueByIndex(&script.m_identifier, pInfo.index, valueOut);
		return true;
	}
	else {
		_WARNING("Warning: Property %s does not exist on script %s", propertyName, script.m_identifier->m_typeInfo->m_typeName.c_str());
		return false;
	}
}

void logIfNeeded(std::string text) {
#ifdef _DEBUG
	if (logEnabled) {
		_MESSAGE(text.c_str());
	}
#endif
}

void killProcessByName(const char* filename)
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	while (hRes)
	{
		if (strcmp(pEntry.szExeFile, filename) == 0)
		{
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
				(DWORD)pEntry.th32ProcessID);
			if (hProcess != NULL)
			{
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
}



void logMessage(std::string aString)
{
	_MESSAGE(("[" + mName + "] " + aString).c_str());
}