#ifndef REGISTRYSTORE_H
#define REGISTRYSTORE_H

#include "decomp.h"
#include "types.h"

// Registry-backed settings object (a buffer holding the subkey string under
// HKEY_LOCAL_MACHINE). Also used as the element type of VideoManager's
// name tables. Created by GameManager's constructor for g_settings.
// SIZE 0x100
class RegistryStore {
public:
	RegistryStore(char* p_subKey);

	TonyS32 ReadInt(char* p_name, TonyS32 p_default);
	void WriteInt(char* p_name, TonyS32 p_value);
	char* ReadString(char* p_name, char* p_default, char* p_buffer, TonyS32 p_size);
	void WriteString(char* p_name, char* p_value);

	char m_subKey[0x100]; // 0x00
};

void __fastcall NoOpHandler(void* p_object);

DECOMP_SIZE_ASSERT(RegistryStore, 0x100)

extern RegistryStore* g_settings;

#endif // REGISTRYSTORE_H
