# Native particle emitter lifetime

Addresses: `00AFF690`, `00B04F00`, `00B053D0`, `00B04E30`, `00B04B50`,
`00B04900`, `00B04910`, `00B04A80`, `00B04C40`.

The lazy emitter getter now constructs its actual 30h owner, cookie-backed index
rows and 6Ch state backing. The state cleanup implements the complete recovered
control and lock behavior through required actual application callbacks. These
are new C++ interfaces; descriptive names are hypotheses, not recovered symbols.

## Coverage and original ABI

| Routine | Inclusive bytes | Original ABI | Coverage |
|---|---|---|---|
| AFF690 acquire | AFF690..AFF6F5 | ECX emitter; EAX current10; RET | Complete through existing CRT allocation/free and recovered constructors |
| B053D0 derived construct | B053D0..B053FB | ECX raw30h; stack(model,emitter,capacity); EAX same; RET0C | Complete |
| B04E30 base construct | B04E30..B04EF5 | Same three arguments; EAX same; RET0C | Complete, including member-unwind order |
| B04B50 replace rows | B04B50..B04BFE | ECX actual embedded0C; stack DWORD count; RET4 | Complete for concrete B04900/B04910 row callbacks |
| B04900 initialize row | B04900..B0490F | ECX actual8h row; EAX same; RET | Complete |
| B04910 destroy row | B04910 | ECX actual row; RET | Complete: original instruction really is RET |
| B04C40 destroy rows | B04C40..B04C76 | ECX actual embedded0C; RET | Complete |
| B04A80 destroy states | B04A80..B04AA0 | ECX actual embedded14; RET | Complete |
| B04F00 cleanup state | B04F00..B04FB0 | ECX actual6Ch state; stack full DWORD; AL result; RET4 | Complete through required current definition1C,72B740,B7C160,B6DFA0 |

The worker used the verifying `bsp.py ghidra` commands against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 LE32, image base
00400000. It made no Ghidra mutations or shared metadata changes. Sixteen exact
live/installed-PE spans, 1422 bytes, matched; their hashes are in the report.

## Actual storage and construction

The containing emitter is borrowed raw storage already produced by AFF5F0.
AFF690 allocates only when its current10 is null. After allocation it reads
current definition0C+20, then current model08, invokes the constructor, publishes
the result to emitter10, and reloads that slot for return. Construction failure
frees the raw30h slot without publishing. The complete nonnull getter remains
available to AF6BE0, whose existing call site first tests emitter10 itself.

The base writes D5DF20 to00, model04/emitter08, zero row0C/10 and state14/18
descriptors, and zero1C/20/24/28. It preserves2C. Only a positive **signed**
capacity allocates backing. Rows allocate saturated `(capacity*8)+4`, including
the DWORD count cookie; each initializer writes WORD0=0 and float4=+0, preserving
bytes2..3. State backing allocates saturated `capacity*6C` without initializing
state bytes. The final loop stores truncated WORD indices in the current row
array. The derived constructor writes D5DF24 and clears2C. The container has no
reference count: model04 and emitter08 are borrowed pointers, not retained owners.

B04B50 always replaces backing, even when the descriptor capacity is sufficient.
Old destruction visits rows backwards using the allocation **cookie**, not the
descriptor capacity. New rows initialize forwards using the signed CRT iterator
count; only afterward are data and capacity published. The concrete B04900 cannot
throw. On allocation failure after an old array was freed, the old descriptor
remains dangling, matching native partial state. No repair or successful fallback
is inserted. Member destruction frees first, then zeroes capacity and pointer.

The actual CRT allocation service is the existing `singleton_lifetime_allocate`
malloc/new-handler/throw loop; BF55BE and BF681B have that same observed contract.
BF6989 tail-jumps BF65AC, represented by existing `singleton_lifetime_free`.
This reuses the repository's CRT boundary rather than introducing another heap.
The original allocator's internal exception-object/global identity is not claimed.

DF3868 contains base unwind states `(-1,CBB760)` and `(0,CBB76B)`: destroy states14
through B04A80, then rows0C through B04C40. AFF690's CBB340 frees the raw container
through BF65AC after the constructor's member unwind. B04B50's CBB740 frees its
cookie allocation if its vector construction fails; with concrete nonthrowing
B04900/B04910 there is no C++ row-construction exception path to emulate.

## Current callbacks, aliases and lock behavior

B04F00 captures current state64, its current table, and current virtual1C before
calling with ECX definition and stack(state,full DWORD argument), RET8. The binding
capture method is a nonmutating dispatch lookup for those already captured
identities; it must not reread them. The original AL is retained across all later
calls. B05070/B05110 pass0; B04FC0 passes1. The entire DWORD and exact returned byte
are preserved, including non-boolean bytes. B05070 ignores that result.

After the virtual returns, cleanup tests **current** state60. A nonnull value
invokes the complete actual72B740 getter, then captures owner04 as a physical
`TrackedCriticalSection*`. It enters that actual Win32 section and increments its
actual DWORD18. It reloads state60 for B7C160, reloads state60 again for B6DFA0,
then clears60 before decrementing and leaving the captured section. A callback
may replace the pointer; there is no second null test. The source retains these
capture/reload boundaries and DWORD counter arithmetic.

The two application operations remain explicit required real bindings:

- B7C160 visits the actual light's current1E0 backlink array, calls B6F3C0 on each
  model with that actual light, and resizes the light's array to zero.
- B6DFA0 unlinks the actual parent/root and captures the node's **current**
  virtual18. Its real lifetime binding must preserve the sole actual04 count,
  existing canonical node association, terminal destruction and pool ownership.

There is no canonical native point-light owner or72B740 singleton binding in this
checkout. `GeneratedModelPointLightLinks` is documented as a separate diagnostic
representation and cannot fulfill this actual-owner contract. No point-light,
model, reference count, publication, critical section or lifetime domain is
created as a substitute. Application composition supplies these real operations.

The original guard is armed only after getter/Enter completes. On an unlink or
release exception, the source invokes existing canonical411EE0 with the actual
eight-byte guard and captured raw section; current state60 stays uncleared. Normal
exit decrements/leaves directly. This is C++ exception behavior, not reproduction
of the original MSVC SEH frame or arbitrary hardware exceptions.

## Calls and assembly corrections

The report supplies `address`, `native`, `function` and original ABI for every
body call, iterator callback and relevant unwind call. Numeric tail destinations
carry `kind: tail_jump`. The concrete row initializer/destructor targets at
BF7CF5/BF7C99 were verified from B04B50/B04C40 argument pushes and CRT bodies;
the constructor iterator returns14h and destructor iterator returns10h.

The free thunk is incorrectly marked no-return in saved analysis. Installed
assembly establishes the omitted continuations:

| Gap | Instructions | Owner |
|---|---|---|
| B04B8A..B04B8C | ADD ESP,4 | B04B50 |
| B04EA7..B04EA9 | ADD ESP,4 | B04E30 |
| B04A8F..B04A91 | ADD ESP,4 | B04A80 |
| B04C64..B04C67 | ADD ESP,4; POP EDI | B04C40 |
| CBB349..CBB34A | POP ECX; RET | CBB340 unwind |
| CBB749..CBB74A | POP ECX; RET | CBB740 unwind |

Each ordinary allocator/free stack argument is balanced by ADD ESP,4; unwind
free calls use POP ECX. B04B50 is a replacement allocator, despite its current
`CG_vector_deleting_dtor` analysis label. The integrator owns definition/name and
export/ledger updates. Missing row definitions are exact: B04900 ends with the
one-byte RET at B0490F, end-exclusive B04910; B04910 is a one-byte RET, end-exclusive
B04911. The five unwind entries are already defined; only the two free suffixes
listed above lie beyond their saved body ends.

The read-only review on 2026-09-12 confirmed the existing CBB340 and CBB740
bodies end at CBB348 and CBB748, respectively. Extend them through CBB34A and
CBB74A; do not create duplicate unwind functions. CBB760, CBB76B and CBB780
already have complete bodies ending at CBB76A, CBB775 and CBB787. The report
records exact live bytes and end-exclusive repair ranges.

## Verification and limits

Strict MSVC Win32 `/W4 /WX /fp:strict /EHsc` compilation passes. The full
`scripts/build.ps1` run, then the seeded run after all eight `verify-seeds` matches,
passes `reconstructed_math` and `native_math_differential`. New source is compiled
separately because this worker does not own CMake integration.

The final review repeated strict compilation, the existing fixture, and all 16
live/installed byte-span comparisons successfully. The call-report check examined
24 rows and failed only the two numeric iterator targets whose B04900/B04910
function definitions are missing. The integrator must add those definitions and
rerun that check; the worker leaves Ghidra unchanged.

One ignored fixture, `local/emitter_probe.cpp`, executes the nine original spans
and seven actual native ownership callees through relocated original bytes.
Its allocation/free adapters forward the real existing CRT service; vector
adapters invoke the relocated actual row constructor/destructor. The cleanup
provider invokes actual original B7C160/B6DFA0/B6F310 and their pointer helpers.
Unreached reserve/root/attachment/terminal paths fail if reached, rather than
reporting successful behavior.

Comparisons pass for row padding/+0 float, preserved base2C, lazy publication and
repeat acquisition, positive/zero/negative signed capacities, cookie-based row
replacement and member free/reset. Eight cleanup combinations cover an actual
model backlink removal, real parent unlink, actual04 decrement2-to1, the physical
Win32 lock, null lights, and a callback replacing state60 between unlink/release.
The definition callback returns5A with full argument12345678. Source-only checks
confirm real CRT allocation failure leaves the replacement descriptor dangling
and lazy publication null, and an unlink exception leaves state60 current while
the canonical guard releases the actual lock. Original SEH throwing paths are
not executed by this fixture.

This is bounded differential evidence, not completed real application bindings,
terminal zero-reference destruction coverage, original ABI/SEH compatibility,
simulation update, particle model construction, game validation or visual proof.
Compose the getter/cleanup through the existing
`RegisteredModelEffectBehaviorCallees::call_00aff690/call_00b04f00` interface; AF6DD0,
the full72B740 domain, current definition callbacks and actual light/node lifetime
bindings remain required. No shared CMake, ledger, packet or Ghidra file was edited.

## AK saved-analysis and combined-build integration

The seven-module AK batch is registered in bsp_core. Strict MSVC Win32
compilation and both seeded CTests passed with explicit `--parallel 1`; the
standard parallel script hit environment MSB3491 before compiling C++.
The report records saved Ghidra name/signature preimages, prior-comment
preservation and readback, original-byte fixture coverage and exact call checks.
Reported returning-free continuations and missing definitions are now repaired
and saved; worker-era pending-integration notes above describe the earlier snapshot.
New C++ interfaces and required real runtime bindings remain as documented.
Successful full construction, native EH compatibility and gameplay are not implied.

## AK final merged validation

After merging current main at `aab1373abde21d9a8d03ad113f8a53317cfd8ff5`, the repository standard
`./scripts/build.ps1` completed successfully and both existing seeded CTests
passed. The focused original-byte fixture was rebuilt with `/fp:strict` and
replayed against that combined library; it passed. The report pins its log and
library hash. Earlier parallel MSBuild failures and worker-pending notes above
are historical; the final build required no global configuration change.
All stated constructor, simulation, current-slot, native EH and gameplay limits
remain in force.

The parallel MSBuild invocation remains intermittent: a later documentation-only
rerun again hit MSB3491 before C++ compilation. The final serial full build and
both existing CTests passed again on unchanged source. This environment issue
was recorded rather than changing global permissions or build configuration.
