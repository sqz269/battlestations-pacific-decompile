# String equality raw fields

Only the four field accesses inside `equal_native_string_headers_00435c40` now use the existing private fenced-byte `raw_header_word`/`raw_header_pointer` helpers. Capture left length then right length; unequal lengths return false and equal zero lengths return true before data reads. Otherwise capture right data before left data and call the same current `_stricmp` in the same argument order.

No API, comparison semantics, callback/provider, cleanup/EH, helper or other body changed. Exact full-file inverse projection preserves the unequal comparator, raw assignment/concat, typed overloads and every shared function literally. This removes the unnecessary live-C++-DWORD-field prerequisite for raw8B header bytes, without constructing NativeString or copying/owning either header.

Caller supplies valid live header/data backing and excludes conflicting writers/invalidation. Compiler barriers provide no synchronization, Native fault/SEH, private-frame or binary ABI guarantee. Current CRT comparison behavior is unchanged.

One strict Win32 object compile is recorded in the report. No tests/probe executable/game, Ghidra/CMake/ledger edit, new Native/vendor body credit or loading/startup/runtime claim. This is an AA09D0 prerequisite only, not raw loader or consumer activation.
