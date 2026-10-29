# Qualified raw Model storage lifetime

Status: **UNBUILT, static source packet only**. Base `f031fb80e` has the accepted
common C++ state `6216ea1af`. This packet adds only the raw construction,
member-cleanup, destruction, scalar-deletion and canonical-reference providers.
It changes no semantic Model implementation, Group context, CMake/ledger entry,
activation flag or native GUI call site. It adds no function-body credit.

The actual Model payload occupies `[0,184h)` in one genuine 188h slot of the
existing actual01090054 `NativeModelPool`. The final DWORD at184 belongs to
the pool. The Source constructor establishes the existing raw node prefix and
a trivial four-word tail, preserving constructor-unwritten bytes and index184.
There is no NativeModelOwner, NativeNodeBinding, logical SceneResource view,
scene association, copied reference count or extra native credit.

## Entries and evidence

| Entry | Existing Original contract | Qualified Source interface |
| --- | --- | --- |
| B75030, 132B | ECX actual slot, stacked name pointer, EAX self, RET4 | actual slot/name plus retained context and construction acquisition |
| B74F20, 43B | ECX model+178, RET | actual member178 plus context and member acquisition |
| B750C0, 165B | ECX model, no arguments, RET | actual model plus retained frame/context/lifetime acquisition |
| B75290, 32B | ECX model, stacked flags, EAX original identity, RET4 | actual model plus a borrowed volatile flags DWORD, frame/context/acquisition |

Static assembly was read through `bsp.py show --asm` in the audited session.
The `bsp.py ghidra` client guard verified live project `bsp`, program
`/battlestationspacific.exe`, language and image base against `config/target.json`
before the live read batch. That configuration selects
`C:/Users/sqz269/bsp.gpr`. Read-only disassembly confirmed:

- B750E5 captures174 before B750ED samples CE2220. B75101 and B7512C both
  call that saved target;180 is reloaded after the174 callback/clear.
- B74F2F independently calls CURRENT CE2220. Its captured geometry is cleared
  at the original member+8 only after any terminal returns.
- B75298 tests the CURRENT low flags byte after B750C0. The Source API keeps
  a real volatile argument-cell view; it does not replace that read with an
  entry-time by-value snapshot.
- EH funclet CC1C58 adds178 then CC1C61 jumps to B74F20; CC1C50/CC1C53 jumps
  to B6F440. Existing FuncInfo DFAC50/map DFAC40 and the accepted semantic
  evidence in `docs/NATIVE_MODEL_OWNER.md:101-106` establish states1/0.
- Current D62DE8 bytes begin BD30E0/B75290.

The assembly evidence supports source ordering and captured/current distinctions.
It does not turn these context-bearing C++ interfaces into Original calling,
private-stack, saved-register, FH3/SEH, CRT-exception or binary-detour ABIs.
Raw B6F5A0's existing prefix materialization/transient-store qualification applies.
Names and descriptive C++ types remain reconstruction hypotheses.

## Context, frames and identity

`NativeModelStorageContext` borrows the actual node context, initialized Model
pool, live raw node constants and D7A208 negative-zero cell. Construction and
destruction use that SAME context, including its raw string pool. Scenes/trees
must share their canonical actual-owner registry and the same current CE2220
cell. Contexts/constants/providers outlive all callbacks and failure disposition.

Construction acquisitions record the actual slot and context, successful raw
node return and tail lifetime. Canonical admission requires that completed
record, fresh lifetime diagnostics, current D62DE8[0/4], actual live+4 and exact
pool slab/index/free-stack provenance. Admission borrows that atomic and binds
one canonical registry entry; it does not initialize or retain the count.
Registry failure leaves the completed raw Model for explicit disposition.

The lifetime frame contains a separate raw node destruction frame and a
persistent scalar flags argument cell. BD30E0's Source current04 binding seeds
that cell with1; B75290 reads it after destruction. A callback can change this
cell, and direct scalar callers can supply their own initialized volatile DWORD.
That cell may alias still-accessible Model bytes but not context/diagnostics.
No private Original stack alias is claimed.

Acquisitions preserve the sampled decrement target, captured member identities,
completed releases/clears, EH state, nested B74F20 unwind and raw-node diagnostics.
They are disjoint metadata, not ownership credits or rollback. Model/frame/
companion/context lifetime must extend through pool return and metadata unbinding;
external quiescence prevents reuse while any callback can still refer to them.
Constructor or destructor completion alone does not authorize frame recycling.

## Normal release and failure

B750C0 stamps Model, captures174, samples CE2220 once, and releases captured174.
It clears the original174 cell after callback even if replaced. It then reads
CURRENT180 and uses the same sampled decrement for its normal release. It does
not call B74F20 for this normal path, since that would resample the import.

Only a returned zero reaches `NativeRenderActualOwners::resolve_actual`.
The existing contract (`native_render_context.hpp:29-34`) requires SAME actual+4
and genuine CURRENT0 dispatch to real destruction/pool return. The provider
checks the physical count identity and observed zero before that dispatch.
Neither member is assumed to have node/model extent or a scene170 field.
Missing canonical owners/profiles are errors, never successful defaults.

State1 failure uses fresh-current B74F20 on CURRENT180, then raw B6F440.
State0 failure runs only raw B6F440; the failed normal geometry release is not
retried. State-1 does not repeat a failing base cleanup. Cleanup failure during
the Source C++ unwind projection terminates. A throwing call may have effects
beyond completed flags; no credit is inferred from an unreturned callback.

The canonical zero path requires observed actual+4 zero and current BD30E0,
then BD30E0 reloads current04 B75290. The reference enters destruction once and
retires/unbinds after normal return or an escaping Source failure, without dead
payload reads. Failure retirement does NOT assert destruction or pool return.
The noexcept zero boundary terminates on an escaping failure.

Explicit scalar deletion adds no count gate/decrement. Flags0 ends payload
lifetime without returning its slot. Scalar completion, flags observation and
pool return have distinct diagnostics. Retain the raw backing and unresolved
credits after failure; do not retry destruction or invent a cleanup path.

## Required providers and remaining bridge

All admitted operations require real, current providers:

- The actual initialized Model pool, B74D00 allocation, B74750 return and its
  live slab/free-index state. Model construction never returns a slot on error.
- Existing raw B6F5A0/B6F440 and their string, scene, hierarchy, world, point-light
  and retained-owner dependencies, with initialized persistent frames. Any
  reached current target must have a genuine admitted provider.
- Current CE4970/CE4ADC/D7A24C/D7A208 cells, SAME string/publication/allocator
  domain and successful accessible memory operations through callbacks.
- Actual canonical member companions for any nonnull174/180, the genuine
  current decrement cell and the existing actual-owner CURRENT0 contract.
- Current D62DE8 BD30E0/B75290 table cells for canonical Model zero dispatch.

These are caller-required interfaces, not dummy Model scene/parenting services.
The generic AAA710 suffix still creates semantic NativeModelOwner companions,
resolves CameraTransform bindings and uses NativeNodeParentingRuntime. Its
logical scene/terminal transport is not compatible with P3a2's raw Group and
actual3Ch resources. No cast or placeholder association is supplied here.

Further integration needs a proper raw child transport, the existing raw
B6E680/B6E010/B6D7B0/B8F4F0 kernels, real Model006EF860/GroupB8F650 predicates,
Model scene/tree/zero bindings and genuine widget scene-release providers.
Geometry/property transports and Text/Icon factories remain their own work.
Shared Group/tree/resource contracts must close before independence or actual
FE_initial readiness can be claimed.

Validation here is bounded source/assembly inspection and `git diff --check`.
No build, test, probe, Original-body execution or game run was performed during
the strict A1 hold. Primary integration must add the build entry and perform the
required Win32 build/checks in an authorized window. This packet has no compile,
runtime, raw-GUI lifecycle or game-parity proof.
