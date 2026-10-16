# Native CRT counted wide comparison at 00BFDCB7

This is the whole ordinary-domain Source reconstruction of the Original library
function `_wcsncmp`, at `00BFDCB7..00BFDCEB`: 53 contiguous bytes, 26 instructions,
body SHA-256 `971cfb19ceeeeeff5d9b47a8eb83c1821a559f274979cfd27913f41932e226a2`.
The correct Original library name remains `_wcsncmp`. The new, default potentially
throwing interface is
`int __cdecl bsp::compare_native_crt_wide_prefix_00bfdcb7(const void*, const void*, unsigned int)`.
Source credit remains zero until Root completes its acceptance/compiler/Main gates.

The authoritative Source gate is
`reports/cc12_native_crt_wcsncmp_readiness.json`, incorporated from Main
`02b89a4acfac4138c2e375a4d06668ce16e3cc50` through a clean merge into this
worker branch. The external author manifest is
`J:/PROG/battlestations-pacific-decompile-cc12_crt_strrchr_readiness_oct16/local/NativeCrtWcsncmpReadiness01/root_manifest.json`
(SHA-256 `d88a3a53f33f26c72c07ea7f4c3859b8e8135ad8a5516196ff31c669362c958c`),
and the external independent manifest is
`J:/PROG/battlestations-pacific-decompile-cc12_library_insert_resume/local/NativeCrtWcsncmpIndependentReview01/root_manifest.json`
(SHA-256 `dd695bb748732eb9c1926307fae8c34c4e6258a5cd68d69018e85c5241ff1a8c`).
These 23/8-file families are external evidence, not this worker's compilation
provenance. Its new own Source2 evidence is `local/WholeNativeCrtWcsncmpSource01/`.
The earlier 57-file `_memchr` Source family and all four earlier tracked files
remain unchanged. Root owns registration, ledger, annotations and Main integration.

## Ordinary domain

The target is MSVC Win32: 8-bit bytes, 32-bit pointers/int/unsigned count and
little-endian unsigned 16-bit code units. There is no Unicode scalar, surrogate,
normalization, locale, case or collation interpretation. Count zero admits any
pointer values, including null, and returns zero without payload reads.

For positive count, only the actually reached WORD reads must have genuine live,
stable, determinate, readable storage. Each reached two-byte access lies wholly
inside its actual allocation, including both final rereads. An early LHS zero
or mismatch does not require the unused count-sized extent. Reached pointer
advances do not wrap and remain inside or one past the same actual allocation;
one-past is never dereferenced. Unaligned raw byte-region addresses and stable
identical/overlapping read-only aliases are admitted. No typed `wchar_t`/WORD
object ownership, initial alignment, padding or arbitrary signed-count cap is
introduced. No copies, runtime validation, fallback, provider, global/TLS,
SDK `wcsncmp`/`memcpy`, byte-only substitute or Original caller binding is added.

Invalid reached accesses, mutation, concurrency, indeterminate bytes, faults,
MMIO/volatile observation protocols and lifetime changes are outside this domain.

## Whole Original read schedule and count-home boundary

| Original sites | Established behavior |
| --- | --- |
| `00BFDCB7..00BFDCC3` | Establish EBP frame; count-zero return with no payload reads. |
| `00BFDCC4`, `00BFDCC7` | Load the RHS pointer home before the LHS pointer home. |
| `00BFDCCA..00BFDCCD` | Decrement the by-value count home before preliminary reads; zero goes directly to the final pair. |
| `00BFDCCF..00BFDCD5` | Preliminary LHS WORD; LHS zero skips preliminary RHS. |
| `00BFDCD7..00BFDCDA` | Preliminary RHS WORD equality comparison; mismatch goes to the final pair. |
| `00BFDCDC..00BFDCE0` | Only an equal nonzero pair advances each pointer by two, then repeats. |
| `00BFDCE2..00BFDCEB` | Fresh LHS WORD, then fresh RHS WORD, zero-extend both and return their exact 32-bit difference. |

All 26 Original instruction rows are retained in the own disk observation. It has no
calls, globals, search instructions, SSE/CPUID or locale dependencies. Names of
new C++ locals/helper are interpretations, not recovered symbols.

Original `DEC DWORD PTR [EBP+0x10]` at `00BFDCCA` writes its own by-value count
argument home on every visited iteration. The new typed API uses ordinary local
unsigned count state. Original mutable argument-home/stack alias protocols,
their physical writes and fault ordering are explicitly outside equivalence.
No native argument block, stack identity or extra Source parameter is invented.
The RHS-home-before-LHS-home observation is preserved as Original evidence;
the new compiler's physical pointer-home order is not asserted to match it.

The exact positive read schedules at the stopping position are:

| Trigger | Payload reads at that position, in order |
| --- | --- |
| Remaining becomes zero after decrement (including count one) | Final LHS WORD, final RHS WORD only. |
| Preliminary LHS is zero | Preliminary LHS WORD, final LHS WORD, final RHS WORD. |
| Preliminary mismatch | Preliminary LHS WORD, preliminary RHS WORD, final LHS WORD, final RHS WORD. |
| Preliminary equal nonzero pair | LHS WORD, RHS WORD, then advance both raw pointers by two and repeat. |

The final result subtracts zero-extended WORD values after widening to signed
32-bit `int`; its exact range is `[-65535,65535]`. It is neither a normalized
sign nor signed-16-bit subtraction. Count state is decremented only after
positive entry or a prior nonzero residual; it cannot cause a wrapped read gate.

## Target load mechanism and actual strict object

A private MSVC Win32 inline-assembly helper performs `MOVZX EAX, WORD PTR [EAX]`
over actual raw byte storage. It fully initializes the 32-bit result and adds no
typed-WORD lifetime/aliasing or alignment precondition. Each helper read is a
separate sequenced Source operation. This approved target mechanism is not a
portable ISO C++ claim; actual emitted width, order and rereads require review.

The single strict compile passed `/std:c++17 /O2 /W4 /WX /permissive- /EHsc /MD`
with compiler `19.51.36244.0`; `/Bv` stderr contains version/pass inventory only.
Source/header, compiler family, SDK/include candidates, support DLL candidates
and capture/inspection tools were hashed before that genuine invocation.
The actual `/showIncludes` and `/sourceDependencies` closure is exactly the Source
and owned header, both covered by unchanged before hashes. One preparation guard
initially compared the prior report's Git-normalized blob directly to its
unchanged CRLF working bytes. It failed before compilation; the guard was
corrected to check both the existing blob normalization and the earlier sealed
working-byte/hash/mtime evidence. The original failure is retained. No compiler
failure or compilation replay occurred.

The actual 1242-byte COFF object SHA-256 is
`c2ed927a2f2b86bde07c3e295b51b964526a271a588709ae215e3677cd238979`.
Its public body is 139 bytes / 46 instructions. All public payload loads are
zero-extending WORD loads at section-3 offsets `0x34` (preliminary LHS), `0x47`
(preliminary RHS), `0x6c` (final LHS) and `0x77` (final RHS). The count gate at
`0x23`/`0x26` precedes preliminary reads; the loop decrement at `0x62`/`0x65`
precedes each later preliminary pair. LHS-zero branch `0x41` and mismatch branch
`0x52` converge through `0x67` to the fresh final pair; count one jumps directly
to `0x68`. Only the equal nonzero path advances both pointers at `0x54`/`0x57`.
Final subtraction at `0x82` uses the two full zero-extended values.

The compiler emits an unreferenced static helper COMDAT, 16 bytes / 7 instructions,
with its sole WORD load at section-5 offset `0x05`. All four public uses are
inlined. Both code sections have no calls, indirect branches, code relocations,
payload stores, globals, search instructions or SSE/CPUID. The encoded multi-byte
NOP at public offset `0x29` has a syntactic memory operand and performs no payload
read. The sole defined external symbol is the new C++ API; no undefined external
symbol exists. All 18 symbol/auxiliary records, 108 string-table bytes, 22 ASCII
runs, zero UTF-16 runs and every section byte are inventoried.

The seven sections contain `/MD` default-library directives for `MSVCRT` and
`OLDNAMES`, compiler/object-name CodeView metadata, the two code COMDATs, their
two discardable FPO records and compiler checksums. The only two relocations are
FPO `DIR32NB` function-address entries; no runtime data section or search binding
is present. Public FPO metadata records 12 local bytes, one saved register and
a four-byte prologue, 12 argument bytes, no EBP/SEH and plain `ret`. The public
body additionally saves/restores ESI only around its preliminary loop. The
private helper has four local bytes, a one-byte prologue and four argument bytes.
Stack spills and argument-home reuse differ from the Original physical frame.

The default potentially throwing Source signature is preserved. No Original
physical flags/register/home/stack/segment/EH/fault ABI identity, drop-in binary
replacement, runtime or game claim follows from the strict object. Root's Main
`/Oy-` frame variant needs its own actual artifact review; no matching recompile
is forced. No separate unregistered worker baseline, new tests, probes, body
execution or SDK comparison is performed. The report-call checker checks call
claims only; zero rows is not semantic proof. Root's peer/registration/Main build
and actual Main object/archive gates remain separate.
