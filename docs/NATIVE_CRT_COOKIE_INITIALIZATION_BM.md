# Native CRT cookie initialization and hook clear

This packet reconstructs the complete 148-byte `___security_init_cookie` body at
`00C1815E` and the complete eight-byte body at `00C04EF3`. It supplies two MSVC
Win32 source functions using borrowed writable state. It does not supply the
canonical cookie owner, `BFE120` cookie checking, `C185A4` failure reporting, or
`BF65BB` Watson handling. The interfaces are qualified source interfaces, not
native entry-point replacements.

Source: `include/bsp/native_crt_cookie_initialization.hpp` and
`src/native_crt_cookie_initialization.cpp`. Exact hashes, toolchain traces,
compiled bytes, archive proof and local evidence inventory are in
`reports/native_crt_cookie_initialization_bm.json`.

## Cookie behavior

Native `00C1815E..00C181F1` has no stack arguments and ends in plain `RET`.
The source function takes a cdecl context reference. Its two reference members
borrow the actual cookie word corresponding to `00E15590` and its distinct
complement word at `00E15594`; MSVC Win32 layout puts their pointer bindings at
offsets zero and four. Both bindings must remain stable and belong to the same
cookie domain. No private copies or binding to the host CRT cookie are created.
The original PE contains `BB40E64E` and `44BF19B1` at those addresses.

The initializer reads the current cookie before clearing the two FILETIME local
words. A current value unequal to `BB40E64E` with any high-16 bit set takes the
fast path: it writes only the complement, using the bitwise inverse of that
current value. Otherwise it calls these real KERNEL32 APIs in order:

| Native call | IAT slot | API |
| --- | --- | --- |
| `00C18193` | `00CE2100` | `GetSystemTimeAsFileTime` |
| `00C1819F` | `00CE21DC` | `GetCurrentProcessId` |
| `00C181A7` | `00CE223C` | `GetCurrentThreadId` |
| `00C181AF` | `00CE22AC` | `GetTickCount` |
| `00C181BB` | `00CE2270` | `QueryPerformanceCounter` |

It XORs both FILETIME words, PID, TID, tick count, and both performance-counter
words. The QPC output occupies the other eight bytes of the native 16-byte local
area and is deliberately not initialized. The BOOL result is ignored, including
on failure. Inline assembly retains those native local reads without expressing
an uninitialized C++ value read. No additional entropy source or repair appears.

An exact `BB40E64E` result becomes `BB40E64F`. Otherwise, a result with zero high
16 bits becomes `value | (value << 16)`; zero can remain zero. Publication writes
the actual cookie first, then its bitwise complement. The source preserves
EBP/EBX/EDI and the conditionally used ESI. It adds context loads and ECX/EDX
clobbers. Its 156 emitted bytes therefore do not assert native instruction,
volatile-register, stack-address, fault-continuation or entry ABI identity.

## Hook behavior

Native `00C04EF3..00C04EFA` is `AND dword ptr [0109EEA8], 0; RET` and does
not inspect its caller's reason argument. The source accepts that ignored reason
and an additional reference to the actual hook word. Its emitted ten bytes are
`50 8B 44 24 0C 83 20 00 58 C3`: save EAX, fetch the supplied address, perform
the real volatile read/modify/write, restore EAX, and return. This retains the
AND instruction's flag effects and EAX preservation. The extra saved word and
address load change the source stack and fault behavior; the source entry is
not ABI-identical to the original one-argument caller contract.

## Verification and limits

The base is `45980f8a3a0512d10cf4f874fff2fde1eb59ee5e`. The accepted Watson
discovery remains unchanged at `ed185cb2dfdf1ba045dbf30ebfd47f083eeece80`.
All 135 of its retained artifacts were checked by size, SHA256 and SHA512 before
reuse. Its complete 22 spans and 28 direct-call/tail rows remain discovery
evidence, not new implementations. Current target-verified Ghidra bytes and
the installed PE match both complete source bodies and the eight-byte cookie
pair. The complete source bodies contain no direct calls. Their five indirect
import sites are independently decoded from the original bytes and resolved
against the installed PE import table; they are not counted as direct-call
checker successes. No Ghidra names, comments or program state were changed.

MSVC x86 `19.51.36244.0`, toolset `14.51.36231`, built the packet through the
existing deferred CMake source registration with `/W4 /WX /fp:strict`. Retained
command/read/write records identify the exact source, header and output object.
The COFF object contains the two expected functions, exactly five ordered API
import relocations in the initializer, and no hook relocations. The full
retained `bsp_core.lib` contains exactly one matching source object member,
byte-identical to the compiler output. A compile-only layout inspection checks
the reference-binding offsets; no helper executable was run.

`verify-seeds` matched all eight existing seeds. `scripts/build.ps1` completed
successfully, including `reconstructed_math` and `native_math_differential`
(two of two CTests). No tests were added, and those existing math checks do not
exercise these APIs. There was no runtime, game or original-process execution
of either reconstructed routine. The complete local evidence directory is
inventoried in two independent SHA256/SHA512 passes.

The next owning prerequisite remains a canonical cookie/frame/error domain for
the native checking and reporting chain. This packet introduces neither that
owner nor substitute callbacks, exception objects or fallback state.
