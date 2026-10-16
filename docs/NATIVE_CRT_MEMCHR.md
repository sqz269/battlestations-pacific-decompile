# Native CRT counted byte search at 00BFA700

This packet reconstructs the whole ordinary behavior of the Original library
function `_memchr`, at `00BFA700..00BFA7AC`: 173 contiguous bytes and 76
instructions, body SHA-256
`4e96ec5152dff3c4648f00572c6ed10d221e170be7101db0a0a0d6fe9e7b503f`.
The correct Original library name remains `_memchr`. The new interface is
`void* __cdecl bsp::find_native_crt_byte_00bfa700(void*, int, unsigned int)`.
It has no registration, Original caller binding, provider, global, fallback or
search-helper dependency. Root owns registration, annotations and Main integration.

The authoritative Source gate is
`reports/cc12_native_crt_memchr_readiness.json`. Its author family is
`J:/PROG/battlestations-pacific-decompile-cc12_crt_strrchr_readiness_oct16/local/NativeCrtMemchrReadiness01/root_manifest.json`
(SHA-256 `ac5185f9b22f868b88aa46832eba86adb253ab90cb12aa8223bbd5761687fea9`),
and its independent peer family is
`J:/PROG/battlestations-pacific-decompile-cc12_library_insert_resume/local/NativeCrtMemchrIndependentReview01/root_manifest.json`
(SHA-256 `8f9746d592b0fdb623608b2131b53ac9a42b59ed22ba5fb8509e20d4b5c4d7e6`).
Those families remain external evidence. This worker's own disk observation,
compilation, inspection and task provenance live separately in
`local/WholeNativeCrtMemchrSource01/`.

## Ordinary domain and result

This API targets MSVC Win32, 8-bit bytes, 32-bit pointer/int/unsigned int and
little-endian storage. Count zero admits any pointer, including null, and returns
null before the Source converts the target or reads payload. A positive count
requires genuine live storage whose requested bytes are determinate, readable
and stable. The requested endpoint may be inside or one past that same actual
allocation: interior subranges and prefixes are admitted. The actual 32-bit
extent and pointer advances must not wrap. There is no signed-count cap, NUL,
padding, initial-alignment or typed-DWORD-allocation requirement. Every `int`
selects its low eight bits, including negative values.

The result is the actual first matching byte's pointer or null; it borrows the
original allocation's lifetime. Stable read-only payload aliases are admitted.
The routine writes no payload. Mutation, concurrency, invalid positive-count
storage, faults and volatile/MMIO observation protocols are outside this domain.

## Whole-body reconstruction

| Original sites | Established behavior |
| --- | --- |
| `00BFA700..00BFA719` | Read count first; zero returns before buffer and target homes. Test actual buffer alignment. |
| `00BFA71B..00BFA72F` | Peel individual BYTEs to actual four-byte alignment, with first-match and exhaustion exits. |
| `00BFA731..00BFA745` | Admit a word only with at least four remaining bytes; repeat the low target byte in all four lanes. |
| `00BFA762..00BFA77B` | One aligned in-range DWORD, XOR with the repeated target and apply the unsigned 32-bit candidate filter. |
| `00BFA77D..00BFA793` | On a nonzero candidate only, reread that same DWORD once and compare register lanes from low to high. Reject false positives. |
| `00BFA75D..00BFA760` | Continue only while another complete word remains. |
| `00BFA747..00BFA759` | Restore the zero-to-three-byte residual and read ordered BYTEs. |
| `00BFA75B..00BFA75C`, `00BFA795..00BFA7AC` | Return null or the genuine matching byte address, with no final payload reread. |

The pseudocode widens some peel/tail accesses incorrectly; the complete listing
establishes their BYTE width. The Original has no calls, SSE/CPUID dispatch,
global access or DF/ES search-instruction dependency. Descriptive C++ names are
new interpretations rather than recovered symbols.

For each complete word, let `x = word ^ repeated_target`. The retained filter is
`((x + 0x7efefeffU) ^ ~x) & 0x81010100U`, entirely unsigned 32-bit arithmetic.
The accepted static readiness proof establishes that it cannot miss a zero XOR
lane. Its no-match false-positive class has high XOR byte `0x80`; that class
must still trigger the same-word reread and then fail all four comparisons.
A zero candidate has no reread. A nonzero candidate has exactly one reread,
followed solely by register comparisons. A word can include later requested
bytes beyond its first match. No extra final BYTE read is introduced.

The Source maintains a genuine byte pointer and bounded unsigned residual count.
Every BYTE read has a positive residual; each DWORD read has at least four
remaining bytes and actual address alignment. Advances stay inside the requested
region or at its endpoint. It creates no before-base pointer, integer-underflow
access, pointer subtraction, runtime validation or extra ownership condition.

## Load mechanism and validation boundary

Peel and tail accesses use `const volatile unsigned char` reads to retain their
width and ordering in the actual compiler result. A private target-specific
MSVC Win32 inline-assembly helper performs an addressed `MOV` of one DWORD over
genuine byte storage. It does not dereference an invented `unsigned int` object,
so it adds no such object-lifetime or aliasing precondition. This mechanism is
the specific Source-gate exception to portable ISO C++ and requires inspection
of each actual compiled object.

The worker performs one strict compile with `/std:c++17 /O2 /W4 /WX
/permissive- /EHsc /MD`, plus evidence-output switches. Source, header, compiler
family, candidate include/SDK/support inputs and the capture driver are hashed
before that invocation. Actual `/showIncludes` and `/sourceDependencies` inputs
must be covered by those genuine before hashes. The report records actual
compiler/object facts and the complete code, data, relocation, symbol, metadata,
string and public-binding inventory.

The actual sole compile passed with compiler version `19.51.36244.0`, with no
warning or error diagnostics. Its 1326-byte COFF object has SHA-256
`83da982660ed525141ad29460028f44f71859bbf8d94b88f3cab02fe6eef6256`.
The public body is 236 bytes / 97 instructions. Its complete payload-load
inventory is BYTE at section-3 offset `0x23`, DWORD at `0x54`, conditional
same-address DWORD at `0x75`, and BYTE at `0xb0`. The zero-candidate branch at
`0x6f` skips the second DWORD load; the four comparisons at `0x7f`, `0x88`,
`0x91` and `0x98` use only the reread register value. Count zero branches at
`0x0c` to the null return before loading buffer or target homes. The unsigned
word gates are at `0x49`/`0xa6`, and all first-match exits add only the matching
lane offset. There is no final payload reread or payload store.

The compiler also emitted an unreferenced private 15-byte / 7-instruction helper
COMDAT. It contains one addressed DWORD load and stack-only spills; the public
body has inlined both helper uses. Both complete code sections contain no call,
indirect jump, global reference, search instruction, SSE/CPUID instruction or
code relocation. There is one defined external symbol, the new C++ API, and no
undefined external symbol. The only two relocations are `DIR32NB` entries in
discardable FPO metadata, one for each function. `/MD` emits the usual
`MSVCRT`/`OLDNAMES` default-library directives without adding a search binding.
The other sections are compiler/object-name CodeView metadata, two FPO records
and compiler checksums; no runtime data section or payload reference is present.
The inventory records every raw section byte, symbol/auxiliary record, relocation
and printable string (22 ASCII runs, no UTF-16 runs).

This strict body uses FPO: eight local bytes, three saved registers, a ten-byte
prologue, 12 argument bytes and plain `ret`; the private helper has four local
bytes and a four-byte argument. It uses stack spills and may reuse a by-value
argument home, so it does not preserve the Original physical frame/register
arrangement. The actual two-file dependency closure is the Source and owned
header; both match the hashes recorded before compilation. Two preparation
failures preceded the compile: command-line quoting during environment setup,
then handling the peer manifest's `files` schema. Their genuine current-task tool
results are retained. Neither invoked the compiler; no compilation was replayed.

No new tests, probes, target-body execution or SDK comparison are performed.
The report-call checker verifies call-claim syntax only; zero call rows do not
prove these semantics. The ordinary Source API does not establish Original
physical flags/register/home/stack/EH/fault ABI identity, binary replacement,
Native execution or game validation. Root's registered Main build and actual
Main object review remain separate. Main's `/Oy-` frame choice can differ from
this strict object's compiler-selected frame without justifying a matching
recompile.
