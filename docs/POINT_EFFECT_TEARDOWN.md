# Point-effect instance teardown

`src/point_effect_teardown.cpp` implements complete typed bodies for array
destruction `008675B0`, direct instance destruction `00867680`, and scalar
deletion `00867CE0`. It reuses the actual `PointEffectReferenceArray` and
`PointEffectInstanceStorage` from `point_effect_instance.hpp`. These are new
C++ interfaces with existing borrowed companions, not native vtable overlays.

## Target, ownership and original ABI

Every live query used `bsp.py ghidra`, which verifies project `bsp`, program
`/battlestationspacific.exe`, x86 language and image base `00400000`. The
configured `C:/Users/sqz269/bsp.gpr` file was also checked to exist. This packet
made no Ghidra mutations; the integrator owns repair, annotation, save and
export refresh.

The owner relationship is corroborated by native evidence rather than a type
name: `86769F` writes `D0D3EC`; `8680EB` writes that same table in the constructor;
the destructor accesses the constructor's arrays `+0C/+18`, retained members
`+84/+8C`, and node `+110`. Live table words at `D0D3EC` are `8683E0,867CE0`.
Thus `867CE0` is virtual **+04**, and `8683E0` is the separate virtual **+00**
terminal dispatcher. The latter chooses immediate scalar deletion or manager
deferral using frame-job-pool virtual+10 and `868010`; this packet does not
replace that behavior with direct scalar deletion.

| Address | Original ABI | Inclusive end |
| --- | --- | --- |
| `008675B0` | ECX points to actual `0C`-byte header; no stack arguments; RET | `008675C6` |
| `00867680` | ECX points to actual `114`-byte instance; no stack arguments; RET | `0086778B` |
| `00867CE0` | ECX instance; one flags DWORD; EAX original pointer, including after free; RET4 | `00867CFD` |

No useful EAX result is established for the two direct destructors. Original
saved names were `FUN_008675b0`, `FUN_00867680`, and
`CG_scalar_deleting_dtor_00867ce0`, with no entry annotations returned by the
comments query. Descriptive names remain provisional interpretations.

## Complete operation order

`8675B5` calls canonical `8672A0(0)`, then `8675BA` reloads the current backing
pointer and frees it through `BF6989 -> BF65AC`. Count reaches zero through the
existing resize semantics, including descending release and callback reentry.
The native body does not clear the dangling backing pointer or capacity.

| Native instructions | Direct instance operation |
| --- | --- |
| `86769F..8676B2` | Publish `D0D3EC`, capture node+110, decrement actual `F87600`, activate unwind state4. |
| `8676BA` | Canonical `B6DFA0` on that captured node: unlink parent/root, then actual virtual+18. |
| `8676BF..8676DB` | Reload current node+110, decrement its actual +04, invoke its current real virtual+00 only at zero. |
| `8676DD..8676FE` | Capture parent+8C; activate state3; release if nonnull; clear +8C after terminal return. |
| `867708..867729` | Reload template+84; activate state2; release if nonnull; clear +84 after terminal return. |
| `867733..867747` | Activate state1; resize auxiliary+18 to zero, reload and free backing. |
| `86774F..867763` | Activate state0; resize entries+0C to zero, reload and free backing. |
| `86776B..867775` | Set state-1; call `BD30F0`, whose complete body writes `CEB130` and returns. |

The first node operation can release a logical self reference independently
of the subsequent +110 reference release. Those are not combined. In
particular, virtual+18 can cause +110 to change: the second operation must use
the new current node. There is no null check on either native node use. The
instance and any captured reference slot must survive reentry; a terminal
callback may retire its node companion, which is not accessed afterward.

The destructor decrements only `F87600`, using unsigned modulo32 arithmetic.
It never writes `F87604`, instance reference count+04, owner+88, or node+110.
The two latter pointers can remain stale. No assumed owner+88 release or
additional counter rollback is added. Parent/template slots are cleared only
when their captured pointer was nonnull, after the callback; such a clear
overwrites any replacement installed in that same slot by its callback.

`867CE3` calls the direct destructor. Only after successful return does
`867CE8` test low flags bit0. If set, `867CF0` calls ordinary `BF65AC` free.
Both branches then return the captured original pointer in EAX. This free is
the physical instance allocation return, separate from all node ownership.

## Returning-free flow and exception evidence

Saved Ghidra flow incorrectly stops after the free at `8675BD` and `867747`;
the scalar deletion free branch omits stack repair at `867CF5`. Live bytes
matched independent Capstone decoding of installed disk bytes. Complete tails
are `8675C2..8675C6`, `86774C..86778B`, and `867CF5..867CF7` respectively. In
particular the omitted direct-destructor tail contains the entire entries+0C
cleanup and base destruction; it is not an unreachable epilogue.

The destructor handler `C94EFA..C94F03` loads descriptor `DC6D4C`, then jumps
to `BF6B43`. Its five-state unwind map at `DC6D70` is:

| State | Next | Funclet | Cleanup |
| --- | --- | --- | --- |
| 0 | -1 | `C94EC0..C94EC7` | `BD30F0` base identity. |
| 1 | 0 | `C94EC8..C94ED2` | Array+0C via `8675B0`. |
| 2 | 1 | `C94ED3..C94EDD` | Array+18 via `8675B0`. |
| 3 | 2 | `C94EDE..C94EEB` | Template+84 via `41DE40`. |
| 4 | 3 | `C94EEC..C94EF9` | Parent+8C via `605FD0`. |

The implementation's guard follows these state transitions, cleaning current
remaining members during unwinding. It does not retry the member whose normal
cleanup had already begun. A node terminal-lookup exception therefore cleans
parent, template, auxiliary, entries and base; it neither retries the node nor
frees the physical instance. Secondary cleanup failure terminates under the
nonthrowing unwind guard. Arbitrary throwing native virtual implementations
are outside the existing canonical nonthrowing terminal contract.

## Required existing bindings

`PointEffectTeardownBindings` only borrows the existing node lifetime runtime,
actual-owner lookup, parent reference projection and actual counters.
`B6DFA0` uses `GeneratedModelLifetimeRuntime::resolve` and its current required
virtual+18 implementation over the same `NativeNodeBinding::transform`.
The subsequent node release calls `release_native_render_actual_owner` with
the actual `NativeNodeStorage` address, not a host companion address. That
helper decrements raw+04 first and resolves/validates the canonical companion
only when it reaches zero. The companion must borrow that exact atomic and
provide the current real terminal operation. Missing bindings never become
no-op cleanup or an assumed constructed-type destructor.

The branch has no complete retained companion for the plain node constructed
by `B6F5A0`. That node's terminal destruction and pool return remain required
caller-provided capabilities, even though the complete calling destructor
body is now present. No second node map, array container, backing allocation,
reference count or implicit owner lifetime is created. Parent/template and
entry releases reuse their existing `RenderCommandReference` companions.

## Validation and remaining boundaries

Strict direct MSVC Win32 `/std:c++17 /O2 /W4 /WX /fp:strict /MD /EHsc`
compilation passed. The ignored `local/point_effect_teardown_probe.cpp`, linked
with `/MANIFEST:EMBED`, passed one focused lifecycle fixture with injected
virtual callback reentry, current+110 replacement, zero-only actual lookup,
parent/template slot mutation, auxiliary-before-entry release, preserved
count/pointer words, unsigned counter wrap, flags2 versus flags1, and complete
remaining-member unwind after lookup failure. It uses synthetic terminal
observers over actual borrowed count words and canonical node logical release;
it does not validate the missing plain-node terminal disposal policy.

`verify-seeds` passed. `scripts/build.ps1` passed `reconstructed_math` and
`native_math_differential`. The new teardown and its previously unregistered
entry-array dependency were compiled directly for the fixture; the integrator
must register them in `cmake/startup.cmake`. Existing CTests do not exercise
these teardown bodies. No permanent tests were added.

This packet does not complete the instance constructor, manager deferred
release, effect virtual+00 dispatcher, or plain-node retained owner. No native
differential teardown test, ABI compatibility, game execution, game validation
or installation change is claimed. See `reports/point_effect_teardown.json`
for the concrete evidence and validation record.
