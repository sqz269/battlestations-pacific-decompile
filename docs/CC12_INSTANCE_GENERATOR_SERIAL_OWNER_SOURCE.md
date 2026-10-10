# Instance-generator binding serial process owner

Packet: `cc12_instance_generator_serial_owner_source`
Source baseline: `16c74d6599ebcc8ff7a488a78bddeab631c7ac77`

## Implemented boundary

`GameNativeRendererScalarProcess` now retains one private, zero-initialized
`std::uint32_t instance_generator_binding_serial_0108fd30_` after every existing
member. Its new `volatile std::uint32_t&
instance_generator_binding_serial_0108fd30() noexcept` accessor returns that
cell. The existing private constructor, deleted copy operations and function-local
process singleton remain the sole owner and identity mechanism.

The cell is distinct from `logical_texture_serial_0108d6e8_`. Borrowing it does
not increment, reset, allocate, create callbacks, construct another owner, map
an original address or attach an application/generator context. The address in
the name identifies the researched original cell; this Source object is not
placed at that original address. No CMake registration or tests were added.

## Original evidence and limits

The accepted [readiness packet](CC12_INSTANCE_GENERATOR_SERIAL_OWNER_READINESS.md)
and its [primary review](CC12_INSTANCE_GENERATOR_SERIAL_OWNER_READINESS_PRIMARY_REVIEW.md)
establish the original DWORD's loader-zero preimage from the original PE section
headers. The complete interval `[0108FD30,0108FD34)` is in the virtual tail of
`.data`, beyond its file-backed raw data. That accepted packet retains the
original file's SHA-256 identity and bounded header bytes.

Retained `00B451D0` evidence consumes the old serial at `00B452BB`, stores it at
binding `+8`, then performs the unlocked increment at `00B452C4`. The current
generator context already accepts a `volatile std::uint32_t&`; this packet
provides its future canonical owner but does not bind that context. The serial
is consumed before subsequent companion registration and is not rolled back
if a later operation fails.

The accepted xref response contained only the known load and increment. It did
not prove that other direct, indirect, dynamic or omitted writers cannot exist.
The original runtime or pre-first-use value remains unproved. The loader-zero
preimage is not a claim about the original game's whole initialization sequence.
This packet made no new Ghidra query/export/mutation, original-image byte read,
Native mapping/probe/fixture call, startup or gameplay run.

## Whole-owner Win32 comparison

Fresh MSVC x86 Release compilations used the retained normal project flags and
complete reported dependencies. Baseline and candidate each have a normal and
class-layout diagnostic compilation for both the scalar owner and the existing
renderer application. All eight compiler invocations and forty complete
`dumpbin` invocations succeeded. Twelve whole COFF inventories additionally
cover the existing Root objects and the fresh normal worker build objects.
Every inventoried function decoded completely; physical primary/AUX symbols,
code, noncode, relocation records and raw bytes are retained.

| Observation | Baseline | Candidate |
| --- | ---: | ---: |
| Scalar process size | 48 bytes | 52 bytes |
| Existing member count and offsets | 11 | Same 11, same offsets |
| New serial offset | Absent | 48 (`0x30`) |
| Owner function count | 26 | 27 |
| Existing owner functions with identical full bytes and edges | 24 | 24 |
| Renderer application functions | 792 | All 792 identical |

The two changed old owner functions are the private constructor and the
singleton factory's inlined construction path. Each adds exactly one zero
store for the appended cell. The comparison maps every old instruction and
relocation to its candidate counterpart; only direct branch distances and two
code-label offsets move around that insertion. The factory's existing guard,
import resolution, exception handling and abort path are preserved.

The new accessor is exactly `LEA EAX,[ECX+0x30]; RET` (four bytes), with no
relocations or calls. All eleven old accessors retain their complete bytes and
relocation edges. The process `.bss` allocation grows from 48 to 52 bytes while
retaining its existing symbol; its guard storage remains unchanged. The class
member initializer supplies the Source construction-time zero store in addition
to static zero-initialized storage. This is one successful singleton
initialization, subject to the existing C++ exception/retry behavior, and not
a new application reset path.

All 58 prior named defined symbols retain type, storage and section kind,
including both physical definitions of the repeated local-label name `$LN4`. The
only moved symbol values are the two explicitly mapped code labels. The
undefined-symbol set is unchanged. The SafeSEH DWORD changes physical symbol
index from 126 to 131 and resolves to the same existing handler. All other
differences are explicitly retained compiler checksum/debug records or three
anonymous-namespace RTTI name strings in the relevant application comparisons;
these are recorded as differences, never as raw equality. The added 16-byte
function-debug record relocates to the new accessor. Every other noncode/EH
section preserves its full payload, size, characteristics, symbol values and
relocation edges.

## Source, link and build evidence

The immutable evidence retains all 750 pins from Root's accepted
`Source746_frozen.json`: 746 selected inputs plus four artifacts. All 746 inputs
were compared with the current worker; the only overlapping change is the scalar
header. Separately, 17 relevant translation-unit roots have a complete
539-file Source/context closure, 1,014 resolved repository include edges and no
unresolved quoted repository include. Baseline full contents were checked
against their Git blobs; exactly the two owned Source files differ afterward.
All 1,092 unique compiler-reported Source and dependency files are retained.
A complete 4,209-file tracked Source census finds the existing context/consumer
and the four added serial declaration/definition lines, with no new application
wiring.

The complete successful normal link context retains all 95 explicit object,
library and resource inputs, including the system libraries, and the final map.
The scalar member in the 1,989-member `bsp_core.lib` is byte-identical to the
normal worker scalar object. The archive contains exactly one positive
definition each of the singleton and new accessor. This establishes ordinary
Source link membership, not execution of an otherwise unreferenced accessor.

`scripts/build.ps1` passed on 2026-10-10, 07:53:13–07:53:31 UTC. The two checks
configured here, `reconstructed_math` and `tool_tests`, passed. This worker lacks
`local/seed_reference.hpp`, so `native_math_differential` was not configured;
no seed was copied or generated and no third test is claimed. Root retains the
seeded three-test integration check. No executable was launched for a new
serial behavior test; no new test cases were needed for the borrowed cell.

## Review artifacts and remaining work

The [machine-readable report](../reports/cc12_instance_generator_serial_owner_source.json)
pins the immutable evidence under
`local/cc12_instance_generator_serial_owner_source/`. `review_evidence_revision02.py` is
a small read-only replay: it checks the complete manifest and ZIP payloads,
reindexes the twelve COFF objects, recomputes every function comparison, verifies
the Source/Git pins and checks the exact linked scalar archive member. It does
not compile, launch an executable, access Ghidra, mutate a tracked file or
read the original game image. `bundle_receipt_revision02.json` is detached to
avoid a self-referential archive hash. Revision 02 retains the final documentation
format and LF report bytes; the original archive and every prior frozen payload
remain unchanged. No Source or build evidence changed between archive revisions.

Later generator application wiring must borrow this exact process cell together
with the already researched graphics/layout/geometry/string/pool/profile/name
domains, and retain the required failure-frame and payload-retirement ordering
before application drain. Material/compiler/sampler/full-mesh/parser activation
gates remain separate. This change establishes a reconstructed and build-checked
Source owner; it does not establish Native ABI compatibility, startup, renderer
activation, mesh loading or gameplay equivalence.
