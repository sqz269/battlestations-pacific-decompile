# CC16: qualified Source C++ cleanup for raw Lua variant retirement

Frozen Source base: `feb4586baf3a521f137f9d10cb35014da61a1ada`; its normal
retirement implementation is the exact 17b77 input. Authorized design 018096266
and sealed independent review 288e47ff are immutable inputs. This packet adds
three qualified ordinary C++ entry points and actual name/key cleanup effects.
It does not install a Native FH3 interpreter or close full host/adoption gates.

Only `native_lua_variant_retirement.hpp/.cpp` and this doc/report change.
Normal signatures, the complete original Frame/Operation declarations/layouts
and legacy Host implementations remain intact. The normal mode still only
retains/rethrows genuine C++ exceptions. It does not execute the new actions.

## Complete-cycle mode and backing

The additive public names are the existing clear/destruct/subtree names suffixed
`_cpp_cleanup`. Each additionally borrows `const NativeLuaVariantCppCleanupBacking&`.
Private compile-time kernels contain the complete three normal bodies and the
qualified whole-range composition. All four recursive edges use the same mode:
clear -> subtree, subtree -> right subtree, subtree -> value destructor, and
destructor -> clear. The new public tail jumps select full qualified kernels;
these are not outer wrappers attempting to replay failed normal-only calls.

The old Frame remains120 bytes; Operation remains24. New Incoming/ActionRecord/
Record/Backing are16/36/56/8 bytes on Win32. They are separate caller-owned,
typed, trivially destructible metadata. There is no owning/cleanup destructor.
Both genuine disjoint spans must cover the actual maximum active call chain,
including range and nested-map frames. No capacity is inferred from map.count,
no fixed maximum or address/history admission is introduced, and no metadata
allocation/logging/callback is added. Each entry checks/prepares its own slot.
Insufficient backing is the existing Source non-completion, outside the admitted
valid/sufficient-backing domain. Every caller propagates it before later raw
reads/free/reset/output effects. It neither throws nor triggers a cleanup action.
A stopped armed scope can therefore retain native cleanup_state0.

Each lexical handler holds its own frame/sidecar reference, local eligibility
and saved home VALUE. Failed active_depth retains inner frames; it is never
used as active_depth-1 to find the handler. Records are passive observations,
not an execution journal. Completed slots can be reused at normal entries;
failed records remain independently live for actual resource disposition.
Callbacks that reenter use independent backing and do not mutate active metadata.

## Exact actions, reads and failure order

| Scope | Normal eligibility/demotion | Qualified exceptional action |
| --- | --- | --- |
| 884AF0 destructor | Numerical D0E6F4; saved V; arm0 before clear. After clear returns, capture CURRENT V+10h, then demote before conditional late V+Ch length/getter/return. | Retain incoming observation; if armed demote before entering the actual C970D0 ->41DD20 contract at saved V+Ch. Capture CURRENT buffer+4, null-test, conditionally current length+0. |
| 6EE890 subtree | Right recursion unarmed. After return capture CURRENT LEFT, K=N+Ch and rewritten argument-home VALUE, then arm0 before value destructor. After return capture CURRENT K+8, then demote before conditional K+4 length/getter/return. | Retain incoming observation; if armed demote before actual C830A0 ->6EE020 contract at saved K VALUE. Capture CURRENT buffer K+8, null-test, conditionally current length K+4. |

The native DC9464/DB10B8 maps are state0 -> -1 with C970D0/C830A0.
Their saved/native FH-adjusted frame locations are evidence, not Source stack
locations. K is the Source VALUE retained at the real rewrite epoch. Native
C830A0 physically reloads its current private argument home; mutable private-home
replacement and arbitrary Original private-stack aliases are **not** supplied
by this Source value custody. Do not reconstruct K from a later node/head/LEFT,
or substitute a failed normal operation's captured buffer. A stale prior K in
a later right recursion never authorizes cleanup because eligibility is unarmed.

The two private action kernels compose the already present full raw41DD20 and
6EE020 read/service definitions to retain captures at their actual epochs. They
do not pre-read and then call a rereading opaque helper. Each nonnull action
captures one late length, wraps length+1 as uint32, calls the SAME actual raw
419CC0 getter, then real BD1510 with captured buffer/size and live AA4. Null
skips length and both services. Each action calls the getter even for large
blocks or disabled small returns. Headers remain untouched; neither action frees
an object/node/map/descriptor or resumes a normal suffix. Numeric profiles and
native action addresses are evidence data, never dynamically executed pointers.

Use genuine AA8 publication, AA0 raw-manager publication and AA4 gate references
from the actual NativeStringRawPoolContext, matching pool deletion binding,
manager/critical-section/ring/storage and existing singleton malloc/free domain.
These bodies manufacture no receiver, provider or context and authenticate none
of them. Real lazy creation and its own existing cleanup/registration behavior
remain the provider's contract. No generic callback/fallback allocator is added.

Incoming site/state/armed/home observations are independent of action buffer,
length/size/pool and stage/entered/completed/secondary records. Original Frame
buffer/site captures are not overwritten by an action. On a genuine C++ escape,
an armed handler demotes, runs its real action, records only action completion,
and bare-rethrows its incoming exception if cleanup returns normally. It does
not pop failed frames, mark the body completed, free a node, traverse LEFT,
reset a map, or perform any remaining normal suffix. An unarmed handler simply
records/rethrows. Clear/range have no added raw cleanup action.

If a real action getter throws, its separate secondary flag preserves the last
action stage and captured custody, and the actual escaping exception is bare-
rethrown without retry. Outer still-armed local scopes can run their own actions
against their own saved homes. Passive records retain no exception object or
identity and cannot promise that the first exception reaches the root. There
is no nested wrapping/replacement exception, catch-and-success, forced terminal
policy, rollback, replay or abandonment destructor. An action returning its
buffer can leave a stale header: actual independent disposition remains required.

The positive primary-propagation claim requires genuine C++ delivery across
these Source scopes and all invoked actions returning normally. A concrete
sufficient pool subset has genuine nonnull AA8 at each action getter, the
matching owner/section/ring live/valid, and existing pool/CRT/OS operations
returning normally. This is a custody subset, not a cached path or added runtime
precheck. Lazy getter completion remains allowed but allocation/constructor/
registration failure cannot imply no secondary escape. Existing BD1510/free
noexcept/CRT/OS boundaries remain unchanged. SEH, access faults, longjmp/panic,
existing noexcept termination and other nonlocal exits need not reach /EHsc
handlers. Original secondary-terminal behavior remains unclosed.

## Complete strict object and explicit normal/EH drift

MSVC19.51 Win32 /c passed without warnings using /std:c++17 /O2 /Ob2 /MD
/EHsc /W4 /WX /fp:strict. Three local compile iterations were justified by code
inspection changes; only the final object is the candidate receipt. No executable
was linked and no reconstructed body was run. Complete symbolic decoding covers
all36 code sections,5428 bytes,1631 operations and73 direct calls. Only real
pool getter/return/free plus existing C++ runtime/cookie symbols are unresolved.
The current Core definition index contains all three real provider definitions;
its hash/member receipts are static, not a new whole-source/archive build claim.

The complete656 EH bytes include six SafeSEH references and all six xdata tables,
with every relocation/handler target inspected. Four tables have maxState2 and
one catch-all; qualified destructor/subtree have maxState4 and two catch-alls.
Their unwind entries are (-1,-1,1,1), with no native cleanup-action pointer.
The actual actions are explicit Source calls inside local handlers. The nested
secondary handlers set their distinct flag and share the bare-rethrow tail.
This generated Source EH structure differs from the original one-entry maps.

| Function | 17b77 full public bytes/ops | New normal public selector | New complete normal kernel bytes/ops | Qualified kernel bytes/ops |
| --- | --- | --- | --- | --- |
| clear | 849/260 | 23 bytes | 852/261 | 973/300 |
| destructor | 356/120 | 23 bytes | 359/121 | 493/160 |
| subtree | 478/155 | 27 bytes | 484/157 | 630/198 |

Normal kernels differ by3/3/6 bytes because internal mode/backing arguments
change calls/stack adjustments. All four normal raw schedules/status exits and
retain/rethrow catches were reviewed. Ten shared support bodies and normalized
relocations exactly match17b77. The three normal EH handler byte bodies match,
but their FuncInfo relocation identities change. Normal xdata raw88-byte tables
match the old raw tables; symbols/handler offsets relocate to the new kernels.
SafeSEH grows12 ->24 bytes. There is no unchanged whole-object/EH relocation claim.

Compiler-generated state placement is qualified explicitly. In the qualified
destructor, Source EH state0 appears at7Bh and physical eligibility=1 at84h,
before nonthrowing volatile profile91h/home stores; saved home is present atB5h
before the actual clear callBFh. Normal buffer captureF4h precedes retained
demotionFAh and physical disarm104h. On exceptional entry, retained demotion1B3h
and action-stage demotion1BBh precede real action call1C6h; the now-dead local
false store is eliminated. These facts establish synchronous C++ behavior at
the reached throwing calls, **not** Native AV/SEH/unmasked-FP/private-frame state
epochs across pure volatile operations. Subtree eligibility is physically zero
before right recursion and after normal key demotion, and one before value call.
Metadata initialization includes SSE bit copies; it is not floating arithmetic
or an Original FP/fault/atomic-access claim.

Both normal and qualified endpoint-to-predicates/root intervals contain zero
CALLs: D9h..17Fh and126h..1CCh in the respective clear kernels. Fallible/backing
preparation occurs before them. Both modes retain four distinct post-subtree
CURRENT-head reads, count/link resets and owner-then-node output order, followed
by a separate current-head free reload. Every native tag and any nonzero nil
byte remain accepted; traversal is never admitted from count/map+0.

## Inputs, custody and admission limits

The installed PE hash and all11 frozen spans/751 bytes match current disk.
Twenty-one unedited dependency pins (the design's23 minus the two owned Source
files), five original compile artifacts, immutable normal/design/review inputs
and full new Source/object/EH/artifact receipts are pinned in the report. The
old full host, queue/payload/vector/reserve/copy, scalar dispatch, genuine producer
and CF/D0 callable transport remain separate. Parent/independent review and common
compile/link/build admission remain Root's next gate. No Original ABI/FH3/SEH,
secondary-terminal, vendor/duplicate/new Original body, runtime or activation
credit is claimed. No new repository tests, CMake/shared ledger/guard/caller/
Ghidra/peer/install change, full build, executable/probe/DLL/game/body run occurred.
