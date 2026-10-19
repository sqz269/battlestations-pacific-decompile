# Audit of the live `stl_probable` rows (cc15-stlA)

Packet cc15-stlA, 2026-10-18. Tags-only pass over the 54 `stl_probable` bookmark rows that were still live
(segments 5, 7, 8, 22, 23, 27, 28, 29, 40, 47, 56, 61) after cc13-stlreview1
(`docs/CC13_STL_PROBABLE_REVIEW.md`), which had kept them as library-shaped without a shape name, kept them as uncertain,
or deferred them under another lease. `007F0280` (cc9) was not touched. No host or C++ code changed.

Status: tag and name metadata from static evidence (exports, callers, host call sites in `src/`). Nothing here is
reconstructed, build-tested or game-validated. Game names are provisional hypotheses, not recovered symbols.

Result: 24 reclassified to game code, 30 confirmed library, 0 unsure.

- Game rows: the tag row becomes `game_code` / `reviewed` (evidence `cc15stlA_game`), with a reviewed name in
  `config/names/` (`007D05E0` already had one and keeps it, evidence `cc15stlA_game_kept`). Ghidra renamed by
  `tools/ghidra_annotate.py --apply` with an evidence plate comment appended to any existing comment.
- Library rows: the tag row becomes `stl_shape` / `tmpl_shape` / `rename` with a `STL_<Shape>_<addr>` or
  `TMPL_<Shape>_<addr>` name; four keep the container names cc13-seg27 had already put in the names ledger.
  Ghidra renamed from `FUN_` by `ghidra_tag.apply` under `ghidra_lock`, with an `Inventory: <category>` bookmark.
- Every row's `Inventory: stl_probable` bookmark was deleted. The decision list and apply scripts were
  `local/cc15stlA_*` in the worker tree (not committed); the table below is the record.

| address | verdict | name | category | evidence |
| --- | --- | --- | --- | --- |
| `0045B420` | game | `BSP_MovieSpline_AddPoint` | game_code | movie spline point insert keyed by float time into the owner list at +F8h (NativeString payload, sized-pool frees); only caller is the Point reader movie_spline_read_point_0045eca0 (host movie_spline_add_point_0045b420) |
| `0045DDA0` | game | `BSP_EngineMovie_Reset` | game_code | deletes every MovieEntityTrack in the vector at +68h (BSP_MovieEntityTrack_Destroy + _free), empties it, then reloads movie+0/+4 from 00D7A24C/00CE3800; called from BSP_EngineMovie_ParseFile (host engine_movie_reset_0045dda0) |
| `00480A90` | game | `BSP_WeightedIndex_PickBySeed` | game_code | cumulative float weights in the vector arg; threshold ((seed*7B913h+67933h) mod 5A1C9h)/[00CE6428]; returns the first index whose running sum exceeds it (host weighted_pick_00480a90, soldier/vehicle class picks) |
| `004872D0` | game | `BSP_ListNode_RemoveChild` | game_code | ECX = parent ([child+8]), arg = child: erases every entry equal to child from the parent's child list at +Ch; called from the node destructor 00489DD0 (PUSH ESI at 00489E04) |
| `004881F0` | game | `BSP_Manager21D0_DeleteKind1And2Objects` | game_code | walks the object list of [this+8] (+Ch list) and calls the scalar deleting destructor of every object whose kind +4 is 1 or 2; caller BSP_SceneDatabase_KillPass on [game+21D0h] |
| `00489DD0` | game | `BSP_ListNode_Destruct` | game_code | base destructor for vtable 00CE6600: detaches from parent +8 through 004872D0, deletes every child object in the list at +Ch, list tidy 00485290; 25 callers incl. BSP_SceneProxy_Destruct (host list_node_destruct_00489dd0) |
| `0048CA60` | game | `BSP_Manager21D0_CountEntryEvent` | game_code | under the manager critical section ([this+4], depth +18h), finds the unit key (unit+1E4h) in the tree at +28h and increments the entry record [+10h]+D8h counter +10h (flag set) or +14h (flag clear); caller 006AC370 (host game_21d0_register_0048ca60) |
| `0048CB30` | game | `BSP_Manager21D0_ClearEntryActive` | game_code | under the manager critical section, finds the key in the tree at +28h and zeroes the entry record [+10h]+DCh; caller BSP_EntryList_UnregisterFromManager21D0 (host manager21d0_call_0048cb30) |
| `0048CBB0` | game | `BSP_Manager21D0_IsEntryActiveAndSet` | game_code | under the manager critical section, returns 1 when the key is found and the entry record has +DCh and +BCh non-zero; caller BSP_CommandBuilding_AnyPadOccupied (host pad_group_query_0048cbb0) |
| `0048D9E0` | library | `STL_BitIterator_Inc_0048d9e0` | stl_shape | vector<bool> checked iterator operator++: bit index <1Fh -> +1 else word+4 and bit 0, range hooks to _invalid_parameter_noinfo; callers are STL_IterWrap rows |
| `0048F4E0` | library | `STL_BitIterator_Minus_0048f4e0` | stl_shape | vector<bool> iterator operator-(n): copy, STL_BitIterator_Advance(-n), return by value |
| `0048F930` | library | `STL_BitVector_MakeIter_0048f930` | stl_shape | vector<bool> checked _Make_iter: container hooks then STL_CheckedBitIterator_CopyAdvance into the out iterator |
| `004913B0` | library | `STL_BitVector_FillRange_004913b0` | stl_shape | fill(first,last,bool) over vector<bool> words: set or clear bit, checked bit-iterator ++ |
| `00492E50` | library | `STL_BitVector_Trim_00492e50` | stl_shape | vector<bool>::_Trim(size): erase words beyond (size+31)/32, store size, mask the last word; caller STL_BitVector_Erase_00499700 |
| `004946E0` | library | `STL_BitVector_FillDispatch_004946e0` | stl_shape | checked-iterator unwrap hooks then STL_BitVector_FillRange 004913B0 |
| `00494770` | library | `STL_BitVector_CopyDispatch_00494770` | stl_shape | copy(first,last,dest) for vector<bool>: dest+distance via 0048F2E0, hooks, STL_BitVector_CopyRange 004914E0 |
| `00494850` | library | `STL_BitVector_CopyBackwardDispatch_00494850` | stl_shape | copy_backward for vector<bool>: dest-distance via 0048F4E0, hooks, STL_BitVector_CopyBackward 004916A0 |
| `00496670` | library | `STL_BitVector_CheckedCopy_00496670` | stl_shape | checked-iterator wrapper over 00494770; caller STL_BitVector_Erase_00499700 |
| `00496720` | library | `STL_BitVector_CheckedCopyBackward_00496720` | stl_shape | checked-iterator wrapper over 00494850; caller 00499AC0 |
| `00499AC0` | library | `STL_BitVector_InsertGap_00499ac0` | stl_shape | vector<bool>::_Insert_x: length check (xlen 00490CE0), resize words, size+=n, copy_backward the tail; returns the bit offset |
| `0049CB60` | library | `STL_Map_Subscript_StringCI_0049cb60` | stl_shape | map<NativeString,T,case-insensitive less>::operator[]: lower_bound 00485A70, LessCaseInsensitive, insert-hint 004998A0 with a default value, return &node->value (+14h) |
| `0049CC50` | library | `STL_BitVector_InsertN_0049cc50` | stl_shape | vector<bool>::_Insert_n(where,n,val): 00499AC0 gap then fill dispatch 004946E0; caller STL_CheckedBitVector_Resize |
| `0049E150` | library | `STL_BitVector_Insert_0049e150` | stl_shape | vector<bool>::insert(where,n,val) returning begin()+offset through 0049CC50 and 0048F2E0 |
| `004A5750` | library | `STL_Map_Subscript_U32_004a5750` | stl_shape | map<unsigned,pair-of-vectors>::operator[]: isnil +21h walk, default value from STL_Vector_CopyConstruct, insert-hint 004A49B0, temporaries freed |
| `004A5AA0` | game | `BSP_BreakupRegistry_DestroyUnitEntries` | game_code | while map[unit] (004A5750) is non-empty deletes its front object by scalar deleting destructor, then erases the key's equal_range; callers BSP_UnitInstance_OnBreakupMessage/OnWrecked (host breakup_register_wreck_004a5aa0) |
| `004AD380` | game | `BSP_PtrVectorOwner00CE6F68_Destruct` | game_code | class destructor: vtable 00CE6F68, deletes every owned object in the vector at +18h through its scalar deleting destructor, then frees the storage; only caller the class deleting destructor 004AD5C0 |
| `005DA670` | library | `STL_List_UniqueStringCI_005da670` | stl_shape | list<string>::unique(pred): adjacent nodes with equal length and __stricmp==0 erased through 004D0990; size>1 guard |
| `005DEE40` | library | `STL_List_MergeStringCI_005dee40` | stl_shape | list<string>::merge(other, pred): __stricmp ordering, splice node with _Incsize 004CE780, size--, tail splice 005DADB0 |
| `005FE8B0` | library | `STL_List_RemoveStringCI_005fe8b0` | stl_shape | list<string>::remove(val) with a copied NativeString key, length+__stricmp compare, erase 004D0990 |
| `005FF5D0` | library | `STL_Map_Subscript_Int_005ff5d0` | stl_shape | map<int,IdNamePair>::operator[]: isnil +19h walk, BSP_IdNamePair_Construct default, insert-hint 005FF130 |
| `005FFB20` | library | `STL_Map_Subscript_StringCI_005ffb20` | stl_shape | map<NativeString,named value,case-insensitive>::operator[]: lower_bound 005FD0C0, LessCaseInsensitive, default value 005FE370, insert-hint 005FF6F0 |
| `00667EB0` | library | `STL_Map_U16Int_Subscript_00667eb0` | stl_shape | map<unsigned short,int>::operator[] (isnil +15h), insert-hint 006658D0; ledger name from cc13-seg27 kept |
| `0066E610` | library | `STL_Map_U16Map_Subscript_0066e610` | stl_shape | map<unsigned short,map>::operator[] (isnil +1Dh), default map 00659DF0/00665340, insert-hint 0066D650; ledger name kept |
| `0066FC30` | library | `STL_Map_U16Map_Subscript_0066fc30` | stl_shape | map<unsigned short,map>::operator[] twin of 0066E610 (00659B40/00665040/0066E770); ledger name kept |
| `0067D5A0` | library | `STL_WString_AssignN_0067d5a0` | stl_shape | basic_string<wchar_t>::assign(ptr,n): aliasing branch to 0056F160, max_size 7FFFFFFEh, _Grow 0056B0F0, _memcpy_s, SSO cap 7; ledger name kept |
| `00687860` | game | `BSP_MenuManager_HasScreen` | game_code | on the 00E198B4 menu manager: true when +20h (current screen id) equals the id and the flag is 0, else when the id is in the dword vector at +5Ch; ids 4, 0Bh and 14h at the callers |
| `00694760` | library | `STL_RemoveCopy_00694760` | stl_shape | remove_copy(first,last,dest,val) over dwords with checked output-iterator hooks; caller 00695170 |
| `00695170` | library | `STL_Remove_00695170` | stl_shape | remove(first,last,val): find first match then remove_copy 00694760, checked iterators |
| `00695C50` | library | `STL_Vector_Reserve_00695c50` | stl_shape | vector<T*>::reserve(n): max 3FFFFFFFh (xlen 00695940), allocate 00693830, _memmove_s, free old, reset +4/+8/+0Ch |
| `006A9EA0` | game | `BSP_InputSettings_DeviceHasBindingKind2` | game_code | device map +8 lookup (0055C110), walks the device's binding tree and returns 1 when any of the two 14h-byte slots of an entry has kind 2; caller BSP_ControlBindingScreen_RefreshPage |
| `006AA1C0` | game | `BSP_InputSettings_CopyPrimaryToShadow` | game_code | for each device in the map at +8, assigns its three binding trees (+20h, +38h, +80h) into the matching entry of the map at +6Ch; caller BSP_ControlBindingScreen_ListboxListener04 |
| `006AA2D0` | game | `BSP_InputSettings_CopyShadowToPrimary` | game_code | byte-for-byte mirror of 006AA1C0 with the operands swapped (LEA ESI,[EBX+0Ch] vs [EDI+20h]): the +6Ch map entries are assigned back into the +8 map; callers ReloadInputConfig, OnRevertPromptResult, ConfirmOrNextDevice |
| `00734870` | game | `BSP_NarrativeList_Append` | game_code | appends a NarrativeEntry (vtable 00CFE960) at the tail of the list at +8, copies two NativeStrings to +0Ch/+14h and the int to +1Ch; mission narrative senders (host narrative_list_append_00734870) |
| `007349C0` | game | `BSP_NarrativeList_PushFrontUnlessSame` | game_code | returns early when the list is non-empty and the front entry equals both strings (EqualsInsensitive); otherwise inserts a NarrativeEntry before the front; callers BSP_LuaBinding_MissionNarrativeUrgent, BSP_Game_ShowHighLatencyWarning |
| `00734E50` | game | `BSP_NarrativeList_ReplaceAll` | game_code | BSP_NarrativeList_Clear then appends one NarrativeEntry with the two strings and int; callers BSP_LuaBinding_MissionNarrativeOverride, BSP_Game_CheckMultiplayerPlayerCount |
| `007C3000` | library | `TMPL_PointerArray_Resize_007c3000` | tmpl_shape | PodArray4 {data,size,cap} resize(n): reserve 007BD940 when cap<n, zero new slots, size=n |
| `007D05E0` | game | `BSP_Node_AppendChildrenByNamePrefix` | game_code | existing cc13-gpr47 game name kept: BSP_Node_CollectChildrenByNamePrefix then push_back of each result into the vector arg |
| `008CFC10` | library | `STL_Map_Subscript_Int_008cfc10` | stl_shape | map<int,record>::operator[]: isnil +25h walk, default value 0088B5A0, insert-hint 008CCCF0, two NativeString temporaries freed |
| `008DC500` | game | `BSP_Objective_RemovePositionEntries` | game_code | erases from the list at +20h every entry whose record has +0==0 and float3 +4/+8/+0Ch equal to the position arg, freeing the record; caller BSP_ObjectiveSet_RemovePositionFromNamedObjective |
| `0093BA80` | library | `STL_Vector_Resize_0093ba80` | stl_shape | vector<16-byte T>::resize(n,val): grow through _Insert_n 0093B690 or erase 0093B5C0, by-value NativeString arg freed |
| `00943E40` | game | `BSP_SpawnManager_RemoveRequest` | game_code | erases the first list entry whose object +7Ch equals the id (message +58h at the caller); host spawn_manager_remove_request_00943e40 |
| `00943EC0` | game | `BSP_SpawnManager_RemoveRequestForEntity` | game_code | takes entity +28Ch as the id and erases the matching list entry (+7Ch) through 009439B0; caller BSP_Replication_ApplyPendingEntityCreates |
| `00946970` | game | `BSP_SpawnManager_SumRequestValue` | game_code | sums 00946870(key) over every node of the list at +4 and returns the float on ST0; callers BSP_AiPlanner_QuickSpawn/DefendThink (host spawn_manager_player_usage_00946970) |
| `0094EDC0` | game | `BSP_String_SplitQuoted` | game_code | splits a std::string at characters in the delimiter set (_strchr), quote '"' toggles, pushing substrings into the vector; caller BSP_ShipyardStockRecord_Construct with "," (host string_vector_split_0094edc0) |

## Notes

- The rule (calls `_invalid_parameter_noinfo` with a 97-256 byte body) is a poor discriminator in this band.
  VC9 checked iterators put the hook into every inlined `list`/`vector` walk, so 24 of the 54 rows were game code:
  class destructors, manager queries under a critical section, narrative and spawn queues, input-binding copies.
- `006AA1C0` and `006AA2D0` are the same template-free body with the source and destination maps swapped
  (+8h and +6Ch). Which map is the live input configuration was not proven; the Primary/Shadow names only
  record the direction.
- `0045B420` is listed at 1712 bytes, above the rule's cap; it was still bookmarked through rule `st1_scl97`.
