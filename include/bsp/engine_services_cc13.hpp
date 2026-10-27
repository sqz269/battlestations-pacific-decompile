#pragma once

// Segments 96-97 (00BD9220-00C30570: startup, the allocator, perftime, cFileStore, the profile code,
// cShaderTexture and related engine services) leaves that had a reviewed ledger name but no
// reconstruction record and no tag (cc13-files1, docs/CC13_FILES_1.md).
//
// The routines operate on the ACTUAL native storage, as this band's sibling files do
// (src/native_vfs_*.cpp, src/native_filestore_*.cpp, src/file_store_cc13.cpp, src/sound_cc13.cpp):
// eight-byte length/data strings, {data, count, capacity} arrays, MSVC 2005 list and red-black tree
// nodes, intrusive reference counts at +4h released through the object's own vtable slot 0, and
// singleton bases.
//
// Allocation model: the image's operator new (00BF55BE, 00BF681B) and free (00BF9DC8, 00BF65AC) are the
// project's singleton_lifetime_allocate/free boundary (bsp/singleton_lifetime.hpp), as in
// src/sound_cc13.cpp. Strings go through NativeStringStorage and the string services 0041DD40 (resize),
// 0041DD20 (release), 004BCC00 (lowercase), 0054AA70 (append byte) and 00BEE690 (normalize).
// Image globals are never invented: every published word the image reads or writes is passed in.
// Calls to routines reconstructed elsewhere, whose host signatures need owner contexts this file does
// not have, go through EngineServiceCalls; each virtual names its image address.
//
// 00BF6713 (the CRT invalid-parameter path of the checked iterators) can return; validation sites
// continue after it, as in the listings. Original C++ EH and SEH frames are not claimed; nothing here
// is wired into a game host. Names are hypotheses, not recovered symbols.

#include <cstddef>
#include <cstdint>

#include "bsp/native_string.hpp"
#include "bsp/native_string_vector.hpp"

namespace bsp {
class SingletonLifetimeDomain;
}

namespace bsp::engine_services_cc13 {

// ---------------------------------------------------------------------------------------------------
// Shared native shapes (Win32 layouts read from the listings).

// {container, node} checked iterator of the MSVC 2005 list and tree templates.
struct NativeIterator {
    void* container;
    void* node;
};

// Tree and list headers: +0h allocator word (unused here), head node +4h, size +8h.
struct NativeContainerHeader {
    std::uint32_t allocator;
    void* head;
    std::uint32_t size;
};

// List node: next +0h, prev +4h, value from +8h.
struct NativeListNode {
    NativeListNode* next;
    NativeListNode* prev;
};

// {data, count, capacity} array header.
struct NativeArrayHeader {
    void* data;
    std::int32_t count;
    std::int32_t capacity;
};

// A native CRITICAL_SECTION (18h bytes on Win32) followed by the recursion counter at +18h that the
// lock sites bump after EnterCriticalSection and drop before LeaveCriticalSection (the tracked section
// BSP_CriticalSection_Create 00BD1860 returns).
struct NativeTrackedSection {
    std::uint8_t section[0x18];
    std::uint32_t recursion_18;
};

// 00BF6713. The handler can return; callers continue.
class InvalidParameterSink {
public:
    virtual ~InvalidParameterSink() = default;
    virtual void invalid_parameter_00bf6713() = 0;
};

// Routines reconstructed elsewhere that these bodies call. Each takes the native operands.
class EngineServiceCalls : public InvalidParameterSink {
public:
    // Iterator steps (ECX iterator).
    virtual void advance_vfs_mount_iterator_00bd97e0(NativeIterator&) = 0;
    virtual void advance_physical_index_iterator_00bd9860(NativeIterator&) = 0;
    virtual void retreat_vfs_mount_iterator_00bd99b0(NativeIterator&) = 0;
    virtual void retreat_physical_index_iterator_00bd9a40(NativeIterator&) = 0;
    virtual void advance_filestore_resident_iterator_00be4c30(NativeIterator&) = 0;
    virtual void retreat_filestore_resident_iterator_00be4d20(NativeIterator&) = 0;
    virtual void advance_filestore_pending_iterator_00be4e40(NativeIterator&) = 0;
    virtual void retreat_filestore_pending_iterator_00be4db0(NativeIterator&) = 0;
    // Head node allocations (no input, EAX node).
    virtual void* allocate_list_head_0c_00bda960() = 0;
    virtual void* allocate_list_head_28_00bda980() = 0;
    virtual void* allocate_tree_head_20_00bdaba0() = 0;
    virtual void* allocate_tree_head_24_00bdabf0() = 0;
    virtual void* allocate_filestore_resident_head_00be55e0() = 0;
    virtual void* allocate_filestore_pending_head_00be5690() = 0;
    virtual void* allocate_pooled_string_list_head_004c3020() = 0;
    // Subtree and range erasure (the argument is the root node, or the tree and range).
    virtual void destroy_physical_index_subtree_00bdf7a0(void* root) = 0;
    virtual void destroy_vfs_mount_subtree_00bdf7e0(void* root) = 0;
    virtual void erase_filestore_pending_subtree_00be66c0(void* root) = 0;
    virtual void erase_filestore_resident_subtree_00be6720(void* root) = 0;
    virtual void erase_physical_index_range_00be0c30(NativeIterator& result, NativeContainerHeader& tree,
        NativeIterator first, NativeIterator last) = 0;
    virtual void erase_vfs_mount_range_00be0d00(NativeIterator& result, NativeContainerHeader& tree,
        NativeIterator first, NativeIterator last) = 0;
    virtual void erase_vfs_mount_iterator_00be0080(NativeIterator& result, NativeContainerHeader& tree,
        NativeIterator where) = 0;
    // Payload and object destruction (ECX object).
    virtual void destroy_vfs_request_payload_00be1220(void* payload) = 0;
    virtual void clear_vfs_request_list_00be19e0(NativeContainerHeader& list) = 0;
    virtual void destroy_allocation_stats_00be27f0(void* object) = 0;
    virtual void construct_allocation_stats_base_00be2750(void* object) = 0;
    virtual void destroy_window_platform_base_00be2b10(void* object) = 0;
    virtual void destroy_filestore_payload_00be5d70(void* payload) = 0;
    virtual void destroy_frame_clock_base_00bedea0(void* object) = 0;
    virtual void construct_frame_clock_base_00bede00(void* object) = 0;
    virtual void destroy_physical_read_request_names_00bf3880(void* request) = 0;
    virtual void destroy_lua_state_owner_00bd4270(void* object) = 0;
    virtual void destroy_text_queue_00bec730(void* queue) = 0;
    virtual void* create_text_queue_sentinel_00bec710() = 0;
    virtual void destroy_ref_counted_base_00bd30f0(void* object) = 0;
    // Pooled string lists (ECX destination list header, stack source).
    virtual void copy_pooled_string_list_004d48a0(void* destination, const void* source) = 0;
    // VFS services.
    virtual void reserve_vfs_pair_vector_00bdcd10(NativeArrayHeader& vector, std::int32_t capacity) = 0;
    virtual void copy_vfs_string_pair_00bdcc60(void* destination, const void* source) = 0;
    virtual void destroy_vfs_string_pair_00bdb850(void* pair) = 0;
    virtual void resize_vfs_pair_vector_00bdec70(NativeArrayHeader& vector, std::int32_t count) = 0;
    virtual bool resolve_existing_vfs_name_00bdf4c0(void* vfs, const NativeString& name) = 0;
    virtual void visit_matching_mounts_00bdd0a0(void* vfs, const NativeString& name, void* visitor) = 0;
    virtual void* select_search_group_value_list_00bdb2e0(const NativeIterator& group, std::int32_t kind) = 0;
    virtual void erase_render_alias_list_node_004d0990(NativeIterator& result, void* list, void* node) = 0;
    virtual void list_size_check_00bded80(NativeContainerHeader& list, std::uint32_t added) = 0;
    // Native C++ objects called through the image's virtual tables.
    virtual void* create_tracked_section_00bd1860() = 0;
    virtual void destroy_tracked_section_0041cc80(void* owner_slot) = 0;
    virtual void set_debug_feature_enabled_00be8680(void* owner, const NativeString& name, bool enabled) = 0;
    virtual void reserve_debug_feature_records_00be8df0(NativeArrayHeader& records, std::int32_t capacity) = 0;
    virtual void reserve_stream_writer_chunks_00bf05d0(NativeArrayHeader& chunks, std::int32_t capacity) = 0;
    virtual void reserve_string_table_rows_00b80c70(NativeArrayHeader& rows, std::int32_t capacity) = 0;
    virtual void reserve_physical_stream_slots_00bf30c0(NativeArrayHeader& slots, std::int32_t capacity) = 0;
    virtual void physical_stream_open_00bf52a0(void* stream, std::uint32_t name, std::uint32_t mode) = 0;
    virtual bool pc_storage_file_exists_00beb040(void* storage, std::uint32_t slot, const char* file) = 0;
    virtual void compose_error_message_00bf1b50(void* reporter, NativeString& out, const NativeString& text) = 0;
    virtual void* read_structured_string_00bea010(void* handle, NativeString& out) = 0;
    virtual std::uint32_t decode_tga_00befdc0(void* stream) = 0;
    virtual std::uint32_t read_dword_slot34_and_debit_00bf0280(void* reader, std::uint32_t* remaining) = 0;
    virtual std::uint32_t read_dword_and_debit_00bf02a0(void* reader, std::uint32_t* remaining) = 0;
    virtual float read_float_and_account_00bf02c0(void* reader, std::uint32_t* remaining) = 0;
    virtual void stream_reader_construct_00bf09a0(void* reader) = 0;
    virtual void stream_reader_assign_stream_00bf0430(void* reader, void* stream) = 0;
    virtual void profiler_clear_history_00be3380(void* profiler) = 0;
    virtual char* text_scanner_peek_token_00bee8e0(void* scanner) = 0;
    virtual std::uint32_t text_scanner_skip_whitespace_00beedb0(void* scanner) = 0;
    virtual bool text_scanner_peek_bool_00beec40(void* scanner, char* out) = 0;
    virtual void append_checked_string_vector_00450540(void* vector, const NativeString& value) = 0;
    // Tree bounds (ECX tree, stack key) -> EAX node.
    virtual void* lower_bound_filestore_pending_00be5530(NativeContainerHeader& tree, const void* key) = 0;
    virtual void* upper_bound_filestore_pending_00be5630(NativeContainerHeader& tree, const void* key) = 0;
    virtual void* upper_bound_extension_prefix_00bda2c0(NativeContainerHeader& tree, const NativeString& key) = 0;
    virtual void* lower_bound_indexed_name_00bda260(NativeContainerHeader& tree, const NativeString& key) = 0;
    // 00BEE390 (ECX output header, EDX input header).
    virtual void canonicalize_path_00bee390(NativeString& out, const NativeString& in) = 0;
    // File-block scope services.
    virtual void prepare_fileblock_identifier_00bdf950(void* scope) = 0;
    virtual void enter_fileblock_00be0980(void* vfs, const NativeString* name, std::uint32_t arg) = 0;
};

// ---------------------------------------------------------------------------------------------------
// Allocation and small frees.

// 00BD9440, 00BD95A0, 00BD95E0, 00BD9780 BSP_RawFree_Unreferenced_*, __cdecl(void*): CRT free (00BF9DC8).
// Unreferenced COMDAT copies.
void raw_free(void* block) noexcept;

// Typed element allocators, __fastcall(ECX count) -> EAX: count 0 allocates 0 bytes; otherwise when
// 0xFFFFFFFF / count < size, throw std::bad_alloc (00BF6340 + _CxxThrowException); then operator new
// (00BF55BE, or 00BF681B for 00BEC310) of count * size bytes.
// 00BD9B20 (0Ch), 00BD9BD0 (28h), 00BD9CB0 (20h), 00BD9D30 (24h), 00BE4F30 and 00BE4FA0 (1Ch),
// 00BF1760 (20h), 00BEC310 BSP_RenderEntryArray_AllocateElements (0Ch).
void* allocate_native_elements(std::uint32_t count, std::uint32_t element_size);

// __fastcall(ECX slot): free *slot when non-null, then clear it. 00BE2BB0, 00BE2BD0, 00BE2BF0
// BSP_OwnedBlock_Free_* and 00BF2E70 BSP_PointerArray_FreeStorage are byte-identical.
void free_owned_block(void** slot) noexcept;

// 00BE3240 BSP_PerfCounter_Query_Unreferenced, no input, EDX:EAX: QueryPerformanceCounter into a local.
std::int64_t query_performance_counter_00be3240() noexcept;

// ---------------------------------------------------------------------------------------------------
// Constant leaves.

// No input; AL or EAX constant. 00BDBEB0 (AL 0), 00BE2740 BSP_AllocationStats_ReturnFalse,
// 00BE2C40 BSP_FrameJobPoolBase_ReturnFalse, 00BEC180 BSP_Win32Platform_Slot18_ReturnFalse return false;
// 00BEC150 BSP_Win32Platform_Slot14_ReturnTrue and 00BF0A30 BSP_CsvTable_SaveStub return true.
bool return_false() noexcept;
bool return_true() noexcept;
// 00BEC140 / 00BEC190 BSP_Win32Platform_Slot10_NoOp / Slot1C_NoOp: RET.
void no_operation() noexcept;
// 00BECA30 BSP_Win32Platform_GetWindowStyle_Provisional: EAX 00C40000h.
std::uint32_t win32_platform_window_style_00beca30() noexcept;
// 00BE2720 BSP_AllocationStats_ReadZeroedBudgetVariant: memset a 34h-byte local to zero and return its
// DWORD at +1Ch, so always 0.
std::uint32_t allocation_stats_zeroed_budget_00be2720() noexcept;
// Getters returning a published word (passed in): 00BE4080 BSP_FileStream_GetTypeId (0109DB58),
// 00BE4090 BSP_FileStream_GetBaseTypeId (0109DB60), 00BEF480 BSP_MemoryStream_GetFileType (0109DBA0),
// 00C301C0 BSP_TextureSourceBase_GetRuntimeTypeNameAddress (0109E9D0).
std::uint32_t read_published_word(const std::uint32_t& word) noexcept;
// 00BE9620 BSP_FingerprintPayload_GetData, __fastcall(ECX) -> EAX ECX+4.
void* fingerprint_payload_data_00be9620(void* payload) noexcept;
// DWORD field getters, __fastcall(ECX object) -> EAX: 00BE8200 BSP_DebugFeatureOwner_GetProfileCount
// (+1Ch), 00BEFB60 BSP_RamImage_GetData (+18h), 00BEFB70 GetWidth (+8h), 00BEFB80 GetHeight (+0Ch),
// 00BEFB90 GetDepth (+10h), 00BEFBA0 GetFormat (+14h).
std::uint32_t read_dword_field(const void* object, std::size_t offset) noexcept;
// 00C30230 BSP_SceneTypeInfo_IsKindOf_0109e9c4, __cdecl(int type) -> AL: scan the three DWORDs at
// 0109E9C4..0109E9CF (passed in); a match returns AL 1. The miss path returns EAX 0109E900, whose low
// byte is 0, so the boolean result is false.
bool scene_type_is_kind_of_00c30230(const std::uint32_t (&types_0109e9c4)[3], std::uint32_t type) noexcept;
// 00BEE140 BSP_Vector4_ScaleByScalarRef, __fastcall(ECX out, EDX const float* scalar, stack const float*
// v) RET 4: out[i] = scalar * v[i], single-precision FMUL per lane (the scalar is loaded once).
void scale_vector4_00bee140(float* out, const float* scalar, const float* v) noexcept;
// The HUD host's caller runs on the main thread at 24-bit precision control, where each FMUL
// rounds to 24 bits with the extended exponent before the FSTP DWORD: a denormal product is
// rounded twice (cc13-fixture7, docs/CC13_FIXTURE_PASS_7.md). scale_vector4_pc24_00bee140
// models that and is fixture-tested; scale_vector4_00bee140 takes it once this is flipped.
inline constexpr bool kScaleVector4Pc24Cc13Bound = false;
void scale_vector4_pc24_00bee140(float* out, const float* scalar, const float* v) noexcept;

// ---------------------------------------------------------------------------------------------------
// Virtual-table and singleton-base teardown.

// Store a vtable word at +0h, then free when flags & 1; __thiscall(ECX, flags) RET 4, EAX object.
// 00BD9E40 BSP_VfsOpenVisitor_ScalarDelete, 00BD9F30 BSP_VfsFileDateVisitor_ScalarDelete,
// 00BD9F50 BSP_VfsExistsVisitor_ScalarDelete and 00BD9F70 BSP_VfsFlaggedExistsVisitor_ScalarDelete
// store the visitor base 00D68380; 00BE2C50 BSP_FrameJobPoolBase_ScalarDelete stores 00D68638.
void* scalar_delete_storing_vtable(void* object, std::uint32_t flags, std::uint32_t vtable) noexcept;

// Singleton base teardown: clear the published word, store BSP_SingletonBase 00CE3818 at +0h.
// 00BE3780 BSP_ProfilerBase_UnwindDestruct (0109DB50), 00BE8230 BSP_DebugFeatureLock_ResetBase
// (0109DB74), 00BE9710 BSP_FingerprintPayload_ResetBase (0109DB78), 00BF14F0 BSP_ErrorReporter_ResetBase
// (0109DBDC), __fastcall(ECX object).
void reset_singleton_base(void* object, std::uint32_t& published) noexcept;
// The scalar-deleting forms: 00BE3BC0 BSP_ProfilerBase_ScalarDeletingDestructor (0109DB50),
// 00BE98B0 BSP_FingerprintPayloadBase_ScalarDelete (0109DB78), 00BED8C0
// BSP_PhysicalDirectoryFactoryBase_ScalarDelete (0109DBE8), 00BF1520 BSP_ErrorReporterBase_ScalarDelete
// (0109DBDC).
void* scalar_delete_singleton_base(void* object, std::uint32_t flags, std::uint32_t& published) noexcept;

// Scalar-deleting destructors over a destroy routine, __thiscall(ECX, flags) RET 4, EAX object.
enum class DestroyRoutine {
    allocation_stats_00be27f0,          // 00BE28E0 BSP_AllocationStats_DerivedScalarDelete
    window_platform_base_00be2b10,      // 00BE2B80 BSP_WindowPlatformBase_ScalarDeletingDestructor
    filestore_payload_00be5d70,         // 00BE6330 BSP_FileStorePayload_ScalarDelete
    frame_clock_base_00bedea0,          // 00BEDF40 BSP_FrameClockBase_ScalarDelete, 00BEE0F0 ..Variant_..
    physical_read_request_00bf3880,     // 00BF3900 BSP_PhysicalReadRequest_ScalarDelete
    vfs_request_payload_00be1220,       // 00BE1690 BSP_VfsSearchGroupPayload_ScalarDelete_Unreferenced
};
void* scalar_delete_via(void* object, std::uint32_t flags, DestroyRoutine routine, EngineServiceCalls& calls);

// ---------------------------------------------------------------------------------------------------
// Intrusive reference slots: the object's count is the DWORD at +4h, released with InterlockedDecrement;
// at zero the object's vtable slot 0 runs with ECX = object.

// __fastcall(ECX slot): release and clear when non-null. 00BE56E0 BSP_StreamRef_Release, 00BE59C0
// BSP_StreamRef_ReleaseForUnwind, 00BE9D10 / 00BE9D90 BSP_StructuredNodeRef_Release2/3, 00BF0470
// BSP_StreamRef_Release2 are byte-identical; 00BE9B80 BSP_StructuredNodeRef_Release also returns EAX slot.
void release_ref_slot(void** slot);
void** release_ref_slot_return_self_00be9b80(void** slot);
// 00BE44A0 BSP_VFS_ReleaseStream: release when non-null; the slot is NOT cleared.
void release_ref_slot_keep_00be44a0(void** slot);
// 00BE4500 BSP_StreamRef_ReleaseUnchecked: release WITHOUT a null test, then clear.
void release_ref_slot_unchecked_00be4500(void** slot);
// 00BEF1E0 BSP_RefPtr_Assign, __thiscall(ECX slot, object) RET 4, EAX slot: release and clear the old
// object, store the new one without adding a reference.
void** assign_ref_slot_00bef1e0(void** slot, void* object);
// 00BE4380 BSP_VFS_OpenStreamInto, __thiscall(ECX out, name, flags) RET 8, EAX out: *out = the VFS
// object's (published word 0109CEEC, passed in) vtable +4h (name, flags).
void** vfs_open_stream_into_00be4380(void** out, void* vfs_0109ceec, std::uint32_t name, std::uint32_t flags);
// 00BE44C0 BSP_StreamRef_ReopenFromVfs, __thiscall(ECX slot, name, flags) RET 8: release the old object
// (the slot is not cleared first), then *slot = vfs vtable +4h (name, flags).
void stream_ref_reopen_00be44c0(void** slot, void* vfs_0109ceec, std::uint32_t name, std::uint32_t flags);

// ---------------------------------------------------------------------------------------------------
// Native stream wrappers. The stream is a native C++ object called through its own vtable.

// 00BE40B0 BSP_Stream_WriteDwordSlot50 and 00BE40F0 BSP_Stream_WriteFloat, __thiscall(ECX stream, DWORD
// value, arg) RET 8: vtable +28h (&value, 4, arg). (00BE40B0 is itself the stream vtables' slot +50h.)
void stream_write_dword_00be40b0(void* stream, std::uint32_t value, std::uint32_t arg);
void stream_write_float_00be40f0(void* stream, float value, std::uint32_t arg);
// 00BE4120 BSP_Stream_WriteStringOperator_Unreferenced, __thiscall(ECX stream, text) RET 4, EAX stream:
// vtable +50h (text, 0).
void* stream_write_string_00be4120(void* stream, const void* text);
// __thiscall(ECX stream, out*) RET 4, EAX stream: *out = vtable +34h (0) (DWORD) or +44h (0) (x87 result
// stored by FSTP dword). 00BE4160 / 00BE4180 on the stream itself; 00BE43F0 / 00BE4410 on the stream held
// in the slot at ECX.
void* stream_read_dword_00be4160(void* stream, std::uint32_t* out);
void* stream_read_float_00be4180(void* stream, float* out);
void** stream_ref_read_dword_00be43f0(void** slot, std::uint32_t* out);
void** stream_ref_read_float_00be4410(void** slot, float* out);
// Writer wrappers, __thiscall(ECX writer = slot holding the stream, ..., DWORD* remaining): the call
// receives the address of its own value argument as an out-parameter, and *remaining is debited by
// whatever the call leaves there (the bytes written).
// 00BF0340 WriteDwordAndDebit: vtable +54h (value, &value), RET 8.
// 00BF0360 WriteFloatAndDebit: vtable +58h (value, &value), RET 8.
// 00BF0390 WriteStringAndDebit: vtable +5Ch (text, &text), RET 8.
// 00BF03B0 WriteBytesAndDebit: vtable +28h (data, size, &size), RET 0Ch.
void stream_writer_write_dword_00bf0340(void** writer, std::uint32_t value, std::uint32_t* remaining);
void stream_writer_write_float_00bf0360(void** writer, float value, std::uint32_t* remaining);
void stream_writer_write_string_00bf0390(void** writer, const void* text, std::uint32_t* remaining);
void stream_writer_write_bytes_00bf03b0(void** writer, const void* data, std::uint32_t size,
                                        std::uint32_t* remaining);
// 00BEF4D0 BSP_MemoryStream_CastFromStream, __fastcall(ECX stream) -> EAX: non-null and vtable +0Ch
// (memory-stream type word 0109DBA0, passed in) answers non-zero -> the stream, else null.
void* memory_stream_cast_00bef4d0(void* stream, std::uint32_t memory_type_0109dba0);

// ---------------------------------------------------------------------------------------------------
// Checked iterators. Each validation calls 00BF6713, which may return; execution then continues.

// Dereference, __fastcall(ECX it) -> EAX node+0Ch: invalid when the container is null or the node is
// the container's head. 00BE4CA0, 00BE5190, 00BE51D0, 00BE53B0, 00BE5400, 00BE5430, 00BE5800
// (BSP_FileStoreTreeIterator_Dereference*), 00BF1390, 00BF1570, 00BF17D0
// (BSP_ErrorContextMapIterator_Dereference*) and 00BF3610 BSP_PhysicalDirectoryIterator_Dereference.
void* tree_iterator_value(const NativeIterator& it, InvalidParameterSink& sink);
// The list form returns node+8h: 00BEC4C0 BSP_CheckedListIterator_Dereference, 00BEC780
// BSP_TextQueueIterator_Dereference.
void* list_iterator_value(const NativeIterator& it, InvalidParameterSink& sink);
// __thiscall(ECX it, const it* other) RET 4 -> AL: invalid when this container is null or differs from
// the other's; then node equality. Equals 00BE4860, 00BE4B90, 00BF12E0; NotEquals 00BE4890, 00BE50A0,
// 00BF13D0. The upper EAX bytes carry this->node >> 8.
bool iterator_equals(const NativeIterator& it, const NativeIterator& other, InvalidParameterSink& sink);
bool iterator_not_equals(const NativeIterator& it, const NativeIterator& other, InvalidParameterSink& sink);
// List increment, __fastcall(ECX it) -> EAX it: validate, then node = node->next. 00BEC2D0
// BSP_CheckedListIterator_Increment and 00BEC530 BSP_TextQueueIterator_Increment.
NativeIterator& list_iterator_increment(NativeIterator& it, InvalidParameterSink& sink);
// 00BEC830 BSP_TextQueueIterator_PostIncrement, __thiscall(ECX it, it* result) RET 4, EAX result.
NativeIterator& list_iterator_post_increment_00bec830(NativeIterator& it, NativeIterator& result,
                                                      InvalidParameterSink& sink);

// Pre-step wrappers, __fastcall(ECX it) -> EAX it, around a step routine:
enum class IteratorStep {
    vfs_mount_advance_00bd97e0,       // 00BDA130, 00BDA860 BSP_VfsMountTree_IteratorPreIncrement_*
    physical_index_advance_00bd9860,  // 00BDA210, 00BDA8D0 BSP_NativePhysicalIndex_IteratorPreIncrement_*
    vfs_mount_retreat_00bd99b0,       // 00BDA590, 00BDAB80 BSP_VfsMountTree_IteratorPreDecrement_*
    physical_index_retreat_00bd9a40,  // 00BDA5A0, 00BDAB90 BSP_NativePhysicalIndex_IteratorPreDecrement_*
    resident_advance_00be4c30,        // 00BE51C0, 00BE5460 BSP_FileStoreResidentIterator_Increment*
    resident_retreat_00be4d20,        // 00BE5250, 00BE5470 BSP_FileStoreResidentIterator_Decrement*
    pending_advance_00be4e40,         // 00BE5260, 00BE54B0 BSP_FileStorePendingIterator_Increment*
    pending_retreat_00be4db0,         // 00BE5270, 00BE54C0 BSP_FileStorePendingIterator_Decrement*
};
NativeIterator& iterator_pre_step(NativeIterator& it, IteratorStep step, EngineServiceCalls& calls);

// ---------------------------------------------------------------------------------------------------
// Container headers.

// Tree header construction, __fastcall(ECX tree): head = allocate(); head.isnil (byte at the offset
// below) = 1; head.parent = head.left = head.right = head; size = 0. The Construct forms return EAX tree.
enum class TreeHead {
    physical_index_00bdaba0,     // isnil +1Dh: 00BDAFA0 InitEmpty, 00BDB450 / 00BE1910 Construct
    vfs_mount_00bdabf0,          // isnil +21h: 00BDAFE0 InitEmpty, 00BDB480 / 00BE1940 Construct
    filestore_resident_00be55e0, // isnil +19h: 00BE58E0 InitializeEmpty, 00BE5AD0 / 00BE7F70 Construct
    filestore_pending_00be5690,  // isnil +19h: 00BE5950 InitializeEmpty, 00BE5B30 / 00BE7B80 Construct
};
NativeContainerHeader& construct_tree_header(NativeContainerHeader& tree, TreeHead kind, EngineServiceCalls& calls);
// List header construction, __fastcall(ECX list) -> EAX list: head = allocate(); size = 0 (the head's
// links are set by the allocator). 00BDB3A0 STL_PointerList_Construct_Unreferenced (00BDA960) and
// 00BDB3D0 BSP_VfsSearchGroupList_Construct_Unreferenced (00BDA980).
NativeContainerHeader& construct_pointer_list_00bdb3a0(NativeContainerHeader& list, EngineServiceCalls& calls);
NativeContainerHeader& construct_search_group_list_00bdb3d0(NativeContainerHeader& list, EngineServiceCalls& calls);
// 00BECD40 BSP_TextQueue_Construct: the same with BSP_TextQueue_CreateSentinel 00BEC710.
NativeContainerHeader& construct_text_queue_00becd40(NativeContainerHeader& list, EngineServiceCalls& calls);

// Tree clear, __fastcall(ECX tree): erase the subtree at head->parent, then head->parent = head, size = 0,
// head->left = head->right = head.
enum class TreeSubtree {
    physical_index_00bdf7a0,     // 00BE0560 BSP_NativePhysicalIndex_Clear_Unreferenced
    vfs_mount_00bdf7e0,          // 00BE0590 BSP_VfsMountTree_Clear_Unreferenced
    filestore_pending_00be66c0,  // 00BE70D0 BSP_FileStorePendingTree_Clear
    filestore_resident_00be6720, // 00BE7100 BSP_FileStoreResidentTree_Clear
};
void clear_tree(NativeContainerHeader& tree, TreeSubtree kind, EngineServiceCalls& calls);
// Tree destruct, __fastcall(ECX tree): erase [begin, end) through the range routine, free the head, clear
// head and size. 00BE12F0 / 00BE15A0 use 00BE0C30 (physical index); 00BE13A0 / 00BE1610 use 00BE0D00
// (VFS mount tree).
void destruct_physical_index_tree(NativeContainerHeader& tree, EngineServiceCalls& calls);
void destruct_vfs_mount_tree(NativeContainerHeader& tree, EngineServiceCalls& calls);

// 00BDAAE0 STL_PointerList_Clear_Unreferenced, __fastcall(ECX list): detach the chain (head->next = head,
// head->prev = head, size 0), then free every former node.
void clear_pointer_list_00bdaae0(NativeContainerHeader& list) noexcept;
// 00BDAD10 STL_PointerList_Remove, __thiscall(ECX list, const DWORD* value) RET 4: erase every node whose
// DWORD at +8h equals *value (read once), with the checked-iterator validations of the listing.
void remove_from_pointer_list_00bdad10(NativeContainerHeader& list, const std::uint32_t* value,
                                       InvalidParameterSink& sink);
// 00BEC990 BSP_TextQueue_EraseIterator, __thiscall(ECX list, it* result, it where) RET 0Ch: validate
// where; when the node is not the head, unlink, free and decrement size; result = {container, next}.
void erase_text_queue_node_00bec990(NativeContainerHeader& list, NativeIterator& result, NativeIterator where,
                                    InvalidParameterSink& sink);

// Pointer arrays (4-byte slots).
// 00BF2EE0 BSP_PointerArray_Grow, __fastcall(ECX array): capacity = capacity * 2 + 2; copy count slots
// into operator new(capacity * 4) (each destination tested for null), free the old data.
void grow_pointer_array_00bf2ee0(NativeArrayHeader& array);
// 00BF3120 BSP_PointerArray_PushBack, __thiscall(ECX array, const DWORD* value) RET 4: the same growth
// inline when count == capacity, then store and count++.
void push_back_pointer_array_00bf3120(NativeArrayHeader& array, const std::uint32_t* value);
// 00BF5100 BSP_PhysicalStreamPool_PushBackSlot: growth to max(capacity * 2, 1) through 00BF30C0.
void push_back_physical_stream_slot_00bf5100(NativeArrayHeader& array, const std::uint32_t* value,
                                             EngineServiceCalls& calls);

// ---------------------------------------------------------------------------------------------------
// String-bearing records.

// Release one eight-byte string at a byte offset through 0041DD20 (header untouched).
// 00BDB550 (+0h), 00BDB570 and 00BDB590 (+4h) BSP_PooledString_ReleaseStorage_*; 00BE5E50, 00BE5E70,
// 00BE8800, 00BF04B0 BSP_NativeString_ReleaseForUnwind* (+0h).
void release_string_at(void* object, std::size_t offset, NativeStringStorage& strings) noexcept;
// 00BDB7C0 BSP_VfsExtensionPrefixPair_Destroy: the second string (+8h), then the first (+0h).
void destroy_string_pair_00bdb7c0(void* pair, NativeStringStorage& strings) noexcept;
// Scalar-deleting string records: release the string, free when flags & 1.
// 00BE88C0 BSP_DebugFeatureRecord_ScalarDelete and 00BF04D0 BSP_NativeString_ScalarDelete (string at
// +0h); 00BE6360 BSP_NativeStringAt0C_ScalarDelete (string at +0Ch).
void* scalar_delete_string_record(void* object, std::uint32_t flags, std::size_t offset,
                                  NativeStringStorage& strings) noexcept;

// Copy-constructs: zero the destination header, then unless copying onto itself resize to the source
// length (preserve) and copy the CURRENT destination length when the source length is non-zero.
// 00BDB920 BSP_NamedValueRecord_Construct_Unreferenced, __thiscall(ECX, string*, DWORD, BYTE) RET 0Ch:
// {string, +8h DWORD, +0Ch BYTE}.
void* construct_named_value_record_00bdb920(void* record, const NativeString& name, std::uint32_t value,
                                            std::uint8_t flag, NativeStringStorage& strings);
// 00BDCDF0 BSP_VfsExtensionPrefixPair_CopyConstruct_Unreferenced: copy of that 10h-byte record.
void* copy_named_value_record_00bdcdf0(void* record, const void* source, NativeStringStorage& strings);
// {string, +8h DWORD} copies: 00BE6200 BSP_NativeStringIntPair_CopyConstruct and 00BF0580
// BSP_StreamWriterChunk_CopyConstruct, __thiscall(ECX, source*) RET 4, EAX this.
void* copy_string_int_pair(void* record, const void* source, NativeStringStorage& strings);
// Two-string pairs: 00BDCBB0 BSP_VfsExtensionPrefixPair_Construct (ECX, first*, second*) RET 8;
// 00BDCEA0 BSP_VfsExtensionPrefixPair_CopyConstruct (ECX, pair*) RET 4 (the same body as
// BSP_TrafficStringPair_CopyConstruct 0048E500).
void* construct_string_pair_00bdcbb0(void* pair, const NativeString& first, const NativeString& second,
                                     NativeStringStorage& strings);
void* copy_string_pair_00bdcea0(void* pair, const void* source, NativeStringStorage& strings);
// 00BE8CA0 BSP_DebugFeatureOwner_CopySelectedProfileName, __thiscall(ECX owner, string* out) RET 4,
// EAX out: construct *out as a copy of the string the owner's +30h points to.
NativeString& copy_selected_profile_name_00be8ca0(const void* owner, NativeString& out,
                                                  NativeStringStorage& strings);
// Debug feature records (88h bytes: string + 80h bytes).
// 00BE87D0 BSP_DebugFeatureRecord_ConstructEmpty: zero the header, memset(+8h, 0, 80h).
void* construct_debug_feature_record_00be87d0(void* record) noexcept;
// 00BE8D90 BSP_DebugFeatureRecord_CopyConstruct: string copy, then 20h DWORDs (REP MOVSD).
void* copy_debug_feature_record_00be8d90(void* record, const void* source, NativeStringStorage& strings);
// 00BE8FD0 BSP_DebugFeatureRecordArray_Append, __thiscall(ECX array, record*) RET 4: when full, grow to
// max(capacity * 2, 1) through 00BE8DF0; copy-construct at data + count * 88h (skipped for a null
// address); count++.
void append_debug_feature_record_00be8fd0(NativeArrayHeader& array, const void* record,
                                          NativeStringStorage& strings, EngineServiceCalls& calls);
// 00BF0810 BSP_StreamWriterChunkStack_PushBack: the same over 0Ch {string, DWORD} chunks with
// 00BF05D0 and 00BF0580.
void push_back_stream_writer_chunk_00bf0810(NativeArrayHeader& array, const void* chunk,
                                            NativeStringStorage& strings, EngineServiceCalls& calls);
// 00BF0540 BSP_StreamWriterChunkStack_PopBack, __fastcall(ECX array): when count != 0 release the last
// chunk's string and count--.
void pop_back_stream_writer_chunk_00bf0540(NativeArrayHeader& array, NativeStringStorage& strings) noexcept;
// 00BDEC00 BSP_NativeVfsPairVector_PushBack: the append shape over 10h-byte string pairs with 00BDCD10
// (reserve) and 00BDCC60 (copy).
void push_back_vfs_pair_00bdec00(NativeArrayHeader& vector, const void* pair, EngineServiceCalls& calls);

// Native string vectors ({NativeString*, count, capacity}) and arrays of them (0Ch elements).
// 00BF0AC0 BSP_NativeStringVector_GrowToIndex, __thiscall(ECX vector, int index) RET 4 -> EAX
// data + index * 8: while count <= index append an empty string (resize 0, copy length+1 bytes of the
// image's empty string 00CE3A0C when data is non-null) through 004CDC20, releasing the temporary.
NativeString* string_vector_grow_to_index_00bf0ac0(NativeStringVectorStorage& vector, std::int32_t index,
                                                   NativeStringStorage& strings);
// 00BF0BD0 BSP_NativeStringVectorArray_PushBack: grow to max(capacity * 2, 1) through 00B80C70; zero the
// new element's header and copy the source vector into it (00543E50); count++.
void push_back_string_vector_array_00bf0bd0(NativeArrayHeader& array, const NativeStringVectorStorage& value,
                                            NativeStringStorage& strings, EngineServiceCalls& calls);
// 00BF0C60 BSP_NativeStringVectorArray_GrowToIndex: while count <= index push an empty vector (then
// resize the temporary to 0 and free its data); EAX data + index * 0Ch.
NativeStringVectorStorage* string_vector_array_grow_to_index_00bf0c60(NativeArrayHeader& array, std::int32_t index,
    NativeStringStorage& strings, EngineServiceCalls& calls);

// ---------------------------------------------------------------------------------------------------
// VFS search groups, prefixes and aliases. The VFS object: mount tree header +3Ch, extension-prefix
// tree +54h, search-group list header +60h (head +64h), alias pair vector +94h.

// Shared context for the path routines.
struct VfsPathContext {
    NativeStringStorage& strings;
    EngineServiceCalls& calls;
    // The routines read data[length - 1] with data defaulting to the image's empty buffer 0109CEF0, so an
    // empty path reads the byte BEFORE it (0109CEEF).
    char byte_0109ceef;
};

// 00BDBFD0, __fastcall(ECX result*, EDX const string* key, stack first, last) RET 10h: walk list nodes
// (next +0h, string length +8h / data +0Ch) from first.node to last.node; a node matches when the
// lengths are equal and either both are empty or _stricmp is 0. result = {first.container, match or
// last.node}.
NativeIterator& find_string_in_list_00bdbfd0(NativeIterator& result, const NativeString& key,
                                             NativeIterator first, NativeIterator last);
// 00BDC230 BSP_VFS_FindSearchGroup, __thiscall(ECX vfs, it* result, const string* name) RET 8: lowercase a
// copy (004BCC00), then scan the group list at +60h for a group name (node +8h/+0Ch) of equal length
// that is empty or _stricmp-equal; result = {vfs+60h, node or head}.
NativeIterator& vfs_find_search_group_00bdc230(void* vfs, NativeIterator& result, const NativeString& name,
                                               NativeStringStorage& strings, InvalidParameterSink& sink);
// 00BDD520 BSP_VFS_ExistsInFlaggedMounts, __thiscall(ECX vfs, const string* name) RET 4 -> AL: normalize a
// copy (00BEE690), visit the matching mounts with a flagged-exists visitor {vtable 00D683A4, BYTE found
// +4h = 0} (00BDD0A0), reset its vtable to 00D68380, release the copy, return found.
bool vfs_exists_in_flagged_mounts_00bdd520(void* vfs, const NativeString& name, VfsPathContext& context);
// 00BDF590 BSP_VFS_AddAlias, __thiscall(ECX vfs, const string* alias, const string* target) RET 8: build a
// pair of normalized copies, push it onto the alias vector at +94h (00BDEC00), destroy the pair
// (00BDB850).
void vfs_add_alias_00bdf590(void* vfs, const NativeString& alias, const NativeString& target, VfsPathContext& context);
// 00BDF880 BSP_VFS_ResolveExistingNameCopy_Unreferenced, __thiscall(ECX vfs, const string*) RET 4 -> AL:
// copy, 00BDF4C0 on the copy, release.
bool vfs_resolve_existing_name_copy_00bdf880(void* vfs, const NativeString& name, VfsPathContext& context);
// 00BDF940 BSP_VFS_ClearAliases, __thiscall(ECX vfs): resize the alias vector at +94h to 0 (00BDEC70).
void vfs_clear_aliases_00bdf940(void* vfs, EngineServiceCalls& calls);
// 00BE06A0 BSP_VFS_UnmountProvider_Unreferenced, __thiscall(ECX vfs, provider) RET 4 -> EAX 0/1: find the
// first mount (tree +3Ch) whose node +18h is the provider; release that provider reference and erase
// the node (00BE0080).
std::uint32_t vfs_unmount_provider_00be06a0(void* vfs, void* provider, EngineServiceCalls& calls);
// 00BE1B90 BSP_VFS_UnregisterSearchGroupValue, __thiscall(ECX vfs, const string* group, const string*
// value, int kind) RET 0Ch: lowercase a copy of value; for kind 1 also turn '\' into '/' and append '/'
// when the last byte is not '/'; find the group (00BDC230), select its value list (00BDB2E0); erase the
// first value equal (length, then empty or _stricmp) through 004D0990; when both of the group's value
// lists are then empty (+18h, +24h of the node) erase the group (00BE1AC0).
void vfs_unregister_search_group_value_00be1b90(void* vfs, const NativeString& group, const NativeString& value,
                                                std::int32_t kind, VfsPathContext& context);
// 00BE2210 BSP_VFS_UnregisterSearchPrefix_Unreferenced, __thiscall(ECX vfs, group, const string* path)
// RET 8: lowercase, slash-normalize and terminate a copy as above, then 00BE1B90 (vfs, group, copy, 1).
void vfs_unregister_search_prefix_00be2210(void* vfs, const NativeString& group, const NativeString& path,
                                           VfsPathContext& context);

// 00BDBA50 BSP_VFS_FindMountByNameAndPath_Unreferenced, __thiscall(ECX vfs, const string* name, const
// string* path) RET 8 -> EAX provider or 0: canonicalize path (00BEE390), trim its right end with the set
// at 00CE7898 (00584110; passed in); walk the mount tree (+3Ch) for the first node whose provider (node
// +18h) name (provider +8h/+0Ch) equals name and whose mount path (node +10h/+14h) equals the trimmed
// path (equal length, then empty or _stricmp); release the copy.
void* vfs_find_mount_by_name_and_path_00bdba50(void* vfs, const NativeString& name, const NativeString& path,
                                               const char* trim_set_00ce7898, VfsPathContext& context);
// 00BDC3F0 BSP_VFS_FindExtensionPrefixPair, __thiscall(ECX vfs, it* result, const string* extension, const
// string* prefix) RET 0Ch: lowercase copies of both; slash-normalize and terminate the prefix as
// 00BE2210 does; over [lower bound 00BDA260, upper bound 00BDA2C0) of the extension in the tree at +54h,
// the first node whose value string (+14h) equals the prefix (00435C40) is the result; otherwise the head.
NativeIterator& vfs_find_extension_prefix_pair_00bdc3f0(void* vfs, NativeIterator& result, const NativeString& extension,
                                                        const NativeString& prefix, VfsPathContext& context);
// 00BE0AE0 BSP_FileBlockScope_Construct, __thiscall(ECX scope, const string* name, arg) RET 8 -> EAX:
// ref-counted base (00CEB130, count 1), vtable 00D68494, +8h..+10h = 0, name copy at +14h; 00BDF950
// (scope); 00BE0980 (vfs 0109CEEC, &name, arg).
void* fileblock_scope_construct_00be0ae0(void* scope, const NativeString& name, std::uint32_t arg, void* vfs_0109ceec,
                                         NativeStringStorage& strings, EngineServiceCalls& calls);
// 00BE5BB0 BSP_FileStorePendingTree_EqualRange, __thiscall(ECX tree, pair* result, key) RET 8: hi = 00BE5630,
// lo = 00BE5530; result = {{tree, lo}, {tree, hi}}.
void filestore_pending_equal_range_00be5bb0(NativeContainerHeader& tree, NativeIterator (&result)[2], const void* key,
                                            EngineServiceCalls& calls);

// Search-group list (nodes 28h: next, prev, payload 20h = name string, two pooled string lists).
// 00BE10A0 BSP_VfsSearchGroupPayload_Construct, __thiscall(ECX, const string*) RET 4: name copy, then
// two list heads from 004C3020 at +0Ch and +18h with sizes +10h / +1Ch = 0.
void* construct_search_group_payload_00be10a0(void* payload, const NativeString& name, NativeStringStorage& strings,
                                              EngineServiceCalls& calls);
// 00BE13E0 BSP_VfsSearchGroupPayload_CopyConstruct: name copy, then 004D48A0 on (+8h, src+8h) and
// (+14h, src+14h).
void* copy_search_group_payload_00be13e0(void* payload, const void* source, NativeStringStorage& strings,
                                         EngineServiceCalls& calls);
// 00BE1650 (__fastcall ECX place, EDX source) and 00BE1970 (__cdecl place, source): copy-construct at a
// non-null place.
void construct_search_group_payload_at(void* place, const void* source, NativeStringStorage& strings,
                                       EngineServiceCalls& calls);
// 00BE1A20 BSP_VfsSearchGroupList_BuyNode, __cdecl(next, prev, const payload*) -> EAX node: operator
// new(28h); store next and prev (each tested for a null address); copy the payload at +8h.
void* buy_search_group_node_00be1a20(void* next, void* prev, const void* payload, NativeStringStorage& strings,
                                     EngineServiceCalls& calls);
// 00BE1AC0 BSP_VfsSearchGroupList_EraseNode, __thiscall(ECX list, it* result, it where) RET 0Ch.
void erase_search_group_node_00be1ac0(NativeContainerHeader& list, NativeIterator& result, NativeIterator where,
                                      EngineServiceCalls& calls);
// 00BE1B70 BSP_VfsSearchGroupList_Destruct_Unreferenced: 00BE19E0, free the head, clear the head pointer.
void destruct_search_group_list_00be1b70(NativeContainerHeader& list, EngineServiceCalls& calls);
// 00BE1D80 BSP_VfsSearchGroupList_PushFront_Unreferenced, __thiscall(ECX list, payload*) RET 4: node =
// BuyNode(first, first->prev, payload); 00BDED80 (list, 1); first->prev = node; node->prev->next = node.
void push_front_search_group_00be1d80(NativeContainerHeader& list, const void* payload,
                                      NativeStringStorage& strings, EngineServiceCalls& calls);

// Extension-prefix tree: nodes 20h {left, parent, right, string pair +0Ch, colour +1Ch, isnil +1Dh}.
struct PrefixTreeNode {
    PrefixTreeNode* left;
    PrefixTreeNode* parent;
    PrefixTreeNode* right;
    NativeString key_0c;
    NativeString value_14;
    std::uint8_t color_1c;
    std::uint8_t isnil_1d;
    std::uint8_t pad_1e[2];
};
// 00BDF700 BSP_VfsExtensionPrefixTree_BuyNode, __cdecl(left, parent, right, const pair*, BYTE colour) ->
// EAX: operator new(20h); links; copy the pair (0048E500 body); colour; isnil 0.
PrefixTreeNode* buy_prefix_tree_node_00bdf700(PrefixTreeNode* left, PrefixTreeNode* parent, PrefixTreeNode* right,
                                              const void* pair, std::uint8_t color, NativeStringStorage& strings);
// 00BE0370 BSP_VfsExtensionPrefixTree_LinkNewNode, __thiscall(ECX tree, it* result, BYTE add_left,
// node* where, const pair*) RET 10h: size 0FFFFFFEh or more throws length_error("map/set<T> too
// long"); buy a red node under where, link it as the only, left or right child (updating head left/right
// extremes), size++, rebalance (the rotations at 00BDA090 / 00BD93E0 and one inline left rotation), root
// black; result = {tree, node}.
void prefix_tree_link_new_node_00be0370(NativeContainerHeader& tree, NativeIterator& result, bool add_left,
                                        PrefixTreeNode* where, const void* pair, NativeStringStorage& strings);
// 00BE0B90 BSP_VfsExtensionPrefixTree_InsertEqual, __thiscall(ECX tree, result* {it, BYTE}, const pair*)
// RET 8: descend comparing the key with each node key (empty key: -(node non-empty); empty node: 1;
// else _stricmp(key, node)), left when negative; link under the last node; result = {it, true}.
struct InsertResult {
    NativeIterator it;
    std::uint8_t inserted;
};
void prefix_tree_insert_equal_00be0b90(NativeContainerHeader& tree, InsertResult& result, const void* pair,
                                       NativeStringStorage& strings);

// ---------------------------------------------------------------------------------------------------
// Singletons and platform bases. The lifetime manager's section (00415350()->+10h) is taken through the
// domain; published words are passed in.

// 00BE2960 BSP_PlatformBase_RegisterSingleton, __fastcall(ECX) -> EAX: vtable 00D68604; under the manager
// lock publish 0109CF04 = object and register it (00BD0C30).
void* platform_base_register_00be2960(void* object, void*& published_0109cf04, SingletonLifetimeDomain& domain);
// 00BE2A00 BSP_PlatformBase_UnregisterSingleton, __fastcall(ECX): vtable 00D68604; under the lock
// unregister the published object (00BCFCA0) and clear 0109CF04; vtable 00CE3818.
void platform_base_unregister_00be2a00(void* object, void*& published_0109cf04, SingletonLifetimeDomain& domain);
// 00BE2AA0 BSP_PlatformBase_ScalarDeletingDestructor.
void* platform_base_scalar_delete_00be2aa0(void* object, std::uint32_t flags, void*& published_0109cf04,
                                           SingletonLifetimeDomain& domain);
// 00BEA9A0 BSP_Platform_DeletingDestructorSlot1, __thiscall(ECX, flags) RET 4: store 00D68BC8, then the
// 00BEA8B0 body (vtable 00D68BC4; under the lock unregister the object published at 00E1AE90 and clear
// it; vtable 00CE3818), then free when flags & 1. 00BEA8B0 itself is reconstructed in
// src/native_render_service_base.cpp (retire_native_application_singleton_base_00bea8b0) over a different
// context; its steps are repeated inline here rather than reconstructed twice as an entry point.
void* platform_deleting_destructor_00bea9a0(void* object, std::uint32_t flags, void*& published_00e1ae90,
                                            SingletonLifetimeDomain& domain);
// 00BEDFA0 BSP_FrameClockBase_DestroyThunk: vtable 00D68D20; under the lock unregister 01090AB0 and clear
// it; vtable 00CE3818.
void frame_clock_base_destroy_00bedfa0(void* object, void*& published_01090ab0, SingletonLifetimeDomain& domain);
// 00BEDF60 BSP_FrameClockVariant_Construct, __fastcall(ECX) -> EAX: 00BEDE00, vtable 00D68D24, +4h = 0.
void* frame_clock_variant_construct_00bedf60(void* object, EngineServiceCalls& calls);
// 00BE28B0 BSP_AllocationStats_DerivedConstruct_Unreferenced: 00BE2750, vtable 00D685E4.
void* allocation_stats_derived_construct_00be28b0(void* object, EngineServiceCalls& calls);
// 00BEA800 BSP_Platform_RunLoopDispatch: the published platform's (0109CF04) vtable +24h, ECX = platform.
void platform_run_loop_dispatch_00bea800(void* platform_0109cf04);
// 00BEC3B0 BSP_Win32Platform_WindowProcThunk, __stdcall(HWND, UINT, WPARAM, LPARAM) RET 10h: the published
// platform's vtable +28h as a __stdcall (platform, hwnd, message, wparam, lparam); ECX is left holding the
// vtable pointer.
std::int32_t win32_platform_window_proc_00bec3b0(void* platform_0109cf04, void* window, std::uint32_t message,
                                                 std::uint32_t wparam, std::int32_t lparam);

// Window platform base (fields: +4h, +8h DWORDs; +0Ch windowed? BYTE; +0Dh wide BYTE; +10h aspect float;
// +14h 1280x720 BYTE; +18h bit depth; +1Ch, +20h; +24h width; +28h height; +2Ch BYTE; +30h HWND;
// +3Ch fullscreen aspect float).
// 00BE2AC0 BSP_WindowPlatformBase_Construct, __fastcall(ECX) -> EAX: 00BE2960; vtable 00D68608; clear the
// fields; depth 20h; width 280h, height 1E0h; aspect = FILD [+24h] / FIDIV [+28h] taken from the
// PREVIOUS (uninitialized) width and height before those stores, then FSTP to +10h.
void* window_platform_base_construct_00be2ac0(void* object, void*& published_0109cf04, SingletonLifetimeDomain& domain);
// 00BEBFA0 BSP_Win32Platform_ApplyWindowMode, __thiscall(ECX, BYTE fullscreen, width, height) RET 0Ch:
// store the size and mode; aspect = width / height (windowed) or +3Ch (fullscreen); wide = aspect >
// 00D5BD98 (passed in); +14h = width 500h and height 2D0h. Windowed: AdjustWindowRect(WS_CAPTION),
// style C00000h, exstyle 300h, SetWindowPos(HWND_NOTOPMOST, current origin, adjusted size, 20h),
// ShowWindow 5, RedrawWindow(desktop, 181h), focus. Fullscreen: style 80000000h, exstyle 8,
// SetWindowPos(HWND_TOPMOST, 0, 0, size, 20h), ShowWindow 5, focus.
void win32_platform_apply_window_mode_00bebfa0(void* platform, bool fullscreen, std::int32_t width,
                                               std::int32_t height, float wide_threshold_00d5bd98);
// 00BECA00 BSP_Win32Platform_Destroy: vtable 00D68CC4; SystemParametersInfoA(11h, 1, 0, 0); 00BEC730 on
// the queue at +174h; tail jump to 00BE2B10. 00BECE30 BSP_Win32Platform_ScalarDelete adds the flag-1 free.
void win32_platform_destroy_00beca00(void* platform, EngineServiceCalls& calls);
void* win32_platform_scalar_delete_00bece30(void* platform, std::uint32_t flags, EngineServiceCalls& calls);

// Profiler (2Ch bytes): vtable, +4h, +8h, +0Ch, +10h BYTE, then six arrays +14h..+28h and +2Ch.
struct ProfilerSlotColours {
    std::uint32_t slot_count_00e15118;      // the profiled slot count; the arrays hold count + 32h
    std::uint32_t word_00d7a2f0;            // copied to +4h
    std::uint32_t word_00d68680;            // copied to +8h
    // Slot indices that get a fixed ARGB colour (each an image global, in listing order).
    std::uint32_t indices[17];
};
// 00BE3820 BSP_Profiler_Construct, __fastcall(ECX) -> EAX: vtable 00D6867C; n = count + 32h; +14h = n
// zeroed 28h-byte records (vector constructor over 0053B9E0), +18h and +1Ch = n * 50h bytes, +20h and
// +24h = n * 4, +28h = n * 1Ch (operator new; 64-bit size overflow saturates to 0FFFFFFFFh); fields;
// 00BE3380; every colour = FFFFFF0Fh; then the 17 listed slots get their colours.
void* profiler_construct_00be3820(void* object, const ProfilerSlotColours& colours, EngineServiceCalls& calls);
// 00BE37A0 BSP_Profiler_Destruct: vtable 00D6867C; free the six arrays +14h..+28h; clear 0109DB50;
// vtable 00CE3818. 00BE3800 BSP_Profiler_ScalarDeletingDestructor adds the flag-1 free.
void profiler_destruct_00be37a0(void* object, std::uint32_t& published_0109db50) noexcept;
void* profiler_scalar_delete_00be3800(void* object, std::uint32_t flags, std::uint32_t& published_0109db50) noexcept;

// Debug feature lock (8 bytes: vtable 00D68B68, tracked section +4h) and owner (+4h NativeString array,
// +8h count, +1Ch profile count, +24h any-enabled BYTE, +30h selected profile name pointer).
// 00BE82A0 BSP_DebugFeatureLock_Construct, __fastcall(ECX) -> EAX: vtable; +4h = 00BD1860().
void* debug_feature_lock_construct_00be82a0(void* lock, EngineServiceCalls& calls);
// 00BE8360 BSP_DebugFeatureLock_GetSingleton: publish 0109DB74 under the manager lock (operator new(8) +
// construct, then register 00BD0C30) the first time; EAX the published lock.
void* debug_feature_lock_singleton_00be8360(void*& published_0109db74, SingletonLifetimeDomain& domain,
                                            EngineServiceCalls& calls);
// 00BE8420 BSP_DebugFeatureLock_Destroy: vtable 00D68B68; 0041CC80 on +4h; clear 0109DB74; vtable
// 00CE3818. 00BE8450 BSP_DebugFeatureLock_ScalarDelete adds the flag-1 free.
void debug_feature_lock_destroy_00be8420(void* lock, void*& published_0109db74, EngineServiceCalls& calls);
void* debug_feature_lock_scalar_delete_00be8450(void* lock, std::uint32_t flags, void*& published_0109db74,
                                                EngineServiceCalls& calls);
// Owner reads under the singleton lock's section (00BE8360 first):
// 00BE84A0 HasFeature, __thiscall(ECX, const string*) RET 4 -> AL: 005EFBA0 on +4h >= 0.
// 00BE84F0 GetFeatureName, __thiscall(ECX, unsigned i) RET 4 -> EAX +4h data + min(i, count - 1) * 8
//          (unsigned clamp; count 0 wraps).
// 00BE8540 GetFeatureCount -> EAX +8h. 00BE8580 IsAnyFeatureEnabled -> AL +24h.
// 00BE8720 DisableFeature, __thiscall(ECX, const string*) RET 4: 00BE8680 (name, 0).
struct DebugFeatureContext {
    void*& published_0109db74;
    SingletonLifetimeDomain& domain;
    EngineServiceCalls& calls;
};
bool debug_feature_has_00be84a0(const void* owner, const NativeString& name, DebugFeatureContext& context);
NativeString* debug_feature_name_00be84f0(const void* owner, std::uint32_t index, DebugFeatureContext& context);
std::uint32_t debug_feature_count_00be8540(const void* owner, DebugFeatureContext& context);
bool debug_feature_any_enabled_00be8580(const void* owner, DebugFeatureContext& context);
void debug_feature_disable_00be8720(void* owner, const NativeString& name, DebugFeatureContext& context);

// ---------------------------------------------------------------------------------------------------
// Structured resource reader and nodes. A node handle holds the node; the node's +8h is the reader and
// +20h the remaining byte count.

// __fastcall(ECX handle, EDX unused) leaves: 00BE99E0 ReadU32Discarded (00BF0280), 00BE9A10 SkipU32
// (00BF02A0). With an out-parameter, RET 4, EAX handle: 00BE9A60 ReadFloat (00BF02C0), 00BE9A80
// ReadU32ToOutParam (00BF0280), 00BE9AA0 ReadU32 (00BF02A0).
void structured_node_read_u32_discarded_00be99e0(void** handle, EngineServiceCalls& calls);
void structured_node_skip_u32_00be9a10(void** handle, EngineServiceCalls& calls);
void** structured_node_read_float_00be9a60(void** handle, float* out, EngineServiceCalls& calls);
void** structured_node_read_u32_slot34_00be9a80(void** handle, std::uint32_t* out, EngineServiceCalls& calls);
void** structured_node_read_u32_00be9aa0(void** handle, std::uint32_t* out, EngineServiceCalls& calls);
// 00BE9B20 BSP_StructuredNode_RemainingBytes, __fastcall(ECX handle) -> EAX: the reader's +64h.
std::uint32_t structured_node_remaining_00be9b20(void* const* handle) noexcept;
// 00BEA030 BSP_StructuredNodeHandle_ReadStringInto, __fastcall(ECX handle, EDX, string* out) RET 4,
// EAX handle: read into a temporary (00BEA010), copy it into *out unless the same object, release it.
void** structured_node_read_string_into_00bea030(void** handle, NativeString& out, NativeStringStorage& strings,
                                                 EngineServiceCalls& calls);
// 00BE9F00 BSP_StructuredReader_DestroyNameStack, __fastcall(ECX): release the ten strings at ECX in
// reverse order (eh vector destructor, 8-byte elements).
void structured_reader_destroy_name_stack_00be9f00(NativeString* names, NativeStringStorage& strings) noexcept;
// 00BEA0D0 BSP_StructuredReader_ConstructWithStream, __fastcall(ECX reader, EDX, stream) RET 4, EAX:
// 00BF09A0; zero the ten name headers at +10h; +60h depth 0, +64h FFFFFFFFh, +68h, +6Ch 0; 00BF0430 (stream).
void* structured_reader_construct_00bea0d0(void* reader, void* stream, EngineServiceCalls& calls);
// Structured nodes (vtable 00D68BB4 over the ref-counted 00CEB130 with count 1): +8h reader, +0Ch 0,
// +10h name copy, +18h 0, +1Ch/+20h size; then push the name onto the reader's name stack (+10h +
// depth * 8, depth++) by a string copy.
// 00BEA4B0 ConstructReadingSize, __fastcall(ECX, EDX, reader, const string* name) RET 8: the size is read
// with 00BF0280 against a local count.
// 00BEA5A0 ConstructWithSize, __thiscall(ECX, reader, const string* name, size) RET 0Ch.
void* structured_node_construct_reading_size_00bea4b0(void* node, void* reader, const NativeString& name,
                                                      NativeStringStorage& strings, EngineServiceCalls& calls);
void* structured_node_construct_with_size_00bea5a0(void* node, void* reader, const NativeString& name,
                                                   std::uint32_t size, NativeStringStorage& strings);

// ---------------------------------------------------------------------------------------------------
// Text scanner (828h bytes: token buffer +1h, saved token +401h, pending flag +801h).
// 00BEED60 BSP_TextScanner_AcceptKeyword, __thiscall(ECX, const char*) RET 4 -> AL: peek (00BEE8E0); on a
// _stricmp match copy the string at +1h to +401h, clear +801h, AL 1; otherwise AL 0.
// 00BEEF60 BSP_TextScanner_ExpectKeyword: the same, and a miss also runs 00BEEDB0.
// 00BEF0B0 BSP_TextScanner_ReadBool, __thiscall(ECX, char* out) RET 4 -> AL: 00BEEC40 (out); a non-empty
// out consumes the token as above and returns its result; otherwise 00BEEDB0, *out = 0, AL 0.
bool text_scanner_accept_keyword_00beed60(void* scanner, const char* keyword, EngineServiceCalls& calls);
bool text_scanner_expect_keyword_00beef60(void* scanner, const char* keyword, EngineServiceCalls& calls);
bool text_scanner_read_bool_00bef0b0(void* scanner, char* out, EngineServiceCalls& calls);

// ---------------------------------------------------------------------------------------------------
// RAM images and physical streams.

// 00BEFC40 BSP_RamImage_Destroy: vtable 00D68D88; free and clear +18h when non-null; 00BD30F0.
// 00BEFC70 BSP_RamImage_ScalarDelete adds the flag-1 free.
void ram_image_destroy_00befc40(void* image, EngineServiceCalls& calls);
void* ram_image_scalar_delete_00befc70(void* image, std::uint32_t flags, EngineServiceCalls& calls);
// 00BEFFA0 BSP_RamImage_LoadTgaFile, __fastcall(ECX const string* path) -> EAX: temporary = 0041E870(path
// data, or the empty buffer 0109DBB8); stream = vfs vtable +4h (temporary, 2); release the temporary;
// result = 00BEFDC0(stream); release the stream reference WITHOUT a null test; return result.
std::uint32_t ram_image_load_tga_00beffa0(const NativeString& path, void* vfs_0109ceec,
                                          const char* empty_0109dbb8, NativeStringStorage& strings,
                                          EngineServiceCalls& calls);
// 00BF2F30 BSP_PhysicalProviderBlock_Destroy_Provisional, __fastcall(ECX): free +0Ch; free and clear +4h
// when non-null; CloseHandle(+0h).
void physical_provider_block_destroy_00bf2f30(void* block) noexcept;
// 00BF4F00 BSP_PhysicalStream_CloseHandle: CloseHandle(+8h); +8h = INVALID_HANDLE_VALUE.
void physical_stream_close_handle_00bf4f00(void* stream) noexcept;
// 00BF4FC0 BSP_PhysicalStream_Destroy: vtable 00D691B0; close as above; vtable 00D5C104; 00BD30F0.
void physical_stream_destroy_00bf4fc0(void* stream, EngineServiceCalls& calls);
// 00BF5520 BSP_PhysicalStream_ConstructAndOpen, __thiscall(ECX, name, mode) RET 8 -> EAX: ref-counted
// base (00CEB130, count 1), +10h..+1Ch = 0, vtable 00D691B0, handle -1, then 00BF52A0 (name, mode).
void* physical_stream_construct_and_open_00bf5520(void* stream, std::uint32_t name, std::uint32_t mode,
                                                  EngineServiceCalls& calls);

// ---------------------------------------------------------------------------------------------------
// PC save storage (root path string at +530h, data +534h; search pattern string at +538h/+53Ch).

// 00BEAA80 BSP_PcStorage_GetZeroPair, __cdecl(DWORD*, DWORD*): both 0.
void pc_storage_zero_pair_00beaa80(std::uint32_t* a, std::uint32_t* b) noexcept;
// 00BEAB30 BSP_PcStorage_GetRootPath, __fastcall(ECX) -> EAX: +534h, or the empty buffer 0109DB8C.
const char* pc_storage_root_path_00beab30(const void* storage, const char* empty_0109db8c) noexcept;
// 00BEAE60 BSP_PcStorage_DeleteSlotFile, __thiscall(ECX, const string* slot, int file) RET 8:
// sprintf("%s\\%s\\%s", root, slot, file name table 00E15324[file]); DeleteFileA; when the file can still
// be opened for reading set +20h.
void pc_storage_delete_slot_file_00beae60(void* storage, const NativeString& slot, const char* file_name,
                                          const char* empty_0109db8c);
// 00BEB0C0 BSP_PcStorage_GetFileWriteTime, __thiscall(ECX, FILETIME* out, const string* slot, const char*
// file) RET 0Ch: the last-write time, or 0:0 when the file cannot be opened or GetFileTime fails.
void pc_storage_file_write_time_00beb0c0(const void* storage, std::uint32_t* out, const NativeString& slot,
                                         const char* file, const char* empty_0109db8c);
// 00BEB190 BSP_PcStorage_CanCreateSlot_Provisional, __thiscall(ECX, slot) RET 4: vtable +18h (ECX
// storage); a zero AL returns that EAX unchanged; otherwise !00BEB040(slot, "valid" 00E15320 entry).
std::uint32_t pc_storage_can_create_slot_00beb190(void* storage, std::uint32_t slot, const char* valid_name,
                                                  EngineServiceCalls& calls);
// 00BEB220 BSP_PcStorage_Destroy: vtable 00D68C10; release the strings at +538h and +530h; 00BD4270.
// 00BEB610 BSP_PcStorage_ScalarDelete adds the flag-1 free.
void pc_storage_destroy_00beb220(void* storage, NativeStringStorage& strings, EngineServiceCalls& calls);
void* pc_storage_scalar_delete_00beb610(void* storage, std::uint32_t flags, NativeStringStorage& strings,
                                        EngineServiceCalls& calls);
// 00BEB810 BSP_PcStorage_HasFreeSpace, __fastcall(ECX) -> EAX 0/1: FindFirstFileA(pattern +53Ch); each
// found name is copied into a string and passed to vtable +20h; then GetDiskFreeSpaceExA(root +534h)
// must succeed with at least 100000h bytes available to the caller.
std::uint32_t pc_storage_has_free_space_00beb810(void* storage, const char* empty_0109db8c,
                                                 NativeStringStorage& strings);
// 00BEBD90 BSP_PcStorage_EnumerateFiles, __thiscall(ECX, vector*) RET 4: the same scan; names for which
// vtable +20h answers non-zero are appended to the vector (00450540).
void pc_storage_enumerate_files_00bebd90(void* storage, void* vector, const char* empty_0109db8c,
                                         NativeStringStorage& strings, EngineServiceCalls& calls);

// ---------------------------------------------------------------------------------------------------
// Error reporter (vtable 00D68E60, tracked section +4h, show-message BYTE +8h, map<thread id, string
// vector> header +0Ch) and its map nodes.
struct ErrorContextNode {
    ErrorContextNode* left;
    ErrorContextNode* parent;
    ErrorContextNode* right;
    std::uint32_t thread_0c;
    NativeStringVectorStorage contexts_10;
    std::uint8_t color_1c;  // 1 = black
    std::uint8_t isnil_1d;
    std::uint8_t pad_1e[2];
};
// The map value as built on the stack: {thread, vector}.
struct ErrorContextValue {
    std::uint32_t thread;
    NativeStringVectorStorage contexts;
};
// 00BF1400 BSP_ErrorContextMap_Maximum / 00BF1420 Minimum, __fastcall(ECX node) -> EAX: follow right / left
// while the child's isnil is 0 (the listing starts from node->right / node->left).
ErrorContextNode* error_context_max_00bf1400(ErrorContextNode* node) noexcept;
ErrorContextNode* error_context_min_00bf1420(ErrorContextNode* node) noexcept;
// 00BF15E0 RotateLeft / 00BF1630 RotateRight, __thiscall(ECX map, node*) RET 4.
void error_context_rotate_left_00bf15e0(NativeContainerHeader& map, ErrorContextNode* where) noexcept;
void error_context_rotate_right_00bf1630(NativeContainerHeader& map, ErrorContextNode* where) noexcept;
// 00BF1450 Advance / 00BF16B0 Retreat, __fastcall(ECX it): in-order successor / predecessor with the
// 00BF6713 validations (null container; advancing the head; retreating to the head).
void error_context_advance_00bf1450(NativeIterator& it, InvalidParameterSink& sink);
void error_context_retreat_00bf16b0(NativeIterator& it, InvalidParameterSink& sink);
// 00BF1750 / 00BF1880 Increment and 00BF1830 / 00BF18C0 Decrement: the pre-step wrappers.
NativeIterator& error_context_increment(NativeIterator& it, InvalidParameterSink& sink);
NativeIterator& error_context_decrement(NativeIterator& it, InvalidParameterSink& sink);
// 00BF18E0 AllocateHeadNode: operator new(20h); zero the links (each tested for a null address); colour 1,
// isnil 0. A non-null allocation is a precondition (operator new throws).
ErrorContextNode* error_context_allocate_head_00bf18e0();
// 00BF19B0 InitializeEmpty, 00BF1A50 / 00BF2C10 Construct (EAX map): the tree-header shape with isnil +1Dh.
NativeContainerHeader& error_context_construct_00bf1a50(NativeContainerHeader& map);
// 00BF19E0 FindThread, __thiscall(ECX map, it* result, const DWORD* thread) RET 8: lower bound by unsigned
// thread id; the head when the bound is the head or its key is greater.
void error_context_find_00bf19e0(NativeContainerHeader& map, NativeIterator& result, const std::uint32_t* thread);
// 00BF2120 ConstructNode, __thiscall(ECX node, left, parent, right, const value*, BYTE colour) RET 14h;
// 00BF21A0 BuyNode, __cdecl(left, parent, right, const value*, colour) -> EAX: operator new(20h), then
// 00BF2120 on a non-null result.
ErrorContextNode* error_context_construct_node_00bf2120(ErrorContextNode* node, ErrorContextNode* left,
    ErrorContextNode* parent, ErrorContextNode* right, const ErrorContextValue& value, std::uint8_t color,
    NativeStringStorage& strings);
ErrorContextNode* error_context_buy_node_00bf21a0(ErrorContextNode* left, ErrorContextNode* parent,
    ErrorContextNode* right, const ErrorContextValue& value, std::uint8_t color, NativeStringStorage& strings);
// 00BF2230 BSP_ErrorContextNodeValue_ScalarDelete, __thiscall(ECX, flags) RET 4: resize the vector at
// +10h to 0 (00427110), free its data (00BF6989), free the object when flags & 1. The vector offset is the
// node layout's, so ECX is a node.
void* error_context_node_scalar_delete_00bf2230(void* node, std::uint32_t flags, NativeStringStorage& strings);
// 00BF20B0 / 00BF20D0 BSP_ErrorContextList_DestroyForUnwind*, __fastcall(ECX value): resize the vector
// at +4h to 0, free its data.
void error_context_value_destroy(ErrorContextValue& value, NativeStringStorage& strings);
// 00BF2270 InsertAt, __thiscall(ECX map, it* result, BYTE add_left, node* where, const value*) RET 10h:
// the LinkNewNode shape over these nodes (rotations 00BF15E0 / 00BF1630).
void error_context_insert_at_00bf2270(NativeContainerHeader& map, NativeIterator& result, bool add_left,
                                      ErrorContextNode* where, const ErrorContextValue& value,
                                      NativeStringStorage& strings);
// 00BF2480 InsertUnique, __thiscall(ECX map, result* {it, BYTE}, const value*) RET 8.
void error_context_insert_unique_00bf2480(NativeContainerHeader& map, InsertResult& result,
                                          const ErrorContextValue& value, NativeStringStorage& strings,
                                          InvalidParameterSink& sink);
// 00BF26C0 EraseIterator, __thiscall(ECX map, it* result, it where) RET 0Ch: an isnil where throws
// out_of_range("invalid map/set<T> iterator"); advance where (00BF1450); unlink and rebalance; destroy
// the node's vector, free the node; size-- when non-zero; result = the advanced where.
void error_context_erase_00bf26c0(NativeContainerHeader& map, NativeIterator& result, NativeIterator where,
                                  NativeStringStorage& strings, InvalidParameterSink& sink);
// 00BF2980 EraseSubtree, __cdecl(node*): recursive right, iterative left; destroy vectors, free nodes.
void error_context_erase_subtree_00bf2980(ErrorContextNode* node, NativeStringStorage& strings);
// 00BF29D0 Clear.
void error_context_clear_00bf29d0(NativeContainerHeader& map, NativeStringStorage& strings);
// 00BF2A00 EraseRange, __thiscall(ECX map, it* result, it first, it last) RET 14h: [begin, end) clears;
// otherwise erase one at a time.
void error_context_erase_range_00bf2a00(NativeContainerHeader& map, NativeIterator& result, NativeIterator first,
                                        NativeIterator last, NativeStringStorage& strings,
                                        InvalidParameterSink& sink);
// 00BF2AD0 / 00BF2B10 Destroy and 00BF2B50 DestroyForUnwind: erase all, free the head, clear head/size.
void error_context_destroy(NativeContainerHeader& map, NativeStringStorage& strings, InvalidParameterSink& sink);

struct ErrorReporterContext {
    NativeStringStorage& strings;
    EngineServiceCalls& calls;
    std::uint32_t& published_0109dbdc;
    const char* empty_0109dbe0;
};
// 00BF2C40 BSP_ErrorReporter_Construct, __fastcall(ECX) -> EAX: vtable 00D68E60, +8h = 1, empty map at
// +0Ch, +4h = 00BD1860().
void* error_reporter_construct_00bf2c40(void* reporter, EngineServiceCalls& calls);
// 00BF2B90 BSP_ErrorReporter_Destroy: vtable; 0041CC80 on +4h; erase the map, free its head; clear
// 0109DBDC; vtable 00CE3818. 00BF2CC0 BSP_ErrorReporter_ScalarDelete adds the flag-1 free.
void error_reporter_destroy_00bf2b90(void* reporter, ErrorReporterContext& context);
void* error_reporter_scalar_delete_00bf2cc0(void* reporter, std::uint32_t flags, ErrorReporterContext& context);
// 00BF2560 BSP_ErrorReporter_PushContext, __thiscall(ECX, const string*) RET 4: under the section,
// insert-unique {GetCurrentThreadId(), empty vector} and append the string to that thread's vector.
void error_reporter_push_context_00bf2560(void* reporter, const NativeString& text, ErrorReporterContext& context);
// 00BF1A80 BSP_ErrorReporter_PopContext, __fastcall(ECX): under the section, find this thread's entry
// (validated) and, when its vector is non-empty, release the last string and count--.
void error_reporter_pop_context_00bf1a80(void* reporter, ErrorReporterContext& context);
// 00BF1F50 BSP_ErrorReporter_Fail, __thiscall(ECX, const string*) RET 4: compose (00BF1B50); when +8h is
// set, MessageBoxA(0, text, "Failure", MB_ICONHAND); release.
void error_reporter_fail_00bf1f50(void* reporter, const NativeString& text, ErrorReporterContext& context);
// 00BF1FB0 BSP_ErrorReporter_FailUnknownAssertion, __thiscall(ECX): Fail("Unknown assertion failed").
void error_reporter_fail_unknown_00bf1fb0(void* reporter, ErrorReporterContext& context);

} // namespace bsp::engine_services_cc13
