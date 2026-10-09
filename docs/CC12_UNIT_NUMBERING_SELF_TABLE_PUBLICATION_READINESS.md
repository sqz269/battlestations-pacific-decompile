# Unit numbering: actual self-table publication and refresh readiness

**Production Source-held; one concrete refresh fragment is ready for review.**
`00928A00` publishes the exact incoming receiver pointer as `Ptr`. The current
selected application paths still publish integer identity tokens or semantic
script records. The complete Native body also proves a second self-table lookup,
assignment and temporary destruction omitted from the explicit control flow of
the existing abstract Source. This packet changes no C++ and adds zero Source,
build, fixture, original-ABI, startup, gameplay or runtime credit.

## Complete Native gate

`00928A00..00928C79`: **634 bytes, 187 operations, 32 direct calls**.
SHA-256: `301502221567c47d0e7a255a94b5beeefb5833d1eba33f21d60e9662fa970d00`.
All operations were read. The complete PE decode, fresh live bytes and every
saved/live instruction start match. The report embeds complete bytes, decoded
rows, saved/live listings, all calls and evidence pins. No other Native body
was opened; every callee and handler `00CA7078` remains uncredited by this audit.

Each live batch used `bsp.py ghidra`, verifying `bsp`,
`/battlestationspacific.exe`, x86 language and image base. Pinned configuration
selects `C:/Users/sqz269/bsp.gpr`; its zero-byte marker is not a hash of the
analysis database. Function count remains 64,729. Original PE SHA-256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Exact published value and ordinary ordering

Entry ECX is captured in EDI at `00928A1C`, before preparation `00927050`.
EDI is never changed until the epilogue restores the caller's saved value.
At `00928C20`, `PUSH EDI` supplies the pointer argument of `00B67530`.
The following push supplies the temporary `Ptr` NativeString header; ECX selects
the persistent Lua object at stabilized stack address S+20. Thus the published
value is **the same receiver R supplied to this invocation**. It is not the
WORD ID, R+178 key header, an interior pointer, wrapper or copied object.
Unopened callers could adjust an enclosing object's pointer before entry;
this body establishes no broader allocation-root or derived-profile identity.

The whole ordinary path is:

1. Install the FS exception frame, save EBX/EBP/ESI/EDI and call preparation.
   Afterwards load current WORD R+174, zero-extend it, format the decimal key,
   and copy into the actual NativeString at R+178/R+17C. The physical alias,
   length and null-data branches remain part of the complete gate.
2. Construct the first self object through `00927B40` at `00928A9F`. Only
   afterwards read current R+C0 and descriptor+4. Nonnull kind 3 jumps directly
   to `00928BEE`, skipping nil test, replacement, refresh, `ID` and `Dead`.
3. Otherwise test the first self through `00B65FB0`, using AL only. On false,
   fetch current `[E188A8]+1A0C` globals and `thisTable`, clear the cached key,
   then destroy the table and globals temporaries.
4. Fetch current E188A8 again, create a fresh `thisTable[key]` at `00928B53`,
   and destroy those table/globals temporaries. Then reacquire and assign self
   as detailed below, before setting `ID` from actual R+178 and `Dead=false`.
5. Both descriptor paths build the `Ptr` key and pass unchanged R to the real
   setter. Release the temporary key and persistent self, restore the FS chain
   and nonvolatile registers, and use plain RET. No stable return value is proved.

Descriptor kind 3 is `[R+C0]+4`, not the dynamic kind at R+C4. Neither that
branch nor pointer publication proves a unit query profile or current slot +5C.
The routine borrows R; preserving and publishing its bits establishes no later
lifetime. It does not construct R+0/+C4/+35C/+360, a model or numbering services.
No universal purity or ownership effect is inferred for unopened callees.

## Concrete omitted refresh

The exact subrange `00928B73..00928B9A` is **40 bytes / 11 operations**:

| Site | Required operation |
| --- | --- |
| 00928B73..00928B7A | Form distinct temporary S+48; restore ECX=R; call 00927B40 |
| 00928B7F..00928B89 | Push returned EAX; select persistent S+20; mark EH state 6; call 00B67690 |
| 00928B8E..00928B96 | Select temporary S+48; restore EH state 1; call 00B67700 |
| 00928B9B..00928BA5 | Only then write ID through the current persistent self |

The existing function in `src/mission_entity_lua_attach.cpp:44` instead calls
`host.self_object()` once at line 57, calls `assign_fresh_table(key)` at line 66,
then immediately calls `set_id_string(self,key)` at line 67. Its line 55-56
one-fetch comment omits the second lookup and assignment. The host declaration
contains no explicit rebind/temporary-destruction step for this point.

If a host retains the first object when the global slot is replaced, later writes
target that stale object unless the refresh is implemented. No concrete host
implementation was found in the inspected Source/include/tests scope; this is
a precise control-flow gap and conditional risk, not an observed gameplay result
or a claim of global absence. An abstract method's name is not proof that it
performs the missing operations.

`include/bsp/lua_object.hpp:4-11` explicitly identifies `LuaObject` as the older
semantic host projection. A second reference binding or ordinary struct copy
would omit the real tracked-object release, late source reads and registration.

## Smallest actual-provider correction contract

A separately leased Source fragment can compose these existing real interfaces:

- `construct_native_mission_entity_lua_self_00927b40(const NativeString&,
  NativeLuaObjectStorage&, void* volatile&)` in
  `src/native_mission_entity_lua_self.cpp:31`.
- `assign_native_lua_object_00b67690(NativeLuaObjectStorage&,
  const NativeLuaObjectStorage&)` in `src/native_lua_objects.cpp:150`.
- `destroy_native_lua_object_00b67700(NativeLuaObjectStorage&)` from
  `include/bsp/native_lua_objects.hpp`.

The admitted boundary starts after fresh-table creation and its two temporary
destructions, only on the normal descriptor path. Borrow the **same actual
R+178 NativeString header**, the **current actual E188A8 cell**, and the already
constructed, persistent actual self object. Use distinct fresh 14h temporary
output storage, pass the getter's returned object to actual assignment, then
destroy that temporary before `ID`. No new opaque service declaration is needed.
The existing lightuserdata setter at `src/native_lua_field_setters.cpp:42`
already calls real Lua with the exact passed pointer; its Native body was not
reopened here.

The ordering has material tracking consequences. Getter cleanup can move its
output index. Assignment releases the destination **before reading source
fields**, so that release can move the temporary's index; assignment then
registers the persistent object's actual address. Temporary destruction can
move the persistent index again. Every later setter must use the persistent
object's current fields. Do not cache indices, pointer/count snapshots or a
semantic LuaObject copy across these operations.

Both object addresses, actual owners, Lua states, key storage and tracked stack
domains must remain valid through callbacks and cleanup. Native tracking has
50 slots and five references per slot; valid capacity remains a prerequisite.
The first self's owner and the newly captured world owner cannot simply be
assumed identical: assignment must release the old and copy the actual new
source as the real provider does. The original stack locals are distinct;
arbitrary aliases are outside this fragment's proposed domain.

State 6 is established after successful temporary construction and before
assignment; state 1 precedes its destruction. A Source implementation must
account for completed temporary cleanup under its stated C++ exception domain.
Full native FH3/SEH, mutable caller-stack aliases, Lua nonlocal errors and hardware
fault cleanup are not established by this gate or the existing provider interfaces.

This fragment would repair the concrete refresh sequence. It would not make
the abstract whole attach complete, bind preparation/all field setters, or
produce a genuine application receiver. Those remain separate prerequisites.

## Current application and available check evidence

`GameMissionLuaInitAllBinding::entity_attach_lua_self_vcall_9c` at
`src/game_hosts_lua.cpp:3938` resolves a semantic `PendingEntity` and forwards
`node.entity_id` to `attach_created_entity_00928a00`. That method at line 3823
publishes the integer cast as lightuserdata at line 3841; the scene path at
7197 does likewise. `build_script_self_table` at
`src/game_hosts_script_orders.cpp:3963` publishes a semantic `GameScriptEntity`
address. These files still match the prior receiver audit's inspected pins.
Stable deque addresses do not establish Native R+0/+C4/+35C/+360 layout.

The parent lifetime readiness document was read as context. It records unresolved
raw ownership contracts; it is not evidence that this selected receiver is owned.
No parent constructor, lifecycle, numbering caller or query body was reopened.

The historical getter check is recorded in
`reports/native_mission_entity_lua_self.json` and
`docs/NATIVE_MISSION_ENTITY_LUA_SELF.md:93-99`. It exercises real Lua key/world
changes and tracked index cleanup. Its reported source/runner
`local/lua_self_probe.cpp` / `local/lua_self_probe_build.ps1` are absent at the
recorded worktree path and current main/PrimaryRoot. The setter report records
five historical cases, including a changed current index; its reported local
probe/runner are also absent in current main/PrimaryRoot. No fixture was rerun.
The attach report only records its historical build/math check; no focused
complete attach/refresh check was found in inspected tracked tests.

For an eventual fragment change, one focused real-Lua case should retain an old
tracked self, replace the global slot, run the three real providers, and check
that subsequent writes reach the new table with correct tracking after temporary
destruction. It should exercise index movement caused by destination release.

## Current Source121 context

The Root build at 2026-10-09 20:46:26.766022 through 20:46:43.654990 UTC
is authoritative here. All **121 Root raw inputs, 121 worker LF-normalized
inputs and four actual Root artifacts** match the mlandfort primary report.
The single raw worker difference is LF/CRLF in the pending-registry constructor
readiness report. Separately inspected provider pins match Root after LF normalization.

Inherited context: three existing checks passed, 37 whole objects captured or
replayed, 41 positive Core definitions and all 35 previous objects unchanged.
Source117 is historical. This packet performed hash replay only, without build,
tests, probes, C++/CMake/ledger changes or Ghidra/GPR mutation.

Evidence: `reports/cc12_unit_numbering_self_table_publication_readiness.json`.
