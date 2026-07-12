#ifndef COMPAT_H
#define COMPAT_H

// Various macros to enable compiling with other/newer compilers.

#define MSVC600_VERSION 1200

#if defined(__MINGW32__) || defined(__clang__) || defined(__GNUC__) || (defined(_MSC_VER) && _MSC_VER > MSVC600_VERSION)
#define COMPAT_MODE
#endif

#ifdef COMPAT_MODE
// The game links MFC statically (nafxcw); newer toolchains default to the DLL CRT, which makes
// <afx.h> reject a static-MFC build unless _AFXDLL is set. Only relevant when compiling with a
// newer compiler than the original VC5/VC6; the decomp build is unaffected.
#ifndef _AFXDLL
#define _AFXDLL
#endif
// Restore the classic three-argument swprintf/vswprintf that the game code uses; newer CRTs hide
// it behind this macro in favour of the count-taking secure variant.
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif
#endif

// Disable "identifier was truncated to '255' characters" warning.
// Impossible to avoid this if using STL map or set.
// This removes most (but not all) occurrences of the warning.
#pragma warning(disable : 4786)

// We use `override` so newer compilers can tell us our vtables are valid,
// however this keyword was added in C++11, so we define it as empty for
// compatibility with older compilers.
#if __cplusplus < 201103L
#define override
#define static_assert(expr, msg)
#endif

#endif // COMPAT_H
