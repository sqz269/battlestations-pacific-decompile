# CC12: dormant particle prepared-record hook adapter

`NativeParticleModelPreparedHookAdapter` supplies the already accepted record to
the two particle profile hooks. It is a final, noncopyable, nonmovable HOST C++
adapter with exactly two borrowed references: the persistent prepared record and
a required real `NativeParticleModelConstructionCallees` delegate. This packet
adds no factory/application caller or default delegate. Its methods remain absent
from the linked game's map and unreferenced outside their defining Core object.

The baseline is Main `0d84b69933f1c577e8366622a12314284e8bf0f6`. The source changes
are appended to the existing lifetime header and translation unit; every previous
header/source character is preserved when only that appended adapter is removed.
No Native address, Ghidra annotation, CMake target, new translation unit, test or
probe was added. The tracked machine-readable evidence is
[`cc12_particle_prepared_record_hook_adapter.json`](../reports/cc12_particle_prepared_record_hook_adapter.json).

## Contract and behavior

The adapter follows the accepted
[hook readiness](CC12_PARTICLE_PREPARED_RECORD_HOOK_BINDING_READINESS.md),
[persistent record](CC12_PARTICLE_PERSISTENT_PREPARED_RECORD.md),
[admitted reference](CC12_PARTICLE_PROFILE_ADMITTED_CONSTRUCTOR.md), and
[prepared canonical entry](CC12_RENDER_ACTUAL_REGISTRY_PREPARED_ENTRY.md)
contracts. It does not expand their ownership or runtime claims.

| Override | Exact action |
|---|---|
| `bind_particle_profile(owner)` | `record_.publish_particle_profile(owner)` |
| `retire_failed_particle_profile(owner)` | `record_.retire_after_failed_construction(owner)` |
| `bind_mesh(mesh)` | `delegate_.bind_mesh(mesh)` |
| `bind_material(material)` | `delegate_.bind_material(material)` |
| `bind_section(section)` | `delegate_.bind_section(section)` |
| `renderer_virtual5c(renderer, entry, first, bytes, descriptor)` | Forward all five arguments and return the delegate's pointer |
| `call_00bd2f10(random, stream, minimum, maximum)` | Forward all four arguments and return the delegate's float |

The two profile hooks never invoke the delegate's profile hooks. Each override
contains exactly one statement, with the same signature and `noexcept` contract
as its interface. The constructor only stores the two references. The default
destructor performs no record disposal or Native cleanup. Compiler-generated
scalar deleting destructors can delete their own HOST C++ object; they do not
constitute Native model ownership or disposal.

The adapter serves synchronous construction hooks. Its record, sole prepared
owner, actual slot and borrowed services must persist through failed acquired
frames, successful Native lifetime, terminal/callback retirement and later
explicit disposal at externally proved quiescence. The record cannot be made a
stack-local temporary merely because this adapter's calls are synchronous. The
real resource/random delegate must remain valid for every delegated call.

Publication still occurs after live `D5DA50` profile initialization, before the
constructor's acquired region. Failed-construction retirement still occurs
after Native owner death. Existing Generated and canonical registry admissions,
physical Native return, opaque-key metadata retirement and explicit quiescent
record disposal are unchanged. No allocation, retain, reload, catch, pool return,
automatic disposal or new lifecycle is introduced by adapter source.

## Source and build evidence

The retained baseline and after-change Source covers 4,228 paths across `src`,
`include`, `cmake`, `scripts`, `tests` and `CMakeLists.txt`. Real Git commit/tree/
blob objects and recursive listings close the census, with 4,245 baseline/staged
rows when three epoch metadata files and fourteen prerequisite documents/reports
are included. The staged Source tree is
`d6bbedeaa1a6e5da3b01587a03ec054e9c697ffb`; final commit verification checks all
six Source roots and every retained row again. Only the two lifetime source files
change within this Source scope.

Every whole project preimage from the five retained historical epochs is checked
again: 3,352/3,352/3,354/3,354/3,356 rows. Their before-change differences are
13/11/7/5/0 respectively. The last is Root's accepted locale-helper Source3356
epoch, now including the persistent record and grid helpers; it differs after
this packet only in the two lifetime source files. Historical build declarations
are retained as metadata and receive no current build credit.

Finite comment-stripped queries cover 4,211 Source files, 48 queries per phase,
45 before/47 after bounded excerpts and twelve order checks. These document the
one new construction-callee subclass and dormant record/adapter boundary. They
are lexical observations, not preprocessing, AST or runtime closure evidence.

Both retained normal `scripts/build.ps1` MSVC Win32 Release builds succeeded and
passed the three existing checks: `reconstructed_math`,
`native_math_differential`, and `tool_tests`. The after build ran on
2026-10-10 at 21:51:36–21:51:54 UTC. The existing verified math seed and whole
build logs are retained. Both builds emitted the existing LNK4006 duplicate
`spawn_request` warning. No new tests or ad hoc executable were run.

Each phase retains the entire Core archive (1,993 archive entries, 1,990 COFF
members), all 80 App objects reconciled against actual project items and disk,
56 compiler logs, seven complete selected COFF indexes, all three lifetime-header
consumers, their 92 Source dependencies, and the whole game image/map. Per-phase
compiler commands, read/write logs and project objects establish this physical
capture; path labels alone are not the evidence.

## Compiled behavior and scope

The complete Core/App code census changes from 174,341 to 174,356 physical code
sections. All 174,341 old sections preserve exact payload bytes and symbolic
relocations. None changes or disappears. Fifteen sections are added only to
`native_particle_model_lifetime.obj`: 676 bytes and 233 decoded instructions.
The adapter accounts for twelve sections, 634 bytes and 215 instructions; the
remaining three are newly emitted constructor/destructor/scalar-deleting-
destructor bodies for its existing base interface. Exactly the three lifetime-
header consumer objects change physically; all 80 App objects remain byte-exact.

The constructor stores record/delegate at HOST object offsets `+4`/`+8` and writes
its vtable. Mesh/material/section/renderer methods are complete 12-byte tail
dispatch bodies using delegate slots `+0C`/`+10`/`+14`/`+18`; they preserve the
incoming argument frame. The 54-byte random forwarder copies the two float
arguments with `MOVSS`, forwards the reference and enum, invokes delegate slot
`+1C`, and leaves the returned `ST0` value unchanged. Complete bodies and symbolic
relocations, not selected heads, are retained.

MSVC inlines the existing record operations into the two new profile overrides.
Publication retains record/owner/phase/admission checks, installs the record
callback/context, constructs the admitted reference, and immediately publishes
the registry entry with no intervening call or branch between those two calls.
Failure retirement retains the owner-dead and reference-state checks, calls
Generated unbind, marks the reference retired, then invokes the existing record
callback. Existing record/reference methods remain byte-exact.

The complete adapter vtable section is **36 bytes**, including the preceding
RTTI relocation: the vtable symbol is at section offset `+4`. All nine physical
relocations are retained and positively resolved. Its destructor entry names a
COFF `??_E` weak external; the auxiliary record resolves to the emitted `??_G`
scalar deleting destructor. No four-byte metadata prefix is omitted and no weak
name is treated as a direct strong definition. Full selected EH and RTTI sections
are retained alongside all raw symbol/auxiliary records.

The selected code audit covers 146/161 complete bodies and 380/404 direct
relocations before/after, backed by 124/130 positive provider sections and whole
current CRT input libraries. This is bounded direct-provider evidence; it does
not claim unbounded transitive closure or the bodies of imported runtime DLLs.
The explicit vtable check additionally resolves all nine of its relocations.

Across every section in all retained Core/App objects, no adapter relocation
originates outside `native_particle_model_lifetime.obj`. The game's map contains
no adapter symbol. Its 3,992,064-byte `.text` is exact with SHA-256
`e34181ca5affdc445a64cc6f0e632c6dac79bc8748ce632f89c49cb2f584d86b`.
Whole images are both retained and differ; `.rdata` and the PE timestamp change,
while the map's only changed line is its timestamp. Whole-image equality is not
claimed.

## Replay and remaining work

`local/cc12_particle_prepared_record_hook_adapter/evidence.zip` retains Source,
real Git objects, historical project preimages, every whole captured artifact,
full COFF/code indexes, provider inputs, queries, helpers and build receipts.
Run its sibling `portable_replay.py` with that ZIP as the only argument. After
preloading its Python/Capstone dependencies, the helper allows one read-only ZIP
open and rejects other file opens, writes, processes, networking, new dynamic
loads and directory enumeration. Replay reconstructs the actual comparisons and
checks, rather than trusting reported counts. The same helper/ZIP are physically
copied and replayed; both outputs must match. Exact pins and final repository
checks accompany the report/local acceptance receipts.

This is build-tested HOST C++ plus static artifact and Source evidence. It is not
a drop-in Native ABI/SEH replacement, an invoked particle factory, runtime
quiescence proof or game validation. Next work should establish the real borrowed
resource/random delegate and persistent custody around the existing construction
frontier before introducing any factory caller or application activation.
