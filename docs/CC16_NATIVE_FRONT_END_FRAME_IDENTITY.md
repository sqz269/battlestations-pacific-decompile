# Raw Frame identity Source leaf

This packet adds only `bsp::native_front_end_frame_id_00517d30() noexcept`, returning the actual unsigned DWORD `0x0c`. It is an ordinary noinline Source C++ function with no input, provider, allocation, lookup or published state. Baseline: `dab547a5a6cd1866b6f359dfc10455d9ab2d846b`; readiness input: immutable `0c5e889ffef0daf1457a0b4ec7ca0e484385189d`.

The complete native body is `00517D30..00517D35`, six bytes and two listed instructions:

```text
00517D30  B8 0C 00 00 00  MOV EAX,0Ch
00517D35  C3              RET
```

Fresh live bytes and the whole listing match the original PE. The configured and checked analysis target is `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Ghidra currently declares `undefined BSP_FrontEndFrame_GetScreenId(void)` with zero parameters and calls; the explicit EAX write establishes the DWORD result. There is no consumed ECX receiver, stack argument, data load, call, arithmetic, x87 or SSE instruction. Native RET performs no argument cleanup. The original `CEC39C` profile's current slot-zero identity is `00517D30`; the full retained ten-word profile is image evidence, never a callable host table.

The new interface is declared in `include/bsp/native_front_end_frame_identity.hpp` and implemented directly in `src/native_front_end_frame_identity.cpp`. No metadata lookup or semantic class is involved. Original virtual dispatch may carry ECX, which this native body ignores; the namespace C++ symbol uses the compiler's ordinary Win32 C++ ABI. The exact six emitted bytes do not establish a loaded-game replacement, Source vtable publication, private-entry compatibility at arbitrary call sites, or exception/SEH/fault transport.

Strict object-only compilation passed with MSVC `19.51.36244.0`, x86, `/std:c++17 /MD /O2 /Gy /fp:strict /EHsc /W4 /WX`. The sole external defined callable is `?native_front_end_frame_id_00517d30@bsp@@YAIXZ`, in a six-byte `.text$mn` COMDAT containing exactly `b80c000000c3`. It has zero code relocations, calls, undefined external symbols or other emitted code. The `.debug$F` metadata relocation refers to this callable and is not a call. No executable was linked or run; no new tests or full build were performed.

Retained static receipts are under `local/native_front_end_frame_identity/` in the named source worktree. `audit.json` SHA256 is `b584a11ca7dfffd3b35c50cf311e314306d60c6d2d5446e5d9a8d5d01e14bcfe`; the strict object SHA256 is `50a0594bf4e7aba202a0ecd5ca160f0cdc9b48dab1377c2bbd2facb54d697e5c`. The tracked report embeds native/profile bytes, source and artifact hashes, COFF symbol/section evidence and qualifications. Its parser treats uninitialized or unstored sections as having no stored raw bytes. Original image SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

This closes only the callable constant-return Source leaf. The current raw95 registry, genuine current vptr/slot-zero dispatch, writable name-table authority, all-row Page acquisition/retirement suppliers, creators, lifetimes and complete Frame registration remain open as recorded in `0c5e889`. The admitted `ed72` constructors/getter do not register a screen or initialize Frame `+0C..+160`. This packet supplies none of those missing producers and changes no caller, CMake entry, ledger, table, guard or Ghidra state. The readonly readiness tree and all frozen `ed72` receipts are unchanged.

Native/recovered/duplicate/vendor/runtime/activation credit is zero. Common integration, any complete caller binding, fixture and gameplay admission remain separate.
