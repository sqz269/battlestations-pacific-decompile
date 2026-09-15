# Device-class resolution: complete factory evidence and remaining source

Owned addresses: `00443090`, `00443490`. Packet `orch4_device_resolution_b2`.
Names below are descriptive hypotheses, not recovered symbols.

## Result

The complete ordinary listings of both owned bodies, all 51 external call
sites, all seven factory branches, and the 13-state factory unwind map have
been reviewed. This packet adds **evidence only**. It does not add C++ that
forwards the unresolved factory, and it does not close native vehicle
activation. The missing source is a real object family, not just a wrapper.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| `00443090..0044348D` (1022 bytes) | ECX signed class id; DL cached-reference retain selector; EAX class pointer; plain RET | Complete ordinary-body and factory-EH analysis; source absent |
| `00443490..004434B9` (42 bytes) | Same ECX/DL input and EAX output; plain RET | Complete ordinary-body analysis; source absent |

Live queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Initial function count was 64,122; after the
parent's two terminal definitions it was 64,124. This worker only read/exported
Ghidra data. Exports remain ignored under the main checkout's `exports/bsp`.
The report contains every owned direct-call row and every external call-site
row, each with its native target and containing function.

## Input and register audit

`004430AB` captures DL in BL; `004430AD` captures ECX in EDI. EDI is never
rewritten in the whole 289-instruction listing. BL is rewritten only at
`00443130` to the EH state value 3 on the fresh-allocation path. Thus the
selector is consumed solely by the cached-object branch at `00443430`.

All 49 live direct callers of `00443490` explicitly execute `XOR DL,DL`
before their call. The other two direct callers of `00443090`, `009614F9`
and `00961591` in `00960230`, execute `MOV DL,1`. The former reads the
authored list node's `+8h` id; the latter uses platform `+38h` after a signed
`id > -1` check. The complete per-site ECX/DL setup is retained in the report.
`00443490` forwards its inputs without modifying them before `00443491`.
The contract must retain DL even though the currently observed activation
callers all request zero.

Index comparisons are signed (`JL`); multiplication by four and `id + 1`
are DWORD operations. The factory does not reject negative ids. A source
implementation must not add a successful invalid-id fallback.

## `00443090`: actual registry, Lua objects, and branch schedule

1. `00441780` returns the registry singleton. Its producer allocates 10h bytes
   and sets `{vptr, data04=0, count08=0, capacity0C=0}`. The array subobject
   starts at `registry+4`; `00440180` takes that address in ECX and one stacked
   requested count, returning with `RET 4`.
2. If `id >= count`, call the resize with `id+1`. Read the current data pointer
   and entry. An existing nonnull entry takes the cached path below.
3. Read the Lua owner at `(*00E188A8)+1A0Ch`. Acquire four actual 14h stack
   objects in order: globals, `globals.DeviceClass`, row `[id]`, row `.Type`.
   `00CE4780` is the NUL-terminated literal `Type`.
4. Construct a native 8h string from Type. Construct a second native string
   `Got`; call `00B673A0(row, Got, 1)` and destroy that key string before any
   type comparison. Thus an unrecognized Type still changes `row.Got=true`.
5. Compare Type case-insensitively in the exact order below. `operator_new`
   calls are cdecl with one stacked byte count and `ADD ESP,4` immediately
   after each return; their null returns converge on a null receiver that is
   then dereferenced at `0044333B`. There is no benign null-allocation return.

| Type | Compare call | Allocate call / bytes | Constructor call | Final vtable | EH allocation state |
| --- | --- | --- | --- | --- | --- |
| `Rapid_Fixed_Slave_Gun` | `0044319E` | `004431AC` / DCh | `004431C5` -> `00442B90`; override at `004431CA` | `00CE45E0` | 6 |
| `Rapid_Turning_Gun` | `004431E5` | `004431F3` / DCh | `0044320C` -> `00442B90`; override at `00443211` | `00CE4614` | 7 |
| `Single_Turning_Gun` | `00443225` | `00443233` / DCh | `0044324E` -> `00442EC0` | `00CE4648` | 8 |
| `BombPlatform` | `00443261` | `0044326F` / E8h | `0044328A` -> `00442C50` | `00CE4560` | 9 |
| `MultiBombPlatform` | `0044329D` | `004432AB` / 18Ch | `004432C2` -> `00442CE0` | `00CE459C` | 10 |
| `Depth_Charge_Launcher` | `004432D2` | `004432E0` / DCh | `004432F7` -> `00442F50` | `00CE467C` | 11 |
| `Catapult` | `00443307` | `00443319` / ECh | `00443330` -> `00442FF0` | `00CE46C0` | 12 |

6. At `00443348`, reset the factory EH state to 4 **before** the virtual
   `+8h` reader at `0044334D`. ECX is the newly constructed class and its
   single stacked argument is the live row object, not a copied profile.
7. Write the authored id to class `+6Ch` at `0044334F`; reacquire the registry,
   repeat the signed count/resize check, then publish the pointer to the
   actual slot at `00443373`.
8. Release Type string, Type Lua object, row Lua object, DeviceClass Lua object,
   globals Lua object, in that order. Then reacquire the registry, repeat the
   count/resize check and return the current slot. Do not cache an earlier
   entry across these calls.
9. An unmatched Type destroys the same five locals and returns zero directly
   at `0044342F`; no object is allocated or published.

The common constructor `00442B90` calls `0087C640`, writes vptr `00CE4534`,
and zeros `+70,+74,+78,+7C,+9C,+A0,+A4,+BC,+C0,+C4`. The five leaf wrappers
change the vptr; Bomb/MultiBomb also write `+80h=10`, and Catapult writes
`+80h=11`. The base constructor produces refcount `+4h=1` and flag `+44h=0`.
It allocates and links a 58h tree sentinel through `00877FA0`. Existing
`vehicle_class_lua_load.cpp` describes that layout through offset tables;
it does not implement the actual-storage base constructor.

The same offset `+6Ch` has a different role in the vehicle-derived layout:
`DamageableClassBaseOffsets::kSecondaryVptr` is explicitly documented as a
write of `00749050`, not of `0087C640`. Device `+6Ch` is the factory-written
integer id. Do not reuse the vehicle secondary-vptr interpretation here.

### Cached reference ownership

For a nonnull cached entry, DL=0 skips retain. DL!=0 reacquires the registry,
rechecks/resizes, then calls `InterlockedIncrement` on the current entry's
`+4h` at `00443457`. Both paths reacquire/recheck again before returning.
A freshly constructed class receives **no extra increment**, irrespective
of the original DL value. Its initial reference comes from `0087C640`.
Neither publication nor ordinary Lua cleanup releases this class reference.

## Exception schedule and failure behavior

Handler `00C5FBCD` loads FuncInfo `00D867B4`; its magic is `19930522`, maximum
state 13, unwind table `00D867D8`. All 13 table pairs and all cleanup action
instructions were read. This is evidence for future native ownership, not
a claim that C++ exception behavior already matches original FH3.

| State | Next state | Cleanup action | Factory local |
| --- | --- | --- | --- |
| 0 | -1 | `00C5FB50` -> `00B67700` | globals at frame `-20h` |
| 1 | 0 | `00C5FB58` -> `00B67700` | DeviceClass at `-34h` |
| 2 | 1 | `00C5FB60` -> `00B67700` | row at `-5Ch` |
| 3 | 2 | `00C5FB68` -> `00B67700` | Type at `-48h` |
| 4 | 3 | `00C5FB70` -> `0041DD20` | Type native string at `-64h` |
| 5 | 4 | `00C5FB78` -> `0041DD20` | Got native string at `-6Ch` |
| 6 | 4 | `00C5FB80` calls `00BF65AC` | raw first-branch allocation stored at `-6Ch` |
| 7 | 4 | `00C5FB8B` calls `00BF65AC` | raw second-branch allocation |
| 8 | 4 | `00C5FB96` calls `00BF65AC` | raw third-branch allocation |
| 9 | 4 | `00C5FBA1` calls `00BF65AC` | raw fourth-branch allocation |
| 10 | 4 | `00C5FBAC` calls `00BF65AC` | raw fifth-branch allocation |
| 11 | 4 | `00C5FBB7` calls `00BF65AC` | raw sixth-branch allocation |
| 12 | 4 | `00C5FBC2` calls `00BF65AC` | raw seventh-branch allocation |

The allocation states only cover construction. Once the virtual reader runs,
state 4 is active: an exception there cleans the Type string and four Lua
objects, but this factory does **not** delete the allocated class. Likewise,
the factory has no rollback of `Got=true`, no removal of a published slot,
and no reference compensation after its cached increment. Adding RAII that
automatically deletes the class for every later failure would change native
behavior. Constructor-internal unwinds require their own evidence packet.

## `00443490`: activation and actual vtable dependencies

After resolution, retain the exact returned pointer in ESI. Null returns it
unchanged. Otherwise read byte `+44h`; if nonzero, return immediately. For a
zero flag, fetch the current vtable `+10h` and call it with ECX=class and one
stacked zero (`004434AB`), reload the class vtable, then call current `+14h`
with ECX=class and no stacked argument (`004434B4`). Return the original
class pointer; ignore the second call's return value. There is no flag write
or release in this wrapper itself, and no second flag check between calls.

| Vtables | `+8h` Lua reader | `+10h` | `+14h` |
| --- | --- | --- | --- |
| `00CE45E0, 00CE4614, 00CE4648, 00CE467C` | `007327B0` | `00731A50` | `00730CB0` |
| `00CE4560` | `006E01C0` | `00731A50` | `00730CB0` |
| `00CE459C` | `006E02C0` | `00731A50` | `00730CB0` |
| `00CE46C0` | `006EB6B0` | `00731A50` | `00730CB0` |

The vtable DWORDs were read directly at `00CE4534..00CE46E7` and reconciled
with the constructor stores. The four leaf reader bodies have been read;
there is no actual-storage source for them in this checkout. Existing
`game_hosts_gunnery.cpp` reads a semantic subset of the Lua data, stores it in
`GameDeviceClassRow` vectors, and logs a Function-key operation. That is not
the native class constructor, reader, factory, or activation path.

The parent was given these exact Ghidra repair proposals before source work:

| Issue found | Exact evidence / requested repair |
| --- | --- |
| `00731A50` no live function | Body `00731A50..00731AD2`; all branch targets are inside; `RET 4` at `00731AD0`; INT3 padding through `00731ADF`, next known entry `00731AE0` |
| `00730CB0` no live function | Body `00730CB0..00730D4A`; forward branch `00730CEE -> 00730D47` extends past first RET at `00730D46`; tail `FSTP ST2; JMP 00730D30`; INT3 through `00730D4F` |
| `0043FA60` returning-free listing gap | Declared body ends `0043FABE`, but `0043FAB1..0043FAB9` is absent. Exact missing bytes decode `ADD ESP,4; MOV [ESI],EBX; MOV [ESI+8],EDI; POP EBX` after free at `0043FAAC` |
| Seven factory EH cleanup extents | `FB80..FB8A`, `FB8B..FB95`, `FB96..FBA0`, `FBA1..FBAB`, `FBAC..FBB6`, `FBB7..FBC1`, `FBC2..FBCC` (all `00C5` prefix); each existing body omitted its final `POP ECX; RET` after delete |

The first terminal gates flag `+44`, invokes the existing `0043EBD0` class
model path, then walks 48h entries at class `+74h/count+78h` and calls the
unimplemented bullet acquire/finalise `006EAFE0`. That resolver in turn needs
`006EA910`. The second terminal contains an x87 recoil loop using `+A8h`,
`+ACh`, `+B0h`, then calls existing `00879590` when `+38h` is nonzero.
The parent repaired the first three issues under its own lease and write
lock, saved the project, and verified disk/live byte parity. This worker then
refreshed those exports and read both complete terminal pseudocode bodies and
stored assembly listings. `0043FA60` now includes all four missing instructions.
The seven factory EH tail extents were subsequently repaired by the primary
coordinator under a separate lease and write lock. All seven complete 11-byte
actions matched disk/live before recreation; each now lists five instructions
through `POP ECX; RET` with zero flow gaps. Existing `Unwind@<address>` names
and comments were retained. The saved mutation and preflight records are
`reports/native_factory_eh_flow_repairs_orch4_e5.json` and
`reports/native_factory_eh_body_repairs_orch4_e5.json`. Exports were refreshed.

`00731A50` is 131 bytes, ECX class plus one stacked enemy argument, `RET 4`.
It captures the entry end once after model activation and does not reread count
inside the loop. It atomically decrements each nonnull temporary bullet result,
invokes its deleting slot 0 only when the resulting reference count is zero,
then clears that temporary holder. It has no EH frame of its own.

`00730CB0` is 155 bytes, ECX class, no stacked argument, `AL=1` and plain RET.
Its `+A8h/+ACh/+B0h` producer is `007327B0`, reading `BackSpeedStart`,
`BackSpeedFact`, and `DistMax`. The live constants at `00D7A218` and
`00D7A258` are float +0 and double +0. The loop makes no class-field stores.
Its SSE unordered test and x87 comparisons, stack permutations and forced
float stores must be retained; decompiled binary32 arithmetic is insufficient.

## Dependency packets identified by this audit

These are dependency work units, not claims that every transitive leaf is
already closed. Claim only the addresses actually implemented in each unit.
The subsequent source-closure table below supersedes this initial work list.

1. **Registry storage**: `00441780`, `00440180`, `0043FA60`, plus inspect the
   `00441840` deleting destructor for singleton lifetime. Preserve raw 10h
   singleton and `{data,count,capacity}` pointer storage, native signed growth,
   and actual registration/lock contracts. Existing sized-pool, singleton
   lifetime and allocator source can be reused; do not put classes in a vector.
2. **Native class construction**: `00877FA0`, `0087C640`, `00442B90`,
   `00442C50`, `00442CE0`, `00442EC0`, `00442F50`, `00442FF0`. Implement actual
   tree sentinel, untouched bytes, writes, and constructor-internal EH. The
   vtables must ultimately target genuine reader/activation/destruction source.
3. **Missing canonical Lua setter**: `00B673A0` (81 bytes). Actual row object,
   native string key and low-byte boolean, `RET 8`; `lua_checkstack(2)`,
   `lua_pushlstring`, `lua_pushboolean`, `lua_settable` at the object's current
   index. Existing native Lua storage and Lua 5.1.1 API cover the dependency.
4. **Shared readers before leaves**: `0087CA80` -> `00440AC0`, then
   `007313E0` and `007327B0`; actual fields, raw 48h bullet array and side-effect
   ordering are required. `00442A30` depends on `004427A0/00442940`; these are
   exact container leaf candidates. Bullet-class `006EA910` and effect-handle
   calls remain explicit prerequisites, not fabricated successful callbacks.
   Finally implement the small leaf readers `006E01C0`, `006E02C0`, `006EB6B0`.
5. **Activation terminals**: with listing repair complete, `00730CB0` is a bounded
   x87 packet composing existing `00879590`. `00731A50` additionally needs
   `006EAFE0/006EA910`, current model binding, and actual retained bullet
   references. Check native layout producers before naming entry `+34/+0C`.
6. **Factory integration**: only after those interfaces exist, implement the
   owned `00443090/00443490` against the existing native Lua, native string,
   sized pool, allocator and class-model implementations, preserving this
   schedule. Then connect the mandatory `call_00443490` vehicle boundary.

## Verification boundary

`verify_report_calls.py` passes all 105 direct-call rows; three indirect
vtable rows are intentionally skipped by that checker and are covered by the
listing/vtable evidence above. This packet has no
C++ changes, so no new build or CTest run is claimed. No fixture, original-body
differential, ABI bridge, original FH3, executable reachability or gameplay
validation is claimed. In particular, semantic gunnery logs cannot establish
that this missing native factory executes in `bsp_game.exe`.

## Subsequent source closures, 2026-09-15

The following helpers were reconstructed after the evidence-only factory
audit above. Each has its own complete ordinary-body record and explicit
source ABI/provider boundary. The factory and activation wrapper themselves
remain evidence-only.

| Helper | Current source evidence | Verification |
| --- | --- | --- |
| `0043FA60/00440180` registry array reserve/resize | [Native device registry array](NATIVE_DEVICE_REGISTRY_ARRAY_ORCH4.md) | Strict Win32 build, combined 3 CTests, ten live call rows; count-only shrink and free-before-publication reviewed against both complete listings |
| `00B673A0` native-string-key boolean setter | [Native Lua boolean setter](NATIVE_LUA_OBJECT_BOOLEAN_ORCH4.md) | Strict Win32 build, combined 3 CTests, 20 live call rows; the additional undefined-region caller is retained as raw-byte evidence |
| `00730CB0` device vslot+14h recoil and conditional model load | [Native gun recoil activation](NATIVE_GUN_RECOIL_ACTIVATION_ORCH4.md) | Strict Win32 build, combined 3 CTests, nine original/source pairs including selected floating invalid-operation traps; model path covers the existing-model guard |
| `00441780/00441360/00441840/0043EBF0` registry lifecycle | [Native device registry](NATIVE_DEVICE_REGISTRY_ORCH4.md) | Strict Win32 build, combined 3 CTests, eight call rows; focused canonical manager-drain and retained-owner teardown checks |
| `0087C640/00877FA0` damageable base and five cleanup helpers | [Native damageable construction](NATIVE_DAMAGEABLE_CLASS_CONSTRUCTION_ORCH4.md) | Strict Win32 build, combined 3 CTests, eight call/tail rows; ordinary original/source object and sentinel comparison plus source failure-order fixture |
| `00442B90` and seven derived constructors | [Native device constructors](NATIVE_DEVICE_CLASS_CONSTRUCTION_ORCH4.md) | All 260 bytes matched disk/live, 15 direct call rows, strict Win32 build and 3 CTests; genuine base composition and partial ordered stores |
| `007149D0` borrowed category-table lookup | [Damageable Lua reader audit](NATIVE_DAMAGEABLE_CLASS_LUA_ORCH4.md) | Strict Win32 build and 3 CTests; repeated actual-table loads through existing comparison provider with explicit host CRT/locale boundary |

Native class readers, complete class destruction, bullet resolution,
vslot+10h activation and final `00443090/00443490` composition remain open.
The complete `0087CA80` reader now has a separate 858-instruction/37-state
audit, including its container, SoldierClass and effect-provider dependencies;
the reader itself remains source absent. The repaired factory EH listings
improve evidence coverage and do not implement the factory.
Source/helper checks do not establish original ABI/FH3 compatibility,
fresh resource loading, complete native vehicle activation or gameplay.
