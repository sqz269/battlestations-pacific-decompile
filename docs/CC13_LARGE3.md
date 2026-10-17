# cc13-large3: large deferred bodies

Packet cc13-large3 (2026-10-16). The eight large bodies that docs/CC13_SWEEPC.md deferred and
docs/CC13_SWEEPC2.md did not reach. Code: `src/cc13_large3.cpp`, `include/bsp/cc13_large3.hpp`
(namespace `bsp::cc13_large3`, host `Large3Host`). The model is that of cc13_sweepC: raw Win32
offsets, every image call through a named host method at the point the image makes it, tagged
archive values as `cc13_sweepC::ArchiveArg`. New C++ interfaces, not drop-in binary replacements;
nothing is bound into bsp_game. Names are hypotheses, not recovered symbols.

Rule 1 search: `git grep` on each address and `bsp.py lookup` found host hooks only (for example
`ship_stop_wreck_effects_00818970` in cc13_sweepC.hpp, `fallback` in text_input.hpp,
`update_shadow_map_view_00a8f3b0` in game_render_frame.hpp) and no implementation of any of the
eight bodies. Two partial models exist and are cross-referenced below: the 0092AAE0 tail
(`install_tickable_entity_shape`, collision_shapes.hpp) and the key table of 00A96750
(docs/TEXT_INPUT_FALLBACK.md). All eight bodies are reconstructed; RNG draws are listed at the
end (only 008EC650 draws).

## Reconstructed

| Address | Function | Original ABI | What the C++ does |
| --- | --- | --- | --- |
| `00818970` | `ship_stop_wreck_effects_00818970` | `__thiscall(ECX = ship)`, plain RET | Stops (00867B10, effect+9h = 1) and resets the intrusive effect pointers: four pairs (+BB0h/+BC0h, +BACh/+BBCh, +BA4h/+BB4h, +BA8h/+BB8h; the second reset only inside the first's non-null branch), singles +9F0h, +9F4h, +9E8h, +9ECh, the four slots +A00h..+A0Ch (reset unconditional), the array [+A14h] with count +A18h, the checked vector +1118h/+111Ch (00BF6713 at its three range checks), then +B54h..+B64h and +B44h..+B50h. |
| `00818340` | `ship_describe_state_00818340` | `__thiscall(ECX = ship, archive)`, RET 4 | 0095BB10, opens "_ship", writes sinkTime / steeringJam / engineJam / turnEfficiency / thrustMod only when 00BD5680 says they differ from 0.0, 0, 0, 1.0f, 1.0f; then camoColor, camoColorGun (kind 8, pointer), torpedoStock, thrust, spd (0092D730 on [ship+1018h], FSTP float), toTurn, maxSpeed, shipYardLaunch, the six repairPreferences, repairTimer, helmsmanControl {thrust, toTurn}; when 00778890 answers true, formaciosGenya/pathstuff with 40 {point (kind 5), dist, dir} records of 18h bytes from +BD8h and actpoint; then gameUnit (008362A0 on [+738h]), navigatorParams (009D5E00 on [+73Ch]), leakManager (0074EF20 on +10D4h). |
| `00AD0290` | `gui_frame_box_describe_properties_00ad0290` | `__thiscall(ECX = frame box, sink)`, RET 4 | With exactly one 2Ch state record and word +ECh = 0, a +20h/+24h size equal to 00ACF0C0(out, 0) is zeroed for the base 00AAAED0 and restored after it. Then HasTexture (kind 3 vs 1), FrameSizesX/Y (kind 6 vs a pair of 00D5D11C / 00D5D10C floats), ShaderName when +104h is non-zero (vs "GuiDefault.mshd"), and a "States" block with one {1, i+1} record per state: Texture (record+8h data, "" fallback 00F8C1D0, vs "" 00CE3A0C), Size (record+14h vs frame+20h), Pivot (record+0Ch vs frame+18h). |
| `00AD1A80` | `gui_frame_box_visit_child_paths_00ad1a80` | `__thiscall(ECX = frame box, const NativeString* prefix)`, RET 4 | Per 2Ch state record: the four literals ")" "(" ":" "." built inline (0041DD40 + memcpy), prefix + "." + i (004263B0) + ":" + name(+4h) + "(" + *00B33E40([record]) + ")" through 004261A0, every temporary released through the pool (00419CC0/00BD1510) in the image's order; then 00AAA0F0(prefix). Checked-iterator reports at 00AD1ABD, 00AD1AE1, 00AD1B09, 00AD1E3F. |
| `00A96750` | `text_owner_handle_key_fallback_00a96750` | `__thiscall(ECX = owner, event, category)`, RET 8 | Category 0: Enter vtable+8, Escape vtable+0Ch, Backspace (cursor > 0 signed) rebuilds text = substr(0, cursor-1) + substr(cursor) with 0041DD20 destroys and cursor -= 1, a character accepted by 00AB6D30(ECX = [owner+24h]) is translated by 00A96450 and inserted (overwriting one character when byte 00E12F34 is 0) with cursor += 1. Other categories: Delete, Insert toggle of 00E12F34, Up, Down (only when 00F8BC01 is 0), Tab, PageUp, PageDown, Left, Right, Home, End. Every edit ends with vtable+4. |
| `00A8F3B0` | `shadow_map_update_view_from_camera_00a8f3b0` | `__thiscall(ECX = shadow owner, camera)`, RET 4 | Eight identity copies (004134F0) into owner+44h + 40h*k; when [00F8BBF0]+0Ch and +0Dh are set: four 00B71990 viewport calls, the +30h ref pointer takes the camera, the light direction is normalized from [owner+0Ch]+1E0h and its y clamped by max/min against +-tan(00D5B564) (or set to (0, 1, 0) when the context's vtable+0Ch declines), the camera node refreshes, eye = 100000.0 * dir + position, 00B63F10 / 00B63B30 on locals, 00AD7A30 (foliage), 00A8EA20 (cascades), and the +30h reference is released. |
| `0092AAE0` | `tickable_entity_install_collision_0092aae0` | `__thiscall(ECX = entity, const source*)`, RET 4 | Builds the 84h-byte body descriptor and the 48h-byte shape descriptor on the stack, looks up the unit's mesh entry for the source node (checked vector at [[unit+360h]+160h]+4Ch), transforms the mesh offset by the pose 00C336C0 produced, derives the initial linear (unit vtable+34h of 2 * the unit offset direction) and angular (scale * cross(dir, up)) terms, creates the body (00C5D580 on [[00E188A8]+18h]), stores extents, mass ([[unit+538h]+B0h] * 0.5) and a box inertia (mass / 5 * (b2 + c2) per axis), then publishes the collision node as `install_tickable_entity_shape` does. |
| `008EC650` | `powerup_pick_from_bracket_list_008ec650` | `__thiscall(ECX = manager, NativeString* out, NativeString key by value)`, RET 0Ch | first = key[0]; s = key without its first character, trimmed of "[]" (00585EB0 / 00584110), plus ","; the local list is cleared (004954F0). When manager+1ACh has no entry for the key (008E65E0), each comma token of s is expanded: "*" gets the first character inserted (008E64A0), a token found in manager+1B8h contributes that node's strings, otherwise the token itself (00450540); then the key's cached vector (008EC460) receives one random element for 'A' lists or the whole list (00506C80). The lookup (008E5BB0 plus the inline case-insensitive compare) then returns the cached vector's first element for 'A', or a random element (00BD2FC0, stream 0) for other lists, erasing it for 'T' (00589690); out is a copy, and s, the list, the result and the by-value key are released. |

### Per-function notes and uncertainty

- `00818970`: the unspecified-bool test (NEG/SBB/TEST 0E186ECh) is a plain non-null test. The
  vector loop re-reads +1118h/+111Ch on every check, as the image does.
- `00818340`: the two jam values reuse the stack slot of sinkTime and overwrite only its low byte,
  so the dword passed to the sink carries sinkTime's upper three bytes; the C++ reproduces that
  dword. Kind 3 equality (00BD5680) compares one byte. Key strings are host literals standing for
  the image strings (00D095B8 "_ship" and so on); "spd" is 00D09548, "dist" 00CFD71C, "dir"
  00D094E0.
- `00AD0290`: HasTexture's current dword is the +100h byte over stack bytes the function never
  writes; the model passes zero there (only the byte is compared, and the sink's reading of the
  upper bytes is unknown). The default dword carries the saved +24h copy's upper bytes, as in the
  image. The two FUCOMIP tests skip on unequal or unordered (JP), modelled as float `==`.
- `00AD1A80`: the container-identity tests at 00AD1AE6-00AD1AFF compare frame+F0h with itself and
  with null; they cannot fire for a frame box pointer other than -F0h and are not modelled. The
  literal construction's memcpy (00BF7680) is done in C++ on the host-sized buffer.
- `00A96750`: the event's low byte is replaced in the argument slot by the 00A96450 result before
  00531030 reads it, so the constructed byte is the translated one. 00AB6D30 receives ECX =
  [owner+24h] although its reconstructed body (`text_accepts_input_char_00ab6d30`) reads only the
  stack byte. Owner virtuals go through one `text_owner_virtual(owner, slot)` method.
- `00A8F3B0`: x87 operand direction from the bytes: DC C9 (FMUL ST1,ST0) at 00A8F939 and D8 C9 at
  00A8F956; the eye products are exact in double and rounded once to float. The look-at and its
  inverse are written to locals that nothing reads afterwards (as in the image). The settings
  pointer [00F8BBF0] and the foliage pointer [00F8C210] come through `global_pointer`.
- `0092AAE0`: the x87 block was traced from the disk bytes with `local/cc13-large3_x87sim.py`
  (symbolic stack, reg-reg forms decoded from the opcode bytes) and modelled in float in the
  image's association order, which is exact under the main thread's 24-bit precision control
  (docs/CC13_FPU_PRECISION.md). Descriptor bytes the image leaves unwritten (D+1h..+3h) are zero
  in the model. The node publish (0092B289-0092B30C) duplicates the sequence that
  `install_tickable_entity_shape` models over its own host; it is inlined here so the whole body
  runs over one host. The search loop's null-container checks cannot fire unless the table
  pointer is -4Ch; they are kept as written.

- `008EC650`: the STL instantiations (004954F0, 00589690, 008E65E0, 008E5BB0, 008EC460, 00450540,
  00506C80, 00432050) are vendor code and are called through named host methods with the
  register inputs read from the call sites; CRT strcspn, _stricmp, memcpy and memmove are called
  directly. The first character goes to 00531030 as a dword whose upper bytes the image never
  writes (only its low byte is stored at [ESP+18h]); the model passes it zero-extended. The
  EH bookkeeping dword at S-58h (set to 1 before each cleanup) only matters to unwinding and is
  not modelled. The 00BF6713 test at 008ECB93 compares the lookup iterator's container with the
  map it was built from on both paths and cannot fire; it is not modelled.

## RNG draws

`008EC650` draws `00BD2FC0` (BSP_RandomThreads_NextU32) with ECX = 0, random-thread stream 0
through 00BD2ED0, the state set the shared generator entry 00BD2F10 also uses: one draw when an
'A' list is built and cached, and one per pick from a cached non-'A' list ('T' lists then erase
the drawn element). A pick from a cached 'A' list returns its first element without a draw. No
other reconstructed body draws.