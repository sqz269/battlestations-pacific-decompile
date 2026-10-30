# Raw assignment and concatenation field access

Only the NativeStringRawPoolContext overloads of 425F40 assignment and4261A0 concatenation now access their actual8B header fields through private fenced four-byte memcpy helpers. Pointer fields are raw DWORD loads followed by pointer conversion; no live C++ primitive field object, NativeString lifetime, new callback/provider or semantic cast is introduced. Raw concat's two initial zero stores also use these helpers rather than the unchanged typed shared clear_header.

Assignment still skips all header access on identity. Otherwise it reads source length, resizes, then reloads source length and reads destination length, source data, destination data in that exact order. Concat captures identity before clearing output even on alias; initial resize/copy remains unowned and reloads length, output data BEFORE left data. Right appended length is captured before the owned-output try; append reloads right data BEFORE output data after resize. Existing memmove/zero-byte omission/unsigned arithmetic behavior is unchanged.

Concat initial copy failure adds no output cleanup. Entering the existing append catch consumes its sole output edge before the real raw41DD20 call. That cleanup may throw a newer exception that replaces the original; there is no retry or EH repair. All existing rawresize/header-destruction/CRT/alias/NativeFH3/fault qualifications remain; outer fences do not upgrade their transitive contracts.

The shared word/put/pointer/clear helpers, typed overloads and every other body remain literal unchanged. Exact complete-file inverse projection is recorded with the one strict MSVC Win32 object compile. Caller supplies live addressable header/data/context backing without conflicting writers or invalidating callbacks; compiler barriers are not synchronization or an ABI guarantee.

No new Native entry/body/vendor credit, shared CMake/ledger/Ghidra edit, test/probe executable/game, consumer/startup/loaded-locale or runtime readiness is claimed.
