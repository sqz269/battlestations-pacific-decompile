# Conditional outer land constructor source, cc11

This packet reconstructs the complete ordinary caller `009B3240` and the
nonnull-task domain of alias binder `009F9980`. Both use one borrowed view of
the same caller-owned stable task storage, including its canonical task+404h
identity cell. Complete base/composite construction, actual executable profiles
and selected-state entry are REQUIRED external bindings. This is conditional
SOURCE reconstruction, object/fixture checked, not a constructor/arena adapter,
binary replacement or game/lifetime validation.

## Evidence and original ABI

Primary recovered the fixed integer/register/field contract in
[PILOT_BOT_LAND_CONSTRUCTOR_RECOVERY_CC11.md](PILOT_BOT_LAND_CONSTRUCTOR_RECOVERY_CC11.md)
and [its byte receipts](../reports/pilot_bot_land_constructor_recovery_cc11.json).
That recovery reviewed complete assembly, matched disk/live bytes for all five
constructor bodies, annotated/saved Ghidra and refreshed exports. Worker source
work adds no ABI/x87 inference and performs no Ghidra writes.

| Body, end exclusive | Original ABI and coverage |
|---|---|
| `009B3240..009B3307` | ECX=task, stack owner/block, EAX=original task, RET8, preserves ESI/EDI; private x86 EH frame. Complete ordinary caller represented. |
| `009F9980..009F99A6` | ECX=approach, stack task, RET4. Complete nonnull-task alias stores represented. |

The primary matched199 bytes for the outer body (SHA256
`c720447858efbe295ce5c4c4c51290b6e5adb679591e333aadfd9ed071fc6c60`)
and38 bytes for the binder
(`39e8fe278b59d4acb8ea72abf9b80a47f1c73d36b8d37640c7c08bbfb95c25df`).
Live BSP prototypes and call checks verified the configured
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
`x86:LE:32:default`, base00400000 target. Descriptive names are hypotheses,
not recovered symbols. New C++ signatures do not reproduce original entry ABI.

## Actual borrowed field and service contract

`NativeLandTaskConstructorView` has no owned storage, initial values, arena,
automatic construction, raw-layout overlay or default profile. Its task handle
and every member identity/reference must map to the SAME actual stable task.
Subobjects cannot migrate or be invalidated during this caller. A handle cannot
be reused while an active/retired reference or callback can reach it.

| View member | Actual represented storage |
|---|---|
| `task`, `approach_3f8`, `registry_4b0` | Original task and embedded approach/registry identities. |
| `retained` | Actual cells3FC plane,404 canonical squadron,424 block,428 block owner/target,42C returned queue-record identity. |
| `task_profile_000`, `approach_profile_3f8`, `registry_profile_4b0` | Actual profile pointer cells receiving required executable tables corresponding to00D1FFA0/00D1FF94/00D1FF90. |
| `moveto_4c4`, `follow_500`, `park_620`, `current_state_310` | Actual constructed state identities and current-state publication cell. |
| `command_004`, `gun_314`, `control_38c` | Actual task subobject identities aliased by the approach. |
| `command_alias_410`, `gun_alias_414`, `control_alias_418` | Approach18/1C/20 alias cells. |

Existing hook/cruise/validity views borrow `retained.squadron_404` directly;
cruise separately borrows `retained.plane_3fc`, and validity borrows424/428/42C.
There is one canonical squadron cell, not translated view-pointer caches.
The existing required pure identity-to-view mappings remain external.

The caller executes complete `construct_base_0099c6f0` on this same view with
original owner and kind3. Only AFTER that call does a faithful raw owner+50h
read obtain the plane. Complete `construct_composite_009b2e50` then constructs
the same actual approach with that captured plane and the original block.
It must produce the retained cells through their actual storage, including full
lower constructors, queue-record production, registry/states/profiles and
observer/name services. A partial field initializer does not satisfy this
production contract.

The caller captures current3FC BEFORE its three profile stores. A required
`NativeLandTaskExecutableProfiles` value supplies nonnull actual executable
table identities corresponding to the three observed native profiles. The
bindings are invariant through the caller. Source stores them in order root,
approach, registry. No fabricated native address literal, descriptive-only
metadata table, no-op publisher or fallback table is supplied. Actual profile
executability and receiver compatibility are the caller's unbound obligations.

The actual virtual+38h predicate uses the captured PLANE's EMBEDDED+72Ch
receiver; this is not a dereferenced controller-pointer field. False selects
park620 without a leader probe. True reloads current3FC after that call and
executes actual007B8AD0 on the fresh plane: true selects MoveTo4C4, false Follow500.
The selected identity publishes to310 BEFORE actual selected-state entry+4.
Entry may update current-state/retained fields; the caller does not restore
them. Only AFTER entry does the binder store actual command4/gun314/control38C
identities to410/414/418 in order. It returns the original captured task handle.

The source binder admits nonnull task/subobject mappings. Native null-helper
behavior clears only command alias18 and computes address words314h/38Ch for
the other aliases; that nullable domain is excluded, not replaced with three
null values or a guard/fallback.

## Normal-flow and unfinished boundaries

Admit nonnull valid task, owner, input/fresh/captured planes, block, subobjects
and executable profiles, with coherent live cells and complete required
services returning normally. Raw owner reads and field/profile mappings cannot
invent callback effects. Actual calls can mutate field values within their
admitted contracts; they cannot invalidate captured identities/cells or
structurally reenter this caller. Volatile preserves publications/observations,
not ownership, synchronization or atomicity. Allocation failure, corruption,
concurrent mutation, private EH/fault/unwind cleanup and stale/death lifetime
are not modeled by this ordinary source caller.

Required transitive services remain unimplemented here:

- `0099C6F0..0099C8A4`: full base command/gun/control/random/descriptor work.
- `009B2E50..009B302B`: full composite approach, registry and state construction.
- `009F9CE0..009F9D78`, `009AFE70..009AFFEB`: actual retained producers and
  primary-recovered numeric/tuning/random/pose/geometry contracts.
- Complete `006C0B50`: actual returned record and queue/slot/observer/message
  work; returned record lifetime is external. Current host queue lacks native
  entry+4 record identity and slot-tail transitions, so it is excluded.
- Actual selected entries: MoveTo007B3DB0 is proved empty RET; Follow009BED80
  and Park009B21A0 have real work. Required dispatch cannot substitute no-ops.
- Actual executable table binding, task670 allocation/scalar destruction,
  common/embedded ownership, owner FIFO/scheduler, world/controller services,
  observer routing and retained404/plane3FC lifetime after death.

The sparse `BotTaskHost/BotTaskRecord`, Boolean landing fields and current queue
projection do not satisfy these services. Existing actual observer endpoint and
registration primitives may be reused by complete state constructors; their
MoveTo target observer does not establish lifetime for task404 or plane3FC.
No factory/arena/provider/game adapter is wired. No arithmetic-heavy lower
routine or broad tick/scheduler path is ported.

## Focused validation

MSVC19.51.36244.0 x86 compiled the actual new translation unit with
`/EHsc /std:c++17 /MD /O2 /DNDEBUG /Iinclude`. The existing ignored hook probe
linked that object, the canonical caller object and primary `bsp_core.lib`
using `/MANIFEST:EMBED /INCREMENTAL:NO /OPT:REF` and passed.

One added source-fixture scenario checks same-task base kind3, a base mutation
before owner50 observation, composite publication into shared canonical cells,
captured control-plane reaching the predicate after ordered profile stores,
fresh3FC leader selection ofFollow,
310 publication before a real executable C++ fixture entry, and all aliases
absent until entry completes. Entry replaces canonical404 and current-state;
both mutations survive while the aliases publish afterward. Existing land
views observe the same404 cell and final identities. These C++ fixture tables
and provider effects test caller ordering; they do not implement or execute
original native base/composite/profile/state/arena services.

Existing hook/deferred-route/canonical/validity/numeric checks remain passing.
COFF disassembly of the actual new source object confirms retained3FC capture
at caller offset39 before profile stores43/4C/55 and predicate call5F; fresh3FC
load6D before leader call78; state publication8A before entry91; aliases9E/A6/AF
after entry. Standalone binder stores appear at0A/12/1A in order. These are
compiled SOURCE offsets, not original executable offsets or ABI parity.
JSON and `git diff --check` pass. Four direct native call rows pass with zero
failures; two virtual rows are explicitly unresolved. Root owns CMake inclusion,
the full Win32 build and integration. Worker performed no full build, native
runtime differential test, original ABI test or game/lifetime validation.

Primary integration: 7a7b06aaf63200e18847fb6b4630c28bd3a814e9; actual main sources independently compiled for manifested focused probe, PASS. Complete MSVC Win32 Release rebuild and all three existing CTests passed. Executable SHA256 e4c52573577519e7435eeda2f7bf1aaba70ad8fd52cb782252fdb19b386edbef. Original ABI, actual runtime binding and game validation remain unclaimed. Build receipt: J:\PROG\battlestations-pacific-decompile\local\cc11_vectors_outer_integrated_build.log.
