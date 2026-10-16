# Segment 41 standalone reconstructions (cc13-seg41)

Range 00735F90-00758EB0 (segment 41: land convoys and forts, landing ships,
land vehicles, plane-unit slots, the decal manager, singleton lifetime
helpers, save-reader refs and the mothership). Code:
`include/bsp/cc13_seg41.hpp` + `src/cc13_seg41_misc.cpp` (containers,
singletons, decals, locale, VFS) and `include/bsp/cc13_seg41_land.hpp` +
`src/cc13_seg41_land.cpp` (units and their messages), namespace
`bsp::cc13_seg41`.

Status of every reconstructed function: reconstructed and build-tested only.
New C++ interfaces, not ABI-compatible, not fixture-tested, not wired into any
host, not game-validated. Nothing in the game build calls these files, so no
behaviour changes and no switch.

Function and field names are hypotheses, not recovered symbols.

## Conventions

- Entity and native objects are addressed by their image base
  (`std::uint8_t*`) and read or written at the image offsets (`at<T>`); no
  struct field is invented for them. Small objects whose whole layout the
  function establishes (vectors, decal records, messages) get structs with
  Win32 `static_assert`s.
- Every call the image makes goes through a named host hook invoked at the
  same point with the same arguments, or through an existing reconstruction
  called directly (`build_entity_create_00766fc0`, `write_entity_create_00765000`,
  `read_entity_create_00767e30`, the bit-cursor primitives,
  `construct_native_session_message_0075b430`,
  `return_native_locale_node_slot_00736090`, `cc13_sys::lazy_singleton_get` /
  `lazy_singleton_destroy`).
- Vtable stores are stores of the image address (raw objects) or go through
  `SessionMessageHost::vtable(image_address)` for `NativeSessionMessageStorage`.
- Numeric floats in the codecs use (zero 0, signed 1, scale [00D7A248] =
  FLT_MAX, 32 bits) and raw bits; the image's FLD/FSTP quiets a signalling NaN
  (MINOR, NaN inputs only). The same applies to the FLD/FSTP copies in 00745130
  and 00747560.
- Allocation failure paths that the image follows into a null dereference are
  kept as they are (no added guards): 00740660, 00747000, 00748C40, 0074BE00.
- The entity create messages embed the existing `net_cc13::EntityCreateMessage`
  codec model for the 110h head; the image's `char*` name at +C0h is the model's
  string plus presence flag (used by 007474E0).

## Reconstructed: containers, singletons, decals, VFS (src/cc13_seg41_misc.cpp)

| Address | Function | Original ABI | Evidence and notes |
|---|---|---|---|
| 00735F90 | `reserve_00735f90` | __thiscall(ECX vector {data,size,cap}, int n) RET 4 | Template reserve (listing 00735F90): n<1 -> 1; when cap<n operator new 00BF55BE(n*4), copy size dwords (store skipped for a null slot pointer), free 00BF6989(old), store data/cap. Signed compares as in the listing. |
| 00736050 | `release_ref_00736050` | __fastcall(ECX slot) -> EAX slot | 00736050: identical body to 00736050: when *slot, InterlockedDecrement([00CE2220]) on +4 and vtable slot 0 (ECX object, no stack args) at zero, then *slot=0. |
| 00736100 | `singleton_register_00736100` | __fastcall(ECX this) -> this | [this]=00CFEA54; lock (00415350, section +10h, depth +18h, skipped when null); [0109CF00]=this; 00BD0C30(re-read global) through a second 00415350; unlock. Shared body singleton_register. |
| 007361A0 | `singleton_destroy_007361a0` | __cdecl(void) RET | Global 00F8BBF8: the shared cc13_sys::lazy_singleton_destroy body (same template as 004C0E40): test, lock, re-test, 00BCFCA0, vtable[0](1) when still set, clear, unlock. |
| 00736300 | `singleton_destroy_00736300` | __cdecl(void) RET | Global 00F8BF44: the shared cc13_sys::lazy_singleton_destroy body (same template as 004C0E40): test, lock, re-test, 00BCFCA0, vtable[0](1) when still set, clear, unlock. |
| 00736460 | `push_back_00736460` | __thiscall(ECX vector, const dword* value) RET 4 | Template push_back: when size==cap reserve through 00735EC0 (host hook) with 2*cap (values below 2 become 1), store *value unless the slot pointer is null, ++size. |
| 007364A0 | `singleton_register_007364a0` | __fastcall(ECX this) -> this | [this]=00CFEA58; lock (00415350, section +10h, depth +18h, skipped when null); [00F8C26C]=this; 00BD0C30(re-read global) through a second 00415350; unlock. Shared body singleton_register. |
| 00736540 | `singleton_register_00736540` | __fastcall(ECX this) -> this | [this]=00CFEA5C; lock (00415350, section +10h, depth +18h, skipped when null); [00F8BC4C]=this; 00BD0C30(re-read global) through a second 00415350; unlock. Shared body singleton_register. |
| 007365E0 | `singleton_unregister_007365e0` | __fastcall(ECX this) RET | Holder destructor: [this]=00CFEA5C; lock; 00BCFCA0([00F8BC4C]); [00F8BC4C]=0; unlock; [this]=00CE3818. |
| 007366B0 | `singleton_register_007366b0` | __fastcall(ECX this) -> this | [this]=00CFEA60; lock (00415350, section +10h, depth +18h, skipped when null); [00F8AEFC]=this; 00BD0C30(re-read global) through a second 00415350; unlock. Shared body singleton_register. |
| 00736750 | `singleton_unregister_00736750` | __fastcall(ECX this) RET | Holder destructor: [this]=00CFEA60; lock; 00BCFCA0([00F8AEFC]); [00F8AEFC]=0; unlock; [this]=00CE3818. |
| 007367F0 | `singleton_register_007367f0` | __fastcall(ECX this) -> this | [this]=00CFEA64; lock (00415350, section +10h, depth +18h, skipped when null); [010909B0]=this; 00BD0C30(re-read global) through a second 00415350; unlock. Shared body singleton_register. |
| 00736890 | `singleton_unregister_00736890` | __fastcall(ECX this) RET | Holder destructor: [this]=00CFEA64; lock; 00BCFCA0([010909B0]); [010909B0]=0; unlock; [this]=00CE3818. |
| 007369E0 | `singleton_destroy_007369e0` | __cdecl(void) RET | Global 01090490: the shared cc13_sys::lazy_singleton_destroy body (same template as 004C0E40): test, lock, re-test, 00BCFCA0, vtable[0](1) when still set, clear, unlock. |
| 00736D00 | `msar_provider_factory_get_00736d00` | __cdecl(void) -> EAX [010904F8] | Shared cc13_sys::lazy_singleton_get with row {size 8, inline vtables [+4]00CFEA04,[+0]00CFEA2C,[+4]00CFEA28, register global+4 or null}. |
| 00736F70 | `singleton_unregister_00736f70` | __fastcall(ECX this) RET | Holder destructor: [this]=00CFEA68; lock; 00BCFCA0([00F88C20]); [00F88C20]=0; unlock; [this]=00CE3818. |
| 00737340 | `resize_00737340` | __thiscall(ECX vector, int n) RET 4 | Template resize: reserve 00735F90 when cap<n, zero-fill [size,n), count size down to n (trivial element dtor), size=n. |
| 007373F0 | `locale_node_return_slot_thunk_007373f0` | __fastcall(ECX slot) RET | PUSH ECX; MOV ECX,00E1768C; CALL 00736090; RET: calls the existing return_native_locale_node_slot_00736090(pool E1768C, slot). |
| 00737400 | `singleton_unregister_00737400` | __fastcall(ECX this) RET | Holder destructor: [this]=00CFEA54; lock; 00BCFCA0([0109CF00]); [0109CF00]=0; unlock; [this]=00CE3818. |
| 007375E0 | `singleton_unregister_007375e0` | __fastcall(ECX this) RET | Holder destructor: [this]=00CFEA58; lock; 00BCFCA0([00F8C26C]); [00F8C26C]=0; unlock; [this]=00CE3818. |
| 007376E0 | `holder_construct_007376e0` | __fastcall(ECX this) -> this | 007366B0(same ECX) then [this]=00CFEA70. |
| 007377B0 | `holder_construct_007377b0` | __fastcall(ECX this) -> this | 007367F0(same ECX) then [this]=00CFEA80. |
| 00737810 | `destroy_00737810` | __fastcall(ECX vector) RET | resize 00735F30 (host hook)(0) then free(data) through 00BF6989. |
| 00737850 | `frame_hook_slot04_noop_00737850` | bare RET | Frame hook vtable slot: a single RET at 00737850. |
| 00737890 | `frame_hook_slot14_noop_00737890` | bare RET | Frame hook vtable slot: a single RET at 00737890. |
| 00737A00 | `holder_construct_00737a00` | __fastcall(ECX this) -> this | 007367F0(same ECX) then [this]=00CFEAC8. |
| 00737BD0 | `destroy_00737bd0` | __fastcall(ECX vector) RET | resize 00737340(0) then free(data) through 00BF6989. |
| 00737E20 | `application_lifetime_construct_00737e20` | __fastcall(ECX this) -> this | 00736100, then a temporary native string from the empty literal 00CE3A0C: 0041DD40(&s,0,1), memcpy len+1, 00419CC0 + 00BD1510(data,len+1,1) (00419CC0 takes no stack args; the pushes belong to 00BD1510), [this]=00CFEAE4. |
| 00737EE0 | `holder_construct_00737ee0` | __fastcall(ECX this) -> this | 007364A0(same ECX) then [this]=00CFEAE8; then +4..+18h = 0. |
| 0073BC40 | `mount_content_packages_0073bc40` | __cdecl(void) RET | No caller in the image. 00BE0660(ECX [0109CEEC], 00736D00()); n=1..15, priority -11..-25: "%08x", "ContentPackage/%s.msar" (004B8490); vfs slot+8 find, 00BBBB90 validate (ECX string), 00BE1890(path, root "." (00CE3A70), priority, 0, -1), 00BDEF80(suffix). Argument order of 00BE1890 follows the decompiler. |
| 0073C9F0 | `locale_table_manager_destroy_0073c9f0` | __fastcall(ECX manager) RET | vtable 00CFF140; free/zero +4034h and +4024h vectors (00BF65AC); release the string {len +4018h, data +401Ch}; 0073C240(+14h); 00432050(ECX begin +8, EDX end +0Ch, push this+4, push this) + free when begin set; zero +8..+10h; 007365E0. |
| 0073E6B0 | `operator_delete_0073e6b0` | __stdcall(void* p, unused) RET 8 | free(p) through 00BF65AC. |
| 0073E8A0 | `reserve_0073e8a0` | __thiscall(ECX vector {data,size,cap}, int n) RET 4 | Template reserve (byte-identical body to 00735F90): n<1 -> 1; when cap<n operator new 00BF55BE(n*4), copy size dwords (store skipped for a null slot pointer), free 00BF6989(old), store data/cap. Signed compares as in the listing. |
| 0073E940 | `reserve_0073e940` | __thiscall(ECX vector {data,size,cap}, int n) RET 4 | Template reserve (byte-identical body to 00735F90): n<1 -> 1; when cap<n operator new 00BF55BE(n*4), copy size dwords (store skipped for a null slot pointer), free 00BF6989(old), store data/cap. Signed compares as in the listing. |
| 0073EA40 | `resize_0073ea40` | __thiscall(ECX vector, int n) RET 4 | Template resize: reserve 0073E940 when cap<n, zero-fill [size,n), count size down to n (trivial element dtor), size=n. |
| 0073EAC0 | `reserve_0073eac0` | __thiscall(ECX vector {data,size,cap}, int n) RET 4 | Template reserve (byte-identical body to 00735F90): n<1 -> 1; when cap<n operator new 00BF55BE(n*4), copy size dwords (store skipped for a null slot pointer), free 00BF6989(old), store data/cap. Signed compares as in the listing. |
| 0073F9C0 | `named_key_index_of_0073f9c0` | __thiscall(ECX table, const {len,data}* key) RET 4 -> index or -1 | Linear scan of the pointer array +10h (count +14h, unsigned); equal lengths, zero length matches, else _stricmp 00BF7FBF on the data pointers. |
| 0073FA30 | `push_back_0073fa30` | __thiscall(ECX vector, const dword* value) RET 4 | Template push_back: when size==cap reserve through 0073E8A0 with 2*cap (values below 2 become 1), store *value unless the slot pointer is null, ++size. |
| 0073FB10 | `decal_system_destruct_0073fb10` | __fastcall(ECX this) RET | Holder destructor: [this]=00CFF218; lock; 00BCFCA0([00E1AEA0]); [00E1AEA0]=0; unlock; [this]=00CE3818. |
| 0073FBB0 | `push_back_0073fbb0` | __thiscall(ECX vector, const dword* value) RET 4 | Template push_back: when size==cap reserve through 0073E940 with 2*cap (values below 2 become 1), store *value unless the slot pointer is null, ++size. |
| 0073FBF0 | `push_back_0073fbf0` | __thiscall(ECX vector, const dword* value) RET 4 | Template push_back: when size==cap reserve through 0073E9A0 (host hook) with 2*cap (values below 2 become 1), store *value unless the slot pointer is null, ++size. |
| 0073FC70 | `resize_0073fc70` | __thiscall(ECX vector, int n) RET 4 | Template resize: reserve 0073E8A0 when cap<n, zero-fill [size,n), count size down to n (trivial element dtor), size=n. |
| 0073FCC0 | `erase_swap_back_0073fcc0` | __thiscall(ECX vector, int index) RET 4 | index -1 is a no-op; else the last element moves into index unless index is last, --size. No bounds check (none in the image). |
| 0073FD00 | `resize_0073fd00` | __thiscall(ECX vector, int n) RET 4 | Template resize: reserve 0073E9A0 (host hook) when cap<n, zero-fill [size,n), count size down to n (trivial element dtor), size=n. |
| 0073FDA0 | `resize_0073fda0` | __thiscall(ECX vector, int n) RET 4 | Template resize: reserve 0073EAC0 when cap<n, zero-fill [size,n), count size down to n (trivial element dtor), size=n. |
| 0073FDF0 | `release_ref_00736050` | __fastcall(ECX slot) -> EAX slot | 0073FDF0: identical body to 00736050: when *slot, InterlockedDecrement([00CE2220]) on +4 and vtable slot 0 (ECX object, no stack args) at zero, then *slot=0. |
| 0073FE20 | `release_ref_00736050` | __fastcall(ECX slot) -> EAX slot | 0073FE20: identical body to 00736050: when *slot, InterlockedDecrement([00CE2220]) on +4 and vtable slot 0 (ECX object, no stack args) at zero, then *slot=0. |
| 0073FE50 | `release_ref_00736050` | __fastcall(ECX slot) -> EAX slot | 0073FE50: identical body to 00736050: when *slot, InterlockedDecrement([00CE2220]) on +4 and vtable slot 0 (ECX object, no stack args) at zero, then *slot=0. |
| 0073FE80 | `release_ref_00736050` | __fastcall(ECX slot) -> EAX slot | 0073FE80: identical body to 00736050: when *slot, InterlockedDecrement([00CE2220]) on +4 and vtable slot 0 (ECX object, no stack args) at zero, then *slot=0. |
| 0073FEF0 | `release_ref_00736050` | __fastcall(ECX slot) -> EAX slot | 0073FEF0: identical body to 00736050: when *slot, InterlockedDecrement([00CE2220]) on +4 and vtable slot 0 (ECX object, no stack args) at zero, then *slot=0. |
| 0073FF20 | `release_ref_00736050` | __fastcall(ECX slot) -> EAX slot | 0073FF20: identical body to 00736050: when *slot, InterlockedDecrement([00CE2220]) on +4 and vtable slot 0 (ECX object, no stack args) at zero, then *slot=0. |
| 0073FF50 | `decal_batch_ensure_vertex_capacity_0073ff50` | __stdcall(record, count) RET 8 -> base vertex | No stream (section+4Ch==0): device[00F8D394]+5Ch(count+n,1,format [[def+14h]+8]), 00B85B80 append, cap=slot20, release, return 0. Else when cap<base+n: new buffer, lock both slot10(0,0,0) (Ghidra dropped one argument of the first), memcpy stride(slot24 -> +CCh)*count, unlock old then new, 00B86550 clear, 00B85B80 append, cap, release; returns base. |
| 007400A0 | `destroy_007400a0` | __fastcall(ECX vector) RET | resize 0073EA40(0) then free(data) through 00BF6989. |
| 007400C0 | `erase_value_007400c0` | __thiscall(ECX vector, const dword* value) RET 4 -> bool | Find the first equal element, swap-erase it. |
| 00740140 | `list_clear_00740140` | __fastcall(ECX list {.., head +4, size +8}) RET | Relink head to itself, size 0, free every node (00BF65AC) comparing against the re-read head, free the head, head=0. |
| 007401B0 | `decal_definition_destruct_007401b0` | __fastcall(ECX definition) RET | Release +18h, +1Ch, +14h in that order (InterlockedDecrement + slot 0), then return the name storage (data +4, len+1) through 00419CC0/00BD1510. |
| 00740270 | `life_ref_release_00740270` | __thiscall(ECX owner, RefCounted* value) RET 4 | 007400C0(ECX owner+4, &value) then release the by-value reference. |
| 007402A0 | `list_construct_007402a0` | __fastcall(ECX list) -> list | +4 = 0073FC50(), +8 = 0. |
| 007402D0 | `destroy_007402d0` | __fastcall(ECX vector) RET | resize 0073FC70(0) then free(data) through 00BF6989. |
| 007402F0 | `destroy_007402f0` | __fastcall(ECX vector) RET | resize 0073FD00(0) then free(data) through 00BF6989. |
| 00740310 | `destroy_00740310` | __fastcall(ECX vector) RET | resize 0073FDA0(0) then free(data) through 00BF6989. |
| 00740350 | `decal_record_construct_00740350` | __fastcall(ECX record) -> record | vtable 00CEB130, refs 1, vtable 00CFF21C, +10h/+18h/+1Ch/+20h = 0, +28h = 0073FC50(), +2Ch = 0. |
| 007403C0 | `decal_record_destruct_007403c0` | __fastcall(ECX record), tail JMP | 00740140(ECX record+24h) then JMP 00BD30F0 (refcounted base). No vtable store in the image. |
| 007403F0 | `destroy_007403f0` | __fastcall(ECX vector) RET | resize 0073FDA0(0) then free(data) through 00BF6989. |
| 00740410 | `decal_definition_construct_00740410` | __fastcall(ECX definition) -> definition | Zero +0/+4/+14h..+1Ch; temp string "decal.mvfm" (00CFF224, 0041DD40(10,1) + memcpy); +14h = device[00F8D394] vtable+34h(&string); release the temp. +8..+10h are not stored. app_init_tail.cpp parse_decal_definitions_00740840 models the records at a higher level. |
| 007404D0 | `decal_list_destruct_007404d0` | __fastcall(ECX list) RET | vtable 00CFF230; per record 00B85B30(ECX [r+20h], [r+1Ch]), release +20h, +10h, +1Ch, the record slot; 0073FC70(0) twice (the inlined vector dtor), free 00BF6989, 00BD30F0. |
| 00740660 | `decal_list_create_record_00740660` | __thiscall(ECX list, owner, field14, definition) RET 0Ch -> record | new 30h (00BF681B) + 00740350 (the image continues with a null record on failure and faults); owner ref swap (InterlockedIncrement new, release old); 00533FA0 section, +8=4, 00B85600(0); 00B18780(110h), 00B18900(def+1Ch effect), 00B189F0(material,0,def+18h texture), 00B864C0, release material; counts 0; inline push_back (0073E8A0 growth). |
| 007407C0 | `decal_list_add_decal_007407c0` | __thiscall(ECX list, 4 dwords) RET 10h | The release body is only the profile-scope static for "cDecalList::AddDecal" (00CFF238): guard bit 0 of [00E1AEA8], 0041E870 assign, [00E1AEA4]=0, release. ECX and arguments unread. Body ends at RET 10h 00740834. |
| 00740EB0 | `decal_definition_delete_00740eb0` | __fastcall(ECX slot) RET | When *slot: 007401B0 then free 00BF65AC, *slot=0. |
| 00740F10 | `decal_manager_destruct_00740f10` | __fastcall(ECX manager) RET | vtable 00CFF2A0; delete every definition (007401B0 + 00BF65AC); 0073FD00(0) twice + free on +10h; 0073EA40(0) + free on +4; 0073FB10. |
| 00740FE0 | `decal_manager_register_owner_00740fe0` | __fastcall(ECX manager) -> list | new 14h (00BF681B), inline construct (00CEB130, refs 1, 00CFF230, zero vector), inline push_back into +4 (0073E940 growth, CMP 1/JG). |
| 007454B0 | `release_ref_00736050` | __fastcall(ECX slot) -> EAX slot | 007454B0: identical body to 00736050: when *slot, InterlockedDecrement([00CE2220]) on +4 and vtable slot 0 (ECX object, no stack args) at zero, then *slot=0. |
| 007457E0 | `release_ref_void_007457e0` | __fastcall(ECX slot) RET (no return value) | 007457E0: the 00736050 body without the EAX return. |
| 00745910 | `release_ref_void_007457e0` | __fastcall(ECX slot) RET (no return value) | 00745910: the 00736050 body without the EAX return. |

## Reconstructed: land convoy, land fort, stationary, landing ship (src/cc13_seg41_land.cpp)

| Address | Function | Original ABI | Evidence and notes |
|---|---|---|---|
| 00741180 | `land_convoy_create_destruct_00741180` | __fastcall(ECX message), tail JMP | vtable 00CFF2BC, free(+128h) 00BF65AC, JMP 007668E0 (host hook). |
| 007412F0 | `entity_ref_kill_and_clear_007412f0` | __fastcall(ECX slot, EDX value) RET | When *slot: 00926D90(ECX *slot, value), *slot = 0. |
| 00741310 | `free_and_null_00741310` | __fastcall(ECX slot) RET | When *slot: free 00BF65AC, *slot = 0. |
| 00741420 | `land_convoy_create_is_type_00741420` | RET 4, ECX ignored -> bool | t == CFh // 47h // 46h. |
| 00741450 | `land_convoy_create_write_00741450` | __thiscall(ECX message, cursor) RET 4 | 00765000 base; word +110h 13 bits; u32 +114h/+118h 8 bits; three raw-32 floats; rows*cols words 12 bits; bool +12Ch; float +130h. Raw-bit floats (the image's FLD/FSTP quiets an sNaN: MINOR). |
| 00741570 | `land_convoy_create_read_00741570` | __thiscall(ECX message, stream) RET 4 | 00767E30 base; mirror of the writer; the id array is a fresh operator new (00BF55BE, saturating size) and the old +128h is not freed. |
| 007416F0 | `land_convoy_get_property_007416f0` | __thiscall(ECX convoy, lua, key) RET 8 | 00779BB0 first; InitialSize via _stricmp on key+4 (+3B8h int), ActualSize +3BCh int, CollectedDamage +3C0h float, LastHit clock[00F876A4]-+3C4h float, Reverse +3A9h bool; Speed +368h, Offset +3ACh, DistanceTraveled +3B4h are pushed as a BOOLEAN of value != 0.0f (UCOMISS [00D7A218], TEST AH,44h / JNP; NaN true) through 00B66450 - image behaviour kept. |
| 00741A40 | `land_convoy_create_construct_00741a40` | __thiscall(ECX message, word, convoy) RET 8 -> message | 00766FC0(CFh, word, convoy, 1) via build_entity_create_00766fc0; vtable 00CFF2BC; +110h = [007ACA30(+344h)]+174h when +344h set; +114h=+358h, +118h=+354h, floats +360h/+364h/+35Ch; ids = new(rows*cols*2, saturating, 00BF55BE) from the checked std::vector +398h/+39Ch (00BF6713 on a bad index; null members give 0); +12Ch=+3A9h, +130h=+368h. |
| 00741D40 | `land_convoy_send_create_00741d40` | __thiscall(ECX convoy, peer) RET 4 | Stack message 00741A40(word 0, convoy), 00779FC0(ECX convoy, peer, &msg), inline 00741180 (vtable, free +128h, 007668E0). |
| 00742A70 | `land_convoy_member_name_00742a70` | __fastcall(ECX out string, EDX n) -> out | Roman numeral of n for 1..3999 (table 00E0CC68 -> M/C/X/I tables 00E0CC5C/38/14/CBF0, read from the PE), else "N/A" (00CECE08). Temp string built with 0041E870/0041DD40/memcpy and released; out zeroed then 0041DD40 + memcpy(out.len). ADOPTION: game_hosts_units.cpp:11918 and plane_squadron.cpp:62 use decimal. |
| 00743B70 | `land_convoy_create_entity_00743b70` | __fastcall(ECX message) -> convoy (vtable +1Ch) | new 3CCh (00BF55BE) + memset before the null test, 004F2410(id +1Ch); path +344h = 007AC9D0 of the id +110h entity (two-bank id table 00F89A10/0C/54/60/A8 inlined); dimensions/floats; members appended to the vector +394h (inline push_back, 00743350 when full, 00BF6713 when first > last); +3A9h, +368h. |
| 00743DB0 | `land_fort_create_construct_00743db0` | __thiscall(ECX message, word, fort) RET 8 -> message | 00766FC0(D0h, word, fort, 1); vtable 00CFF358; handle +110h vtable 00CFF354; 00809290(ECX +110h, fort+72Ch or 0). |
| 00743E90 | `land_fort_init_lua_and_weapons_00743e90` | __fastcall(ECX fort) RET | 0077E830; new 240h (00BF681B), 008363E0(fort+310h or 0, 0) into +74Ch (0 on failed allocation); 0095A880(0,1,0) when [[fort+C0h]+4] == 1. |
| 00743F90 | `land_fort_send_create_00743f90` | __thiscall(ECX fort, peer) RET 4 | Stack 00743DB0(word [[fort+538h]+70h], fort), 00779FC0, vtables restored, 007668E0. |
| 00744760 | `entity_handle_write_00744760` | __thiscall(ECX handle, cursor) RET 4 | Bool +4, then word +6 in 13 bits when set. |
| 007447C0 | `land_fort_create_is_type_007447c0` | RET 4, ECX ignored -> bool | t == D0h // 47h // 46h. |
| 007447F0 | `land_fort_create_write_007447f0` | __thiscall(ECX message, cursor) RET 4 | 00765000 base, then +114h bool and +116h 13 bits when set. |
| 00744830 | `land_fort_create_read_00744830` | __thiscall(ECX message, stream) RET 4 | 00767E30 base, then +114h bool and +116h 13 bits when set. |
| 00744870 | `land_fort_effect_construct_00744870` | __thiscall(ECX message, kind, value) RET 8 -> message | 0075B430(D1h), id/relay 0, +28h=0, delivery 1, vtable 00CFF37C (host profile), +1Ch kind, +20h value; +24h untouched. |
| 007448B0 | `land_fort_effect_construct_point_007448b0` | __thiscall(ECX message, kind, float) RET 8 -> message | 0075B430(D1h), id/relay 0, delivery 1, +28h=1, vtable 00CFF37C, +1Ch kind, +24h float; +20h untouched. |
| 00744920 | `land_fort_effect_write_00744920` | __thiscall(ECX message, cursor) RET 4 | 0075B480 header (host hook), bool +28h, kind 3 bits, then float raw-32 (+24h) or value 3 bits (+20h). |
| 00744990 | `land_fort_effect_read_00744990` | __thiscall(ECX message, stream) RET 4 | 0075B4C0 header, mirror of 00744920. |
| 00744A90 | `land_fort_apply_fire_target_order_00744a90` | __thiscall(ECX fort, message) RET 4 | Copies the target record (+24h..+38h) to a local, 0077A050(ECX message); 00E08EF8: 00835860(ECX [+74Ch], 00521EA0(ECX &record), 1) (the 1 is pre-pushed before 00521EA0, which takes no stack argument); 00E08F00/00E08F08: 00835860(0, 1). |
| 00744F70 | `linked_list_unlink_00744f70` | __thiscall(ECX list {count,head,tail}, node) RET 4 -> next node | Node {prev +0, next +4}; relink head/prev and next/tail, --count, free 00BF65AC; returns next or null. |
| 00745130 | `land_fort_set_effect_anchor_00745130` | __thiscall(ECX effect, const float[3]) RET 4 | Anchor to +100h..+108h (FLD/FSTP: sNaN quieting MINOR); parent +8Ch: 00B6DB70 unless parent+5Ch bit 1, 00413920(ECX effect+D0h, &local, parent+F0h), [+110h] vtable+34h(result); without a parent vtable+34h(effect+D0h). |
| 00745200 | `linked_list_clear_00745200` | __fastcall(ECX list) RET | While count: unlink the head (both directions), --count, free 00BF65AC. |
| 007453C0 | `save_reader_read_observer_ref_007453c0` | __thiscall(ECX reader, dword* out, a2, a3) RET 0Ch | reader vtable+10h(out, a2), then *out = the local 4 the image built ({4,&,&} record never passed). a3 unread. Ghidra's decompile of the stores is wrong; read from the listing. |
| 007455F0 | `land_fort_serialize_007455f0` | __thiscall(ECX fort, archive) RET 4 | 0095BB10; begin "_landFort"; with a master +738h: begin "gazdi", write "ID" {4, master}, section = index whose element ADDRESS equals +73Ch (-1 default; doubled 00BF6713 check kept), write "section" {1, idx}, end; begin "gameUnit", 008360C0(director +74Ch, archive), end, end. |
| 00745940 | `land_fort_construct_00745940` | __thiscall(ECX fort, row) RET 4 -> fort | 0095CC90(fort, row, 0); 00809270(ECX +72Ch); eight vtables (00CFF3F8, +10h 00CFF3E0, +24h 00CFF3D8, +170h 00CFF3D4, +1E4h 00CFF3CC, +310h 00CFF3B4, +38Ch 00CFF3B0, +72Ch 00CFF3AC); +740h/+744h/+738h/+73Ch = 0, +C4h = 1Bh, the inlined release of the (null) +740h, +750h = 0.0f. |
| 00745AA0 | `land_fort_destruct_00745aa0` | __fastcall(ECX fort) RET | Vtables; fire +740h: 00867B10 and +9 = 1; body +754h: 00C34F70(ECX [[00E188A8]+18h], body); release +744h, +740h; 00809650(+72Ch); 00959940. |
| 00745FA0 | `land_fort_create_damage_effect_00745fa0` | __thiscall(ECX fort, index, anchor) RET 8 | Identity 4x4 ([00D7A24C] diagonal); definition class+154h[index] checked against class+150h (00BF6713), AddRef'd by value; 008687C0(ECX &out, EDX +4A4h, def, matrix, 1, 0, 0); out released at once; effect+9 = 1; anchor != 0: 00745130(effect, [class+28h] + anchor*0Ch). The decompiler mis-assigned the arguments; read from the listing. |
| 007460E0 | `land_fort_create_state_effect_007460e0` | __thiscall(ECX fort, kind, float duration) RET 8 | kind 7: stop fire +740h (00867B10, +9=1, 00484620(0)); return if smoke +744h set or class+160h null; else 008689C0(parent +4A4h, class+160h, 00744C40(class+18h,0,1,0,0)+18h) -> 004845D0 into +744h. Other kinds: stop smoke, return if fire set, create class+144h[kind] (checked) into +740h. Then +748h = duration. No RNG. |
| 007469B0 | `release_string_storage_007469b0` | __fastcall(ECX object) RET | Release the string {len +8, data +0Ch} (00419CC0/00BD1510) when data set. |
| 00746A00 | `ref_member_set_release_00746a00` | __fastcall(ECX set) RET | Release the ten references +24h down to +0 (each zeroed). |
| 00747000 | `land_fort_allocate_00747000` | __thiscall(ECX class, row) RET 4 -> fort | new 758h (00BF55BE), memset before the null test, 00745940, 009553D0(ECX fort, class) (faults on null as the image does), +354h = class. |
| 007474E0 | `land_fort_create_entity_007474e0` | __fastcall(ECX message) -> unit (vtable +1Ch) | 00964790(ECX class id +1Eh, DL 1), class vtable+28h(id +1Ch), name +C0h copied into unit+154h (0041DD40 with flag 0, memcpy len bytes when data set), 00876EC0(ECX unit, 0). The codec model keeps +C0h as string+presence. |
| 00747560 | `register_shadow_factor_00747560` | __fastcall(ECX node, EDX const float*) RET, recursive | node vtable+0Ch(00B74310()); per geometry (00B74650/00B74640) per element (00B72B40/00B732C0): temp "cShadowFactor" (00CFF638), 00B18B20(ECX el+20h, &name, factor), release, 00B179F0(ECX el+20h, 0)+0Ch = *factor; children +34h / next +3Ch recurse. |
| 00747870 | `ref_array_tidy_00747870` | __fastcall(ECX array) RET | 00747870: identical to 00747870: 005471B0(ECX, 0), free(data) 00BF6989. |
| 00747C10 | `release_string_storage_00747c10` | __stdcall(object) RET 4 | Same body as 007469B0 with the object on the stack. |
| 00747CA0 | `ref_array_tidy_00747870` | __fastcall(ECX array) RET | 00747CA0: identical to 00747870: 005471B0(ECX, 0), free(data) 00BF6989. |
| 007480F0 | `ref_array_tidy_00747870` | __fastcall(ECX array) RET | 007480F0: identical to 00747870: 005471B0(ECX, 0), free(data) 00BF6989. |
| 00748B40 | `stationary_is_kind_of_00748b40` | __thiscall(ECX entity, kind) RET 4 -> bool | kind == 36h // 0 // entity+C4h. |
| 00748B70 | `stationary_prop_destruct_00748b70` | __fastcall(ECX prop) RET | Vtables 00CFF678/+10h 00CFF65C/+24h 00CFF654; 00C34F70(world, +1A8h) when set; release +184h; 00925780. |
| 00748C40 | `land_fort_allocate_stationary_prop_00748c40` | __fastcall(ECX class) -> prop | new 1ACh (00BF55BE), memset, 00748A40 when allocated, 0084F7F0(ECX prop, class) unconditionally. |
| 00748EC0 | `land_fort_class_destruct_00748ec0` | __fastcall(ECX class) RET | Vtables; release +160h; vectors +150h and +140h: 004CC760(ECX first, EDX last, vector, class) + free 00BF65AC when first set, zero three dwords; release +13Ch; 0095F1C0. |
| 00749030 | `land_fort_class_is_kind_of_00749030` | RET 4 -> bool | kind == 1Bh // 5 // 4. |
| 00749150 | `vehicle_class_create_instance_null_00749150` | XOR EAX,EAX; RET 4 | Returns 0. |
| 00749180 | `land_fort_class_construct_00749180` | __fastcall(ECX class) -> class | 00749050, vtables 00CFF790 / +6Ch 00CFF78C, zero +13Ch, +144h..+14Ch, +154h..+160h. |
| 00749790 | `create_motion_controller_00749790` | __fastcall(ECX unit) RET | new 390h (00BF681B), 00939F40(ECX memory, unit) into +1018h, 0 on failure. |
| 007497F0 | `landing_ship_on_destroyed_007497f0` | __fastcall(ECX ship), tail JMP | 006AC550(ECX +1200h) when set, JMP 00824B60. |
| 00749AA0 | `landing_ship_open_construct_00749aa0` | __fastcall(ECX message) -> message | 0075B430(A6h), +18h word 0, +1Ah 0, delivery 1, vtable 00CFF8D4. |
| 00749AD0 | `landing_ship_open_is_type_00749ad0` | RET 4 -> bool | t == A6h // 46h. |
| 00749BF0 | `landing_ship_create_construct_00749bf0` | __thiscall(ECX message, ship, a, b, c) RET 10h -> message | 00813A20(word [[ship+538h]+70h], ship, A4h), vtable 00CFF8E8, +12Ch id(a)/0, +12Eh id(b)/0, +130h c. |
| 00749C70 | `ship_create_is_type_00749c70` | RET 4 -> bool | t == 8Ah // 47h // 46h. |
| 00749CD0 | `landing_ship_create_write_00749cd0` | __thiscall(ECX message, cursor) RET 4 | 00813B10 base, word +12Ch 13 bits, word +12Eh 12 bits, signed +130h 4 bits (00429090). |
| 00749D90 | `land_at_pad_construct_00749d90` | __thiscall(ECX message, pad, building) RET 8 -> message | 0075B430(A5h), header 0, delivery 1, vtable 00CFF908, +1Ch id(pad)/0, +1Eh id(building)/0. |
| 0074A5A0 | `landing_ship_replicate_0074a5a0` | __thiscall(ECX ship, peer) RET 4 | Stack 00749BF0(ship, +1200h, +1204h, +1208h), 00779FC0(ECX ship, peer, &msg), 0080E380. |
| 0074A8D0 | `unit_is_rocket_armed_0074a8d0` | __fastcall(ECX unit) -> AL | Undefined head (cc13-splits1) read from the listing: class byte +809h -> true; lists +394h+i*0Ch for i in {2,3,4,6} with a non-zero first dword: count nodes and names starting "Rocket" (strncmp 6, 00CFA6B4; null -> 00E1AEC2); true when total > 7 and 2*rockets > total. Purpose name is a guess. |
| 0074BB00 | `landing_ship_construct_0074bb00` | __thiscall(ECX ship, row) RET 4 -> ship | 0081ED40, eight vtables (00CFFA30, 00CFFA18, 00CFFA10, 00CFFA0C, 00CFFA04, 00CFF9EC, 00CFF9E8, 00CFF9E4), zero +1198h..+11A0h and +1200h..+1208h, +C4h = 0Ch, +1210h = 0.0f. |
| 0074BD20 | `landing_ship_detach_0074bd20` | __fastcall(ECX ship) RET | 006DFFC0, then find the node (next +4, value +8) holding the ship in [ship+30h]+A8h and 004837D0(ECX list, node). |
| 0074BD60 | `landing_ship_destruct_0074bd60` | __fastcall(ECX ship), tail JMP | free +1198h (00BF65AC) when set, zero three dwords, JMP 0081F3A0. |
| 0074BE00 | `landing_ship_create_0074be00` | __thiscall(ECX class, row) RET 4 -> ship | new 122Ch (00BF55BE), memset, 0074BB00, 009553D0(class), +354h = class. |
| 0074BE90 | `record4c_resize_tail_0074be90` | __thiscall(ECX vector, n) RET 4 | 0074BA20(ECX vector, n, 4Ch record by value copied with REP MOVSD from the uninitialised local): the C++ passes an uninitialised local array. |
| 0074CCF0 | `land_vehicle_init_lua_and_devices_0074ccf0` | __fastcall(ECX vehicle) RET | 0077E830, then 0095A880(ECX vehicle, 0, 1, 0). |
| 0074CD10 | `land_vehicle_on_death_0074cd10` | __fastcall(ECX vehicle), tail JMP | 007422A0(ECX convoy +738h, vehicle) when +738h is set, then JMP 00951FB0. |
| 0074CDB0 | `land_vehicle_send_create_0074cdb0` | __thiscall(ECX vehicle, peer) RET 4 | Stack CDh message: 00766FC0(CDh, word [[v+538h]+70h], v, 1), vtable 00CFFD18, 00779FC0(ECX v, peer, &msg), vtable again, 007668E0. |
| 0074D030 | `land_vehicle_replicate_sync_0074d030` | __thiscall(ECX vehicle, const dword* peer_ref) RET 4 | party = 00927850(ECX vehicle, *peer ? [*peer+20h] : 8) (RET 4); 0077B280 (cdecl, 15 args, ADD ESP 3Ch): (vehicle, peer_ref, party, 0.0f, 1, 200.0f [00CE386C], 3, 500.0f [00CE397C], 5, 1000.0f [00CE3804], 0Ch, 2000.0f [00CFFD60], 14h, FLT_MAX [00D7A248], 100); the ints come from LEA EAX-k with EAX = 64h. |
| 0074D100 | `land_vehicle_send_sync_0074d100` | __thiscall(ECX vehicle, const dword* peer_ref, unused) RET 8 | Stack CEh message: 0075B430, +18h/+1Ah 0, delivery 0, vtable 00CFFD4C; 00779FC0(vehicle, *peer_ref, &msg); vtable 00CE4974; 00779F80(ECX vehicle, peer_ref). |
| 0074D230 | `land_vehicle_apply_sync_0074d230` | __thiscall(ECX vehicle, const message*) RET 4 | 004134F0(ECX v+674h, identity 4x4); 008746A0(ECX v+310h, [00F876B0] - [msg+0Ch]); [v+310h] vtable+4([00F876AC]). |
| 0074D2F0 | `points_push_back_0074d2f0` | __thiscall(ECX vector of 0Ch points, const float[3]) RET 4 | 0074D2F0 (0074D2F0 and 0074D450 are identical): growth through 0074D190 (2*cap, CMP 1/JG), copy three floats with FLD/FSTP (sNaN quieting MINOR) unless the slot pointer is null, ++size. |
| 0074D3B0 | `land_vehicle_serialize_0074d3b0` | __thiscall(ECX vehicle, archive) RET 4 | 0095BB10; begin "_landVehicle" (00CFFD80); write "convoyID" (00CFFD74) {4, +738h}; write "convoyPosition" (00CFFD64) {1, +73Ch}; end. |
| 0074D450 | `points_push_back_0074d2f0` | __thiscall(ECX vector of 0Ch points, const float[3]) RET 4 | 0074D450 (0074D2F0 and 0074D450 are identical): growth through 0074D190 (2*cap, CMP 1/JG), copy three floats with FLD/FSTP (sNaN quieting MINOR) unless the slot pointer is null, ++size. |
| 0074D710 | `ground_vehicle_create_entity_0074d710` | __fastcall(ECX message) -> unit | 00964790(ECX class id +1Eh, DL 1), vtable+28h(id +1Ch), name +C0h into unit+154h (0041DD40 flag 0, memcpy len). No 00876EC0 (unlike 007474E0). |
| 0074DA10 | `land_vehicle_class_bind_model_0074da10` | __fastcall(ECX class) RET | 0095F500; for i = 1..: temp std::string "kemeny" (00CFFD90, 00408720 length 6), node = 00718870(ECX [class+50h], &str, i), string destroyed (free 00BF65AC when cap >= 10h, reset to SSO); stop on null; first vertex of node+48h (12-byte elements; 00BF6713 when empty) appended to class+140h (0074D190 growth). |
| 0074DB70 | `land_vehicle_on_destroyed_0074db70` | __fastcall(ECX vehicle) RET | +70h == 1: 00414DB0 unless +C8h (the two zeros pushed first belong to 008685E0); 008685E0(ECX &out, EDX [[00E188A8]+19ECh], class+138h AddRef'd by value, &v+FCh, 0, 0); out released; +9 = 1; 0047BB70(ECX [[00E188A8]+21D0h], class+13Ch, v) when set. Then 00959450; per effect in +72Ch (count +730h, end fixed first): 00867B10, +9=1, release, zero; 004C9550(ECX +72Ch, 0); 00926D90(ECX v, +70h). |
| 0074DCC0 | `land_vehicle_construct_0074dcc0` | __thiscall(ECX vehicle, row) RET 4 -> vehicle | 0095CC90(row, 0); vtables 00CFFDE0, +10h 00CFFDC8, +24h 00CFFDC0, +170h 00CFFDBC, +1E4h 00CFFDB4, +310h 00CFFD9C, +38Ch 00CFFD98; +72Ch..+734h = 0; +C4h = 19h. |
| 0074DD90 | `land_vehicle_is_kind_of_0074dd90` | __thiscall(ECX unit, kind) RET 4 -> bool | kind in {19h, 5, 4, 2, 1, 0} or kind == unit+C4h. |
| 0074DDD0 | `land_vehicle_vslot_b4_0074ddd0` | __fastcall(ECX vehicle) | 008DDF00(ECX [[00E188A8] + 21A4h + [[00E188A8]+18ECh]*4], vehicle); EAX passes through. |
| 0074DE50 | `land_vehicle_detach_0074de50` | __fastcall(ECX vehicle) RET | 006D3620; erase the vehicle's node from [v+30h]+54h, then from [v+30h]+144h (004837D0). |
| 0074DEB0 | `land_vehicle_destruct_0074deb0` | __fastcall(ECX vehicle) RET | 004C9550(ECX +72Ch, 0), free(+72Ch) 00BF6989, 00959940. |
| 0074DF10 | `land_vehicle_allocate_0074df10` | __thiscall(ECX class, row) RET 4 -> vehicle | new 740h (00BF55BE), memset, 0074DCC0, 009553D0(class), +354h = class. |
| 0074DFC0 | `land_vehicle_load_saved_state_0074dfc0` | __fastcall(ECX vehicle) RET | 0095E5B0; when [+C0h]+4 == 3: 009238A0(ECX v, &rec14), 004425C0(ECX reader, rec14 by value), 00BD8E20 {0,"_landVehicle"}, reader vtable+10h({0,"convoyID"}, {4,&local}) -> +738h, 00BD6830({0,"convoyPosition"}, {1,&+73Ch}), 00BD7A20, 00441A20; then vtable+148h(1FFh, 8). The image's local is uninitialised before the read; the C++ zeroes it (MINOR). The reader object is passed as a 12-byte stack buffer (size inferred from the frame). |
| 0074E0F0 | `recon_plane_construct_0074e0f0` | __thiscall(ECX plane, row) RET 4 -> plane | 007CFD20(row, 0); vtables 00D00070, 00D00058, 00D00050, 00D0004C, 00D00044, 00D0002C, 00D00028, +72Ch 00CFFFE8; +C4h = 14h; +AACh = 0Bh. |
| 0074E160 | `plane_unit_vslot1c0_0074e160` | __fastcall(ECX unit) -> EAX | MOV EAX,[ECX+808h]; RET. |
| 0074E1A0 | `plane_unit_vslot10c_0074e1a0` | __fastcall(ECX plane) -> bool | vtable+38h() (x87) > 3.0f [00CE3854]: FLD const, FXCH, FCOMIP ST0,ST1 (DF F1), JBE -> 0 (NaN 0). The hook returns the x87 value as a double, compared unrounded. |
| 0074E1E0 | `plane_set_forward_speed_0074e1e0` | __thiscall(ECX plane, float) RET 4 | 007D9E80(ECX plane+AB0h, speed) (FLD/FSTP of the float). game_hosts_lua.cpp ~5982 cites it in the squadron speed path. |
| 0074E210 | `plane_control_mode_is_free_flight_0074e210` | __fastcall(ECX control) -> bool | [ECX+1D4h] == 7 (SETZ). game_hosts_units.cpp reads the same field as unit+900h. |
| 0074E2D0 | `large_recon_plane_construct_0074e2d0` | __thiscall(ECX plane, row) RET 4 -> plane | 0074E0F0(row); vtables 00D00308, 00D002F0, 00D002E8, 00D002E4, 00D002DC, 00D002C4, 00D002C0, +72Ch 00D00280; +C4h = 16h; +AACh = 0Bh. |
| 0074E440 | `plane_unit_state_admits_ai_0074e440` | __thiscall(ECX plane, unused) RET 4 -> bool | false for +900h in {7, 6, 4, 5, 2, 3}, else true. |
| 0074E540 | `large_recon_plane_create_0074e540` | __thiscall(ECX class, row) RET 4 -> plane | new E94h, memset, 0074E2D0, 009553D0(class), +354h = class. |
| 0074E5D0 | `plane_unit_register_in_world_lists_0074e5d0` | __fastcall(ECX plane) RET | 00928560; 00484540(ECX [p+30h]+30h / +48h / +54h / +CCh, plane). |
| 0074E610 | `plane_unit_detach_0074e610` | __fastcall(ECX plane) RET | 006D3620; erase from [p+30h]+54h, then +CCh (004837D0). |
| 0074E6C0 | `recon_plane_detach_0074e6c0` | __fastcall(ECX plane) RET | 0074E610, then erase from [p+30h]+108h. |
| 0074E750 | `large_recon_plane_detach_0074e750` | __fastcall(ECX plane) RET | 0074E610, then erase from [p+30h]+120h. |
| 0074E7F0 | `unit_subobject_10d4_destruct_0074e7f0` | __fastcall(ECX object) RET | vtable 00D00514; free (00BF6989) +1Ch, +18h, +20h when set, in that order, without clearing. |
| 0074EAB0 | `ship_sink_construct_0074eab0` | __thiscall(ECX message, count, const float*) RET 8 -> message | 0075B430(92h), header 0, delivery 1, vtable 00D00518, +1Ch = count, values copied (FLD/FSTP) to +20h.. while i < count (unsigned). |
| 0074EB00 | `ship_sink_is_type_0074eb00` | RET 4 -> bool | t == 92h // 46h. |
| 0074FAB0 | `trigger_list_add_0074fab0` | __fastcall(ECX list, EDX trigger) | trigger->vtable[4](list). |
| 0074FB30 | `mission_lua_variant_init_tag6_0074fb30` | __fastcall(ECX variant) RET | 00886920, then +4 = 6. |
| 0074FBB0 | `trigger_list_invoke_by_name_0074fbb0` | __fastcall(ECX list, EDX name) -> bool | Walk [list+1Ch] / next +0Ch: vtable+8(1), flag +24h, 00435C40(ECX t+1Ch, name); fire vtable+10h; result = any fired. |
| 0074FC10 | `native_string_release_0074fc10` | __fastcall(ECX string) RET | Return {len, data} storage (00419CC0/00BD1510) when data is set. |
| 0074FC40 | `lua_trigger_destruct_0074fc40` | __fastcall(ECX trigger) RET | vtable 00D00554; release +28h/+2Ch then +1Ch/+20h storage; 00875B30. |
| 0074FCD0 | `stub_arg_is_one_0074fcd0` | RET 4 -> bool | argument == 1 (SETZ). |
| 0074FD40 | `flagged_string_copy_construct_0074fd40` | __thiscall(ECX out, const source*) RET 4 -> out | Zero {len, data}; when out != source: 0041DD40(out, src.len, 1), memcpy(out.data, src.data, out.len) for a non-empty source; byte +8 copied. |
| 0074FDD0 | `lua_trigger_fire_callback_0074fdd0` | __fastcall(ECX trigger) RET | args vector {0,0,0}; 00884A90(&v1, [[t+4]+28h]); 006EDF00 push; 00884AF0; 008849B0(&v2); flag +24h: 00886A10(&v2, t+1Ch) else 00886920 + tag 6; push; 00887E50(ECX [[00E188A8]+1A08h], 0, t+28h, &args, 0, -1); vtable+10h(); 00884AF0(&v2); 006B88E0(&args, 0); free(begin) 00BF6989. Variants are 14h-byte opaque buffers. |
| 0074FFE0 | `proximity_trigger_on_target_gone_0074ffe0` | __thiscall(ECX observer, entity) RET 4 | entity == 00521EA0(ECX observer+14h): (observer-30h) vtable+10h(). |
| 00750090 | `observer_trigger_destruct_00750090` | __fastcall(ECX trigger) RET | 00750090 (00750090 and 00750430 are identical): 00695870(ECX trigger+30h, or 0 for a null trigger), then 0074FC40. |
| 00750190 | `line_of_sight_trigger_on_target_gone_00750190` | __thiscall(ECX observer, entity) RET 4 | entity == [observer+14h]: (observer-30h) vtable+10h(). |
| 007501F0 | `entity_as_kind18_007501f0` | __fastcall(ECX entity) -> entity or 0 | Null -> 0; entity->vtable[5Ch](18h) true -> entity, else 0. |
| 00750430 | `observer_trigger_destruct_00750090` | __fastcall(ECX trigger) RET | 00750430 (00750090 and 00750430 are identical): 00695870(ECX trigger+30h, or 0 for a null trigger), then 0074FC40. |
| 007504C0 | `squad_ordnance_trigger_attach_007504c0` | __thiscall(ECX trigger, owner) RET 4 | 00876020(ECX owner, trigger); trigger+30h = owner+28h. |
| 00750550 | `line_of_sight_trigger_create_00750550` | __thiscall(ECX target, float range) RET 4 -> trigger | new 50h (00BF681B); 007500E0(ECX mem, target, range) or 0. |
| 007505C0 | `proximity_trigger_create_007505c0` | __thiscall(ECX target record, float range) RET 4 -> trigger | new 60h; 0074FEE0(ECX mem, 18h record by value (words +0/+2, dword +4, four floats via FLD/FSTP), range) or 0. |
| 00750660 | `squad_ordnance_trigger_create_00750660` | __cdecl() -> trigger | new 34h; inline base: +4/+8/+0Ch 0, +10h 0, +18h 2, +11h 1, +1Ch/+20h 0, +24h 0, +28h/+2Ch 0, +14h 1, vtable 00D005C0, +30h 0; null -> 0. |
| 007506B0 | `lua_trigger_construct_007506b0` | __thiscall(ECX trigger, const name*, const callback*) RET 8 -> trigger | Base fields (+11h 1, +18h 2), vtable 00D00554, +1Ch = copy of name (and flag +24h), +28h = copy of callback (no flag), +14h = 1. |
| 00750790 | `lua_trigger_set_callback_name_00750790` | __thiscall(ECX trigger, const name*, const callback*) RET 8 | Assign +1Ch from name when not the same object (0041DD40 + memcpy), flag +24h always copied; assign +28h from callback. |
| 00751850 | `telemetry_stream_write_00751850` | __thiscall(ECX stream, value) RET 4 | 00A4D610(value, stream+4) (cdecl, ADD ESP 8). |
| 00752BF0 | `metrics_session_is_type_00752bf0` | RET 4 -> bool | t == E9h (XOR/CMP/SETZ). |
| 00752C30 | `metrics_session_read_00752c30` | __thiscall(ECX message, stream) RET 4 | 00428C70(stream+4, &+10h, 8) via read_native_u8_bits_00428c70; 00429F20(stream+4, message+18h) (host hook: the owned-string reader needs a pool context). |
| 00753530 | `telemetry_list_tidy_00753530` | __fastcall(ECX object) RET (owner argument = the saved ECX) | 00753530: vector object+10h: 00752900(ECX first +14h, EDX last +18h, vector, object) + free 00BF65AC when first set; zero +14h..+1Ch. |
| 007547A0 | `telemetry_list_tidy_00753530` | __fastcall(ECX object) RET (owner argument = the saved ECX) | 007547A0: vector object+10h: 00752900(ECX first +14h, EDX last +18h, vector, object) + free 00BF65AC when first set; zero +14h..+1Ch. |
| 00754A40 | `telemetry_list_tidy_00753530` | __stdcall(object) RET 4 | 00754A40: vector object+10h: 00752900(ECX first +14h, EDX last +18h, vector, object) + free 00BF65AC when first set; zero +14h..+1Ch. |
| 00758090 | `air_ops_holder_step_00758090` | __thiscall(ECX holder, float dt) RET 4 | x87 from the bytes: heading = [[holder+4]+7Ch] vtable+50h rounded to float -> +88h; acc = float(+94h + dt) -> +94h; acc > [00CE3D10] (0.2f widened) else return; r = 00438B10(heading, +90h) (stdcall); DC C9 (ST1 *= ST0: r*0.5 [00D7A280]), +94h = 0, DE FA (ST2 /= ST0: /acc), FMUL +8Ch by 0.5, DE C1 add, FSTP +8Ch; +90h = +88h. The C++ carries x87 values as doubles (MINOR: extended vs double rounding). |
| 00758150 | `mothership_destruct_00758150` | __fastcall(ECX ship) RET | Eight vtables (00D01630 .. +72Ch 00D015E0), block +1188h vtable 00D015C4, 006CA410(ECX block), 0081F3A0. |
| 00758210 | `mothership_init_pass_c_00758210` | __fastcall(ECX ship) RET | 0081F980, then dword +11A8h = 1. game_hosts_lua.cpp ~4154 lists it in its pass table. |
| 00758240 | `mothership_update_00758240` | __thiscall(ECX ship, float dt) RET 4 | 008255B0(ECX ship, dt), 006BA670(ECX ship+1188h, dt). |
| 00758270 | `mothership_update_motion_00758270` | __thiscall(ECX ship, float dt) RET 4 | 00825F20(dt), 006CDC70(ECX ship+E78h, dt), 00758090(ECX [ship+EF8h], dt) when set. Hosted as a motion-coverage row in game_hosts_units.cpp (no per-call body). |
| 007582B0 | `mothership_may_follow_007582b0` | __thiscall(ECX ship, a, b) RET 8 -> bool | 006BA6A0(ECX block, a, b) true -> true, else 008162B0(ECX ship, a, b) != 0. |
| 007582F0 | `mothership_supports_command_007582f0` | __thiscall(ECX ship, const char*) RET 4 -> bool | 00438E10(ECX cmd, EDX "attackmove" 00CECCA8) == 0 -> false; == "launchairstrike" (00CF85E0) -> true; else 00812B00(ECX ship, cmd) != 0. |
| 00758340 | `mothership_get_property_00758340` | __thiscall(ECX ship, lua, key) RET 8 | 00815870(ECX ship, lua, key), then 006C6630(ECX ship+1188h, lua, key). |
| 00758370 | `mothership_reset_condition_00758370` | __fastcall(ECX ship) RET | 0080E410, then bytes +11A5h and +11A4h = 0. |
| 007584B0 | `air_ops_block_has_queue_records_007584b0` | __fastcall(ECX block) -> bool | Section [block+8]: Enter ([00CE2218]) and ++[s+18h] when set; count of the 20-byte records +A8h..+ACh (magic 66666667h, SAR 3) != 0; --depth and Leave ([00CE2210]). |
| 00758550 | `mothership_construct_00758550` | __thiscall(ECX ship, row) RET 4 -> ship | 0081ED40(row); 006CAC00(ECX ship+1188h, ship); block vtable 00D015C4; eight vtables; +C4h = 9; +11A8h = 1. |
| 00758630 | `mothership_vslot228_00758630` | __fastcall(ECX ship) -> bool | /+984h/ > [00D7A270] (0.05f widened) and /0092D730(ECX [ship+1018h])/ > 1.0f and (007584B0(block) or dword +11C0h != 0); /x/ = x > 0 ? x : [00D7A208] (-0.0f) - x. |
| 007586D0 | `mothership_step_elevator_frame_007586d0` | __thiscall(ECX ship, float dt) RET 4 | Copy the 4x4 at +1288h; row3 -= dt*row1 (DE CA FMULP ST2 and D8 C9 FMUL ST0,ST1 read from the bytes; products stored as floats); [+1284h] vtable+38h(&copy). cc13_seg30.cpp calls it as a hook. |
| 00758B90 | `elevator_move_construct_00758b90` | __thiscall(ECX message, entity, byte flag) RET 8 -> message | 0075B430(87h), header 0, delivery 1, +1Ch 0, vtable 00D01870, +23h = entity != 0, +22h = flag, +20h = id or 0. |
| 00758BF0 | `elevator_move_is_type_00758bf0` | RET 4 -> bool | t == 87h // 49h // 46h. |
| 00758C20 | `elevator_move_write_00758c20` | __thiscall(ECX message, cursor) RET 4 | 0075B480 header; bools +1Ch, +22h, +23h; word +20h in 12 bits when +23h. |
| 00758C70 | `elevator_move_read_00758c70` | __thiscall(ECX message, stream) RET 4 | 0075B4C0 header; bools into +1Ch, +22h, +23h (stream+4); +23h: word 12 bits into +20h, else +20h = 0. |
| 00758CF0 | `elevator_move_entity_00758cf0` | __fastcall(ECX message) -> entity | +23h == 0 -> 0; else the two-bank id table (00F89A10/0C/54/60/A8) entity of +20h. |
| 00758D30 | `mothership_create_00758d30` | __thiscall(ECX class, row) RET 4 -> ship | new 12C8h, memset, 00758550, 009553D0(class), +354h = class (same body as 0074E540 with the size and constructor changed). |
| 00758DC0 | `mothership_serialize_00758dc0` | __thiscall(ECX ship, archive) RET 4 | +11A8h = 1 first; 00818340; begin "_airBase" (00CF88BC), 006C19D0(ECX block, archive), end; begin "_motherShip" (00D01884), end. |
| 00758E40 | `mothership_bind_elevator_node_00758e40` | __fastcall(ECX ship) RET | +1284h = 0071AD50(ECX [ship+360h] ? [[ship+360h]+160h] : 0, "elevator_2" 00D01890); 004134F0(ECX +1288h, 00B6DB60(ECX node)). |
| 00758E80 | `mothership_controller_refresh_00758e80` | __thiscall(ECX ship, float dt) RET 4 | 00811AB0(dt), then 006BEE40(ECX [ship+EF8h], dt) when set. game_hosts_units.cpp models the holder re-frame it causes. |
| 00758EB0 | `mothership_handle_message_00758eb0` | __thiscall(ECX ship, message) RET 4 -> bool | IsType (vtable+0Ch) 72h -> vtable+220h, 73h -> +224h, 74h -> +220h, 75h -> +224h (return 1); 87h -> [ship+11C4h] vtable+50h(byte +22h, 00758CF0(msg), CVTSI2SS([00F876B0] - [msg+0Ch])) (return 1); else 006CD6C0(ECX block, msg) true -> 1, else 00821E80(ECX ship, msg). |

## Existing code, recorded ledger-only

| Address | Function | Original ABI | Evidence and notes |
|---|---|---|---|
| 007372A0 | `frame_hook_construct` | __fastcall(ECX hook) -> hook | cc13-seg41 verified: frame_hook_construct is 00737830 (calls 007372A0: vtable 00CFEA68, lock, publish 00F88C20, 00BD0C30, unlock; then 00CFEA98, +4 byte 0, +8 = 0). 007372A0 is covered there. |
| 00737830 | `frame_hook_construct` | __fastcall(ECX hook) -> hook | cc13-seg41 verified: frame_hook_construct is 00737830 (calls 007372A0: vtable 00CFEA68, lock, publish 00F88C20, 00BD0C30, unlock; then 00CFEA98, +4 byte 0, +8 = 0). 00737830 is covered there. |
| 007379A0 | `retire_native_application_007379a0` | __fastcall(ECX application) RET | cc13-seg41 verified: 00735F30(+8 rows, 0), free 00BF6989(data), 00BEA990 match the existing body. |
| 0073FA70 | `publish_startup_singleton_00af06a0` | __fastcall(ECX system) -> system | cc13-seg41: 0073FA70 is the singleton register template (vtable 00CFF218, global 00E1AEA0); the hosted GameDecalTable::run (game_hosts_init_tail.cpp) models it through publish_startup_singleton_00af06a0 with kDecalSystemBase_0073fa70. Matches the listing. |
| 00746320 | `GameShipAiHost::Impl (landfort fire roll)` | __fastcall(ECX fort) RET | cc13-seg41 verify-only (cc9 host): the extend path (00BD2F10 stream 0, keep the max) matches. Mismatches noted for adoption: the image first stops/releases the smoke +744h (00867B10, +9=1) which the host skips; the effect index uses 00BD2FC0(ECX 0) u32 % count of class+144h where the host draws 00BD2F10(0, 0, 1) - same stream, equal advance only if 00BD2F10 consumes one 00BD2FC0 step. |
| 00749210 | `kVehicleClassStructureFieldSchema` | __thiscall(ECX class, lua) (schema) | cc13-seg41 checked the stored offsets against 00749210: ExplosionEfx +13Ch, SmokeFireEfx +160h, SmokeFireChanceMul +164h, DurationMin +168h, DurationMax +16Ch, CapturePost +174h, SecondaryExplosionChanceMul +170h, FakedType +178h, MapIcon +17Ch, Fortress +175h, DamageModelChangeDelay +138h. Schema model, not a per-call reconstruction. |
| 0074D4A0 | `kVehicleClassStructureFieldSchema` | __thiscall(ECX class, lua) (schema) | cc13-seg41 checked the stored offsets against 0074D4A0: ExplosionEfx +138h, Wreck +13Ch (0 when absent), Smoke +14Ch. Schema model, not a per-call reconstruction. |
| 0074DD50 | `kSensorCategoryLandVehicle_0074dd50` | MOV EAX,1; RET | cc13-seg41 verified: the body is MOV EAX,1 / RET; sensor_tables.hpp models it as SensorCategory::surface (1). |
| 0074E190 | `kSensorCategoryPlane_0074e190` | XOR EAX,EAX; RET | cc13-seg41 verified: XOR EAX,EAX / RET; sensor_tables.hpp models it as SensorCategory::air (0). |
| 0074E2B0 | `recon_slot_lists category switch` | MOV EAX,0Bh; RET | cc13-seg41 verified: 0074E2B0 is MOV EAX,0Bh / RET; recon_slot_lists.cpp returns 0Bh for this class. |
| 0074E340 | `recon_slot_lists category switch` | MOV EAX,0Bh; RET | cc13-seg41 verified: 0074E340 is MOV EAX,0Bh / RET; recon_slot_lists.cpp returns 0Bh for this class. |
| 007581F0 | `recon_slot_lists category switch` | XOR EAX,EAX; RET | cc13-seg41 verified: 007581F0 is XOR EAX,EAX / RET; recon_slot_lists.cpp returns 0 for MMothership. |
| 007583A0 | `mothership_apply_failure_message` | __thiscall(ECX ship, message) RET 4 | cc13-seg41 verify-only: 72h -> byte +11A4h = 1, 74h -> +11A5h = 1, else 008198A0 (IsType vtable+0Ch); the physics lane's mothership_apply_failure_message writes deck block+1Ch/+1Dh (= unit+11A4h/+11A5h, block at +1188h). Matches; the application at the call (not the pump) is the host's labelled substitution. |
| 007583F0 | `mothership_apply_failure_message` | __thiscall(ECX ship, message) RET 4 | cc13-seg41 verify-only: 73h -> +11A4h = 0, 75h -> +11A5h = 0, else 008137B0 (IsType vtable+0Ch); the physics lane's mothership_apply_failure_message writes deck block+1Ch/+1Dh (= unit+11A4h/+11A5h, block at +1188h). Matches; the application at the call (not the pump) is the host's labelled substitution. |
| 00758750 | `mothership_failure_arm` | __thiscall(ECX ship, const key*, duration) RET 8 -> AL | cc13-seg41 verify-only: "RunwayFailure" via _stricmp on key+4 (null skipped) -> 72h message (vtable 00CF8B40, float +20h = [00CF8B3C]) routed by 0077C2A0(msg, 7, 0); "HangarFailure" via 00425850 -> 74h (vtable 00CF8B68); else 008132C0(key, duration). The duration is not read on the two arms. Matches the header's description. |
| 00758890 | `mothership_failure_arm` | __thiscall(ECX ship, const key*, duration) RET 8 -> AL | cc13-seg41 verify-only: "RunwayFailure" via _stricmp on key+4 (null skipped) -> 72h message (vtable 00CF8B40, float +20h = [00CF8B3C]) routed by 0077C2A0(msg, 7, 0); "HangarFailure" via 00425850 -> 74h (vtable 00CF8B68); else 00827B90(key, duration). The duration is not read on the two arms. Matches the header's description. |

## Adoption notes for hosted code

Each note: address, instruction addresses, what the binary does, what the
host does, whether it is live in a normal mission, and the shared RNG
(00BD2F10 / 00BD2FC0).

1. **00742A70 member names are Roman numerals.** 00742AAB rejects n outside
   1..3999 ("N/A", 00CECE08); otherwise 00742AD0..00742B64 concatenate the
   digit strings of the table 00E0CC68 (M/MM/MMM, C..CM, X..XC, I..IX).
   `src/game_hosts_units.cpp:11918` names convoy members
   `convoy.name + "-" + std::to_string(slot + 1)` and
   `src/plane_squadron.cpp:62` formats the multi-wing plane suffix with
   `"%s%d"`. The image's two callers are 00743904 (LandConvoy roster) and
   007F4A13 (squadron planes); both comments place slot+1 / index+1 there, so
   the image names read "...-I", "...-II", ... Live in any mission with a land
   convoy or a multi-plane squadron (names are visible to Lua lookups). No RNG.
2. **00746320 LandFort StartFire** (`src/game_hosts_ship_ai.cpp` ~12706).
   (a) 00746338..00746384: the image first stops and releases an active
   smoke effect +744h (00867B10, +9 = 1, release); the host skips this on the
   premise that +744h is never set on its path. (b) 007463C1: the effect index
   is 00BD2FC0(ECX = 0) % count(class+144h), a u32 draw on stream 0; the host
   draws 00BD2F10(0, 0.0, 1.0) at that point. Same stream; the stream advance
   is equal only if one 00BD2F10 draw consumes exactly one 00BD2FC0 step.
   Live for capture buildings that roll a fire; touches the shared RNG.
   The extend path (0074657E..007465C5, keep the larger duration) matches.
3. **007416F0 LandConvoy GetProperty** pushes Speed, Offset and
   DistanceTraveled as a boolean (00741827: UCOMISS against 0.0f, push
   through 00B66450). Not hosted; noted so a future host does not "fix" it.

## Not reconstructed (skipped, with reasons)

- 0073EBA0 (mesh-decal triangle clip to box, 420 instructions), 0073F1B0
  (triangle/sphere contact, 399) and 0073F6E0 (collect tree triangles, 238):
  long x87 geometry bodies; not attempted within this packet's context budget.
  Every DC-form operand would have to be read from the bytes.
- 007482B0 (LandFort scene init + static body, 438 instructions): hosted by
  the gameplay lane (`game_hosts_units.cpp` ~6815, `hull_terrain_contact.cpp`,
  `mmod_hull_convex_box.cpp`, packet cc9_hull_fort_contact); not re-verified
  here, so no ledger record was added.
- 00742C10 (LandConvoy cache parent and placement law, 308 instructions):
  only its 00742C70 decision is modelled (`land_and_structures.hpp`
  `land_convoy_uses_world_pose_law_00742c70`); the full body was not
  attempted, so no record was added.
- 00748CC0 (LandFort load from the saved Lua table): needs the Lua-reader
  stack objects and the section index -> element conversion (0043F8E0); not
  attempted.
- 0074D800 (LandVehicle InitAll: gunnery AI, smoke effects per class anchor):
  not attempted.
