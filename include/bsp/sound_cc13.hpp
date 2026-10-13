#pragma once

// Segment 74 (00A7A440-00A9AC40: sound, streaming, FMOD wrappers, shadow targets and input) leaves that
// had a ledger name but no reconstruction record (cc13-sound1, docs/CC13_SOUND_1.md). They operate on the
// ACTUAL native storage, as the sibling files of this segment do (src/sound_sample.cpp and
// src/sound_file_callbacks.cpp): pointer-array headers, intrusive reference counts dispatched through the
// object's own vtable, native red-black tree nodes, and singleton bases.
//
// Allocation model: the image's operator new (00BF55BE, 00BF681B) and free (00BF6989, 00BF65AC, 00BF9DC8)
// are the project's singleton_lifetime_allocate/free boundary (bsp/singleton_lifetime.hpp), as in
// src/sound_sample.cpp. Storage passed in must belong to that domain. Original C++ EH and SEH are not
// claimed; nothing here is wired into a game host. Names are hypotheses, not recovered symbols.

#include <array>
#include <cstdint>

#include "bsp/native_input_action_records.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_input_binding_storage.hpp"

namespace bsp::sound_cc13 {

// The native {data, count, capacity} header of a pointer array (4-byte slots). Win32 layout.
struct NativePointerArray {
    std::uint32_t* data;
    std::int32_t count;
    std::int32_t capacity;
};

// 00A7A850 BSP_SoundGroupEffectPointers_Reserve and 00A7A8B0 BSP_SoundSystemDspPointers_Reserve,
// __thiscall(ECX array, int requested), RET 4. requested is raised to 8 (A850) or 16 (A8B0); when the
// capacity is below it, operator new(requested*4), copy count slots, free the old data, store data and
// capacity.
void reserve_sound_group_effect_pointers_00a7a850(NativePointerArray& array, std::int32_t requested);
void reserve_sound_system_dsp_pointers_00a7a8b0(NativePointerArray& array, std::int32_t requested);

// 00A7A930 BSP_SoundGroupEffectPointers_Resize, __thiscall(ECX array, int count), RET 4: reserve through
// 00A7A850 when count exceeds the capacity, zero the new slots, then set count (the native shrink is a
// decrement loop to the same value).
void resize_sound_group_effect_pointers_00a7a930(NativePointerArray& array, std::int32_t count);

// Appends, __thiscall(ECX array, const DWORD* value), RET 4. When full, the capacity grows to:
// 00A7B2D0 BSP_SoundGroupEffectPointers_Append  capacity + 4, at least 8, through 00A7A850;
// 00A7B310 BSP_SoundSystemDspPointers_Append    capacity + 16, at least 16, through 00A7A8B0;
// 00A7B390 BSP_SoundSamplePointers_Append       capacity * 2, at least 1, through 00A7A9E0
//                                                (reserve_sound_sample_pointers_00a7a9e0).
void append_sound_group_effect_pointer_00a7b2d0(NativePointerArray& array, const std::uint32_t& value);
void append_sound_system_dsp_pointer_00a7b310(NativePointerArray& array, const std::uint32_t& value);
void append_sound_sample_pointer_00a7b390(NativePointerArray& array, const std::uint32_t& value);

// Destroys, __fastcall(ECX array): resize to 0, then free(data).
// 00A7B7A0 BSP_SoundGroupEffectPointers_Destroy uses 00A7A930; 00A7B830 BSP_SoundSystemDsps_Destroy uses
// 00A7A980 (the same resize body with 00A7A8B0); 00A7BA80, 00A7BB20 and 00A7BF20
// BSP_SoundSamplePointers_Destroy are three copies over 00A7B3D0 (resize_sound_sample_pointers_00a7b3d0).
void destroy_sound_group_effect_pointers_00a7b7a0(NativePointerArray& array);
void destroy_sound_system_dsps_00a7b830(NativePointerArray& array);
void destroy_sound_sample_pointers_00a7ba80(NativePointerArray& array);
void destroy_sound_sample_pointers_00a7bb20(NativePointerArray& array);
void destroy_sound_sample_pointers_00a7bf20(NativePointerArray& array);

// Assigns, __thiscall(ECX this, const array* source), RET 4, EAX = this: resize this to 0, reserve
// source.count, then append each source slot with the append's growth rule. The source slot address is
// formed before the growth check, as in the listing.
// 00A7B7C0 BSP_SoundGroupEffectPointers_Assign (00A7A930, 00A7A850, +4 / at least 8);
// 00A7BAA0 BSP_SoundSamplePointers_Assign (00A7B3D0, 00A7A9E0, *2 / at least 1).
NativePointerArray& assign_sound_group_effect_pointers_00a7b7c0(NativePointerArray& self, const NativePointerArray& source);
NativePointerArray& assign_sound_sample_pointers_00a7baa0(NativePointerArray& self, const NativePointerArray& source);

// Intrusive reference slots: the slot holds an object whose reference count is the DWORD at +4h. Release
// is InterlockedDecrement (IAT 00CE2220) and, at zero, the object's vtable slot 0 with ECX = object (the
// BSP_RefCounted deleting path). The slot is cleared whenever it was non-null.
// ResetReturnSelf, __fastcall(ECX slot) -> EAX slot: 00A7B4B0, 00A7B4E0, 00A82880, 00A8DE20, 00A8DE50,
// 00A90AF0 and 00A929F0 are byte-identical.
void** reset_ref_slot_return_self(void** slot);
// Release, __fastcall(ECX slot), no return: 00A7B960, 00A7B990, 00A7BB40, 00A7BB70, 00A92B40 and 00A92FA0
// are byte-identical.
void release_ref_slot(void** slot);
// 00A92B00 TMPL_RefPtrAssign_00a92b00, __thiscall(ECX slot, object*), RET 4, EAX = slot: releases the old
// object, then stores the new pointer WITHOUT adding a reference.
void** assign_ref_slot_no_addref_00a92b00(void** slot, void* object);
// 00A92C00 TMPL_RefPtrReset_00a92c00, __fastcall(ECX owner): the same release on the slot at owner+2Ch,
// which is then cleared on every path.
void reset_ref_slot_at_2c_00a92c00(void* owner);

// Owned pointers deleted through a virtual deleting destructor with flag 1, then cleared:
// 00A81C20 and 00A81CF0 (byte-identical) call vtable slot +4h; 00A87CB0 calls slot 0.
void delete_owned_slot4_00a81c20(void** slot);
void delete_owned_slot4_00a81cf0(void** slot);
void delete_owned_slot0_00a87cb0(void** slot);

// Native red-black tree node of the sound trees (MSVC 2005 _Tree layout read from the listings): left +0h,
// parent +4h, right +8h, value +0Ch..+17h, colour byte +18h (1 = black), isnil byte +19h. Size 1Ch.
struct NativeSoundTreeNode {
    NativeSoundTreeNode* left;
    NativeSoundTreeNode* parent;
    NativeSoundTreeNode* right;
    std::uint8_t value[0x0C];
    std::uint8_t color;
    std::uint8_t isnil;
    std::uint8_t pad[2];
};
// Tree header: +0h (allocator word, unused here), head node +4h, size +8h.
struct NativeSoundTree {
    std::uint32_t allocator;
    NativeSoundTreeNode* head;
    std::uint32_t size;
};

// 00A81960 STL_SoundSampleTree_Max and 00A81980 STL_SoundSampleTree_Min, __fastcall(ECX node) -> EAX: follow
// right (Max) or left (Min) links while the child is not the nil head (+19h == 0).
NativeSoundTreeNode* sound_tree_max_00a81960(NativeSoundTreeNode* node) noexcept;
NativeSoundTreeNode* sound_tree_min_00a81980(NativeSoundTreeNode* node) noexcept;
// 00A822E0 STL_SoundSampleTree_RotateLeft and 00A819C0 STL_SoundSampleTree_RotateRight,
// __thiscall(ECX tree, node* where), RET 4: the _Lrotate / _Rrotate relinking, with the root kept in
// head->parent.
void sound_tree_rotate_left_00a822e0(NativeSoundTree& tree, NativeSoundTreeNode* where) noexcept;
void sound_tree_rotate_right_00a819c0(NativeSoundTree& tree, NativeSoundTreeNode* where) noexcept;
// 00A88270 STL_SoundNameTree_AllocateBlackNode_Provisional, no input, EAX node: operator new(1Ch) (00BF681B),
// zero left/parent/right, colour 1, isnil 0; the value bytes are left as allocated. The listing writes the
// fields even for a null result, so a non-null allocation is a precondition (operator new throws).
NativeSoundTreeNode* allocate_sound_tree_black_node_00a88270();
// 00A88370 STL_SoundNameTree_Init (RET, no result), 00A883D0 STL_SoundNameTree_ConstructWithAllocator
// (RET 8, EAX tree) and 00A88620 STL_SoundNameTree_Construct (RET, EAX tree), __thiscall(ECX tree): head =
// 00A88270(), head->isnil = 1, head->parent = head->left = head->right = head, size = 0. The two stack
// arguments of 00A883D0 are not read.
void init_sound_name_tree_00a88370(NativeSoundTree& tree);
NativeSoundTree& construct_sound_name_tree_with_allocator_00a883d0(NativeSoundTree& tree);
NativeSoundTree& construct_sound_name_tree_00a88620(NativeSoundTree& tree);

// Singleton base teardown: store the BSP_SingletonBase vtable 00CE3818 at +0h and clear the singleton's
// published global word (passed in: 00F8BBDC, 00E18AD4 or 00F8BBF8). The scalar-deleting forms,
// __thiscall(ECX object, flags), RET 4, EAX object, free the object when flags & 1 (00BF65AC).
void* scalar_delete_sound_event_query_lock_base_00a894b0(void* object, std::uint32_t flags,
                                                         std::uint32_t& published_00f8bbdc);
void* scalar_delete_shadow_job_base_00a8dd60(void* object, std::uint32_t flags, std::uint32_t& published_00e18ad4);
void destruct_input_manager_base_00a92180(void* object, std::uint32_t& published_00f8bbf8);
void* scalar_delete_input_manager_base_00a92230(void* object, std::uint32_t flags, std::uint32_t& published_00f8bbf8);

// 00A87BD0 BSP_Fmod_FreeCallback, __stdcall(void* memory, FMOD_MEMORY_TYPE), RET 8: the "- FMOD 0x%X "
// debug print goes to the empty body 004254B0, then CRT free (00BF9DC8).
void __stdcall fmod_free_callback_00a87bd0(void* memory, std::uint32_t memory_type);

// 00A90840 STL_FillN_Dword_00a90840, __cdecl(DWORD* first, unsigned count, const DWORD* value), EAX
// unspecified: writes *value count times (an unsigned count; JBE on zero).
void fill_n_dword_00a90840(std::uint32_t* first, std::uint32_t count, const std::uint32_t* value) noexcept;

// 00A90180 BSP_InputDeviceTable_AnyJoystickButtonDown, __fastcall(ECX table) -> AL: true at the first of
// the eight device slots at +44h that is non-null and whose vtable +2Ch (ECX = device) answers non-zero.
bool input_device_table_any_button_down_00a90180(const void* table);

// 00A93EC0 BSP_InputDevice_ClearStates, __fastcall(ECX device): memset(+0Ch, 0, 100h) and
// memset(+10Ch, 0, 100h).
void input_device_clear_states_00a93ec0(void* device) noexcept;

// 00A96450 BSP_KeyCharMap_Translate, __thiscall(ECX map, char c), RET 4 -> AL. The "from" characters are
// the string with size +8h and data +0Ch (null data reads the image's empty buffer 00F8BC02, passed in);
// the first match i (unsigned, i < size) answers the byte at [+14h] + i, otherwise c.
char key_char_map_translate_00a96450(const void* map, char c, const char* null_data_00f8bc02) noexcept;

// Input binding and action vectors (34h-byte bindings, 30h-byte actions; bsp/native_input_binding_storage.hpp
// and bsp/native_input_action_records.hpp hold the element routines these call).
// 00A931A0 BSP_InputBinding_Assign, __thiscall(ECX dst, const binding* src), RET 4, EAX dst: bytes +0h/+1h,
// DWORDs +4h..+14h, the two modifier vectors +18h and +24h through 00A92EE0, and the float +30h through
// FLD/FSTP (an x87 copy, which quiets a signalling NaN; the C++ copy keeps the bits).
void* assign_input_binding_00a931a0(void* dst, const void* src);
// 00A934B0 BSP_InputBindingVector_SwapRemove, __thiscall(ECX header, binding* const* where), RET 4: unless
// *where is the last binding, assign the last into *where (00A931A0); destroy the last (00A93070); count - 1.
void swap_remove_input_binding_00a934b0(void* header, void* const* where);
// 00A938E0 BSP_InputBindingVector_Destroy, __fastcall(ECX header): 00A93500(0) then free(base).
void destroy_input_bindings_00a938e0(void* header, const NativeInputBindingStorageContext& context);
// 00A93D80 BSP_NativeInputActionVector_Destroy, __fastcall(ECX header): 00A93C10(0) then free(base).
void destroy_native_input_actions_00a93d80(void* header, NativeInputActionRecordsContext& context);
// 00A93D60 BSP_InputActionMap_ClearVectors, __fastcall(ECX map): 00A93C10(map+4h, 0), then
// 0086A430(map+10h, 0).
void clear_input_action_map_vectors_00a93d60(void* map, NativeInputActionRecordsContext& context);

// --- String-bearing records (cc13-sound2) ---
// Each record starts with the native 8-byte pooled string header {uint32 length +0h, char* data +4h}. A copy
// zeroes the header, and unless dst == src resizes it to the source length with preserve (0041DD40) and, for
// a non-zero source length, copies the CURRENT destination length of bytes with the overlap-tolerant
// memcpy 00BF7680. String storage is the explicit NativeStringStorage boundary of bsp/native_string.hpp.

// Named values {string, DWORD +8h}, 0Ch bytes. __thiscall, EAX dst.
// 00A82FF0: (ECX dst, string* name, DWORD* value), RET 8; 00A83040 and 00A83090: (ECX dst, record* src), RET 4.
void* copy_named_value_00a82ff0(void* dst, const void* name, const std::uint32_t* value, NativeStringStorage& strings);
void* copy_named_value_00a83040(void* dst, const void* src, NativeStringStorage& strings);
void* copy_named_value_00a83090(void* dst, const void* src, NativeStringStorage& strings);

// Configured sound types {string, sample-pointer array +8h}, 14h bytes, and configured channel groups
// {string, DWORD +8h, effect-pointer array +0Ch}, 18h bytes, over native {data, count, capacity} headers.
// 00A7FB50 BSP_SoundConfiguredType_CopyConstruct / 00A7D880 BSP_SoundConfiguredGroup_CopyConstruct,
// __thiscall(ECX dst, src), RET 4, EAX dst: the string copy, the DWORD (groups), a zeroed array header, then
// 00A7BAA0 / 00A7B7C0. EH state 0 covers the array assign: the string is destroyed if it throws.
void* copy_sound_configured_type_00a7fb50(void* dst, const void* src, NativeStringStorage& strings);
void* copy_sound_configured_group_00a7d880(void* dst, const void* src, NativeStringStorage& strings);
// 00A7C6C0 / 00A7BFE0 over raw storage, ECX record, RET: destroy the inner array (resize 0 through 00A7B3D0
// / 00A7A930, then free its data), then release the name through 0041DD20. The same addresses have C++
// projection records in src/sound_configuration_shutdown.cpp; these are the raw-storage forms the reserves
// below need.
void destroy_sound_configured_type_raw_00a7c6c0(void* record, NativeStringStorage& strings);
void destroy_sound_configured_group_raw_00a7bfe0(void* record, NativeStringStorage& strings);
// 00A7FBD0 BSP_SoundConfiguredTypes_Reserve (minimum 1) and 00A7F160 BSP_SoundConfiguredGroups_Reserve
// (minimum 16), __thiscall(ECX header, int), RET 4: when the capacity is below, operator new, copy-construct
// each record forward, destroy each old record forward, free the old array, store data and capacity. A
// throwing copy leaks the new array (the EH map's placement delete is a no-op).
void reserve_sound_configured_types_00a7fbd0(NativePointerArray& header, std::int32_t requested, NativeStringStorage& strings);
void reserve_sound_configured_groups_00a7f160(NativePointerArray& header, std::int32_t requested, NativeStringStorage& strings);
// 00A7FEB0 BSP_SoundConfiguredTypes_Append, __thiscall(ECX header, const type*), RET 4: when full, reserve
// capacity * 2 (at least 1); copy-construct at the end; count + 1 after the copy returns.
void append_sound_configured_type_00a7feb0(NativePointerArray& header, const void* type, NativeStringStorage& strings);
// 00A7FAF0 BSP_SoundConfiguredGroups_AppendDefault, __fastcall(ECX header) -> EAX new record: when full,
// reserve capacity + 16 (at least 16); zero the six DWORDs of the new 18h record; count + 1.
void* append_default_sound_configured_group_00a7faf0(NativePointerArray& header, NativeStringStorage& strings);
// 00A7F110 BSP_SoundConfiguredGroups_PopBack, __fastcall(ECX header): when count != 0, destroy the last
// record (00A7BFE0) and count - 1.
void pop_back_sound_configured_group_00a7f110(NativePointerArray& header, NativeStringStorage& strings);

// 00A7FCC0 (types) and 00A7F240 (groups) over raw storage, __thiscall(ECX header, int count), RET 4: reserve
// (00A7FBD0 / 00A7F160) when count exceeds the capacity, zero the new records, then while count < current:
// current - 1 and destroy that record (00A7C6C0 / 00A7BFE0); finally store count. The same addresses keep
// their shrink-only fragment records in src/sound_configuration_shutdown.cpp.
void resize_sound_configured_types_raw_00a7fcc0(NativePointerArray& header, std::int32_t count, NativeStringStorage& strings);
void resize_sound_configured_groups_raw_00a7f240(NativePointerArray& header, std::int32_t count, NativeStringStorage& strings);
// 00A81460 BSP_SoundConfiguredTypes_Destroy and 00A7FF30 BSP_SoundConfiguredGroups_Destroy, __fastcall(ECX
// header): resize 0 then free(data).
void destroy_sound_configured_types_00a81460(NativePointerArray& header, NativeStringStorage& strings);
void destroy_sound_configured_groups_00a7ff30(NativePointerArray& header, NativeStringStorage& strings);

// 00A86FB0 BSP_DialogChannelRecords_Append, __thiscall(ECX header, const record*), RET 4: when full, reserve
// capacity * 2 (at least 1) through 00A78DC0; copy the 14h record through 00A77EF0; count + 1.
void append_dialog_channel_record_00a86fb0(void* header, const void* record, NativeStringStorage& strings,
                                           const std::array<std::uint32_t, 4>& format_counts);
// 00A93900 BSP_NativeInputBindingVector_CopyConstruct, __thiscall(ECX dst, const header* src), RET 4, EAX dst:
// zero the header, then 00A937E0.
void* copy_construct_input_binding_vector_00a93900(void* dst, const void* src,
                                                   const NativeInputBindingStorageContext& context);

// Sound-name tree nodes carry a pooled string {length +0Ch, data +10h} as their value.
// 00A88400 STL_SoundSampleTree_EraseSubtree, __thiscall(ECX tree, node*), RET 4: walk left spine; for each
// node erase its right subtree recursively, release its string (length + 1 bytes through the pool) and free
// the node (00BF65AC); stop at the nil head.
void erase_sound_name_subtree_00a88400(NativeSoundTree& tree, NativeSoundTreeNode* node, NativeStringStorage& strings);
// 00A88460 STL_SoundSampleTree_Clear, __fastcall(ECX tree): erase from the root, then head->parent = head,
// size = 0, head->left = head, head->right = head.
void clear_sound_name_tree_00a88460(NativeSoundTree& tree, NativeStringStorage& strings);

// Trivial bodies, from the bytes:
// 00A7BE70 BSP_SoundInstance_DefaultSetParameter: RET 8. 00A7BE80 BSP_SoundInstance_DefaultSlot34NoOp:
// RET 18h. 00A89C50 BSP_SoundEvent_RefreshEndedNoOp: RET. 00A8A9B0 / 00A8A9C0
// BSP_ShadowTargetExtents_VSlot1NoOp / VSlot2NoOp: RET 4. Their arguments are ignored.
void sound_instance_default_set_parameter_00a7be70(void* self, std::uint32_t a, std::uint32_t b) noexcept;
void sound_instance_default_slot34_00a7be80(void* self, const std::uint32_t args[6]) noexcept;
void sound_event_refresh_ended_noop_00a89c50() noexcept;
void shadow_target_extents_vslot1_noop_00a8a9b0(void* self, std::uint32_t arg) noexcept;
void shadow_target_extents_vslot2_noop_00a8a9c0(void* self, std::uint32_t arg) noexcept;
// 00A90EC0 BSP_InputBackendBase_DefaultSlot08False: XOR AL,AL; RET. 00A97220
// BSP_TextListenerBase_Slot00NoOp: XOR AL,AL; RET 8.
bool input_backend_default_slot08_00a90ec0(void* self) noexcept;
bool text_listener_base_slot00_00a97220(void* self, std::uint32_t a, std::uint32_t b) noexcept;
// 00A97B80 BSP_NativeInputBackend_GetByteF4, __fastcall(ECX backend) -> AL: the byte at +F4h.
std::uint8_t native_input_backend_byte_f4_00a97b80(const void* backend) noexcept;
// 00A92290 BSP_InputManager_GetContextLevel, __thiscall(ECX manager, int context), RET 4 -> EAX:
// ([+10h])[context].
std::uint32_t input_manager_context_level_00a92290(const void* manager, std::int32_t context) noexcept;
// 00A818D0 BSP_SoundSample_HasResourceWord08_Provisional, __fastcall(ECX sample) -> AL:
// [[sample+78h]+8h] != 0.
bool sound_sample_has_resource_word08_00a818d0(const void* sample) noexcept;

// ---------------------------------------------------------------------------
// cc13-sound3: raw forms of the sound class table and the sample-tree destroys
// (docs/CC13_SOUND_1.md, "Pass 3").
//
// Sound class table: a pointer array whose slots hold intrusive references (count at +4h,
// release through vtable slot 0). SoundClassOwnership (src/sound_class_ownership.cpp)
// models the same bodies over a projection; these work on the native header.
// 00A7BBE0 reserve, __thiscall(ECX header, int requested), RET 4: requested raised to 1; when
// the capacity is below it, operator new(requested*4); every live slot is copied with a
// retain (destination zeroed first); a SECOND pass releases and clears every old slot; the
// old data is freed; data then capacity are stored.
void reserve_sound_class_table_raw_00a7bbe0(NativePointerArray& table, std::int32_t requested);
// 00A7C2C0 resize (and its JMP thunk 00A7D9A0), __thiscall(ECX header, int count), RET 4:
// reserve when count exceeds the capacity, zero the new slots, then pop down to count
// releasing (and clearing) every non-null slot from the top; count = n.
void resize_sound_class_table_raw_00a7c2c0(NativePointerArray& table, std::int32_t count);
// 00A7C240 append-retained (and its JMP thunk 00A7D990), __thiscall(ECX header, const
// object** source), RET 4: grow to max(2*capacity, 1) through 00A7BBE0 when full; the new
// slot is zeroed, then set to *source and retained when *source is non-null; count + 1.
void append_retained_sound_class_raw_00a7c240(NativePointerArray& table, void* const* source);
// 00A7D9B0 and 00A7F2D0 (byte-identical), __fastcall(ECX header): resize 0 through 00A7C2C0,
// then free(data).
void destroy_sound_class_table_raw(NativePointerArray& table);

// Sample-tree iterator (checked iterator {owner tree, node}).
struct NativeSoundTreeIterator {
    NativeSoundTree* owner;
    NativeSoundTreeNode* node;
};
// 00A88490 STL_SoundSampleTree_EraseRange, __thiscall(ECX tree, iterator* out, first, last),
// RET 14h. A first iterator whose owner is null or not this tree goes to 00BF6713 (the CRT
// invalid-iterator report; execution continues). When first is begin() (head->left) and last
// (owner checked the same way) is end() (the head), the whole tree goes through 00A88400
// from the root, the head is reset (parent/left/right = head, size 0) and out = {tree,
// head->left}. Otherwise, while last's owner matches and first != last: first is advanced by
// 00A81AD0 and the old position erased by 00A839C0 (rebalancing erase; host callback here);
// out = first.
NativeSoundTreeIterator erase_sound_tree_range_00a88490(NativeSoundTree& tree, NativeSoundTreeIterator first,
    NativeSoundTreeIterator last, NativeStringStorage& strings, void (*invalid_iterator_00bf6713)(),
    void (*increment_00a81ad0)(NativeSoundTreeIterator&),
    void (*erase_iterator_00a839c0)(NativeSoundTree&, NativeSoundTreeIterator));
// 00A88560, 00A885A0 and 00A885E0 (byte-identical bodies), __fastcall(ECX tree): erase
// [head->left, head) through 00A88490 (always the whole-tree path, so neither callback is
// reached), free the head, then head = 0 and size = 0.
void destroy_sound_sample_tree_raw(NativeSoundTree& tree, NativeStringStorage& strings);
} // namespace bsp::sound_cc13
