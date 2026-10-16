# Native CRT reverse byte search

Original address: `00BF87B0` (`_strrchr`). Coverage: **complete ordinary domain**,
one inclusive body range `00BF87B0..00BF87DC`, 45 bytes and 23 instructions.
The existing correct library name is retained. This packet supplies a new MSVC
Win32 Source interface; it does not claim original binary ABI compatibility.

`char* __cdecl bsp::find_native_crt_last_character_00bf87b0(char*, int)` accepts
the genuine passed pointer and stable integer value. The live native prototype
is `char* __cdecl _strrchr(char* _Str, int _Ch)`: it loads the pointer DWORD at
`EBP+8`, later reads the low BYTE at `EBP+12`, and returns raw pointer/zero in EAX
using plain RET and caller cleanup. Its first REPNE SCASB assumes entry DF=0 and
flat ES; STD reverses the second scan and both return branches join CLD.
Those native stack/register/flag/segment details are evidence, not a new Source
parameter, original compiler lowering, fault/EH identity or runtime proof.

The first NUL is at offset L in the same live allocation. All bytes at offsets
0..L must remain readable and stable. The NUL-inclusive count L+1 and 32-bit
extent do not wrap. Every int value is admitted: unsigned-byte conversion
selects the low eight bits, including negative and greater-than-255 values.
No alignment or readable padding beyond NUL is required. Stable readable
aliases are admitted, and a successful result borrows the actual input storage.
Null, unterminated, dangling, inaccessible or wrapping storage, concurrent
mutation, faults and volatile/MMIO protocols are outside this ordinary contract;
the implementation adds no guard or recovery for them.

| Original inclusive range | Instructions | Established behavior |
| --- | ---: | --- |
| `00BF87B0..00BF87B3` | 3 | Establish frame; save EBP and EDI. |
| `00BF87B4..00BF87B8` | 2 | Load pointer; clear EAX including AL. |
| `00BF87B9..00BF87BD` | 2 | ECX=0xffffffff; forward byte scan through first NUL. |
| `00BF87BE..00BF87C5` | 3 | Derive L+1; position at NUL. |
| `00BF87C6..00BF87CB` | 3 | Load target low byte; STD; backward byte scan. |
| `00BF87CC..00BF87D8` | 6 | Repair EDI; unconditionally reread final byte; select pointer/zero. |
| `00BF87D9..00BF87DC` | 4 | CLD; restore EDI/EBP; plain RET. |

Source reads actual payload bytes forward at 0,1,...,L regardless of the search
target, then converts the target. It reads backward at L,L-1,... until the first
match or exhaustion at zero, then unconditionally rereads that reached byte.
Consequently target zero returns the genuine first terminator, a nonzero match
returns its highest offset, and absence returns null after rereading byte zero.
Empty strings retain both terminator reads and the final repeated comparison.
Volatile unsigned-byte views retain that Source read sequence. Bounded unsigned
32-bit offset/count state forms only reached pointers at 0..L; it avoids the
native SCASB's transient before-base EDI value, including at offset zero.

No SDK strlen/strrchr/search replacement, single-pass last-match shortcut,
provider, invented host/context/global, locale/TLS, allocation, unresolved stub,
original caller binding, input write or word overread supplies the behavior.

Source authorization is recorded in `reports/cc12_native_crt_strrchr_readiness.json`
at Main `1e57eb5e5`. The prior readiness family remains immutable external
evidence. This phase records its own compiler inputs and raw tool provenance.
One fresh strict compile passed using MSVC 19.51.36244 with `/O2 /W4 /WX
/permissive- /std:c++17 /EHsc /MD`. The actual Source/header and 72 compiler
files were copied/pinned read-only before invocation; the compiler executed
from that frozen directory. Compiler dependency output lists only this header.
The actual strict invocation is chunk `3ffbb3`; its 913-byte object SHA256 is
`7395b26ec91eb0198f3d8aab79837e8eea387dbe0984fb8321a48d4962dbf20c`.

The complete current COFF audit covers all five sections, 13 symbol records
(eight primary/five auxiliary), the entire 62-byte string table, all relocations,
directives, and all 64 code bytes/28 instructions including the alignment NOP.
There are no calls, undefined external symbols, code relocations or EH/runtime
helpers. `.drectve` requests MSVCRT and OLDNAMES; these are retained directives,
not call dependencies. `.debug$S`, `.debug$F` and `.chks64` are retained metadata.
The sole relocation is `.debug$F` DIR32NB to this function; FPO metadata reports
no SEH. Complete dumpbin ALL/DISASM and independently parsed raw records are
retained. Current emitted payload reads retain both passes and the final reread;
the typed stable character argument is not an original argument-home protocol.

This document and `reports/native_crt_strrchr.json` pass the live call checker
with zero recognized call rows and zero failures. A call-free leaf has no host
call rows; zero-row checking does not establish semantics or caller bindings.
Own Source-phase evidence is sealed under the worker's
`local/WholeNativeCrtStrrchrSource01/root_manifest.json`, including precompile
pins, current object, complete COFF data and this phase's own raw tool records.
The required normal Main build and existing three checks follow registration
by the integrator. No worker baseline, new tests, probes or body execution are
requested or claimed. Source/build/COFF checks do not establish original ABI,
native execution or gameplay validation.
