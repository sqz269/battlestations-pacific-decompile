# CC13 segment 6 (00465610..0047C680)

Worker cc13-seg6, 2026-10-14. Code: `src/seg6_cc13.cpp`, `include/bsp/seg6_cc13.hpp`
(namespace `bsp::cc13_seg6`). Status of every function below: reconstructed and build-tested
against the image's object layout through the `Seg6Host` interface; not fixture-tested, not
wired into the game host, not game-validated. Names are hypotheses, not recovered symbols.

Every call the image makes is a `Seg6Host` method (named by callee address) or a direct call to
another reconstruction in this file, at the same point with the same arguments. Evidence came from
the live Ghidra listing (`bsp.py ghidra disasm`) checked against the decompiler; register inputs
and the x87 operands were read from the listing and the bytes.

## Triage

The segment's 428 candidates are mostly STL instantiations, scalar deleting destructors, EH
funclets and the 167 functions already reconstructed (engine-movie action constructors, class
registries, vector-geometry cluster). Of the 71 untagged or unrecorded entries, the ones below were
reconstructed; the rest are listed under "not reconstructed". A git grep of each address on main
found only call-site mentions, host hooks or projections, no second implementation:

- `src/scene_record_map.cpp` models 004691B0 / 00469480 as a search over a flattened entry list
  (a projection; the tree, the temporary key and the checked-iterator tests are not modelled).
  The bodies here are the image-layout versions.
- `src/game_hosts_lua.cpp` `apply_scene_yaw_00467050` is a yaw-only special case of 00467050.
- `src/engine_seg4_6_9_cc13.cpp` takes 004674F0 and 00476550 as callbacks; it does not implement them.

## Engine-movie items

| Address | Function | ABI and notes |
|---|---|---|
| 00465610 | `movie_action_destruct_base_00465610` | fastcall(ECX action). Restores vtable 00CE5498, then releases the NativeStrings at +2Ch, +18h, +08h (00419CC0 + 00BD1510 each, size+1, 1). |
| 004658D0 | `movie_timed_flag_construct_004658d0` | thiscall(item, float time), RET 4. 00465850(time) at 004658F6, +34h = 1, the +4 string assigned from the empty literal 00CE3A0C (0041DD40 with flag 0, 00BF7680 when data is set), +18h = 0.0. |
| 00465950 | `movie_timed_box_construct_00465950` | thiscall(item, x, y, z, time), RET 10h. +0Ch..+14h = (x, y, z); +1Ch..+24h and +28h..+30h = (x-50, y, z-50) with the double 50.0 at 00CE3938 (bytes DC E9 at 00465993: ST1 = ST1 - ST0), +34h = 1, +18h = 0. |
| 00465A40 | `movie_timed_record_construct_00465a40` | thiscall, 14 stack dwords, RET 38h (00465B53; corrected by cc15-lfix from "15 stack dwords, RET 3Ch"): three floats to +0Ch..+14h, time to 00465850, six floats to +1Ch..+30h, a dword to +34h, a float to +18h, then a NativeString by value assigned to +4 (0041DD40 flag 1, 00BF7680) and its block released. |

## Matrix and small value helpers

| Address | Function | ABI and notes |
|---|---|---|
| 00467050 | `matrix_set_rotation_from_euler_00467050` | thiscall(float* m, a, b, c), RET 0Ch. Writes m[0..2], m[4..6], m[8..10]; m[9] = -0.0f (00D7A208, SUBSS) - sin a. The products run in double here, 80-bit in the image; two-factor products are exact, three-factor ones can differ by a double rounding. FSIN/FCOS differ from std::sin/cos only for huge arguments. |
| 00468560 | `command_target_init_position_00468560` | thiscall(descriptor, const float pos[3], float extra), RET 8, returns descriptor (EAX = ECX at 00468566). Stores in image order: word +0 = 0, byte +1 = 1, position +8..+10h, +4 = 0, byte +0 = 0, word +2 = 0, +14h = extra (float, MOVSS). |

## Scene database (singleton 00E18680, 170h bytes)

Layout as constructed by 0046F160: vtable 00CE59FC; +4 byte; +8 vector of pointers
(first +0Ch); +18h map of hidden-entity records (head +1Ch, size +20h, record at node+14h);
+24h vector of pending objects (first +28h, last +2Ch); +34h class map (vtable 00CE55B0, count
+38h, 64 buckets at +3Ch); NativeStrings at +13Ch and +144h; command list {count, head, tail} at
+14Ch; integer sets at +158h and +164h.

| Address | Function | ABI and notes |
|---|---|---|
| 00467370 | `scene_database_init_entities_once_00467370` | fastcall(ECX flag byte). When zero: 00925F20 with CL = 0, then the byte = 1. |
| 004674F0 | `pod_pointer_array_reserve_004674f0` | thiscall({data,size,cap}, n), RET 4. n < 1 becomes 1; grows only when cap < n: operator new 00BF55BE(n*4), copy (placement null test kept), free 00BF6989 of the old block. |
| 00467EE0 | `scene_database_find_class_row_by_id_00467ee0` | thiscall(db, id), RET 4. Iterates db+34h with 00466BB0 / 00466C00; returns the row *(node+8) whose first dword is id, else 0. |
| 00468350 | `scene_command_list_push_back_00468350` | thiscall(list, record), RET 4. operator new 00BF681B(0Ch) node {prev, next, record}, appended at the tail. |
| 00468600 | `scene_property_collect_array_elements_00468600` | fastcall(ECX property, EDX array). Appends each dword of property+20h while i < 008EF7F0(property) (re-read each pass), growing by doubling (minimum 1) through 004674F0. |
| 00468660 | `scene_database_class_id_to_name_00468660` | thiscall(db, id), RET 4. The class map's node+4 name for the row whose first dword is id; 00E18560 when the name is null; 0 when absent. |
| 004689D0, 00468A90 | `scene_name_hash_bucket_004689d0`, `scene_class_map_bucket_00468a90` | [ESP+4] NativeString*, RET 4, ECX unused. h = size, step = (size\|20h)>>5, size/step consecutive characters folded as h ^= toupper(c) + (h<<5) + (h>>2) (CRT toupper 00BF924E), result h & 3Fh. Two COMDAT copies of one body. |
| 00468ED0, 00468FB0 | `scene_name_hash_find_node_00468ed0`, `scene_class_map_find_node_00468fb0` | thiscall(table, key, uint* bucket), RET 8. Writes the bucket, walks table+8+bucket*4 matching size first, then 00438E10(00419CA0(key), name) == 0. |
| 00469010 | `scene_record_destruct_fields_00469010` | fastcall(ECX record). Releases the NativeStrings at +0Ch then +04h. |
| 004691B0 | `scene_database_find_hidden_entity_record_004691b0` | thiscall(db, name), RET 4. Key built by 0041E870, 00468CD0 on db+18h, key released, checked-iterator tests (00BF6713), 0 at the head, else node+14h. |
| 00469240 | `scene_hidden_entity_record_copy_construct_00469240` | thiscall(record, source), RET 4. Vtable 00CE5640; bag clone 008F41F0; +8 and +0Ch duplicated by 00438E40; +10h copied; frame copied by 004134F0; the +54h NativeString assigned. |
| 00469300 | `scene_hidden_entity_record_destruct_00469300` | fastcall(ECX record). Bag deleting destructor (slot 0, 1); free 00BF6989 of +8 and +0Ch; +54h released. |
| 004693A0 | `scene_hidden_entity_record_deleting_destructor_004693a0` | thiscall(record, flags), RET 4. 00469300, then free 00BF65AC when bit 0 is set. |
| 004693C0 | `scene_hidden_entity_record_construct_004693c0` | thiscall(record, bag, className, name, party, frame, parentName), RET 18h. +8 = 00438E40(name), +0Ch = 00438E40(className). |
| 00469480 | `scene_database_has_hidden_entity_record_00469480` | thiscall(db, name), RET 4. Head read before 00468CD0; result is iterator node != head; key released after the compare. |
| 00469610 | `scene_database_queue_entity_command_00469610` | thiscall(db, a, b, c), RET 0Ch. operator new 00BF681B(14h), 004690D0(a, b, c) when non-null, then 00468350 on db+14Ch. |
| 00469690 | `scene_database_class_name_to_id_00469690` | thiscall(db, className), RET 4. Iterator begins before the key is built; inline case-insensitive compare through __stricmp 00BF7FBF; returns *(node+8)'s first dword or 0. |
| 004697B0 | `scene_database_destroy_pending_objects_004697b0` | fastcall(ECX db). Slot-0 deleting destructor on each non-null pending object, then the inline erase(begin, end) with its checked tests and memmove_s 00BF67A7. |
| 00469B60 | `scene_name_table_lookup_value_00469b60` | thiscall(owner, name), RET 4. 00468ED0 on owner+4; returns node+8 with no null test. |
| 00469EB0 | `scene_database_find_class_value_by_name_00469eb0` | thiscall(db, className), RET 4. 00468FB0 on db+34h; node+8 or 0. |
| 0046A090 | `scene_record_ptr_delete_0046a090` | fastcall(ECX record**). 00469010, free 00BF65AC, slot = 0. |
| 0046A9F0 | `scene_command_list_clear_0046a9f0` | fastcall(ECX list). Pops each node from the head: record fields (00469010) and record freed, node unlinked and freed (00BF65AC). |
| 0046AA60 | `named_vector_destruct_0046aa60` | [ESP+4] object, RET 4. Frees +8; when the object is non-null releases its NativeString and returns it to the node pool 00E18514 (0043B0A0). |
| 0046B240 | `scene_class_map_destruct_0046b240` | fastcall(ECX map). For 64 buckets: free node+8, release the key, return the node to the 00E18514 pool, clear the bucket; map+4 = 0. The pool return is 0043B0A0's body inlined at 0046B2A4..0046B302; it is issued as the 0043B0A0 host call (identical side effects, checked instruction by instruction). |
| 0046B680 | `scene_database_clear_hidden_records_0046b680` | fastcall(ECX db). When the map size is non-zero: each record taken from node+14h, the slot zeroed, the record's deleting destructor; iterator advanced by 00467980; then 0046AD10(root) and the head reset. Caller BSP_Game_DestroyWorld. |
| 0046BD10 | `scene_database_release_records_and_pending_0046bd10` | fastcall(ECX db). Record deleting destructors (slot not zeroed), pending-object deleting destructors, map cleared, pending vector erased. Callers 0046EFB0, BSP_Game_TeardownSessionState. |
| 0046BE50 | `scene_stock_push_positive_counters_0046be50` | fastcall(ECX owner). For game+1910h..+192Ch (00E188A8 re-read), pushes the address of each positive dword with 00442190 on owner+8. No static callers. |
| 0046BE90 | `scene_stock_register_unit_class_0046be90` | thiscall(owner, unitClass), RET 4. 2ACh returns at once; otherwise unless 00468660([00E18680], unitClass) names "SpawnPoint" (00CE56B8, 00425850), pushes &unitClass with 00442190 on owner+8. |
| 0046BF20 | `scene_stock_register_from_type_property_0046bf20` | thiscall(owner, bag, className), RET 8. Needs a "Type" property (00CE4780, 008F2260) and 00469690(owner, className) != 47h, then 0046BE90 with the property's +0Ch value (the bag searched a second time). |
| 0046BF70 | `scene_stock_register_class_stock_lists_0046bf70` | thiscall(owner, bag, className), RET 8. (1) className "AirField" (00CE5738, inline __stricmp on a 0041E870 copy) or "MotherShipGen" (00CE5728, 00425850 on a second copy): "PlaneStock %d" (00CE5718) for 1..12 and "Slot %d" (00CE56F4) for 1..NumSlots, each a type-6 sub-bag whose "Count" is non-zero; its "Type" goes to 0095C4D0 when the bag's "Hidden" byte is 0, else to 0046BE90. (2) "Shipyard" (00CE56E8): "Stock %d" for 1..20 with "Type" > 0, routed the same way. (3) "SpawnPoint" (00CE56B8): "Stock %d" from 1 in the "JapanList" sub-bag, then in "AlliedList" with the index continuing; each with "Count" != 0 queues "Type" through 0095C4D0 unless 004BCA50 reports game mode 4; the list bag may be null and is still passed to 008F2260. Keys are formatted by 00467310 into 20h-byte buffers. |
| 0046DC10 | `scene_database_create_hidden_entity_at_0046dc10` | thiscall(db, name, second, const float pos[3], float heading), RET 10h. ESP-anchored from the listing (frame 0Ch + 94h + 10h; arguments at [S+B4h..C0h]; the 0046C550 RET 5Ch and the creator's 10h cleanup return ESP to S, matching the epilogue's [ESP+A4h]). Hidden record by name (0041E870, 00468CD0 on +18h, checked-iterator tests; 0 when absent); class row of record+0Ch via 00468FB0 on +34h (no null test); parent from 00925A90 on [00E188A8]+19CCh when the record's +54h string is non-empty; the record frame (+14h) copied with elements 12..14 replaced by pos, and the 3x3 rebuilt by 00467050(0, heading, 0) when the double at 00CE3828 (float 2*pi) is strictly above heading; 0046C550(className from 00468660, second, parent, &frame, bag, 0, identity matrix by value (1.0f from 00D7A24C), 0) gates, a refusal returns 0; then the creator at row+4 (ECX class id, EDX second, parent, &frame, bag, 0), a 0Ch-byte 00922E20 bag holder at entity+C0h (0 if operator new fails), 00925F20 with CL = 0; returns the entity. |
| 0046EFB0 | `scene_database_destruct_0046efb0` | fastcall(ECX db). 0046BD10; sets +164h and +158h erased (0046B170) and their heads freed; 004683B0 on +14Ch; NativeStrings +144h, +13Ch; class map (vtable 00CE55B0, 0046B240); pending vector freed; record map erased (0046B570) and head freed; +0Ch vector freed. |
| 0046F160 | `scene_database_construct_0046f160` | fastcall(ECX db), returns db. Head nodes from 00468260 (isnil +19h) and 004682B0 (isnil +11h); 64 buckets zeroed; +4 byte = 1; 0046AC50 inserts 44h, 5Eh, 1Dh, 47h, 3Dh into +158h and 47h, 3Dh into +164h. |
| 0046F350 | `scene_database_create_singleton_0046f350` | cdecl. operator new 00BF681B(170h), 0046F160, store to 00E18680 (0 on allocation failure). |
| 0046F3E0 | `scene_database_destroy_singleton_0046f3e0` | cdecl. Slot-0 deleting destructor (1) of [00E18680] when set, then 0. |

## Visual class entities, cloud entity and path entity

| Address | Function | ABI and notes |
|---|---|---|
| 00472530, 00473E30 | `class_entity37_sentity_init_00472530`, `..._00473e30` | Each is a lone JMP 0048DBD0 (fastcall ECX entity). |
| 00475990 | `cloud_entity_destruct_00475990` | fastcall(ECX entity). Vtables 00CE5E68 / 00CE5E50 / 00CE5E48; when +1A4h and [00E188A8]+19E8h are set, 00BBDDF0 on the atmosphere then 00BBD910(instance) on the manager; then 00487380. |
| 00476550 | `cloud_particle_array_reserve_00476550` | thiscall({data,size,cap}, n), RET 4. 20h-byte elements; operator new 00BF55BE, free 00BF6989; the three floats at +10h..+18h are copied through FLD/FSTP in the image (a signalling NaN would be quieted there and not here). |
| 00476BD0 | `cloud_entity_create_visual_00476bd0` | fastcall(ECX entity). Without class+34h: +178h = 0 and +1A4h = 00BBD770(manager, class+28h). Otherwise operator new 00BF681B(1ACh), 007135C0(entity, model) with the model from [[+170h]+10h] vtable +8 (6, 1.0f), stored at +178h; parented with 00B6E680 under the nearest kind-4 ancestor with a visual (no null test on the result, as in the image), or the node flagged 40000000h (00475E60 on each child) and registered with 00B6D890 under [00E188A8]+19ECh. |
| 00476D80 | `cloud_entity_update_visibility_00476d80` | fastcall(ECX entity). Particle path: instance vtable +34h(entity+74h), then 00BBE760(0) if class+34h is still 0. Visual path: node vtable +38h(entity+74h), then 00B6DA70([+174h]+1Ch, 1) when +174h is set. |
| 00477180 | `cloud_entity_write_scene_properties_00477180` | thiscall(entity, writer), RET 4. 00924D90, then writer vtable +0Ch({0, "cloudclass" 00CE5FC4}, {0, class+8 or 00E186A4}). |
| 004772C0 | `property_bag_find_by_cstring_004772c0` | thiscall(bag, name), RET 4. The NativeString copy is searched with 008F2260; on a hit the copy is searched again, on a miss the original C string; the second result is returned. |
| 0047B660 | `path_entity_construct_0047b660` | thiscall(entity, id), RET 4. 00928630(0, id), 007B28C0(entity) on +1E4h, vtables 00CE63E8 (+1E4h), 00CE6290, 00CE6274, 00CE626C, 00CE6268 (+170h), +C4h = 47h. |
| 0047B700 | `path_entity_destruct_0047b700` | fastcall(ECX entity). The same vtables, 007B1BD0 on +1E4h, then 009287B0. |

## Not reconstructed

- 0046B730 (scene reset on engine-movie stop: collects script entities, kills them, runs extra
  fixed steps, SetGameMode 8, then clears the database): 280 lines of register-mangled
  pseudocode over game-world state; mission flow is hooked in `src/game_hosts_mission_frame.cpp`.
- Gameplay, belongs to cc9: 004684F0 (squadron auto-target ordnance latch helper), 00470440
  (powerup multiplier), 004704B0 / 004704E0 (hit record chain), 0047B7F0, 0047B850 (attack gate),
  0047B880, 0047B9F0 (unit class helpers).
- 0047BA40 (quad-tree collect helper, tagged STL-probable) and 00468790 (a JMP thunk).
