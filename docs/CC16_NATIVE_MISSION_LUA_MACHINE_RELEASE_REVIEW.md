# Independent review of raw mission Lua machine release

No blocking source-admission finding was found in `f8b4efc7dbfa8eeaccb5e2bcbcea67e5b6aad821` for the bounded normal raw helper and its genuine-provider preconditions. The review tree is frozen at that candidate (source base `5ec9904f55e48f5d3cd871444b510b5cb5e1532b`). Only this document and its report are written. No application binding, activation or broader Lua/Game lifecycle claim follows.

## Boundary and raw input

Fresh configured-target Ghidra inspection, retail PE bytes and the exported listing agree on **27 bytes /12 instructions**, `008844F0..0088450A`, SHA-256 `64dd024293fc26e34fb40751955fa34e556258ec669f77abc1b4663d19f2a886`. The next five bytes, `0088450B..0088450F`, are `CC` padding. The older plate ending `0088450E` is overbroad.

Native `ECX` is the host, retained in `ESI`; there are no native stack arguments and the function ends with plain `RET`. The first field read is unconditional host `+4`. No host-null guard exists, including at its unconditional raw OnDestroy call site. The new helper correctly requires valid genuine raw host storage and does not manufacture successful null-host handling or a complete host/container class.

Only the captured machine is tested. A null machine gets neither call nor write. A nonnull machine supplies its current primary table's byte slot zero with flags one at `00884500`; only after normal return does `00884502` clear the same retained host `+4`. Callback replacement is unconditionally overwritten by that clear, and the child is not read after it may have deleted itself. The source preserves those capture, null and success-store epochs; a C++ exception skips the post-call clear without adding recovery or rollback.

## Actual source dispatch chain

The helper requires the existing **concrete final** `NativeGameCurrentVirtualCalls&`. Its explicitly qualified `virtual_scalar(captured,0,1)` selects the real implementation, which explicitly delegates to existing `NativeGamePhysicsLifetimeCalls::physics_virtual_scalar`. The latter reads the captured object's current actual table/method and invokes the typed Win32 thiscall method. There is no new abstract/default callback, semantic `MissionLuaHostState`/`LuaMachine` cast, table, global, heap or original-PE-address translation.

This closes the source call chain, not receiver provenance. The real component, valid host through clear, genuine child constructor/table/code lifetime, exact call shape and matching deletion heap remain required. A nonnull raw pointer or an existing projected Lua class does not establish those conditions. Original numeric profile/code entries must not be used as callable host tables.

The known original `CF8394 ->6B87C0` role and that scalar's `RET4` were checked as identity/ABI evidence only; the following DWORD is the `nSlu` text, not another method. The original parent has **zero direct calls** and one `CALL EDX`. The candidate correctly avoids a fabricated direct-native-callee row or a forced runtime target claim.

## Fresh compile and independent emitted checks

All three source units passed fresh MSVC Win32 object compilation with `/c /std:c++17 /O2 /Ob2 /DNDEBUG /MD /EHsc /W4 /WX /fp:strict`. No executable was created or run. An independent COFF section/symbol/relocation parser found exact fresh/retained matches for:

| Function | Bytes | Actual transport |
| --- | ---: | --- |
| New raw release helper | 38 | Unconditional host `+4` load, machine-null branch, pushes flags `1`/slot `0`/capture, direct REL32 to the real concrete component; retained-host clear only after return. |
| Concrete current-call component | 8 | Adjusts compiler service `this` by `+18h`, then tail-jumps through a REL32 relocation to the real physics method. |
| Physical current-table dispatcher | 22 | Loads captured child into `ECX`, reads its current table/method, pushes the full flag word, calls indirectly and returns with `RET0Ch`. |

The helper also adjusts compiler service `this` by `+4` before its direct call. These two adjustments belong to the C++ multiple-inheritance service object, not the raw host or child. The final dispatcher replaces `ECX` with the actual captured machine. The child's required ordinary thiscall contract consumes its one flag word with `RET4`; source `RET0Ch` consumes the three source-dispatch arguments. The helper's saved host register survives this chain for the final clear. There is no FP arithmetic or control-word change in the three inspected functions.

All 15 author-retained artifact hashes match, including the three objects/listings and static audit. Original PE/body/padding and the live indirect transfer site were checked independently. There is no claimed direct-call verifier success for a call that is actually indirect.

## Remaining admission limits

Root still owns CMake/build/link registration and actual OnDestroy `call_008844f0` binding to the site's real raw host. Host construction, child callable-table/constructor/free custody and the later host-scalar deletion are separate. The source helper supplies none of those producers and changes no semantic API or caller.

Original register/result/flags/frame ABI, invalid-storage fault identity, FH/SEH/foreign exceptions, Lua nonlocal error transport and concurrent/reentrant teardown remain unproved. No new test, fixture, full build, probe, game, target DLL call, source/tool/ledger edit or Ghidra mutation was performed. Native/vendor-body, activation and new gameplay credit are zero.

Exact input/object/code/relocation/artifact hashes and proof scope are in [the review report](../reports/cc16_native_mission_lua_machine_release_review.json).
