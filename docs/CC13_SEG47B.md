# CC13 SEG47B: segment 47 (007B2DD0-007D1DC0, flight), pass two

Packet `cc13-seg47b` (cc13 lineage, Claude, 2026-10-16). Pool: the 30 functions that
`docs/CC13_SEG47.md` section 4 left unread, then its deferred large bodies. Code: `src/cc13_seg47b.cpp` and
`include/bsp/cc13_seg47b.hpp` (marker `cc13_seg47b`), registered in `cmake/startup.cmake`.

Method and shape are pass one's: every body was read from the disk bytes with Capstone
(`bsp.py disasm-raw`), constants from the PE, jump tables from the PE (`007CD1AC`, `007CD1D8`,
`007CD22C`). Each address, name and behaviour was searched in `src`/`include` and the ledger first.
`Seg47bHost` extends pass one's `cc13_seg47::Seg47Host`; pass-one functions are called directly
(`firing_gun_registry_instance_007c2ec0`, `plane_remote_touchdown_to_state4_007c7430`,
`plane_remote_enter_flight_state6_007cb9e0`, `session_message_resolve_entity_by_net_id_007ba890`,
`plane_store_message_vector_f0_007ba600`, `plane_on_impact_message_007bba10`,
`plane_set_pilot_command_last_time_007bb170`), and `bsp::wrapped_angle_subtract_00438b10` stands
for 00438B10. Nothing is wired and nothing is ABI-compatible; status is **reconstructed and
build-tested** only. Descriptive names are hypotheses.

Recurring simplifications (each also stated at the function): x87 sums of squares are formed in
double (the float products are exact; the sums may round differently from 80-bit); CRT
`sqrt`/`atan2` are `std::` on the stored floats; FLD/FSTP float copies are bitwise; the MSVC
checked-iterator tests (`_invalid_parameter_noinfo`) are kept at the image's points.

## 1. Reconstructed (22)

| Address | C++ | What it does |
|---|---|---|
| `007C1570` | `plane_request_flight_state_007c1570` | __thiscall(plane; requested, force), RET 8. Off-client: the C3h flight-state message (current, requested) through 0075B430 / 0077C2A0(plane; &msg, 7, 0). Client: 007C1430 for requested 3 or the 7/6/4/5 pairs. |
| `007C17D0` | `plane_notify_takeoff_start_007c17d0` | RET 0. With +904h set: C3h (current, 4) off-client; 007C1430(4) on a client from 7/6/4/5. |
| `007C18B0` | `plane_update_pose_derived_state_007c18b0` | __thiscall(plane; dt), RET 4. Pitch +C64h from the forward row; heading +C6Ch and bank +C68h through 004F9B30 / 0085E4D0 / 00B646E0 / 00413920; rates +C70h..+C78h; flight-path angle +C7Ch and +C84h from the velocities; their rates +C80h / +C88h. |
| `007C2090` | `plane_request_flight_state_two_007c2090` | RET 0. C3h (current, 2) unless +900h is 2 or on a client. |
| `007C2130` | `plane_request_flight_state_one_007c2130` | RET 0. C3h (current, 1) unless +900h is 1 or on a client. |
| `007C2AF0` | `plane_state_record_serialize_007c2af0` | __thiscall(record; plane, key), RET 8. Optional fill from the plane (0042D2E0, 007D8330, 007BAA80), then the flag-driven two-way copy with the 007C2810 record. |
| `007C3700` | `plane_update_bow_wave_effect_007c3700` | RET 0. Head effect +AA4h kept or released; the +A3Ch spray elements placed on the water (004142E0, 0078CF20, 00866B70 / 00867B10, 004842C0, 007D99C0) below 14 m, stopped below 20 m. |
| `007C39B0` | `plane_debug_draw_gun_lead_007c39b0` | (ECX plane, EDX target), RET 0; no caller. Two-step lead prediction (007C2610, vtable[48h]), red line and sphere (0085F990, 00860BE0). |
| `007C6E10` | `plane_stall_warning_level_007c6e10` | RET 0, EAX 0/1/2 from speed / class+184h against tuning +22Ch / +230h. |
| `007C74A0` | `plane_register_firing_guns_007c74a0` | RET 0. Under the registry lock: +C40h = 1, push onto the global vector 00F87278 (grow 2n+2). |
| `007C75A0` | `plane_unregister_firing_guns_007c75a0` | RET 0. Under the lock: +C40h = 0, swap-with-last removal. |
| `007C78A0` | `plane_set_flight_state_unguarded_007c78a0` | __thiscall(plane; state, flag), RET 8. Client-side set; air/ground transitions re-parent through 007D9CE0, 007BA020 / 004134F0, 0042ED50, vtable[ACh], 007D9C10. |
| `007C95A0` | `plane_apply_nose_art_decal_007c95a0` | RET 0. Nose-art texture (007F9F70, renderer vtable[64h]) applied to the "pimpdecal.shfx" slot (00B85310). |
| `007C9770` | `plane_sentity_init_pass_c_007c9770` | RET 0. 0095E5B0, 00951F80, 007C95A0, 007BC550, 00922F80(0) for state 0/1, Lua `SquadronID`, 007C5AC0(-1). |
| `007CA5F0` | `plane_set_gun_fire_effects_007ca5f0` | __thiscall(plane; firing), RET 4. 00956EE0, then one point effect per gun matrix (008687C0, 0074D780) or the stop-and-pop loop. |
| `007CB440` | `plane_format_debug_name_007cb440` | __thiscall(plane; out), RET 4. "<plane> of <squadron>" or "<plane> zombie" (0041E870, 004261A0). |
| `007CC580` | `plane_on_killed_007cc580` | RET 0 (vtable[84h]). vtable[10h], 007C75A0, stops/releases the +A3Ch, +A4Ch, +A5Ch and +A30h effects, tail 0095D400. |
| `007CCA50` | `plane_send_sync_ack_batch_007cca50` | RET 0. C2h message to 00770AF0 when the 00F87284 list is non-empty, then 00489370 clears it. |
| `007CCFA0` | `plane_handle_message_007ccfa0` | __thiscall(plane; message), RET 4, AL. Kind 62h -> vtable[208h]; the 7Ah / C3h..CCh switch; default 0095ABE0. |
| `007CD930` | `plane_compute_pilot_fires_007cd930` | RET 0. +C24h from the kind-20h children's platform byte +0Ch (005471B0 grow first). |
| `007CDF20` | `plane_attach_lua_and_read_type_007cdf20` | RET 0. 0077E830; Equipment / Type from the bag (kind 1) or the record (kind 2, squadron insert 007ED0D0); 007CDC70; the kind-25h rack array +974h. |
| `007CFA30` | `plane_class_base_destruct_007cfa30` | RET 0. The plane vehicle-class destructor body, member by member, then 0095F1C0. |

Deferred pass-one bodies reconstructed in this pass (9):

| Address | C++ | What it does |
|---|---|---|
| `007C21D0` | `plane_predict_bomb_impact_point_007c21d0` | __thiscall(plane; out), RET 4, EAX = out. HUD bomb sight (callers 006082D0, 00609BD0 only; not the AI drop). Up to 11 fall-time passes with vtable[34h], [00E08E54], 9.81, 0.05 s and the 007BA340 ground clamp; 00419010 smoothing of out.y over +C60h / +C5Ch. |
| `007C2450` | `plane_predict_rocket_impact_point_007c2450` | __thiscall(plane; out), RET 4. Start (P.x, P.y - 007BD120, P.z), 10 km along row 2, 00904400(44h) hit test, then the y = 0 crossing. |
| `007C2680` | `plane_send_create_message_007c2680` | __thiscall(plane; peer), RET 4. PLANE_CREATE (007BD3E0, RET 28h, the velocity by value) to the peer, plus the C9h 3Ch message when +BCCh..+BD4h is not (1, 1, 1). |
| `007C2880` | `plane_send_control_state_message_007c2880` | __thiscall(plane; holder, force), RET 8. The skip test, then 007BDD30 (RET 64h, the 3Ch 007D8330 block by value) to [holder]; 00779F80 always. |
| `007C29F0` | `plane_send_sync_message_007c29f0` | __thiscall(plane; owner), RET 4. 007BDD30(plane, record, 0, 1, ...) to the host through 00779F90. |
| `007C4850` | `plane_class_derive_flight_constants_007c4850` | __thiscall(class), AL. Three refusals with their messages, the zero defaults, the drag / climb / attitude constants, the bounds extent +500h / +508h, tail 00951F20. |
| `007C6820` | `plane_drive_actuator_channels_007c6820` | __thiscall(plane; dt), RET 4. 00956600, 007E27B0, the +C0Ch latch, the dead +A18h / +A24h stores then zeroes, the +A1Ch / +A28h blends, the LOD flag +C48h (00B70490, vtable[48h], 007C1180), 007DE9A0, the bay channel +60h and gear channel +28h writes, 007C4DD0, then 007C3700 below 30 m or +C34h = 1. |
| `007CD5E0` | `plane_scan_model_part_records_007cd5e0` | RET 0. For each {record, node} of root+7Ch: name split at the first space (00711C30, 00408120, 0070F900, 004CDBE0, 00408720), kind 007E1D80, angle default 15 / 30 deg or the parsed degrees (atof), 007DDD60 record into plane+D9Ch[kind][count]. |
| `007D0820` | `plane_hide_other_mesh_holders_007d0820` | RET 0. Collects engine / rotor / control-surface / cockpit nodes (0071AD50, 00506DD0, 007D06A0, 007D05E0); every other child of [[+4A4h]+34h] not named "MeshHolder" gets 00B6DA70(0, 0) and vtable[2Ch](100000 x3). |

Per-function uncertainty:
- `007C21D0`: the image keeps one 0.0 on the x87 stack across the passes and passes it to both
  00419010 calls; the C++ passes literal zeros. The 2dy g + a^2 and (r + a) / g terms are formed
  in double.
- `007C2880` / `007C29F0` / `007C2680`: byte arguments are pushed as dwords whose upper bytes
  are stale (the callees read bytes); passed zero-extended. 007C29F0 runs no destructor on its
  message after the send, and neither does the C++.
- `007C4850`: extended-precision quotients formed in double; `[+50h]+28h` is read without a test.
- `007C6820`: the two blends and the LOD term are formed in double; 007C4DD0 stays a host hook
  (not reconstructed yet).
- `007D0820`: the image passes "rotor" (00CEB8EC) to 007D06A0 after "rotor_still" too; kept.
  007D06A0 is a host hook (cc13_seg47 deferred it).
- `007CD5E0`: the MSVC string helpers (00408120, 0070F900, 004CDBE0, 00408720) are host hooks
  (vendor STL); the inline destructors (free when capacity >= 16) and strlen are written out.
- `007C18B0`: double-formed sums of squares (see above); the image negates the heading twice
  before 00B646E0 (a no-op, not modelled as two operations).
- `007CDF20`: the image passes the whole `Type` dword to 00964790 in ECX; pass one's host method
  takes a `uint16_t`, so a type above FFFFh would differ.
- `007CCFA0`: pass one's `plane_store_message_vector_f0_007ba600` names its parameter `plane`,
  but 007CD123 passes `plane+AB0h` (the controller); the stored fields are plane+BA0h..+BA8h.
  This file passes `plane.sub(0xAB0)`.
- `007CFA30`: the allocator word the image pushes to 007CD3F0 / 004CC760 is an uninitialised
  stack byte nobody reads; passed as 0.
- `007CCA50`: the C2h message's body is written by its serializer (vptr 00D034F0); only the
  stores the image makes here are made.

## 2. Already implemented: verified and recorded ledger-only (6)

| Address | Status | Source | What was checked |
|---|---|---|---|
| `007C07A0` | existing_implementation_verified_against_listing | `src/game_hosts_units.cpp` | `arm_direction_hold_007c07a0`: normalise (1/len only when len > 0, FCOMI/JBE), the 0.1 double gate (007C082A), atan2(nz, nx) with ST1 = nz, `0 > h` -> +2pi, 00438B10(h, vtable[50h]), interp(15deg, 1, 60deg, 0.4) constants 00D05AA8 / 00D05AAC / 00CE7804, then 007D83D0. Minor: the image rounds atan2 to float (007C0843) before pi/2 - x; the host subtracts the double. |
| `007C6760` | existing_implementation_verified_against_listing | `src/game_hosts_units.cpp` | `landing_radius_006c3e50`'s 007C6760 part: m = +4F8h (min) by default, +4FCh (max) for kind 10h, max x 0.4 + min x 0.6 (00CE65D0 / 00CEFF98, doubles) for 12h / 14h / 11h; class+268h x m. |
| `007C7600` | existing_host_substitution_labelled | `src/game_hosts_units.cpp` | pitch > +568h refuses, +564h > pitch passes, else bank against 00419010(+560h, +558h, +55Ch, +554h, pitch) (JBE: NaN passes) all match. The host omits the early refusal (flag && channel [+DECh]+60h enabled && its value != 1.0 && bay byte [plane+idx*8+9C3h] != 0 -> 0, 007C760B-007C7647; labelled "bay byte not modelled") and the rocket pass (007B9510 rack of kind 33h with descriptor+E0h <= 0 -> 1). |
| `007CB5F0` | existing_host_substitution_labelled | `src/game_hosts_units.cpp` | `plane_touchdown_007cc440`'s authority arm: +900h = 4, +C04h = -1, +904h = +908h > 5.0, +C18h = 3, site vtable[24h] / [48h]. Not carried: 0090F6C0 scoring (labelled contract), 007C11E0(0) (contract) and the 007C3EA0 call (see section 3). |
| `007CB7F0` | existing_host_substitution_labelled | `src/game_hosts_units.cpp` | BL = (class+198h == 0.0, UCOMISS/JP), the 007BC5B0 gate, up-axis +E0h > 0.8 (00CE3D40) skip, "powerlost" (00D059F8) through vtable[194h], the state-6 tail. The host sets +900h = 6 directly (labelled) and raises powerlost only for a live unit. |
| `007CBA50` | existing_host_substitution_labelled | `src/game_hosts_units.cpp` | `water_surface_step_007cba50`, spot-checked against 007CBA50-007CBC3C only (vtable[1ECh], the "splash" arm 00D05A10 on +5Dh / +911h, 007D7A80 / 007D8180(0) / +C10h = 99.0, 007DCDD0); the host labels the splash, hull list and C3h substitutions. The rest of the 550h body was not re-read in this pass. |

## 3. Adoption notes (host files are read-only for this packet)

- **007CB5F0 -> 007C3EA0 (touchdown land command).** At 007CB76A-007CB778 the image calls
  007C3EA0 when the plane's party slot +1B0h is not 8 and 00927F10 says the slot is not AI-held:
  in this single-player host that is the player's slot. 007C3EA0 issues command 00E08FA0 to the
  squadron +9D4h (0077D600), retires the pilot bot's finished tasks (0099A600), sets entity state 2
  (0077C470) and routes message 57h. `plane_touchdown_007cc440` does not make this call. LIVE when a
  player-slot plane lands. No shared-RNG draw.
- **007C6E10.** `game_hosts_hud.cpp:2560` returns false (labelled). The image returns 2 when
  tuning+22Ch (StallRangeMin) > speed / class+184h, 1 when tuning+230h (StallRangeMax) > it;
  `plane_stall_warning_level_007c6e10` reproduces it. HUD only. No RNG.
- **007C7600.** The bay-door refusal and the rocket pass described in section 2. Dive bombing,
  not level bombing. No RNG.
- **007CD930.** The host flattens Lua `Platforms[k].PilotFires` per class; the image walks the
  kind-20h children and indexes class+94h by the child's +38Ch, growing that vector (005471B0)
  when the index is out of range. Same answer when the authored platforms map one-to-one.
- **007CB7F0.** Labelled substitutions only (state 6 set directly; powerlost gated on a live unit).

## 4. Not done in this pass

Skipped, vendor STL (ruling 2b): `007CD2F0` (an `uninitialized_fill_n` of 10h-byte
{reference, float[3]} elements, identical to 007CD370) and `007CDDB0` (its wrapper returning
`first + n`).

Still deferred from pass one (large bodies, one packet each): `007BEEE0` (18C0h, the step
pose commit), `007C4DD0` (CF0h, engine effects), `007C7A40` (1040h, PLANE_SYNC write),
`007C8A80` (650h, PLANE_SYNC read), `007C9860` (B90h, save to archive), `007BE9B0` (530h,
the local point event with x87 and matrix work; read to 007BEC48 only).
