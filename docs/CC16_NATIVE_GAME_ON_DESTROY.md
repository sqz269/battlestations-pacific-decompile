# Complete normal raw Game OnDestroy (CC16)

`on_destroy_native_game_004dc5c0` supplies the complete normal parent sequence
of `004DC5C0..004DC69A`: 219 bytes, 67 instruction starts, seven direct CALLs,
six indirect CALLs and final `JMP004A9AC0`. It uses the actual `71A0h`
`NativeGameStorage`, the SAME `NativeGameLifetimeContext`, and required genuine
site bindings. Existing `mission_lua_teardown_004dc5c0` remains its original
three-step Lua projection; its API, source and callers are unchanged.

This packet is pinned to `605373bf732e4bd3089e8f07ddca226ab4d635a3`. Origin
had advanced to `f1431e56da01920b5eec31747ff86078e7222a57` when the assigned
fresh worktree was created; that later source is not silently included. No
Game runtime attachment, shared startup/CMake/deletion context, ledger or Ghidra
program was changed. New/duplicate native-body, vendor, ABI and activation
credit is zero. Root admission, actual provider closure and gameplay remain
separate.

## Original contract and evidence

Every live analysis batch used the target-verifying `bsp.py ghidra` client:
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 LE32, image base
`00400000`. The complete native assembly and 219 bytes are retained under
`local/native_game_on_destroy`. Native-span SHA256 is
`8c32ce9af196221132a316e98d790da4b2162420d718879aba910e57b2a94cd4`.

Native entry has ECX=Game, captures it in ESI, preserves ESI/EDI, takes no
stack arguments, and restores those registers before tail-jumping at `4DC696`.
This new C++ interface returns `void`. It does not reproduce the physical tail
or an incidental EAX/flags/FP result from `4A9AC0`. The sole original caller
`737FB9` discards that result: its subsequent path reloads Game, then overwrites
EAX with the scalar method at `737FC7`. No binary-entry ABI or native fault/EH
transport is inferred from the source projection.

The source preserves this schedule:

| Native site | Actual effect / source binding |
| --- | --- |
| `4DC5C9` -> `4254B0` | Call with the required mapped CE7C98 trace literal. Original callee is exactly `C3`; a private exact one-byte RET shim reuses that proven no-op, with zero additional body credit and no invented logging provider. |
| `4DC5D1` | Store DWORD10h at Game+5D4. No other Game state is reset here. |
| `4DC5DB` -> `46F3E0` | Required whole scene-database deletion binding. |
| `4DC5E2` -> `4DA780` | Required whole session teardown, carrying the exact raw Game ECX. |
| `4DC5E9` -> `4DA650` | Required whole frontend teardown; raw Game argument retains the call-site ECX value, not a new class layout. |
| `4DC5EE..4DC601` | Load current F8A2FC once. Only if that captured receiver is nonnull, invoke its CURRENT table byte slot8 with no arguments, then call required `992AC0`. A callback change/nulling of F8A2FC does not suppress this second call. That callee must observe its own CURRENT actual publications, not the earlier capture. |
| `4DC60C` -> `B65E80` | Reuse the existing complete normal actual-storage `B669A0` close on Game+1A0C. Capture state04, close iff state nonnull and owns00 nonzero, then clear CURRENT state04 after return. Keep ownership/tracking metadata untouched. |
| `4DC617` -> `8844F0` | Load current Game+1A08 and call required whole raw machine-release helper unconditionally, including null. No added parent null guard. |
| `4DC62C`, `4DC644`, `4DC65C` | Reload Game+1A08, +21A0 and +21D8 in order. If each captured pointer is nonnull, invoke current scalar slot0(flags1), then clear its field at `4DC62E`, `4DC646`, `4DC65E`. |
| `4DC674`, `4DC68C` | Likewise reload actual E18678, then actual E1867C; scalar slot0(flags1), then clear at `4DC676`/`4DC68E`. These are references borrowed from the SAME lifetime context. |
| `4DC696` -> `4A9AC0` | Reuse the qualified `NativeGameLifetimeCalls::call_004a9ac0` on `lifetime.calls` with `lifetime.classes`, `lifetime.profile`, `operation.classes`. This forces the genuine default's matching call service and actual string/pool/manager checks, then existing complete raw class cleanup; an override cannot bypass those checks. |

Every guarded scalar call captures its receiver before the call and clears the
CURRENT owning field/publication afterward. A callback replacement is therefore
also discarded by that clear, as natively; there is no restoration or rollback.
Null initial values receive no scalar call and no clear. The machine release
precedes a separate reload of Game+1A08; its pre-call pointer is not reused for
the following scalar delete. Explicit volatile DWORD accesses preserve these
epochs. Address additions use Win32 unsigned arithmetic; no bounds or pointer
substitution is introduced for the admitted valid owner.

## Genuine dependencies and honest gaps

`NativeGameOnDestroyContext` borrows the whole existing
`NativeGameLifetimeContext&`, actual volatile F8A2FC, mapped CE7C98 and one
`NativeGameOnDestroyCalls&`. It does not duplicate E18678/E1867C, calls, profile
or class authorities. The lifetime scalar provider must admit the actual child
families and their matching heaps. `NativeGameCurrentVirtualCalls` is available
glue only for genuinely callable current tables; numeric PE profiles such as
CFD484 are not admitted merely by being copied into an object.

The required interface has **no defaults** for these six bindings:

* Whole `46F3E0`: existing `scene_database_destroy_singleton_0046f3e0` can drive
  a genuine Seg6 service, but current `SceneDatabaseSeg6Host` explicitly
  `not_reached` on its deleting virtual method. That private application host
  is not a qualified teardown supplier.
* Whole `4DA780`: `clear_all_screen_set_levels_004da780` is only its
  `4DA99D..4DA9CF` fragment, not full session teardown.
* Whole `4DA650`: existing `destroy_front_end_managers_004da650` clears its
  in-mission pointer without implementing that object's destruction. It cannot
  stand in for the full native parent.
* Current captured online slot8: actual table/code/lifetime and the no-argument
  ECX method shape must be supplied. No raw PE function pointer is invoked here.
* `992AC0`: existing semantic `destroy_online_system_00992ac0` needs real
  current publication/factory custody; this entry cannot bind a projected
  factory or replace callback-sensitive globals with local copies.
* Raw `8844F0`: the existing landscape helper takes semantic
  `MissionLuaHostState` with C++ containers. It is not a raw14h host view and is
  not reinterpreted as one. Its whole raw binding remains required.

These are explicit open callee/provider contracts, not successful empty
callbacks. This parent source does not close those callees, their missing
producers, or application admission by itself.

`B65E80` and the existing `B669A0` helper have the same complete normal
state/owns read, Lua-close call, post-call state clear, alias effects and
untouched metadata. The source reuses that helper; it does not infer native
register calling compatibility, invalid-storage behavior, Lua nonlocal transfer,
foreign C++ transport or fault equivalence. The final class-clear operation is
separate from the later storage destructor's own class-clear operation, as in
the original OnDestroy-then-scalar-delete schedule.

## Source operation and validation

`NativeGameOnDestroyOperation` retains the actual raw owner, context, native
site, historical captured address and real class-cleanup operation. One attempt
per operation and serialized access are required. Source C++ failure marks the
operation failed and rethrows after retaining every completed effect. It never
rolls back fields, resets publications, frees children or retries. Running/
failed destruction rejects silent abandonment. Diagnostic acknowledgement is
allowed only after external resolution; it does no cleanup. This is source
protocol metadata, not an irreversible native latch, arbitrary parent reentrancy
guarantee or original exception-frame implementation. Context reference bindings
and services remain stable; allowed callback field/publication mutations are
observed at the native epochs above.

Strict MSVC Win32 object compilation passed with C++17, `/O2 /Ob2 /DNDEBUG /MD
/EHsc /W4 /WX /fp:strict`. The compiler is 19.51.36244.0. No executable was
linked or run. Existing call verification checks the seven direct CALL sites
and final tail JMP against the target; six indirect sites are separately listed
with receiver/slot evidence. No test or probe was added or executed.

Final COFF inspection shows the actual state write at entry+77, online load and
gate at+B3/+BB, slot callback at+CD and unconditional in-arm `992AC0` at+DB,
real raw Lua close at+F1, machine capture/call at+105/+110, distinct reload at
+113, five scalar calls followed by their clears, and final real lifetime class
call at+22D before completion. The private trace section is exactly one byte
`C3`. Initial `__asm ret` encoded `C2 00 00`; that artifact is retained, and the
source now uses `_emit0xC3`, recompiled and rechecked. This is an emitted-source
audit, not whole native/source byte identity, a native differential run, final
link proof or runtime acceptance.

Root must supply the six genuine bindings, exact publications/literal, matching
lifetime contexts and child dispatch custody, then register/build/link this
source and review its normal qualifications. Ordinary shutdown still requires
OnDestroy at737FB9, then scalar-delete(flags1) at737FCB, then Application+14
clear at737FCD. No attach or guard flip is made here; SAP2 class policy and the
accepted accuracy cohort are untouched.
