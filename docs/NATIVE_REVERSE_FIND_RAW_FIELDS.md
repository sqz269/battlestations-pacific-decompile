# Reverse-find raw header field access

The existing `reverse_find_native_string_bytes_004bcb80(const void*,...)` overload now reads only data+4 then length+0 through four-byte memcpy bracketed by MSVC compiler barriers. A null captured data pointer still returns before reading length. Count, set, signed start, clamp, strlen and reverse byte scan order are unchanged. The typed overload still delegates unchanged.

This removes the unnecessary live-C++-DWORD-field prerequisite for the actual raw8B input. The helper's local DWORD is a real C++ object; no NativeString object/header copy, ownership, provider or initialization is fabricated. Caller still supplies valid raw header/data/set backing and externally excludes conflicting writers or invalidating callbacks. Compiler barriers provide no synchronization, stronger SEH/fault guarantee or binary ABI.

Exact inverse removal/restoration recovers the complete baseline CPP bytes, including every BDEF90 and other function body. The only additions beyond the two field loads are the private helper and intrin include. This claims no new Native entry, behavior, vendor body or raw BDEF90 transport.

One strict MSVC Win32 object compile is recorded in the report. No tests/probe executable/game, Ghidra/CMake/ledger edit, new callback/provider, loaded locale or startup/runtime readiness. Existing supported pointer/byte-scan and current CRT behavior qualifications remain.
