# Segment 41 tail rows (packet cc15-rC1)

Scope: the ten segment 41 (00735F30-00758EB0) rows oct18-tA skipped while
cc15-rB held the segment (docs/OCT18_TAIL_A.md, "Held rows"). Exact address
lease only. Build-tested source, no host wired, no game run. Descriptive names
are hypotheses. Every row is referenced from the image
(`tools/image_ref_scan.py`): none is dead.

| Address | Class | Where / why |
|---|---|---|
| 00737A50 | already reconstructed | `run_application_frame`, src/app_frame.cpp:16 (contract include/bsp/app_frame.hpp); hosted by GameFrameHost, src/game_hosts.cpp:1060. Referenced from application vtable word 00CFEAC0. |
| 00737F30 | already reconstructed | `shutdown_application`, src/app_shutdown.cpp:46 over the 42-step table in include/bsp/app_shutdown.hpp. Called by WinMain at 008F843B; vtable word 00CFEAB8. |
| 007420B0 | gameplay, cc9 partial | Built later as a standalone body with host seams (cc15-rD1, docs/CC15_LAND_CONVOY.md); not wired. The gap notes below still describe cc9's host. |
| 00742180 | gameplay, cc9 partial | Built later as a standalone body with host seams (cc15-rD1, docs/CC15_LAND_CONVOY.md); not wired. The gap notes below still describe cc9's host. |
| 007422A0 | gameplay, cc9 partial | Built later as a standalone body with host seams (cc15-rD1, docs/CC15_LAND_CONVOY.md); not wired. The gap notes below still describe cc9's host. |
| 00748A40 | reconstructed here | `stationary_prop_construct_00748a40`, src/cc15_seg41_tail.cpp. |
| 0074CCA0 | game thunk, reconstructed here | JMP 00765000. |
| 0074E180 | game thunk, reconstructed here | JMP 00928860. |
| 00758140 | game thunk, reconstructed here | JMP 00828F20. |
| 00758390 | game thunk, reconstructed here | JMP 00827A90. |

## 00748A40 stationary prop constructor

Evidence: the saved listing 00748A40-00748AF4. `__fastcall(ECX = prop)`,
EAX = prop, RET, FS:[0] frame with handler 00C873C6. Order: 00925CE0 with ECX
unchanged; vtables 00CFF678 (+0), 00CFF65C (+10h), 00CFF654 (+24h); +184h = 0;
+C4h = 36h; 00926BE0 (ECX = prop); +174h = (end-begin) SAR 2 of the vector at
00F8755C (begin 00F87560, end 00F87564; 0 when begin is null, end not read);
00748990 with ECX = 00F8755C and a pointer to a stack word holding the prop
(RET 4); byte +1A4h = 0; float +170h = 0.0 (XORPS/MOVSS, no x87). Callers
00748C40 and 00851500 allocate and zero 1ACh bytes first, so the remaining
fields stay zero.

Limits: the EH unwind (destroying the half-built prop if 00926BE0 or 00748990
throws) is not modelled. The vector is injected as begin/end reads and the
push-back call; no stand-in container. The existing host projection
`publish_stationary_prop_observer_tables_00748a40` (src/game_hosts_units.cpp:252)
publishes only the two observer vtables and is unchanged; the interfaces
`stationary_prop_construct_00748a40` in include/bsp/cc13_seg41_land.hpp and
include/bsp/cc13_seg52.hpp can now be served by this body.

## Game thunks

Disk bytes are `E9 rel32` followed by CC padding:
0074CCA0 -> 00765000, 0074E180 -> 00928860, 00758140 -> 00828F20,
00758390 -> 00827A90. ECX, the stack and the return address pass through, so
each takes the target's ABI: 00765000 `__thiscall(message, cursor)` RET 4;
00928860 `__thiscall(entity, parent, world node, matrix)` RET 0Ch; 00828F20
`bool __thiscall(descriptor)` RET; 00827A90 `__fastcall(ship)` RET. Each is
reached only through vtables (absolute words 00CFFD1C; nine words 00D00108 to
00D1A370; 00D1AED0; 00D017E0). The source wrappers forward to the existing
target reconstructions (`write_entity_create_00765000`,
`game_entity_place_in_world_00928860`, `ship_class_ai_derived_motion_00828f20`,
`ship_on_health_changed_00827a90`) through an injected host; nothing is
duplicated.

## Rows left for the cc9 lane

Land convoy gameplay; cc9's hosts model it only in part, so no body is built.

- 007420B0 (LandConvoy vtable +0A0h, `__fastcall(convoy)`, RET). Body: base
  0077F0E0; when property bag +C0h is in state 1, resolve `Path`
  (008F2260, 008F0DF0, 00925A90), store 007AC9D0's path interface at +344h;
  then read 00927A80 (skill) and call member->vtable[128h](skill) for every
  non-null slot of the member vector +398h/+39Ch, end reread each pass.
  Host: `GameUnitsHost::bind_land_convoy_motion` (src/game_hosts_units.cpp:13231)
  resolves the qualified `Path` against scene records. Missing: the 0077F0E0
  base call, the property-bag state gate and the per-member vtable[128h] skill
  broadcast.
- 00742180 (LandConvoy vtable +080h, `__fastcall(convoy)`, tail JMP 00779AF0).
  For each slot of +398h..+39Ch (end reread each pass): if non-null, zero
  member+738h, then 00926D90(ECX = member, cause = convoy+70h, reread each
  member); finally JMP 00779AF0 with ECX = convoy. Host: the Lua kill path
  (src/game_hosts_lua.cpp:9304) substitutes the convoy's own slot because the
  host keeps no member vector. Missing: the member vector walk, the +738h
  back-pointer clear and the 00779AF0 tail.
- 007422A0 (`__thiscall(convoy, member)`, RET 4; caller 0074CD10 at 0074CD1E).
  Find the first slot equal to member, null it and decrement +3BCh; then,
  whether or not it was found, if +3BCh == 0 call 00926D90(ECX = convoy, 2).
  Host: src/land_and_structures.cpp:225 skips nulled slots and
  src/cc13_seg41_land.cpp:873 calls it through `convoy_member_died_007422a0`;
  no host keeps the live count. Missing: the slot null, the +3BCh count and
  the convoy kill at zero.

## Validation

`scripts/build.ps1` (MSVC Win32 Release plus the existing CTests); see the
commit message for the result.
