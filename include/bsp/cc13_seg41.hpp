#pragma once
// Segment 41 (00735F30-00758EB0) standalone reconstructions, cc13-seg41.
// Names are hypotheses, not recovered symbols. Field names describe offset and
// storage. Nothing here is wired into the game build; every function is a new
// C++ interface (not ABI-compatible), build-tested only. Evidence, original
// ABI and uncertainty per function: docs/CC13_SEG41.md and the comments below.
//
// Calls the image makes to code outside a function's own body go through
// named host hooks, invoked at the point the image makes the call. Vtable
// stores are modelled as stores of the image address of the table.
#include <cstddef>
#include <cstdint>

#include "bsp/engine_seg4_6_9_cc13.hpp"  // LazySingletonGetterRow / lazy_singleton_get / _destroy

namespace bsp::cc13_seg41 {

using cc13_sys::LazySingletonDestroyHost;
using cc13_sys::LazySingletonGetterRow;
using cc13_sys::LazySingletonHost;
using cc13_sys::lazy_singleton_destroy;
using cc13_sys::lazy_singleton_get;

// ---------------------------------------------------------------------------
// Containers. {data +0, size +4, capacity +8}, signed counts as the image
// compares them (JGE/JL). The element type is one dword.
// ---------------------------------------------------------------------------
struct DwordVector {
    std::uint32_t* data_00;
    std::int32_t size_04;
    std::int32_t capacity_08;
};

struct VectorMemory {
    virtual ~VectorMemory() = default;
    virtual void* operator_new_00bf55be(std::size_t bytes) = 0;
    virtual void free_00bf6989(void* block) = 0;
};

// Reserve, __thiscall(ECX vector, int n) RET 4: n < 1 -> 1; when capacity < n
// allocate n dwords (00BF55BE), copy size elements (the store is skipped for a
// null slot pointer, as the image tests the destination), free the old block
// (00BF6989) and store data/capacity. 00735F90, 0073E8A0, 0073E940, 0073EAC0
// are byte-identical template instances.
void reserve_00735f90(DwordVector& v, std::int32_t n, VectorMemory& memory);
void reserve_0073e8a0(DwordVector& v, std::int32_t n, VectorMemory& memory);
void reserve_0073e940(DwordVector& v, std::int32_t n, VectorMemory& memory);
void reserve_0073eac0(DwordVector& v, std::int32_t n, VectorMemory& memory);

// Resize, __thiscall(ECX vector, int n) RET 4: reserve when capacity < n
// (the paired reserve below), zero-fill [size, n), count size down to n, then
// size = n. Pairs: 00737340->00735F90, 0073EA40->0073E940, 0073FC70->0073E8A0,
// 0073FDA0->0073EAC0. 0073FD00 (decal records) calls reserve 0073E9A0, which
// lies outside the pool and is a host hook.
void resize_00737340(DwordVector& v, std::int32_t n, VectorMemory& memory);
void resize_0073ea40(DwordVector& v, std::int32_t n, VectorMemory& memory);
void resize_0073fc70(DwordVector& v, std::int32_t n, VectorMemory& memory);
void resize_0073fda0(DwordVector& v, std::int32_t n, VectorMemory& memory);
struct DecalRecordReserve {
    virtual ~DecalRecordReserve() = default;
    virtual void reserve_0073e9a0(DwordVector& v, std::int32_t n) = 0;
};
void resize_0073fd00(DwordVector& v, std::int32_t n, DecalRecordReserve& reserve);

// PushBack, __thiscall(ECX vector, const dword* value) RET 4: when size ==
// capacity reserve 2*capacity (values below 2 become 1), store *value unless
// the slot pointer is null, ++size. 00736460 grows through 00735EC0 (outside
// the pool: host hook), 0073FA30 through 0073E8A0, 0073FBB0 through 0073E940,
// 0073FBF0 through 0073E9A0 (host hook).
struct AppPointerReserve {
    virtual ~AppPointerReserve() = default;
    virtual void reserve_00735ec0(DwordVector& v, std::int32_t n) = 0;
};
void push_back_00736460(DwordVector& v, const std::uint32_t* value, AppPointerReserve& reserve);
void push_back_0073fa30(DwordVector& v, const std::uint32_t* value, VectorMemory& memory);
void push_back_0073fbb0(DwordVector& v, const std::uint32_t* value, VectorMemory& memory);
void push_back_0073fbf0(DwordVector& v, const std::uint32_t* value, DecalRecordReserve& reserve);

// 0073FCC0, __thiscall(ECX vector, int index) RET 4: index -1 is a no-op;
// otherwise the last element moves into index (unless index is the last) and
// size drops by one. No bounds check in the image.
void erase_swap_back_0073fcc0(DwordVector& v, std::int32_t index);

// Destroy, __fastcall(ECX vector) RET: resize(0) then free(data) (00BF6989).
// 00737BD0 -> 00737340, 007402D0 -> 0073FC70, 00740310 / 007403F0 -> 0073FDA0.
// 00737810 resizes through 00735F30 and 007402F0 through 0073FD00; 00735F30 is
// outside the pool (host hook).
void destroy_00737bd0(DwordVector& v, VectorMemory& memory);
void destroy_007402d0(DwordVector& v, VectorMemory& memory);
void destroy_00740310(DwordVector& v, VectorMemory& memory);
void destroy_007403f0(DwordVector& v, VectorMemory& memory);
// 007400A0 -> 0073EA40.
void destroy_007400a0(DwordVector& v, VectorMemory& memory);
struct AppPointerResize {
    virtual ~AppPointerResize() = default;
    virtual void resize_00735f30(DwordVector& v, std::int32_t n) = 0;
};
void destroy_00737810(DwordVector& v, AppPointerResize& resize, VectorMemory& memory);
void destroy_007402f0(DwordVector& v, DecalRecordReserve& reserve, VectorMemory& memory);

// ---------------------------------------------------------------------------
// Intrusive reference pointers: the pointee is {vtable +0, LONG refs +4}.
// ---------------------------------------------------------------------------
struct RefCounted {
    std::uint32_t vtable_00;
    volatile long refs_04;
};
struct RefCountedHost {
    virtual ~RefCountedHost() = default;
    // [00CE2220] InterlockedDecrement(&object->refs_04).
    virtual long interlocked_decrement(volatile long* counter) = 0;
    // vtable slot 0, ECX = object, no stack arguments.
    virtual void release_slot0(RefCounted* object) = 0;
};
// __fastcall(ECX slot): when *slot is set, decrement +4 and call slot 0 at
// zero, then *slot = 0. 00736050, 0073FDF0, 0073FE20, 0073FE50, 0073FE80,
// 0073FEF0, 0073FF20, 007454B0 return the slot in EAX; 007457E0 and 00745910
// are the same body without the return value.
RefCounted** release_ref_00736050(RefCounted** slot, RefCountedHost& host);
void release_ref_void_007457e0(RefCounted** slot, RefCountedHost& host);

// ---------------------------------------------------------------------------
// Singleton lifetime helpers. The lifetime manager (00415350) owns a critical
// section at +10h with a depth counter at section+18h; lock/unlock below are
// "GetManager, read +10h, Enter + ++depth when non-null" and the reverse.
// ---------------------------------------------------------------------------
struct SingletonLifetimeHost {
    virtual ~SingletonLifetimeHost() = default;
    virtual void* lock() = 0;               // 00415350, section = manager+10h; returns it (may be null)
    virtual void unlock(void* section) = 0;  // --depth, LeaveCriticalSection; nothing when null
    virtual void register_00bd0c30(void* object) = 0;    // 00415350 then 00BD0C30(ECX manager)
    virtual void unregister_00bcfca0(void* object) = 0;  // 00415350 then 00BCFCA0(ECX manager)
};

// Register, __fastcall(ECX this) -> this: [this] = vtable, lock, publish this
// into the global, register the (re-read) global, unlock.
//   00736100 vtable 00CFEA54 global 0109CF00
//   007364A0 vtable 00CFEA58 global 00F8C26C
//   00736540 vtable 00CFEA5C global 00F8BC4C
//   007366B0 vtable 00CFEA60 global 00F8AEFC
//   007367F0 vtable 00CFEA64 global 010909B0
std::uint32_t* singleton_register(std::uint32_t* self, std::uint32_t vtable, void*& global,
                                  SingletonLifetimeHost& host);
std::uint32_t* singleton_register_00736100(std::uint32_t* self, void*& global_0109cf00, SingletonLifetimeHost& host);
std::uint32_t* singleton_register_007364a0(std::uint32_t* self, void*& global_00f8c26c, SingletonLifetimeHost& host);
std::uint32_t* singleton_register_00736540(std::uint32_t* self, void*& global_00f8bc4c, SingletonLifetimeHost& host);
std::uint32_t* singleton_register_007366b0(std::uint32_t* self, void*& global_00f8aefc, SingletonLifetimeHost& host);
std::uint32_t* singleton_register_007367f0(std::uint32_t* self, void*& global_010909b0, SingletonLifetimeHost& host);

// Unregister (the holder destructor), __fastcall(ECX this): [this] = vtable,
// lock, unregister the global, global = 0, unlock, [this] = 00CE3818 (the
// singleton base table).
//   007365E0 vtable 00CFEA5C global 00F8BC4C
//   00736750 vtable 00CFEA60 global 00F8AEFC
//   00736890 vtable 00CFEA64 global 010909B0
//   00736F70 vtable 00CFEA68 global 00F88C20
//   00737400 vtable 00CFEA54 global 0109CF00
//   007375E0 vtable 00CFEA58 global 00F8C26C
inline constexpr std::uint32_t kSingletonBaseVtable_00ce3818 = 0x00CE3818u;
void singleton_unregister(std::uint32_t* self, std::uint32_t vtable, void*& global, SingletonLifetimeHost& host);
void singleton_unregister_007365e0(std::uint32_t* self, void*& global_00f8bc4c, SingletonLifetimeHost& host);
void singleton_unregister_00736750(std::uint32_t* self, void*& global_00f8aefc, SingletonLifetimeHost& host);
void singleton_unregister_00736890(std::uint32_t* self, void*& global_010909b0, SingletonLifetimeHost& host);
void singleton_unregister_00736f70(std::uint32_t* self, void*& global_00f88c20, SingletonLifetimeHost& host);
void singleton_unregister_00737400(std::uint32_t* self, void*& global_0109cf00, SingletonLifetimeHost& host);
void singleton_unregister_007375e0(std::uint32_t* self, void*& global_00f8c26c, SingletonLifetimeHost& host);

// Destroy, __cdecl(void) RET: the shared lazy_singleton_destroy body
// (engine_seg4_6_9_cc13, same template as 004C0E40). 007361A0 global
// 00F8BBF8, 00736300 global 00F8BF44, 007369E0 global 01090490.
void singleton_destroy_007361a0(void*& global_00f8bbf8, LazySingletonDestroyHost& host);
void singleton_destroy_00736300(void*& global_00f8bf44, LazySingletonDestroyHost& host);
void singleton_destroy_007369e0(void*& global_01090490, LazySingletonDestroyHost& host);

// 00736D00, __cdecl(void) -> EAX global 010904F8: the shared lazy getter with
// an 8-byte allocation and inline vtables ([+4] 00CFEA04, [+0] 00CFEA2C,
// [+4] 00CFEA28), registering global+4 (null when the allocation failed).
extern const LazySingletonGetterRow kMsarProviderFactoryGetter_00736d00;
void* msar_provider_factory_get_00736d00(void*& global_010904f8, LazySingletonHost& host);

// Holder constructors, __fastcall(ECX this) -> this: call a register helper
// with the same ECX, then store the derived vtable.
//   007376E0 -> 007366B0, vtable 00CFEA70
//   007377B0 -> 007367F0, vtable 00CFEA80
//   00737A00 -> 007367F0, vtable 00CFEAC8
//   00737EE0 -> 007364A0, vtable 00CFEAE8, then +4..+18h = 0 (six dwords)
std::uint32_t* holder_construct_007376e0(std::uint32_t* self, void*& global_00f8aefc, SingletonLifetimeHost& host);
std::uint32_t* holder_construct_007377b0(std::uint32_t* self, void*& global_010909b0, SingletonLifetimeHost& host);
std::uint32_t* holder_construct_00737a00(std::uint32_t* self, void*& global_010909b0, SingletonLifetimeHost& host);
std::uint32_t* holder_construct_00737ee0(std::uint32_t* self, void*& global_00f8c26c, SingletonLifetimeHost& host);

// 00737E20, __fastcall(ECX this) -> this: 00736100 (register into 0109CF00),
// then a temporary native string built from the empty literal 00CE3A0C and
// released: {len 0, data 0}; 0041DD40(&s, 0, 1); when data is set, memcpy
// (len+1 bytes from 00CE3A0C) and return the block to the sized pool
// (00419CC0 then 00BD1510(ECX pool, data, len+1, 1)). Finally [this] =
// 00CFEAE4. The string never escapes; the calls are kept because the image
// makes them.
struct TempStringHost {
    virtual ~TempStringHost() = default;
    // 0041DD40, __thiscall(ECX string {len +0, data +4}, 0, 1).
    virtual void native_string_resize_0041dd40(std::int32_t* len, char** data, std::uint32_t size,
                                               std::uint32_t flag) = 0;
    virtual void memcpy_00bf7680(void* destination, const void* source, std::size_t bytes) = 0;
    virtual void* sized_pool_00419cc0() = 0;
    virtual void sized_pool_return_00bd1510(void* pool, void* block, std::size_t bytes, std::uint32_t flag) = 0;
};
inline constexpr std::uint32_t kApplicationLifetimeVtable_00cfeae4 = 0x00CFEAE4u;
std::uint32_t* application_lifetime_construct_00737e20(std::uint32_t* self, void*& global_0109cf00,
                                                       const char* empty_literal_00ce3a0c,
                                                       SingletonLifetimeHost& lifetime, TempStringHost& strings);

// ---------------------------------------------------------------------------
// Decal manager (singleton 00E1AEA0, vtable 00CFF2A0 over base 00CFF218).
// Win32 layouts; the static_asserts only hold for 4-byte pointers.
// ---------------------------------------------------------------------------
// Decal definition (00740410 constructor, 20h): name string {len +0, data +4},
// +8..+10h untouched by the constructor, vertex format +14h, texture +18h,
// effect +1Ch (the roles come from 00740660 and 0073FF50).
struct DecalDefinition {
    std::int32_t name_len_00;
    char* name_data_04;
    std::uint32_t field_08, field_0c, field_10;
    RefCounted* vertex_format_14;  // "decal.mvfm" via device slot +34h; format dword at +8
    RefCounted* texture_18;        // 00740660 material texture slot 0
    RefCounted* effect_1c;         // 00740660 material effect
};
// Decal record (00740350 constructor, operator new 30h; vtable 00CFF21C over
// the reference-counted base 00CEB130). Owned by a decal list.
struct DecalRecord {
    RefCounted rc;                 // vtable +0, refs +4 (1 after construction)
    std::uint32_t vertex_count_08;
    std::uint32_t vertex_capacity_0c;
    RefCounted* owner_10;          // reference held (00740660 param 1)
    std::uint32_t field_14;        // 00740660 param 2
    DecalDefinition* definition_18;
    void* draw_section_1c;         // 00533FA0 result, referenced
    RefCounted* ref_20;
    std::uint32_t list_alloc_24;   // list object {+24h, head +28h, size +2Ch}
    void* list_head_28;
    std::uint32_t list_size_2c;
};
// Decal list (operator new 14h in 00740FE0; vtable 00CFF230 over 00CEB130):
// records vector {data +8, size +0Ch, capacity +10h}.
struct DecalList {
    RefCounted rc;
    DwordVector records_08;  // DecalRecord* as dwords
};
// Decal manager (vtable 00CFF2A0): lists vector +4, definitions vector +10h.
struct DecalManager {
    std::uint32_t vtable_00;
    DwordVector lists_04;        // DecalList*
    DwordVector definitions_10;  // DecalDefinition*
};
// A native {head-node, size} list as 007402A0/00740140 see it: the owner
// passes ECX = the list object, head pointer at +4 and size at +8; the head
// node is {next +0, prev +4}.
struct NodeList {
    std::uint32_t alloc_00;
    void** head_04;
    std::uint32_t size_08;
};
#if defined(_M_IX86)
static_assert(sizeof(DecalDefinition) == 0x20);
static_assert(sizeof(DecalRecord) == 0x30);
static_assert(sizeof(DecalList) == 0x14);
static_assert(offsetof(DecalManager, definitions_10) == 0x10);
#endif

struct DecalHost : RefCountedHost, DecalRecordReserve {
    virtual long interlocked_increment(volatile long* counter) = 0;  // [00CE221C]
    virtual void* operator_new_00bf681b(std::size_t bytes) = 0;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void free_00bf6989(void* block) = 0;
    virtual void* buy_list_head_node_0073fc50() = 0;
    virtual void destroy_refcounted_base_00bd30f0(void* object) = 0;   // JMP/CALL, ECX = object
    // 00B85B30, __thiscall(ECX chain, link): unlink and release.
    virtual void ref_chain_unlink_00b85b30(RefCounted* chain, RefCounted* link) = 0;
    // 00533FA0 draw section create (EAX), 00B85600(ECX section, 0) set indexed,
    // 00B18780(ECX = 110h) raw material slot, 00B18900(ECX slot, effect)
    // material construct, 00B189F0(ECX material, 0, texture) texture slot,
    // 00B864C0(ECX section, material) set material.
    virtual void* draw_section_create_00533fa0() = 0;
    virtual void draw_section_set_indexed_00b85600(void* section, std::uint32_t value) = 0;
    virtual void* material_alloc_00b18780(std::uint32_t size) = 0;
    virtual RefCounted* material_construct_00b18900(void* slot, RefCounted* effect) = 0;
    virtual void material_set_texture_00b189f0(RefCounted* material, std::uint32_t slot, RefCounted* texture) = 0;
    virtual void draw_section_set_material_00b864c0(void* section, RefCounted* material) = 0;
    // 00B86550(ECX section) clear vertex streams, 00B85B80(ECX section, buffer)
    // append a vertex stream.
    virtual void draw_section_clear_streams_00b86550(void* section) = 0;
    virtual void draw_section_append_stream_00b85b80(void* section, RefCounted* buffer) = 0;
    // Device [00F8D394]: vtable+5Ch create vertex buffer (count, 1, format),
    // vtable+34h load texture (const native string*).
    virtual RefCounted* device_create_vertex_buffer_5c(std::uint32_t count, std::uint32_t one,
                                                       std::uint32_t format) = 0;
    virtual RefCounted* device_load_texture_34(const std::int32_t* native_string) = 0;
    // Vertex buffer slots: +10h lock(0,0,0) -> data, +14h unlock, +20h capacity,
    // +24h vertex declaration (stride at +CCh).
    virtual void* buffer_lock_10(RefCounted* buffer, std::uint32_t a, std::uint32_t b, std::uint32_t c) = 0;
    virtual void buffer_unlock_14(RefCounted* buffer) = 0;
    virtual std::uint32_t buffer_capacity_20(RefCounted* buffer) = 0;
    virtual std::uint32_t buffer_stride_24_cc(RefCounted* buffer) = 0;  // slot +24h then [result+CCh]
    virtual void memcpy_00bf7680(void* destination, const void* source, std::size_t bytes) = 0;
    // Native strings: 0041DD40 resize, 00419CC0 + 00BD1510 return of len+1 bytes.
    virtual void native_string_resize_0041dd40(std::int32_t* string, std::uint32_t size, std::uint32_t flag) = 0;
    virtual void native_string_release_00419cc0_00bd1510(void* data, std::size_t bytes) = 0;
};

// 0073FB10, __fastcall(ECX manager): singleton_unregister with vtable
// 00CFF218 and global 00E1AEA0.
void decal_system_destruct_0073fb10(std::uint32_t* self, void*& global_00e1aea0, SingletonLifetimeHost& host);
// 0073F9C0, __thiscall(ECX table, const {len, data}* key) RET 4: linear scan of
// the pointer array at +10h (count +14h, unsigned); entries are {len, data}.
// Equal lengths, then: both zero -> match; key zero -> mismatch; else
// _stricmp (00BF8BB7 via host). Returns the index or 0xFFFFFFFF.
struct NamedKeyHost {
    virtual ~NamedKeyHost() = default;
    virtual int stricmp(const char* a, const char* b) = 0;
};
struct NamedKeyArray {
    std::uint8_t head_00[0x10];
    std::int32_t** entries_10;  // each entry: {len +0, data +4}
    std::uint32_t count_14;
};
std::uint32_t named_key_index_of_0073f9c0(const NamedKeyArray& table, const std::int32_t* key, NamedKeyHost& host);
// 0073FF50, __stdcall(record, count) RET 8 -> base vertex index.
std::uint32_t decal_batch_ensure_vertex_capacity_0073ff50(DecalRecord& record, std::uint32_t count, DecalHost& host);
// 007400C0, __thiscall(ECX vector, const dword* value) RET 4 -> bool: swap-
// erase the first element equal to *value.
bool erase_value_007400c0(DwordVector& v, const std::uint32_t* value);
// 00740140, __fastcall(ECX list): unlink every node, size 0, free the nodes
// and the head (00BF65AC), head = 0.
void list_clear_00740140(NodeList& list, DecalHost& host);
// 007401B0, __fastcall(ECX definition): release +18h, +1Ch, +14h (in that order), then return
// the name storage (len+1 bytes) when data is set. The decal definition dtor.
void decal_definition_destruct_007401b0(DecalDefinition& d, DecalHost& host);
// 00740270, __thiscall(ECX owner, RefCounted* value) RET 4: erase the value from
// the vector at owner+4 (007400C0), then release the by-value reference.
void life_ref_release_00740270(DwordVector& owner_04, RefCounted* value, DecalHost& host);
// 007402A0, __fastcall(ECX list) -> list: head = 0073FC50(), size = 0.
NodeList* list_construct_007402a0(NodeList& list, DecalHost& host);
// 00740350, __fastcall(ECX record) -> record.
DecalRecord* decal_record_construct_00740350(DecalRecord& r, DecalHost& host);
// 007403C0, __fastcall(ECX record): 00740140 on +24h, then JMP 00BD30F0.
void decal_record_destruct_007403c0(DecalRecord& r, DecalHost& host);
// 00740410, __fastcall(ECX definition) -> definition: zero fields, load
// "decal.mvfm" (00CFF224) through the device slot +34h into +14h.
DecalDefinition* decal_definition_construct_00740410(DecalDefinition& d, const char* literal_00cff224,
                                                     DecalHost& host);
// 007404D0, __fastcall(ECX list): the decal list destructor.
void decal_list_destruct_007404d0(DecalList& list, DecalHost& host, VectorMemory& memory);
// 00740660, __thiscall(ECX list, owner, field_14, definition) RET 0Ch -> record.
DecalRecord* decal_list_create_record_00740660(DecalList& list, RefCounted* owner, std::uint32_t field_14,
                                               DecalDefinition* definition, DecalHost& host, VectorMemory& memory);
// 00740EB0, __fastcall(ECX slot): delete *slot (007401B0 + 00BF65AC), *slot = 0.
void decal_definition_delete_00740eb0(DecalDefinition** slot, DecalHost& host);
// 00740F10, __fastcall(ECX manager): the manager destructor.
void decal_manager_destruct_00740f10(DecalManager& m, void*& global_00e1aea0, DecalHost& host,
                                     VectorMemory& memory, SingletonLifetimeHost& lifetime);
// 00740FE0, __fastcall(ECX manager) -> new list: allocate 14h (00BF681B),
// construct a DecalList inline (null-safe), push it into +4 (0073E940 growth).
DecalList* decal_manager_register_owner_00740fe0(DecalManager& m, DecalHost& host, VectorMemory& memory);

// 0073C9F0, __fastcall(ECX manager): locale table manager destructor.
// Vectors at +4034h and +4024h (free 00BF65AC, three dwords zeroed), the
// name string {len +4018h, data +401Ch}, the string map at +14h (0073C240),
// the string range at +4 {+8 begin, +0Ch end, +10h cap} through 00432050
// (ECX begin, EDX end, push this+4, push this; callee pops), then 007365E0.
struct LocaleManagerHost {
    virtual ~LocaleManagerHost() = default;
    virtual void free_00bf65ac(void* block) = 0;
    virtual void native_string_release_00419cc0_00bd1510(void* data, std::size_t bytes) = 0;
    virtual void string_map_clear_0073c240(void* map) = 0;
    virtual void destroy_string_range_00432050(void* begin, void* end, void* range_owner, void* manager) = 0;
};
void locale_table_manager_destroy_0073c9f0(std::uint8_t* self, void*& global_00f8bc4c, LocaleManagerHost& host,
                                           SingletonLifetimeHost& lifetime);
// 0073E6B0, __stdcall(void* p, unused) RET 8: free(p) through 00BF65AC.
void operator_delete_0073e6b0(void* p, std::uint32_t unused, DecalHost& host);

// 007407C0, __thiscall(ECX list, four dwords) RET 10h, "cDecalList::AddDecal".
// The release body is only the profiling-scope static: when bit 0 of
// [00E1AEA8] is clear, set it, assign "cDecalList::AddDecal" (00CFF238) to a
// temporary native string (0041E870), store 0 to [00E1AEA4], release the
// string. ECX and the arguments are not read. (00740820 decodes as ENTER only
// when disassembled mid-instruction; the function ends at RET 10h 00740834.)
struct ProfileScopeHost {
    virtual ~ProfileScopeHost() = default;
    virtual void native_string_assign_0041e870(std::int32_t* string, const char* text) = 0;
    virtual void native_string_release_00419cc0_00bd1510(void* data, std::size_t bytes) = 0;
};
void decal_list_add_decal_007407c0(std::uint8_t& guard_00e1aea8, std::uint32_t& scope_00e1aea4,
                                   const char* name_00cff238, ProfileScopeHost& host);

// 0073BC40, __cdecl(void) RET (no caller or reference in the image): register
// the MSAR provider factory with the VFS manager [0109CEEC] (00736D00,
// 00BE0660), then for n = 1..15 with priority -11..-25: suffix = "%08x" (n),
// path = "ContentPackage/%s.msar"; when the manager's slot +8 finds the path
// and 00BBBB90 validates it, mount it (00BE1890: path, root ".", priority, 0,
// -1) and register the suffix (00BDEF80). Every native string is built with
// 0041DD40 + memcpy and released through the sized pool.
struct ContentPackageHost {
    virtual ~ContentPackageHost() = default;
    virtual void* msar_factory_get_00736d00() = 0;
    virtual void vfs_register_factory_00be0660(void* vfs, void* factory) = 0;
    virtual void format_int_004b8490(char* out, const char* format, std::int32_t value) = 0;
    virtual void format_text_004b8490(char* out, const char* format, const char* text) = 0;
    virtual void native_string_resize_0041dd40(std::int32_t* string, std::uint32_t size, std::uint32_t flag) = 0;
    virtual void memcpy_00bf7680(void* destination, const void* source, std::size_t bytes) = 0;
    virtual void native_string_release_00419cc0_00bd1510(void* data, std::size_t bytes) = 0;
    virtual bool vfs_find_slot08(void* vfs, const std::int32_t* path) = 0;
    virtual bool msar_validate_00bbbb90(const std::int32_t* path) = 0;  // ECX = string
    virtual void vfs_mount_00be1890(void* vfs, const std::int32_t* path, const std::int32_t* root,
                                    std::int32_t priority, std::uint32_t zero, std::int32_t minus_one) = 0;
    virtual void vfs_register_suffix_00bdef80(void* vfs, const std::int32_t* suffix) = 0;
};
void mount_content_packages_0073bc40(void* const volatile& vfs_0109ceec, ContentPackageHost& host);

// 007373F0, __fastcall(ECX slot): PUSH ECX; ECX = 00E1768C (the locale node
// pool); CALL 00736090 (return_native_locale_node_slot_00736090); RET.
void locale_node_return_slot_thunk_007373f0(void* slot, void* locale_pool_00e1768c);
// 00737850 / 00737890: frame-hook vtable slots +4h and +14h, a bare RET.
void frame_hook_slot04_noop_00737850();
// 00737860: ECX frame hook, one ignored stack word, RET 4.
void frame_hook_slot08_noop_00737860(void*, std::uint32_t);
void frame_hook_slot14_noop_00737890();

}  // namespace bsp::cc13_seg41
