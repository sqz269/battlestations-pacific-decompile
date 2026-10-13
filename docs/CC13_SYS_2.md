# CC13 engine pass over segments 4, 6 and 9 (packet cc13-sys2)

Addresses: 00449a70, 00449ab0, 0044ccd0, 00465850, 00465b60, 00465d00, 004661a0, 00467310, 00467f30,
00470c90, 004728f0, 00474160, 00476360, 00479fe0, 00472ff0, 0047a800, 0047b9e0, 004bbc50, 004c6750,
004c88a0

Packet cc13-sys2, lineage cc13. Ranges: segment 4 (`004486C0..00453C30`), segment 6
(`00465610..0047C680`), segment 9 (`004BA290..004D0860`). Every name is a descriptive hypothesis,
not a recovered symbol. Earlier passes named most of this code (`docs/CC13_SCENE_BROWSER.md` for
segment 6, `docs/CC13_GUI_SCREENS_3.md` for segment 4) without reconstruction records.

## Scope

The index listed 159 functions in the three ranges with a reviewed ledger name and neither a
reconstruction record nor a tag (the lead's estimate of about 410 included unnamed `FUN_` bodies,
which this pass did not take).

| Group | Count | Action |
| --- | --- | --- |
| New reconstructions in `src/engine_seg4_6_9_cc13.cpp` | 51 | reconstructed, build-tested, recorded |
| Registry construct / destruct bodies, mapped to the existing `std::map` aliases | 10 | recorded as type aliases |
| Existing reconstructions without records | 17 | backfilled after a RET/ABI check |
| `STL_*` and compiler adjustor deleting destructors | 11 | tagged (`stl_shape`, `cg_adjustor_thunk`) |
| cc9 lanes, scene contents, singletons and the rest | 70 | left to owners (below) |

## Status

| Item | Exported | Reconstructed | Build-tested | Validated |
| --- | --- | --- | --- | --- |
| 31 movie-action constructors, base `00465B60`, timed item `00465850` | yes | table + `construct_movie_action` | yes | no |
| `00465D00` engine-failure run, `004661A0` particle-effect stop | yes | yes | yes | no |
| Five visual create-entity slots, `0047B9E0`, `00472FF0`, `0047A800` | yes | field writes and tables | yes | no |
| `00467310`, `00467F30` | yes | yes | yes | no |
| `004BBC50`, `004C88A0`, `004C6750` | yes | yes | yes | no |
| Session message E1 `0044CCD0`, `00449A70`, `00449AB0` | yes | over the existing bit-cursor hosts | yes | no |
| Registry bodies (10) | yes | `std::map` aliases in `scene_class_registry.hpp` | yes | no |

Nothing is ABI compatible or game-validated.

## Segment 6: engine-movie action constructors

Each of the 31 constructors `00465C40..00466370` is `__thiscall(action, float activate_time)`,
`RET 4`. Each one calls `00465B60` with the time, stores its own two-slot vtable `{run, stop}` and
writes the type index at `+10h`. The rows of `kMovieActionConstructors` were read from each listing
(`MOV [ESI],vtable` and `MOV [ESI+10h],type`) by a script. They agree with the type table of
`docs/CC13_SCENE_BROWSER.md`. Index 4 has no constructor. `00466170` (`aParticleEffect`)
also clears the effect handle at `+44h`.

`00465B60` builds the timed item at `+4` through `00465850` (time at `+0`, then an empty
NativeString at `+4`, both relative to the item), stores the base vtable `00CE5498`
(`{_purecall, 004656C0}`), clears `+14h` and `+20h..+28h` and `+34h..+3Ch`, and makes the strings
at `+18h` and `+2Ch` empty (`00CE3A0C`). `construct_movie_action` returns the host
`MovieAction` of `include/bsp/engine_movie_actions.hpp` with the type and time set. It returns
nullopt for an index that has no constructor.

`00465D00` (`pEngineFailure` run) ignores ECX. It builds the NativeString `"explosion_delayed"`
(`00CE54C4`) and calls the entity argument's vtable `+194h` with `(&name, 0)`
(`00465D52..00465D6D`). `004661A0` (`aParticleEffect` stop) passes a non-null `+44h` to
`008673B0` and clears it. The run body `004656D0` (point effect at the entity, then `Num1` seconds
of child updates in fixed steps) is documented only.

## Segment 6: visual class entities and classes

The five create-entity slots (class vtable `+8`, `__fastcall(ECX class)`, `RET`) share one shape.
They call `operator new(bytes)` and `memset` to zero, then `00489F30` (the 37h visual base). They
then store three vtables at `+0`, `+10h`, `+24h` and the class id at `+C4h`, and call `0047B9E0`
(class at `+170h`) and store the class at `+188h`.

| Create | Table | Bytes | Id | Vtables | Pending |
| --- | --- | --- | --- | --- | --- |
| `00470C90` | AnimClass | 194h | 3Eh | `00CE5A20` / `00CE5A08` / `00CE5A00` | no |
| `004728F0` | Bullets | 18Ch | 39h | `00CE5B90` / `00CE5B78` / `00CE5B70` | `00926BE0` at `00472977` |
| `00474160` | BuoyClass | 18Ch | 3Ch | `00CE5CF0` / `00CE5CD8` / `00CE5CD0` | no |
| `00476360` | CloudClass | 1ACh | 3Dh | `00CE5E68` / `00CE5E50` / `00CE5E48` | `00926BE0` at `004763F8` |
| `00479FE0` | VehicleClass | 190h | 38h | `00CE6130` / `00CE6114` / `00CE610C` | no |

Cloud also clears dword `+1A4h` and byte `+1A8h`. The host `create_visual_entity` performs only
the class-link writes and the pending-list callback. Allocation and the 37h base are not
modelled.

`00472FF0` (Bullets) and `0047A800` (VehicleClass), `__thiscall(class, source)`, `RET 4`, run the
named-class base `00489E60`, store their vtable (`00CE5CB4` / `00CE6254`) and the source at
`+28h`, and copy the source's name at `+14h` (the empty string `00E18694` when null) into `+4`.

The registry construct and destruct bodies (`00472100`/`00472170`, `004739F0`/`00473A60`,
`00475540`/`004755B0`, `004797A0`/`00479810`, `0047B1F0`/`0047B260`) build or erase the map that
`NamedSceneClassRegistry`/`SourcedSceneClassRegistry` already model. The destructors also clear
the global and restore the singleton base vtable `00CE3818`, and do not delete the mapped classes.
They are recorded against those aliases. The getters (`00472200`, `00473AF0`, `00475640`,
`004798A0`, `0047B2F0`) involve the singleton lifetime manager and are left.

Backfilled: the six `IsKindOf` slots (`00470BE0`, `00470C10`, `00472870`, `004740E0`, `00476310`,
`00479F60`) are rows of `src/entity_class_ids.cpp`. `00470C10` returns true for 3Eh, 37h, 0 and the
entity's own id at `+C4h` (listing `00470C14..00470C2F`), which is `entity_is_kind_of_inherited`.
Also backfilled: `00469E40`/`004694F0` (`src/scene_traffic_groups.cpp`, the `BrowserGroups` block
reader), `0046C550` (`src/scene_entity_factory.cpp`, the generation gate, `RET 5Ch`), `004660B0`
(`src/engine_movie_actions.cpp`) and `004773A0` (the cloud property lookup in
`src/cloud_scene_properties.cpp`).

## Segment 6: small helpers

- `00467310`: `_vsprintf_s(buffer, 20h, format, args)` (`00467321`).
- `00467F30`, `__thiscall(list, index)`, `RET 4`: null for `index < 0` or `index >= +8h`, otherwise
  `index` links through `node+44h` from the head, stopping at a null link.

## Segment 9

- `004BBC50`, `ECX` = dialog result: byte `[[0109CF04]+180h] = 0`; result 1 sets byte `00E1AE75`.
- `004C88A0`, `ECX` = game: drains the deque at `game+5DCh` (map size `+5E0h`, head `+5E4h`, count
  `+5E8h`). The elements are trivial, so only the head and the count move. The head wraps at
  `4 * map size` and returns to 0 with the last element.
- `004C6750`: clears `+0h`, `+4h`, `+10h`, `+14h` of a 38h-byte slot unit. `004C9800`/`004C9820`
  construct and destroy five of them at `+8` through the EH vector iterators. Those two are not
  recorded.

Backfilled: `004BCA50` and `004BFE50` (`src/simulation_gate.cpp`; `004BCA50` reads `+614h` mode,
`+61Ch` forced byte and `+1FE4h` session from ECX and returns 8 unless forced, in a session, or mode
8/9); `004C71C0`, `004C7730` (`src/world_map_bounds.cpp`); `004CAA90` (`src/session_polls.cpp`);
`004CACA0` (`src/archive_text_writer.cpp`).

## Segment 4: session message E1

Vtable `00CE4B30`, no static caller. `0044CCD0` compares the type with E1h. `00449A70` writes the
type byte `+10h` (8 bits), the NativeString `+18h`, the byte `+20h` (2 bits) and the NativeString
`+24h` through `00428FF0`/`00429AC0` on the cursor argument. `00449AB0` reads the same fields from
`stream+4` through `00428C70`/`00429F20`. The host calls the existing bit-cursor reconstructions.

## Left to owners

- cc9 (`cc9_dialog_sequencer`, `docs/CC13_GUI_SCREENS_3.md`): the dialog command classes
  `00449A30`, `00449A50`, `0044A3F0`, `0044A4C0`, `0044B380`, `0044B3B0`, `0044B3E0`, `0044C060`,
  `0044C220`, `0044C270`, `0044C2F0`, `0044C340`, and the panel maps `0044AB00`, `0044AC00`.
- cc9 gameplay: `00470440` (powerup multiplier), `004704B0`/`004704E0` (hit record), `0047B880`,
  `004BC890` (game mode: copies or resets the per-side flags of eight sides), `00468560`,
  `00467050`, the engine-movie wiring `004C6B20` and `004CCE50`.
- Scene contents / scene database (`src/game_hosts_scene_contents.cpp`, `scene_entity_factory`,
  `scene_record_map`): `00468350`, `00468660`, `00468FB0`, `00469300`, `004693C0`, `00469610`,
  `00469690`, `0046A9F0`, `0046BE90`, `0046BF20`, `0046BF70`, `0046DC10`, `0046F160`, `0046F350`,
  `0047B660`, `004C2C80`, `004C62A0`, `004C63E0`, `004CAF50`, `004CB160`, `004D0640`, `004674F0`.
  Some of these already have host code in those files, and their owners should backfill the records.
- Singleton getters and lifetime (`00472200`, `00473AF0`, `00475640`, `004798A0`, `0047B2F0`,
  `004C0960`, `004C0A30`, `004C0EF0`, `004C17D0`, `004C1AC0`, `004C1B90`, `004C1DD0`): one shared
  pattern under `BSP_SingletonLifetime_GetManager`; best done once for all of them.
- Not taken: `004656D0` (particle run), `004772C0`, `004BC410` (camera mover swap), `004C0170`,
  `004C5E00`, `004C74C0`, `004CA2F0`, `004CD450` (unreferenced screenshot writer), `004CD7F0` (cited
  by the mission Lua hosts), `004CE75C` (catch funclet), `004C9800`, `004C9820`.

## Second batch (after the index refresh)

A fresh index after the first commit listed 321 functions in the ranges: the reviewed-name ledger had
grown since the first listing. The second batch took:

- Registry teardown: `00472420`, `00473CE0`, `00475860`, `00479AC0`, `0047B4E0` were named
  `..._ForEachClass` and are now `..._DeleteAllClasses`. Each one deletes every mapped class (vtable
  slot 0 with 1, at node `+14h` or `+10h`) and then erases the map. This answers open item 3 of
  `docs/CC13_SCENE_BROWSER.md`: `BSP_Game_DestroyWorldContainers` calls the Buoy, Cloud, Anim and
  VehicleClass slots at `004D2B8A..004D2B99`. The Bullets slot has no rel32 (E8/E9) or absolute
  reference (scripted scan of `.text` and the whole file; the same scan finds the 004D2B94 call to
  `00472420`), so the Bullets visual classes are never deleted. The host is the template
  `delete_all_scene_classes`.
- Unwind slots that clear a singleton pointer (`00470D30`, `00472990`, `00474220`, `00476440`,
  `0047A080`, `0047C120`): the global is set to 0 and the vtable to `00CE3818`.
- Entity constructors `004707A0`, `004724D0`, `00473DB0`, `004762D0`, `00479B70`: the create slots'
  field writes without allocation (class ids checked in each listing), added to
  `kVisualEntityKinds`. The class constructors `004707F0`, `00473DF0`, `004770F0` correspond to the
  default member initialisers of `AnimClass`/`BuoyClass`/`CloudClass` and are recorded there.
- `00476B90`: `[[entity+360h]+160h]` or 0.
- `0046FDD0` (project onto an axis), traced from the x87 listing. `s` is the length of the axis from
  `00419440`, stored as a float. `inv = 1/s` when `s > 0`, otherwise 0 (a NaN length also gives 0
  through `FCOMI`/`JBE`). `n = axis * inv` is stored per component. The dot product is
  `(v1*n1 + v0*n0) + v2*n2` on the stack, rounded once to float, and the result is `n * d`. The host
  evaluates the dot in double, which matches 53-bit precision control. Under 24-bit precision control
  (Direct3D's default without `FPU_PRESERVE`) the sums would round differently. The rest of the
  `0046F550..004702E0` cluster (reject, closest point, segment-plane intersection, direction and
  length, orthonormal frame) needs the same x87 tracing and was left.
- `004BE940` (list payload by index; an index past the end returns the last payload),
  `004C2DF0` (`__cdecl(string*, token)`: exact equality through the legacy string compare
  `004BEB60`), `004C6820` (screenshot base name, index `-1`, tail jump to `004BFBF0`).
- 37 more `STL_*`/`TMPL_*` bodies tagged (`stl_shape`, `tmpl_shape`).

## Divergences and uncertainty

- `command_line_matches_token_004c2df0` assumes `004BEB60` has `std::string::compare(pos, n, s,
  count)` semantics; its body was not re-read.

- The create-entity and sourced-class hosts model field writes only; allocation, the base
  constructors and the NativeString storage are not reproduced.
- `format_to_buffer32_00467310` throws where the CRT would call its invalid-parameter handler.
- The E1 message's meaning is unknown (no constructor or sender found); only its wire layout is
  established.

## Evidence method

Live `bsp.py ghidra decompile`/`disasm`, `disasm-raw` for the constructors, create slots and
`00465D00`, scripted constant extraction (`local/sys2_ctors.py`, `local/sys2_create.py` in the
worktree). Call-site rows: `reports/cc13_sys2_calls.json` (`tools/verify_report_calls.py`: 15 rows,
0 failed).

## Handoff

Done: 51 + 21 new reconstructions, 10 + 3 alias/default-init records, 17 backfills, 48 tags, five registry renames. Next: the shared singleton
getter pattern (12 functions), the scene-database cluster with its owner, and `004656D0`.
