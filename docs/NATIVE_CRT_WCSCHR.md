# Native CRT wide character search at 00BFD6C1

This reconstructs the whole ordinary behavior of the Original library function
`_wcschr`, at `00BFD6C1..00BFD6E2`: 34 contiguous bytes and 14 instructions,
body SHA-256 `e4b72b2afb31591ec009972ea1f43046210cdb6eee393458328393a9c29f915e`.
Keep the correct Original library name `_wcschr`. The new, default potentially
throwing interface is
`void* __cdecl bsp::find_native_crt_wide_character_00bfd6c1(void*, int)`.
Source credit remains zero pending Root's independent/compiler/Main gates.

The authoritative Source gate is `reports/cc12_native_crt_wcschr_readiness.json`,
read after the clean pinned fast-forward to Main
`2289532a8555ac0f2e8915069bb4fc34800095fe`. Its canonical packet owns the entry,
entire 34-byte inclusive range and exactly four Source/header/doc/report outputs.
Root owns registration, ledgers, annotations and Main integration.

The external 40-file author readiness manifest is
`J:/PROG/battlestations-pacific-decompile-cc12_crt_memchr_source_oct16/local/NativeCrtWcschrReadiness01/root_manifest.json`
(SHA-256 `ccc6e6a7796efad90d2d7afd751a52b6893cf2071b8e5b2b06749f0c277fc59d`),
and the external 13-file independent manifest is
`J:/PROG/battlestations-pacific-decompile-cc12_library_insert_resume/local/NativeCrtWcschrIndependentReview01/root_manifest.json`
(SHA-256 `9f6909a5440755c80906eb4a752b8181101834837a7272fa37a3b5dc01372298`).
They remain readiness evidence, without supplying this compile's provenance.
Fresh own Source3 evidence is in `local/WholeNativeCrtWcschrSource01/`.

## Ordinary storage and result

The target is MSVC Win32: BYTE8, pointer/int32, little-endian unsigned WORD16
code units. Every `int` selects its low 16 bits modulo 65536. There is no Unicode
scalar/surrogate/normalization, case, locale or collation interpretation.

Require a finite first reached unit equal to the target or NUL. Every reached
two-byte read, including the terminal NUL reread, must be wholly inside genuine
live stable determinate readable storage in the same actual allocation. An
earlier nonzero match requires no unused extent or later NUL promise. Null or
invalid input is outside the ordinary domain; no new guard is introduced.
Admit possibly unaligned raw byte storage, prefixes/interior regions and stable
read-only aliases. Reached advances by two do not wrap and stay within or one
past that same allocation; never read one-past. There is no typed `wchar_t`/WORD
ownership, initial alignment, padding, signed-count cap or copied allocation.

The result is the genuine first matching unit's borrowed pointer or null.
Target zero returns the actual first NUL pointer. No ownership or lifetime is
transferred. The routine writes no payload. Mutation/concurrency, faults,
indeterminate bytes, MMIO/volatile observation protocols and lifetime changes
are excluded. No SDK `wcschr`/`memcpy`/search, provider, global/TLS, fallback,
runtime validation or Original caller binding is introduced.

## Complete Original schedule

| Original sites | Established behavior |
| --- | --- |
| `00BFD6C1`, `00BFD6C5` | Load pointer DWORD from `[ESP+4]`, then target WORD from `[ESP+8]`. Only DX matters; upper EDX is unused. |
| `00BFD6CA`, `00BFD6D3..00BFD6D9` | Reach the first/next unsigned WORD read and test NUL before nonzero target equality. |
| `00BFD6CC..00BFD6CF` | For a nonzero unit compare CX/DX in registers; a match returns without rereading payload. |
| `00BFD6D1..00BFD6D2` | Only an unequal nonzero unit advances the actual address twice by one byte, net two. |
| `00BFD6DB..00BFD6DE` | NUL always triggers one fresh same-address WORD comparison, including target zero, then returns the pointer if equal. |
| `00BFD6E0..00BFD6E2` | On terminal reread unequal to target, clear EAX and return null; common plain RET. |

The Source retains exactly one WORD read per visited unit, tests NUL first,
returns a nonzero match without rereading, and spells the terminal NUL reread as
a separate operation. It performs no after-NUL read or unconditional final
reread. Under stable storage, the NUL reread yields zero: low16 target zero
returns that pointer; other targets return null. The reread remains present.

The Original is a leaf cdecl routine with no stack allocation, saved nonvolatile
registers, calls, globals, hidden register inputs, x87, SSE/CPUID, DF/ES search
dependency, EH frame or no-return annotation repair issue. Its two native stack
homes, load order and EAX result are retained as evidence. Descriptive C++ names
are new interpretations. The new int/void-pointer interface does not supply
Original physical ABI/register/home/stack/flags/segment/EH/instruction/bus/fault
identity or a drop-in binary replacement.

## Approved target mechanism and actual strict object

A private MSVC Win32 inline-assembly helper performs addressed
`MOVZX EAX, WORD PTR [EAX]` over genuine raw storage. It fully initializes the
32-bit result, without inventing a typed WORD object's lifetime/aliasing or
alignment requirement. This is the specifically approved target mechanism,
without a portable ISO C++ claim. Both inlined uses and the additional emitted
private helper require actual-object review.

The sole strict compile passed `/std:c++17 /O2 /W4 /WX /permissive- /EHsc /MD`
using compiler `19.51.36244.0`, without warning/error diagnostics. Its 1164-byte
COFF object SHA-256 is
`466053a3d1bfb1cff7ac12cbe70aacdff72166c117571ff5666b8064e3d365ab`.
Fresh before hashes cover Source/header, compiler family, SDK/include/support
candidates and the capture/inspection tools: 9265 files. The actual
`/showIncludes` and `/sourceDependencies` closure contains exactly the Source
and owned header, both unchanged from those genuine before hashes. `/Bv` stderr
is compiler-pass/version metadata. There were no preparation/compiler failures
and no compile replay.

The entire public body is 64 bytes / 22 instructions. Target-low16 is implemented
by `MOVZX EDX,WORD PTR [ESP+8]` at section-3 offset `0x04`. Its only two payload
loads are `MOVZX EAX,WORD PTR [EAX]` at `0x11` (visited unit) and `0x2d` (NUL-only
same-address reread). Test/branch `0x1c`/`0x1e` sends zero directly to the reread.
Nonzero comparison/branch `0x20`/`0x22` sends a match to the actual pointer return
at `0x3d`, bypassing the reread. Only an unequal nonzero unit advances ECX by two
at `0x24` and loops. The terminal compare `0x34`/`0x38` selects pointer/null
after the fresh WORD. No guard, widened payload access, byte replacement,
payload store or extra final read exists.

The compiler inlines both public helper uses and emits an unreferenced static
16-byte / 7-instruction helper COMDAT, with one WORD load at section-5 offset
`0x05`. Both complete code sections have no call, indirect jump, code relocation,
global or provider binding. The sole external symbol is the new C++ API; no
undefined external symbol exists. Every raw section byte, all 18 symbol/auxiliary
records, 109 string-table bytes, 23 ASCII runs and zero UTF-16 runs are inventoried.

The seven sections are the two code COMDATs, `/MD` default-library directives
for `MSVCRT`/`OLDNAMES`, compiler/object-name CodeView metadata, two discardable
FPO records and compiler checksums. The only relocations are two FPO `DIR32NB`
function-address entries. There is no runtime data section or search binding.
Public FPO records zero locals/saved registers/prologue and eight argument bytes;
it has no stack adjustment, EBP or SEH, and plain `ret`. It reuses by-value
argument homes for pointer/value spills. The private helper has four local bytes
and a four-byte argument. These compiler homes differ from the Original and do
not preserve stack alias/fault protocols. Main's future `/Oy-` variant needs its
own actual frame/load review; no matching recompile is forced.

## Preservation, provenance and remaining gates

Pre-merge preservation pins, post-merge checks and fresh precompiler pins are
distinct stages. The earlier eight actual working files retain their exact
bytes, hashes and mtimes across the pinned fast-forward. Existing Git LF/CRLF
normalization in the earlier two JSON reports is recorded accurately; neither
report is rewritten. All seven earlier families, plus the 13-file readiness
peer, remain unchanged external evidence. Later report/commit/seal tools are
pinned at their actual creation/use, without pretending they preceded compilation.
Only new Source3-phase own tool records are retained and authenticated to the
full Original session prefix; no prior Source2/Root/peer compiler history is used.

No separate unregistered whole-worker baseline, new tests, probes, body execution
or SDK comparison is performed. The call checker checks call claims only; zero
rows does not prove semantics. Root's independent Source/object review,
registration, compiler/Main build, actual Main variant and archive gates remain.
The default potentially throwing signature is preserved. No runtime/game or
Original binary ABI credit follows from this static/compile/object evidence.
