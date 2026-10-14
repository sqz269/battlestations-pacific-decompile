# Genuine tick receiver owner: current Source frontier

The canonical process lists and frame-job application are available, but no
complete ordinary unit receiver producer can yet be attached. This packet adds
two ready raw list-head producers and a precise specialist handoff. It creates
no receiver, completes no parent by relabelling a fragment, and adds no normal
fixed-step attachment.

## Ready compiled component

`native_unit_owner_list_heads.cpp` composes the existing accepted
`detail::allocate_self_linked_list_storage` with the real
`singleton_lifetime_allocate` BF681B malloc/new-handler boundary:

| Complete Original leaf | Request | Caller placement in 77EED0 | Writes |
| --- | --- | --- | --- |
| `0077B940..0077B959`, 26 bytes | 18h | Sentinel pointer at root+2A8h | Only +0/+4 self-links |
| `0077B960..0077B979`, 26 bytes | 0Ch | Sentinel pointer at root+29Ch | Only +0/+4 self-links |

Both Original bodies have eleven instructions, no inputs, a plain RET and an
allocation address retained in EAX. The current decompiler's `void` declaration
does not account for that retained result. Each independently guards the base
and wrapped base+4 address; it does not zero payload holes. The new Source
pointer-return interfaces retain that raw allocation/store contract through the
accepted helper. They do not preserve Original residual ECX/flags or exceptional
ABI. Actual enclosing list teardown must tidy elements and free the sentinel;
these leaves own neither a unit allocation nor a completed list header.

Fresh verified live bytes equal both complete file-backed PE spans. Strict Win32
MSVC `/std:c++20 /EHsc /O2 /MD /W4 /WX` compilation passed. The complete emitted
public Source schedules have the two allocation sizes and only the two ordered
heap self-link writes; remaining writes are caller-stack/compiler bookkeeping.
`scripts/build.ps1` passed Release and both existing configured checks at
`9e8967f4e`; the new TU still requires primary registration in `bsp_core`.
No new tests, fixture execution, Native ABI or gameplay credit are claimed for
these leaves. Existing library names were retained, comments appended under the
Ghidra write lock with prior values journaled, project saved and exports refreshed.

## Current complete pieces and their limits

| Domain | Available Source | Unclosed receiver contract |
| --- | --- | --- |
| Process tick lists | Canonical `GameNativeTickRegistrationProcess`, actual 875890 constructor and qualified 87B699..87B728 fragment | Real completed receiver parent, callable tables and containing-root lifetime |
| Raw observers and weak owners | Actual 20h observer-prefix projections, concrete observer services, canonical weak pool/domain and actual +24 view | Prefix projections do not construct the whole SceneNode or provide complete root/child scalar dispatch |
| Strings/matrix | Canonical string process, actual header/pool APIs and complete matrix helper | Full parent call order, actual globals and exception ownership |
| Entity IDs/pending init | Canonical `GameNativeEntityRegistryProcess`, raw F89A08/F89A5C pair, F899CC list and distinct FC/E4 locks | Root constructor/publication and actual virtual148 provider; these owners do not own a unit |
| World links | Actual 9037F0 header allocation and 903F30/924710 raw unlink leaves | Genuine World/root publication, memberships, child teardown and whole World lifetime |
| Scene/GameEntity Source | Normal raw-offset functions in `entity_mission_seg60_argon.cpp` | `Seg60Host` still requires actual global/image/table, role, weak, matrix, string, registry and scalar providers; no concrete subclass was found in the inspected Source |
| Unit tickable Source | `UnitTickableEntityState` constructor/destructor in `landscape_lua_seg55_argon.cpp` | Uses projected fields/std::vectors and abstract 77EED0/77E380 hooks; it is not native receiver backing |
| Frame application | Actual AA0/CF08 owner, metadata, random registry, worker, descriptors and retention | Required current tick methods and genuine remaining-job dispatcher remain external |

The older scene-base readiness report's ID/pending ownership gaps are superseded
by the present canonical registry process. Its whole-root/world/callable profile
and exception gates are not closed by that addition. The current Seg60 GameEntity
interface accepts a uint16 requested ID while Original forwards a full DWORD and
stores AX. That is a call-interface limitation; this audit does not prove a
business-state divergence from narrowing alone.

## Exact specialist packet

Use a fresh Astra worker for actual raw parent/lifetime closure, not an alternate
unit pool or a semantic-object cast. Fresh typed-flow captures verified project
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, and report these extents:

| Routine | Current address range | Bytes | Existing Source coverage |
| --- | --- | --- | --- |
| SceneNode constructor | `00925CE0..00925EF3` | 532 | Seg60Host normal body plus qualified raw leaves |
| GameEntity constructor | `00928630..0092878F` | 352 | Seg60Host normal body; actual owning adapter absent |
| Unit owner constructor | `0077EED0..0077F08D` | 446 | Layout/evidence and abstract caller hook; no concrete raw body found |
| Unit owner plain destructor | `0077E380..0077E490` | 273 | Historical complete audit; abstract caller hook |
| Unit owner scalar deletion | `0077E810..0077E82D` | 30 | Native metadata/evidence; real root free boundary unbound |
| Tickable constructor | `0087B670..0087B743` | 212 | Projected model and qualified 87B699..87B728 raw fragment |
| Tickable plain destructor | `0087A410..0087A4DA` | 203 | Projected model; concrete raw teardown absent |
| GameEntity plain destructor | `009287B0..0092885B` | 172 | Seg60Host body with required providers |
| SceneNode plain destructor | `00925780..009258C2` | 323 | Seg60Host body with required world/scalar/effect providers |

The old 77E380 comment says its Ghidra body stops after free at 77E43D. Current
typed metadata instead reports the complete E380..E490 AddressSet and
`no_return=false`. Do not schedule listing repair from that stale comment alone.
AddressSet coverage is not proof of every instruction, edge, unwind clause or
executed path; the specialist must verify any actual listing/control-flow gap.

Required inputs/owners are the genuine containing allocation and its factory
preimage; unchanged full flag; canonical manager/string/weak/observer/entity-ID
and pending-init domains; current actual Game publication E188A8, network mode
at game+1FE4 and E0AF20; current CE4ADC and other physical globals; actual World
and hierarchy publication; and real profile/callback resolution. A successful
GameEntity base is not proven immediately destructible before World publication.

77EED0 calls base 928630 with registrySelect=1, constructs three actual 34h recon
records at +1E8/+21C/+250, binds their owner +1E4 and indices 0/1/2, allocates both
list sentinels, installs further physical fields/profiles and conditionally calls
77EB50 using the actual current Game/network globals. Required callback extents
are `00805A70..00805AE9` (122-byte record construction) and
`00804300..0080430D` (14-byte destruction tail). The peer producer is
`0077EB50..0077EC10` (193 bytes), currently a hooks-based Source body. Preserve
actual BF7CD1/BF7D69 iteration and callback-pair/unwind semantics, not generic
vector construction. Normal EH state stores alone establish no cleanup policy.

Teardown must close the session-message arm, F87194 withdrawal, +2B0 observer
member, both circular lists, all three recon records, base GameEntity/SceneNode
destruction, root scalar flags/free, and the embedded +310 tick unlink/current
subnode deleting dispatch. The complete 875490 wrapper is **32 bytes through
8754AF**, not the older 30-byte extent. Its 874F00 tail has a real indirect
slot-zero call; a generic cleanup/no-op cannot replace that target resolution.

Proposed disjoint output files: `include/bsp/native_unit_owner_entity.hpp`,
`src/native_unit_owner_entity.cpp` and a dedicated evidence doc/report. Claim
77EED0/77E380/77E810 and required peers/handlers before writes. Base 925CE0/928630
and recon callbacks 805A70/804300 were held by `cc13-seg49` at this capture;
queue their addresses/files rather than taking that ownership. The present
77B940/77B960 Source component is ready to borrow and must not be duplicated.

The specialist may first deliver a qualified complete dependency whose inputs
and actual lifetime are ready, but must not label that as the whole receiver
producer. Activation still requires a genuine concrete factory/derived class,
complete current tick and remaining-job dispatch, canonical process list
identities, and the accepted application failure-retention/drain ordering.
Command-line/Lua writer hooks to the canonical run-pass byte also remain
explicit; current inspected game Source contains no SetSingleThread binding.

The companion report pins Source, current typed metadata, Original leaf bytes,
compiler/build receipts and annotation journal. Parent bodies were inspected
through bounded existing evidence; no parent body, handler, ABI or flow repair
was recovered by this Sol packet. Normal attachment and gameplay remain false.
