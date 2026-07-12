#ifndef MOVIEIFACES_H
#define MOVIEIFACES_H

#include "decomp.h"
#include "types.h"

#include <ddraw.h>
#include <objbase.h>

// ActiveMovie (quartz IAMovie-family) COM interfaces used by the MPEG intro player
// (CreateMovieGraph/RunMovieGraph). Only the slots the game calls carry real signatures.
struct IMovieGraph {
	virtual TonyS32 __stdcall QueryInterface(const GUID& p_riid, void** p_object) = 0; // 0x00
	virtual TonyU32 __stdcall AddRef() = 0;                                            // 0x04
	virtual TonyU32 __stdcall Release() = 0;                                           // 0x08
	virtual TonyS32 __stdcall Slot3() = 0;                                             // 0x0c
	virtual TonyS32 __stdcall GetFilter(const GUID& p_riid, IUnknown** p_object) = 0;  // 0x10
	virtual TonyS32 __stdcall Slot5() = 0;                                             // 0x14
	virtual TonyS32 __stdcall Poll() = 0;                                              // 0x18
	virtual TonyS32 __stdcall Run(TonyS32 p_a) = 0;                                    // 0x1c
	virtual TonyS32 __stdcall GetSurface() = 0;                                        // 0x20
	virtual TonyS32 __stdcall Slot9() = 0;                                             // 0x24
	virtual TonyS32 __stdcall Slot10() = 0;                                            // 0x28
	virtual TonyS32 __stdcall Slot11() = 0;                                            // 0x2c
	virtual TonyS32 __stdcall Setup(TonyS32 p_a, TonyS32 p_b, TonyS32 p_c) = 0;        // 0x30
	virtual TonyS32 __stdcall GetSurfaceProvider() = 0;                                // 0x34
	virtual TonyS32 __stdcall Slot14() = 0;                                            // 0x38
	virtual TonyS32 __stdcall AddRenderer(void* p_object, const GUID& p_riid, TonyS32 p_c,
										  TonyS32 p_d) = 0;                // 0x3c
	virtual TonyS32 __stdcall RenderFile(LPCWSTR p_path, TonyS32 p_b) = 0; // 0x40
};

struct IMovieSurface;

// Video renderer accessor obtained through QueryInterface on the graph's helper.
struct IMovieFinder {
	virtual TonyS32 __stdcall QueryInterface(const GUID& p_riid, void** p_object) = 0; // 0x00
	virtual TonyU32 __stdcall AddRef() = 0;                                            // 0x04
	virtual TonyU32 __stdcall Release() = 0;                                           // 0x08
	virtual TonyS32 __stdcall Slot3() = 0;                                             // 0x0c
	virtual TonyS32 __stdcall GetFilter() = 0;                                         // 0x10
	virtual TonyS32 __stdcall Slot5() = 0;                                             // 0x14
	virtual TonyS32 __stdcall Poll() = 0;                                              // 0x18
	virtual TonyS32 __stdcall Run() = 0;                                               // 0x1c
	virtual TonyS32 __stdcall GetSurface() = 0;                                        // 0x20
	virtual TonyS32 __stdcall Slot9() = 0;                                             // 0x24
	virtual TonyS32 __stdcall Slot10() = 0;                                            // 0x28
	virtual TonyS32 __stdcall Slot11() = 0;                                            // 0x2c
	virtual TonyS32 __stdcall Setup() = 0;                                             // 0x30
	virtual TonyS32 __stdcall GetSurfaceProvider(
		TonyS32 p_a,
		TonyS32 p_b,
		TonyS32 p_c,
		IMovieSurface** p_object
	) = 0; // 0x34
};

// Movie surface provider: polled during playback and queried for the decode surface.
struct IMovieSurface {
	virtual TonyS32 __stdcall QueryInterface(const GUID& p_riid, void** p_object) = 0; // 0x00
	virtual TonyU32 __stdcall AddRef() = 0;                                            // 0x04
	virtual TonyU32 __stdcall Release() = 0;                                           // 0x08
	virtual TonyS32 __stdcall Slot3() = 0;                                             // 0x0c
	virtual TonyS32 __stdcall GetFilter() = 0;                                         // 0x10
	virtual TonyS32 __stdcall Slot5() = 0;                                             // 0x14
	virtual TonyS32 __stdcall Poll(TonyS32 p_a, TonyS32 p_b, TonyS32 p_c,
								   TonyS32 p_d) = 0;                                        // 0x18
	virtual TonyS32 __stdcall Run() = 0;                                                    // 0x1c
	virtual TonyS32 __stdcall GetSurface(LPDIRECTDRAWSURFACE* p_surface, RECT* p_rect) = 0; // 0x20
};

#endif // MOVIEIFACES_H
