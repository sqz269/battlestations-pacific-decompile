# CC12 canonical property/query fallback owners: Source

Status: primary-accepted and build-tested ordinary Source ownership. This packet adds no Original function or fragment ledger credit and
does not execute the proposed recursive-lifetime case.

The accepted execution-readiness packet identified an actual owner gap:
property lookup/publication needs a stable `00E177E4` fallback, and the query
role at `00E17654` must agree with the existing shader authority. A nonempty
selected key did not waive those whole-provider prerequisites. This packet
closes the Source owner/alias gap without weakening them.

## Authority, alias and lifetime

`GameNativeStringProcess` is the existing privately constructed canonical
process owner, permanently retained by its real function-local static pointer.
Two private `const char` members are appended after every established field.
They are zero-initialized once and exposed only as `const char*` through two
out-of-line `const noexcept` accessors:

| Source role | Actual Win32 owner offset | Whole accessor |
| --- | ---: | --- |
| Property fallback `00E177E4` | `+0x2C` | 4 bytes: `LEA EAX,[ECX+2C]; RET` |
| Query fallback `00E17654` | `+0x2D` | 4 bytes: `LEA EAX,[ECX+2D]; RET` |

These are distinct non-overlapping bytes in one actual permanent owner. No
second manager, pool, copied context, local NUL, callback, or numeric Native
address is used as their backing storage. Source immutability means the
closed const-only ownership contract; it is not a claim about all Original
writes or arbitrary callers casting away const.

`GameNativeShaderProcess` retains every old member offset. Its historical
`+0x4A1` byte is renamed `reserved_non_authoritative_instance_empty_`; it is
initialized as before and is not read by the authority path. Private
construction appends a `const char* const` at `+0x4A4`, captured from the same
canonical string process's query accessor. The existing shader query accessor
now consists of a 7-byte load-and-return from `+0x4A4`. It remains `noexcept`
and performs no allocation or factory call.

The current renderer consumer, the real `CompilerOwnersGraph` constructor in
`game_native_renderer_compiler_owners.inc`, obtains this borrowed alias. Its
compiled query expression changes from owner-address addition `+0x4A1` to a
load from `+0x4A4`; the result is still stored into the same compiler-owner
field at `+0x90`. Its register-limit, scratch and separate source-empty
bindings remain `+0x9C`, `+0xA0`, and `+0x4A0` respectively. All four calls
resolve to the actual whole canonical shader factory definition.

Property lookup/publication still accepts explicitly borrowed fallback
pointers. A future real property caller must pass these two canonical string
accessors; this packet does not introduce an app caller or relax that contract.
The generic shader-field initialization context likewise remains an explicit
borrowed-context API. The Source reference search and actual compiler consumer
are retained so the current authority path is distinguishable from those
uninstantiated/general interfaces.

The constructor dependency is acyclic: Shader borrows PhysicalPool (existing)
and String (new). String's constructor binds its own manager/pool/return-gate
cells into the existing raw context and `ActualNativeStringPoolStorage`; it
does not call Shader, PhysicalPool, or Native lazy getters. The whole emitted
String constructor and factory corroborate this. Both factories keep their
real private static publication cells and thread-initialization guards. The
String allocation grows `0x2C -> 0x30`; Shader grows `0x4A4 -> 0x4A8`.
Neither factory registers a new CRT drain or exit destructor. The permanent
owner and alias remain live through the established process-lifetime contract.

## Preserved physical bindings

| Owner | Existing offsets retained |
| --- | --- |
| String | manager `0`, pool `4`, return-disable `8`, raw references `C/10/14`, actual storage `18` |
| Shader | raw pool `0..38`, wrapper `38`, mutex `40..70`, state `70`, registration `74`, modes `78..7B`, mode operation `7C..94`, cache `94`, stream NUL `98`, register limit `9C`, scratch `A0..4A0`, source NUL `4A0`, reserved byte `4A1` |

The String constructor is now 57 bytes (formerly 51): its only additional
body write is the zero word at `+0x2C`. Its existing prefix and suffix are
byte-identical around that write. The Shader constructor is now 313 bytes
(formerly 244). It preserves the previous direct-field write order, widths,
offsets and values, after accounting for the compiler's `ESI -> EDI` root
register choice. The actual physical-pool factory and wrapper constructor
bindings remain present. The shader factory repeats the same preserved
initialization with its genuine inline constructor body.

Shader private construction now has a potentially throwing call to the
permanent String factory. The resulting real MSVC EH/unwind bodies and their
data/relocation graphs are retained; the ordinary Source construction domain
includes this behavior. No Original FS/SEH, exception, or register ABI parity
is claimed. The existing shader query accessor itself remains a plain load.

## Native attribution

The complete installed Original PE matches the retained physical image.
Fresh read-only `bsp.py ghidra bytes/xrefs` queries confirm both initial NUL
bytes and the current indexed references. Each query uses the configured
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` client verification.

| Role address | Raw file offset | Current indexed references |
| --- | ---: | ---: |
| `00E177E4` | 10581988 | 8 |
| `00E17654` | 10581588 | 9 |

Both reside in raw-file-backed writable `.data`, characteristics `C0000040`;
neither is merely loader-zero storage. Their addresses differ by `0x190`.
There are no indexed direct WRITE references in the retained result. That
does not exclude indirect writes or runtime mutation in Original. The earlier
whole `0043B760`, `0043B8B0`, and `00419CA0` attribution remains read-only
historical evidence; these new Source accessors are not recovered Original
function bodies and are not installed over Native globals.

## Whole-artifact qualification

`local/cc12_property_canonical_fallback_owner_source/qualification.json`
retains independently resolved COFF section/symbol-index/data graphs and
complete instructions for the affected bodies:

| Actual current production body | Complete extent |
| --- | ---: |
| String property accessor | 4 |
| String query accessor | 4 |
| String factory | 209 |
| String private constructor | 57 |
| Shader factory | 425 |
| Shader private constructor | 313 |
| Shader query accessor | 7 |
| Compiler owner constructor | 471 |

The 470-to-471-byte compiler constructor change is bounded precisely: the old
`ADD EAX,4A1` at `+145` becomes `MOV EAX,[EAX+4A4]` at `+14E`; the same
LEA/PUSH/MOV sequence is scheduled before the new load. Code outside that
small expression region is identical, and subsequent ordered relocations
shift by one byte. Whole actual factory/accessor definitions, rather than
unresolved provider names, support the authority path.

The capture includes all 13 actual translation units consuming either edited
header, plus nine supporting property/string/pool/CRT-domain units: 22 whole
objects, 5,328 current complete extents, 13 unique exact `bsp_core.lib` members,
nine application objects, the whole actual archive, application PE, and CRT
import/export inputs. These are inventory counts, not blanket equivalence or
execution credit for thousands of unrelated functions.

Focused preservation checks pass for 18 old public leaf-accessor copies, all
33 complete property-bag TU extents, and all 460 complete extents in the nine
supporting units. The latter include the actual string storage constructor,
node pool, dictionary lookup, array release, singleton free and physical pool.
All six accepted recursive-lifetime bodies and their provider graphs remain
unchanged. The six bodies still have ordinary Source interfaces, with their
earlier ABI and execution limits.

The historical primary build and this worker build have different paths.
The compiler consequently salts some anonymous namespaces and lambda names.
The raw comparison is retained honestly: 4,788 exact old extent/relocation
matches, 535 raw differences or missing identical names. Focused comparisons
first resolve actual code/data targets, then normalize only explicit anonymous
namespace spelling. Three differing lambda symbols are paired to unique whole
extent hashes in the same unchanged TU, and their positive target graphs must
also match. No blanket lambda regex or import-name equivalence is assumed.

## Build and provenance

Base: `7b244d3b8bc3204b1e37759a90842c28a0b78b63`. The primary integrator
accepted the ownership/alias design before the four Source edits. Later
independent main changes are deliberately left to the primary merged build;
they are not claimed tested by this worker's earlier build.

Baseline object/archive capture is inherited production from the actual
primary tree, with pre-edit current Source captured separately; it is not
misrepresented as a new baseline compilation. Before the successful build,
947 physical items were frozen, including 877 candidate consumed inputs and
the actual compiler/linker files and recipes. Postbuild read logs identify
875 actual consumed inputs, every one equal to its physical prebuild copy.
There are 972 postbuild physical pins. No Source edits occurred after the
successful build.

`./scripts/build.ps1` passed once on 2026-10-08, 21:21:50.941Z through
21:22:35.293Z (44.35 seconds). The existing `reconstructed_math`,
`native_math_differential`, and `tool_tests` checks all passed in 7.23 seconds.
The existing unrelated LNK4006 `spawn_request_id_matches` warning remains.
No new tests, CMake targets, or tests of these new accessors were added.

Qualification-script corrections are retained separately: an old factory
register assumption, a consumer instruction-scheduling assumption, explicit
unique lambda binding, and a receipt filename correction. They did not change
Source or trigger another build or target execution. The final static
qualification and complete manifest rehash pass.

Exactly four existing Source files plus this document and the report are
committed. No Ghidra writes, ledgers, build registrations, Native patches,
probes, selected API/entry execution, startup, or gameplay validation occur.
The manifest and report identify the actual physical evidence and the accepted
readiness receipt. Primary acceptance and a separately authorized concrete
five-root execution packet remain the next boundaries.

Primary review rehashed all 13,634 sealed artifacts and 2,933 historical physical
pins, froze 880 current inputs before one merged normal build, and checked all
875 actual selected compiler inputs against those preimages. All three existing
checks passed. The current 22 whole objects / 5,328 complete extents have a
positive code/data graph bijection to the worker artifacts; all 13 actual Core
members are uniquely present. Exact lambda token bindings are recovered from
whole extents and ordered target graphs; differing RTTI spelling bytes are
checked in full, rather than ignoring their hashes. Six failed static-driver
attempts are retained; corrections changed neither Source nor the successful
build count. The final primary receipt is local/cc12_property_canonical_fallback_owner_primary_review/receipt.json
(SHA256 02c5d97fd432f7aa1d0fbb6d2c8638bfdcb1b7124c577bfd4f4f549161c50ba4).
No new entry, startup or gameplay execution was performed.
