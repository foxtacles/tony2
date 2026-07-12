#include "decomp.h"
#include "engine.h"
#include "gameobject.h"

// Third-party sound library ("DirectSound, V2.1 (Dec 10 1998)" banner region).
// Implemented functions live in soundlib.c (VC6-vintage TU); the entries below are
// the not-yet-implemented remainder.

// Null response handler installed for camera-type objects (linked among the CRT
// objects in the original).
// FUNCTION: TONY2 0x0043e6c0
TonyS32 __fastcall NullObjectHandler(GameObject* p_object)
{
	return 0;
}
