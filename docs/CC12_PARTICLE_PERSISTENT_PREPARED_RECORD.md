# CC12 particle persistent prepared record

`NativeParticleModelPreparedRecord` now provides the bounded host composition
identified by [the caller-readiness packet](CC12_PARTICLE_PREPARED_RECORD_CALLER_READINESS.md).
It borrows the caller's sole prepared `NativeModelOwner`, reserves one existing
Generated lifetime credit and one entry in the same canonical actual-owner
registry, and holds the admitted particle reference in inline optional storage.
It does not own, construct, retain, destroy or return a Native allocation.

The implementation is confined to `include/bsp/native_particle_model_lifetime.hpp`
and `src/native_particle_model_lifetime.cpp`. All preexisting declarations,
fields and method bodies remain textually unchanged after removing only the two
new includes and appended class/methods. The existing lifetime translation unit
already belongs to the Win32 build; CMake did not change. Root review and primary
acceptance remain separate from the worker evidence below.

## Preparation and borrowed authority

The caller first prepares its one stable `NativeModelOwner` over the actual
F8D2D0 particle slot, before Native AF74A0 construction. That existing preparation
can allocate for scene association and may throw. The new record borrows it;
there is no second owner, copied Native storage, new scene/runtime/registry,
ordinary `NativeModelReference`, or new pool.

The record constructor accepts the actual slot identity, its stated 2E0h extent,
the prepared owner, `NativeParticleModelLifetimeAccess` and
`NativeRenderActualOwnerRegistry`. Before admitting metadata it checks:

- nonnull slot, exactly 2E0h stated extent, and identity equal to the owner's
  existing node storage;
- the owner's host phase is prepared, the borrowed particle pool/profile are
  nonnull, and the environment has its actual name-storage domain;
- exact registry identity with `owner.environment.retained_owners` and exact
  scene identity with the owner's existing Generated lifetime runtime;
- the existing scene binding's context/key and canonical scene lookup match
  that same owner and slot;
- the borrowed derived profile has the expected current terminal words at
  virtual0 and virtual4.

The stated extent and initialized F8D2D0 pool identity are caller contracts.
A `void*` plus a size does not prove allocation provenance or pool startup. This
record does not read the pool's Native fields or initialize a pool to manufacture
that proof. The existing process pool getter/startup and real allocation remain
the required providers. The slot's +2DC pool ID is untouched.

After validation the constructor reserves exactly one
`owner.environment.nodes.attachments.reserve_binding()` credit, then calls
`registry.prepare_binding(saved_identity)`. Both may allocate or throw. The
optional reference is initially disengaged and has no separate allocation.
All host storage and both admissions must be ready before AF74A0. Preparation
performs no typed load of Native +04 and adds no Native reference. The reads of
the borrowed profile's virtual4 word are not reads of the slot's +04 count.

If record construction fails, its existing admission members cancel through
ordinary C++ member unwind. The caller still owns its prepared owner and physical
raw slot. It ends only prepared/dead host state and allows the existing AF62F0
raw-allocation return to run once. The record never substitutes a Native
destructor, `free`, pool return, decrement or retry.

## Explicit operations

| Operation | Contract and resulting host state |
| --- | --- |
| Constructor | Validate the one prepared owner/domain, allocate metadata before Native construction, and retain two move-only admissions. State is prepared. |
| `publish_particle_profile(owner)` | Require the same owner, prepared record, disengaged reference, both active admissions and live owner. Mark publishing, emplace the admitted particle reference in inline storage, immediately publish the exact registry token/reference, then mark published. |
| `retire_after_failed_construction(owner)` | Require the same owner, published record and dead owner after real B750C0 unwind. Invoke the existing failed-reference retirement, which unbinds Generated lifetime before the record's disposal callback. |
| Private `retire_reference` callback | Require the exact reference in this published record, remove its exact registry binding using the saved opaque identity, then mark the record retired. It does not reset/delete the reference or inspect Native storage. |
| `retired()` | Observe host retirement only; it does not prove quiescence or complete Native factory success. |
| `dispose_retired_at_quiescence()` | Require retired state, engaged retired reference and consumed admissions. The caller proves all borrowers have returned; reset the host reference and mark disposed. |
| Destructor | Permit abandoned preparation only while the borrowed owner is prepared/dead, cancelling host admissions. Otherwise require explicit disposed state. Published or retired-but-undisposed destruction terminates. |

The record is neither copyable nor movable. Its address supplies the retirement
callback context and must remain stable through the entire Native lifetime and
all callback/resolve borrows. The caller separately owns the borrowed model
wrapper and every environment, scene, Generated runtime, registry, pool,
profile/type view, name service, manager and reached terminal provider.
Keep those borrowed lifetimes valid through quiescent record disposal; the
record does not extend them with ownership or a hidden reference count.

The publication operation is intended for the **existing** nonthrowing
`bind_particle_profile` hook after AF74A0 has published D5DA50 and the base owner
is live. The new record does not install or implement that hook in any callee.
The existing admitted `NativeParticleModelReference` validates the live profile
and actual +04, consumes the existing Generated credit, and installs its four
callbacks on the same scene binding while retaining the `NativeModelOwner`
context. The next operation is the same registry's exact-entry publication.
No Native call, external callback or yield is inserted between those operations.

The two existing primitives supply the valid nonallocating publication path.
Invalid reference/profile/capability state still rejects the contract, and
registry mutex failure can throw. The new public publication operation is
`noexcept`, matching the existing intended hook: such failures terminate across
that boundary. They are not recoverable raw-construction cleanup. There is no
blanket no-failure or universal heap-allocation claim.

## Native lifetime and quiescence boundaries

The sole Native +04 is initialized by the existing Native node constructor.
Neither admission is a Native creator reference, and the record adds no retain.
After successful AF74A0 construction, later event parenting/registration failure
still has the existing state-0 behavior: preserve the completed model, creator
reference and persistent record; do not synthesize a model rollback.

A later AF74A0 failure first executes its existing emitter-pointer/base unwind.
Only then may the caller forward the existing failed-profile hook to the new
record method. The owner must already be dead. The existing reference removes
its Generated lifetime and calls the record's private disposal function; that
function unbinds only the canonical metadata and marks retirement. The original
outer caller still performs the one physical AF62F0 raw-slot return.

Successful Native zero-reference deletion takes the existing AF7F40/AF6C50/base
path and returns the F8D2D0 slot **before** Generated unbind and disposal. The
record's callback therefore uses the saved identity only as an opaque hash key.
It does not read +04, look at the model's Native phase/table, return the slot,
destroy the owner, or reset the active reference. Both successful and failed
construction disposal reach the same host-retired state without conflating
their different physical-return order.

`dispose_retired_at_quiescence` is an explicit caller assertion, not a monitor or
proof of thread quiescence. No constructor, profile/failure hook, Native terminal,
disposal callback or resolved-companion borrower may still be using the record.
Never invoke it from disposal or Native callbacks. Keep raw-slot reuse serialized
until the old canonical entry is removed; the registry mutex protects metadata
only. Registry/Generated services outlive pending admissions and live records,
and all Native cleanup services outlive the last terminal/callback borrower.

The record changes no weak-manager policy. The particle manager does not acquire
a model reference when appending a weak pointer, and freeing its pointer array is
not a per-model drain. A future real caller must still establish client and
callback drain before the existing manager, pool, shared-singleton and renderer
service teardown. No application drain or complete producer is introduced here.

## Source and compiled verification

The fresh before build ran at 2026-10-10 20:50:40.511970 through
20:50:55.944743 UTC. The fresh after build ran at 20:54:35.717016 through
20:54:55.148662 UTC. Both used `scripts/build.ps1` for MSVC Win32 and passed the
three existing checks: `reconstructed_math`, `native_math_differential` and
`tool_tests`. The existing verified math seed remained 2,995 bytes with SHA-256
`26cdde71970ec8524c92d5e54e17c5587e817198ef16f3b6f2b8be25cd556d2c`.
The before invocation used up-to-date outputs; the after build recompiled the
three actual header consumers. No additional test case or probe was added or
run. The familiar `spawn_request_id_matches` duplicate-definition LNK4006 appeared
during the after archive rebuild; it is unrelated to this class.

Both phases retain the whole Core archive, whole game image/map, build receipts,
test log, project files, 56 compiler tracking logs and all 80 actual App objects.
The App inventory is reconciled against project entries, compiler item logs and
the live object-directory capture. Seven complete selected COFF indices cover
the three header consumers plus the existing model owner, registry, Generated
lifetime and scene-provider translation units. Their tracked dependency union
contains 92 project paths; 242 before/243 after external compiler/header paths
are enumerated without claiming whole SDK/compiler preimages.

The full census contains 1,989 Core COFF members and all 80 App objects:

- all 174,290 preexisting code sections are byte/relocation exact;
- 41 code sections, totaling 1,756 bytes, are added only in
  `native_particle_model_lifetime.obj`; no old section changes or removals;
- three Core objects change physically, including the two other recompiled
  header consumers; all App whole objects and code remain exact;
- complete selected decodes increase from 105 to 146; the six out-of-line record methods
  and three associated EH/cleanup bodies comprise nine complete sections,
  1,143 bytes and 353 instructions.

The record constructor's complete 416-byte body preserves the opaque slot
identity and checks the host owner/domain before its two admissions; the decoded
Native profile-word check is distinguished from a Native count read. Its
254-byte publication body calls the existing admitted reference constructor at
relocation B0h, then registry publication at D4h. The intervening instructions
contain only host writes/address setup, with no call or jump. All existing
reference/registry/Generated provider code remains byte/relocation exact.

Every direct relocation in the complete selected bodies has a positive retained
COFF or short-import definition: 295 before and 380 after, with 105/124 positive
provider sections. Weak-external fallback records retain their actual symbol,
auxiliary bytes and target index. Four whole current CRT link-library preimages
and the actual after-link logs are retained; machine-zero non-code directive
members are retained without treating them as code providers. This evidence is
bounded direct-provider evidence, not an unbounded transitive Native call closure
or a claim about runtime DLL bodies. No optimizer-changed old path requires a
new equivalence argument because the complete old code census is exact.

The record has zero external Core/App code relocations and no symbol in the
game map. Game `.text` remains exactly 3,992,576 bytes, SHA-256
`5042e761d84fb067b0d07cec326b93127f5dc5f93636ca335ac456d513ea4ea8`.
The whole EXE and map hashes differ: `.rdata` differs, while `.text`, `.data`,
`.rsrc` and `.reloc` remain exact; the map's sole changed line is its timestamp.
The full artifact pins and deltas are retained. Whole-image equality is not
claimed, and this uncalled class is not application or gameplay validation.

## Retained Source, Git and portable proof

The baseline is `35578944bb9c72b48a35c7183bca42d53657b64e`, after merging accepted
Main `98b5113614` into the worker. All 4,226 selected tracked Source/build/test
paths are retained before and after, with 4,209 complete C/C++/header/inline files
in each finite query corpus. Thirty-four lexical searches per phase preserve all
matches; only the existing particle lifetime header/source contain the new
record. Complete bodies and declaration preservation are checked separately
from the lexical searches. No concrete construction callee or application hook
was added.

The real staged Source tree is `806f97d198e344acd8abb92244a069484733cdc7`.
Actual Git blobs and worker files were independently checked for all 4,226 paths,
three epoch metadata files and six prerequisite documents/reports. Ten whole
tree objects preserve the complete selected root coverage. Final commit Source
roots are checked against this staged tree, separately from the ZIP-only replay.

The typed-view 3,352, wide-string 3,352, admitted-particle 3,354 and accepted
registry 3,354 historical project epochs are retained separately. Their whole
preimages are checked against original manifest pins, with before/after
differences enumerated. The latest accepted registry epoch is identical to the
before Source and differs afterward only in the two owned lifetime files.
Historical build declarations receive no fresh-build credit.

The report pins the full ignored evidence bundle and helper, all selected COFF
indices, Source/Git captures, build receipts, provider evidence and matching
in-place/physically relocated replay results. After preloading standard modules
and the artifact disassembler, the helper reads only its supplied ZIP and blocks
other opens/writes, process creation, networking, new library loading and
directory enumeration. Replay recomputes Source/Git coverage, tracking-log
closure, full Core/App code census, complete selected decodes, direct providers
and the exact artifact deltas; it executes no retained game/probe binary.

The remaining limits are explicit: caller-proved pool provenance, initialized
borrowed services, correct 2E0h extent, serialized Native lifetime and quiescence;
no runtime exercise of the record or particle constructor; no Native SEH/drop-in
ABI claim; no complete factory, resource-owner hook, application activation or
shutdown integration. Those require separately accepted work.
