# Native base killed sequence

Address: `00928C80`. Source: `src/native_unit_killed_base.cpp`.

| Routine | Original ABI | Coverage |
|---|---|---|
| `00928C80..00928F44`, 709 bytes | ECX=canonical unit; no stack arguments; callee-saved ESI/EDI/EBX; plain RET | Complete normal source behavior over borrowed actual owners with explicit required wrapper/virtual contracts. Original FH3, SEH, stack aliases and FPU status are unproved. |

The earlier `mission_entity_on_killed_00928c80` remains a separate partial Lua/log projection. This source does not invoke it. `NativeUnitKilledBaseView` borrows the existing `NativeUnitObserverAlias`, owner+30, type+C4, established `PoseRefreshView`, name header+154/+158, unsigned16 ID+174, and cached-key length+178. It creates no unit, endpoint, shadow field, string-layout duplicate or pose owner. A `00779AF0` provider can resolve this view on the unchanged canonical unit and call the complete base API as its final operation.

The actual key-length load at `928C9B` gates the entire Lua section. `004C1570` is a required getter for actual publication `F878FC`; its complete body and constructor `004BD150` were read. The getter returns a borrowed lvalue for the actual holder's +4 section. This caller captures that pointer once, enters the actual Win32 section, increments +18, and keeps it through both Lua-object destructions. A callback may replace the holder's section; release still decrements/leaves the original captured section.

`00927B40` constructs a real14h self object, using the current mission Lua owner and current unit key. Its wrapper remains a required external contract. The caller constructs one8h pooled `NativeString` scratch header repeatedly for `Ptr`, `LastPosition`, `x`, `y`, and `z`. After each setter it first moves cleanup state to the preceding object, then releases the current nonnull buffer using captured data, length+1 and literal third argument1. The actual raw string-pool getter is repeated before every return. The header retains its bytes after release and the next constructor overwrites them. Source uses canonical `NativeString`, `ActualNativeStringPoolStorage`, and the throwing raw-pool cleanup overload; no semantic pool/domain substitutes it.

`Ptr` receives null **lightuserdata**, not nil. `LastPosition` receives a new table; canonical `native_lua_get_by_name_00b67800` constructs its actual tracked stack object. The unit's current C8 byte is then tested and canonical `refresh_pose_00414db0` runs when zero. Native reads position **y, x, z**, in that order, before coordinate-key allocation or callbacks. The source preserves those three captures: MOVSS float32 values for y/z and the x87 float load/store for x. Each setter argument also takes the native x87 float32 load/store path. Later callbacks may alter the world matrix; the captured values remain the ones written to `LastPosition`. Original FPU exception/status/precision-control identity is not claimed.

After z-string return, the LastPosition object is destroyed before self, both through canonical `B67700`, and the captured lock is released. Outside the gate and lock, the source evaluates the native diagnostic arguments in order: name length, current type, actual `E0CD80[type]` pointer, conditional current name data, and unsigned16 ID. Empty name length selects `<null name>`; nonempty length with null data selects the actual `F89AC8` empty-byte address. `004254B0` is a verified one-byte `RET`, so this packet adds no logging callback. It then reloads owner+30; nonnull selects the **current** unit profile's slot134. Finally it preserves the separate `00923050` identity, also a verified one-byte `RET`. Neither is an unresolved no-op fallback. MSVC can eliminate these inert calls while retaining the volatile argument reads.

## Boundaries and ABI evidence

| Callee | Evidence and implementation |
|---|---|
| `004C1570` | Complete189-byte getter read, F878FC,8-byte owner/profileCE7548, section fromBD1860, manager double-check/publication/registration. Required external getter; caller consumes actual+4 lvalue. |
| `00927B40` | Complete162-byte wrapper read; ECXunit/output stack, `RET4` at927BDF. Globals→thisTable→current NativeString178; two temporary tracked-object cleanups. Required wrapper. |
| `0041E870` | Complete83-byte constructor read; ECXactual8h, text stack, `RET4` at41E8C0. Existing actual NativeString implementation. |
| `B67530/B67580/B67400` | Complete80/80/86-byte wrappers read; ECXactual LuaObject; key/value use `RET8`, key-only newtable uses `RET4`. Required external wrappers; actual Lua API implementation remains a library dependency. B67400 converts a float32 stack argument to a double for the Lua library call. |
| `B67800/B67700` | Existing actual tracked-object get/destruct source; ECXtable/output+name/`RET8`, and ECXobject/no arguments/RET. No registry-reference or private-table substitute. |
| `00419CC0/BD1510` | Existing actual pool getter and return. Three already-pushed words survive the no-argument getter; BD1510 consumes block,size,1 with `RET0C`. Canonical raw-pool composition preserves getter exceptions and current publication. |
| `00414DB0` | Existing actual borrowed-pose implementation; ECXowner/no stack arguments/RET. Uses established74/C8/CC/10C layout and current parent resolution. |
| `004254B0` | Native diagnostic call has four pushed DWORDs and `ADD ESP,10h` at928F19. Callee is exactly RET; no logging behavior inferred. |
| Slot134 / `00923050` | Current profile is captured after the +30 gate; same ECXunit, no arguments. Virtual provider required; final923050 exact RET independently proved. |

## Eight-state cleanup

FuncInfo `DDA4E4` points to the eight-row unwind map at `DDA508`. Handler `CA70D0[10]` loads that FuncInfo then jumps to `BF6B43`. The initial worker query found no containing function. Root subsequently defined/saved/exported it as `EH_MissionEntity_OnKilledReleaseLuaSelf` in commit `c8000b51`; the final worker readback confirms its10-byte body and the tail row verifies. The eight cleanup functions themselves are formally defined and fully read.

| State | Next | Funclet, frame local | Target |
|---|---:|---|---|
| 0 | -1 | `CA7090`, EBP-48 | `411EE0`, captured lock release |
| 1 | 0 | `CA7098`, EBP-20 | `B67700`, self |
| 2 | 1 | `CA70A0`, EBP-50 | `41DD20`, Ptr string |
| 3 | 1 | `CA70A8`, EBP-50 | `41DD20`, LastPosition string |
| 4 | 1 | `CA70B0`, EBP-34 | `B67700`, LastPosition object |
| 5 | 4 | `CA70B8`, EBP-50 | `41DD20`, x string |
| 6 | 4 | `CA70C0`, EBP-50 | `41DD20`, y string |
| 7 | 4 | `CA70C8`, EBP-50 | `41DD20`, z string |

Source guards arm only completed objects and disarm before explicit normal destruction. Declaration order releases an active key before LastPosition, self, and the captured section on a source exception. The native original handler was disabled in the byte clone, so original FH3/SEH, faults, cleanup exceptions, Lua error/longjmp execution and native frame aliasing are unproved. The source-only injected setter exception confirms actual Lua stack cleanup, pooled returns, captured-section release and skipped virtual tail.

## Verification and retained evidence

The complete709-byte routine and24 dependency/data spans match live Ghidra and PE-loaded bytes. `F89AC8` lies in loader zero-fill storage and has no file-backed byte at that RVA; its zero value is recorded as loader evidence. No normal range is unread or omitted. All28 native direct call sites, the three indirect transfers, all eight EH cleanup jumps, the handler jump, and the `779B58` tail caller are represented in the report. The historical missing-handler snapshot remains retained beside root's final resolution; all38 direct/tail rows now verify, with three indirect boundaries. The worker made no Ghidra mutation; owned-entry before/after documentation is identical.

Win32 Release, both existing CTests, and all eight seed byte checks passed. Two paired original-byte/source fixture cases cover empty-key tail execution and the full Lua path. Real Lua5.1.1 metamethods replace the lock publication, clear the actual unit key, make its pose dirty, mutate its world matrix after x, and change owner+30/current profile. Source/native traces match, LastPosition retains1.25/2.5/3.75, Ptr is null lightuserdata, Dead remains false, and the updated slot134 target's mutations survive. The fixture uses actual Lua stack objects, canonical raw string-pool allocation/return, actual pose refresh and real tracked critical sections.

The fixture isolates required getter/setter wrapper contracts and producer virtual behavior; it does not establish unrestricted wrapper, mission-owner or application integration. The byte clone retains exact operand relocations and separate original RET bodies for4254B0/923050. One additional source-only setter exception checks completed cleanup. The first fixture assertion mistakenly treated the pool's cached-block counter as an outstanding-allocation counter; the retained failure and corrected provider-body evidence make that distinction explicit. No source behavior change was needed.

Exact code/data spans, relocated bytes, tool paths/versions, compiler inputs, linked object and library hashes, Lua library sources, outputs, failures and the build log are retained in `local/killed_manifest.json`. These are build/fixture results, not original ABI or gameplay validation.
