#pragma once
// cc13_seg95: segment 95 (00BD2F10-00BD9210) leftovers. Engine utilities: the
// random normal sample, the 0109CECC singleton base (ledger "LuaStateOwner"),
// tagged-value / matrix equality, archive reader/writer references, the stdio
// archive text stream (vtables 00D68340 / 00D68350), the Lua archive writer
// helpers built on the game's LuaObject wrapper, and the VFS flagged-exists
// visitor slots. Evidence and uncertainty: docs/CC13_SEG95.md.
//
// Every routine here is a game helper. The LuaObject primitives (00B65F50..
// 00B68100), native string helpers (0041xxxx, 004260B0), the sized pool,
// the archive emit helpers (00BD6990/6AC0/6BD0/6100/7B30/7F50, 00BD49B0) and
// CRT entries are called through the named hooks below at the binary's call
// sites. Nothing is wired into the game. Win32 only: 32-bit layouts.

#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "bsp/cc13_seg41.hpp"      // SingletonLifetimeHost, singleton_register/unregister
#include "bsp/random_threads.hpp"  // RandomThreads::state_00bd2ed0

namespace bsp::cc13_seg95 {

// The 8-byte archive / Lua tagged value passed by value: dword tag, dword
// payload (C-string pointer, int, float bits, bool byte or vector pointer).
struct TaggedValue {
    std::uint32_t tag;
    std::uint32_t payload;
};
static_assert(sizeof(TaggedValue) == 8);

// The game's 8-byte native string {length +0, data +4}.
struct NativeStr {
    std::uint32_t length;
    char* data;
};

// The game's 14h-byte LuaObject (stack-slot wrapper); opaque here, every
// operation goes through a host hook.
struct LuaObjectBytes {
    std::uint8_t bytes[0x14];
};
static_assert(sizeof(LuaObjectBytes) == 0x14);

// The shared archive-output globals the stream writer sets around each call.
struct ArchiveWriterGlobals {
    std::FILE* file_0109cee0{};      // current FILE
    std::uint8_t compact_0109cee4{};  // compact (encoded) output flag
    void* storage_0109cecc{};         // the singleton 00BD49B0 appends to
    const char* empty_0109cee5{};     // the empty string used for a null native string
};

struct Host {
    virtual ~Host() = default;

    // --- LuaObject primitives, all __thiscall(ECX = LuaObject) ---
    virtual void lua_construct_00b65f50(LuaObjectBytes* self) = 0;               // RET
    virtual void lua_destruct_00b67700(LuaObjectBytes* self) = 0;                // RET
    virtual float lua_get_number_00b66270(LuaObjectBytes* self) = 0;             // RET, ST0
    virtual std::int32_t lua_get_integer_00b66290(LuaObjectBytes* self) = 0;    // RET, EAX
    virtual const char* lua_get_string_00b662b0(LuaObjectBytes* self) = 0;       // RET, EAX
    // (self, out, key) RET 8, EAX = out.
    virtual LuaObjectBytes* lua_get_by_name_00b67800(LuaObjectBytes* self, LuaObjectBytes* out,
                                                     const char* name) = 0;
    virtual LuaObjectBytes* lua_get_by_index_00b67720(LuaObjectBytes* self, LuaObjectBytes* out,
                                                      std::int32_t index) = 0;
    virtual LuaObjectBytes* lua_get_by_native_string_00b68100(LuaObjectBytes* self, LuaObjectBytes* out,
                                                              const NativeStr* key) = 0;
    virtual void lua_assign_00b67690(LuaObjectBytes* self, const LuaObjectBytes* source) = 0;  // RET 4
    // Table iteration: (self = table, key*, value*) RET 8; IsUnbound(ignored ECX, object*) RET 4 -> AL.
    virtual void lua_iterate_first_00b67080(LuaObjectBytes* table, LuaObjectBytes* key,
                                            LuaObjectBytes* value) = 0;
    virtual void lua_iterate_next_00b67190(LuaObjectBytes* table, LuaObjectBytes* key,
                                           LuaObjectBytes* value) = 0;
    virtual bool lua_is_unbound_00b66420(LuaObjectBytes* ignored_ecx, LuaObjectBytes* object) = 0;
    // Setters.
    virtual void lua_set_new_table_00b67580(LuaObjectBytes* self, const NativeStr* name) = 0;       // RET 4
    virtual void lua_set_new_table_at_00b66670(LuaObjectBytes* self, std::int32_t index) = 0;      // RET 4
    virtual void lua_set_numeric_element_00b66580(LuaObjectBytes* self, std::int32_t key, float value) = 0;
    virtual void lua_set_number_field_00b67400(LuaObjectBytes* self, const NativeStr* key, float value) = 0;
    virtual void lua_set_cstring_00b66790(LuaObjectBytes* self, const char* key, const char* value) = 0;
    virtual void lua_set_string_at_00b66740(LuaObjectBytes* self, std::int32_t key, const char* value) = 0;
    virtual void lua_set_integer_00b67460(LuaObjectBytes* self, const NativeStr* key, std::int32_t value) = 0;
    virtual void lua_set_integer_at_00b665d0(LuaObjectBytes* self, std::int32_t key, std::int32_t value) = 0;
    // The whole payload dword is pushed as the boolean argument.
    virtual void lua_set_boolean_00b673a0(LuaObjectBytes* self, const NativeStr* key, std::uint32_t value) = 0;
    virtual void lua_set_boolean_at_00b66530(LuaObjectBytes* self, std::int32_t key, std::uint32_t value) = 0;
    virtual void lua_set_string_00b67630(LuaObjectBytes* self, const char* key, const NativeStr* value) = 0;
    virtual void lua_set_lstring_at_00b672f0(LuaObjectBytes* self, std::int32_t key, const NativeStr* value) = 0;
    virtual void lua_set_string_field_00b674c0(LuaObjectBytes* self, const NativeStr* key,
                                               const NativeStr* value) = 0;
    // 00BD5790 BSP_LuaReader_LookupChild, __fastcall(ECX = out, EDX = parent, key tag, key value) RET 8.
    virtual LuaObjectBytes* lua_lookup_child_00bd5790(LuaObjectBytes* out, LuaObjectBytes* parent,
                                                      TaggedValue key) = 0;
    // 00442220 BSP_LuaObjectVector_PushBack, __thiscall(&vector proxy, const LuaObject*) RET 4.
    virtual void lua_object_vector_push_back_00442220(void* vector_proxy, const LuaObjectBytes* element) = 0;
    // Lua state owner embedded at +38h of the 0109CECC object.
    virtual void lua_state_owner_construct_00b66bd0(void* owner) = 0;
    virtual void lua_state_owner_close_00b669a0(void* owner) = 0;

    // --- native string helpers ---
    virtual NativeStr* native_string_assign_0041e870(NativeStr* self, const char* text) = 0;  // RET 4
    virtual void native_string_resize_0041dd40(NativeStr* self, std::uint32_t length,
                                               std::uint32_t preserve) = 0;                  // RET 8
    virtual void native_string_destroy_0041dd20(NativeStr* self) = 0;                       // RET
    virtual NativeStr* native_string_from_int_004260b0(NativeStr* self, std::int32_t value) = 0;  // RET 4
    // The inlined ~NativeString: 00419CC0 returns the pool, 00BD1510 (ECX pool;
    // block, size, 1) RET 0Ch takes the block back.
    virtual void* sized_pool_00419cc0() = 0;
    virtual void sized_pool_return_00bd1510(void* pool, void* block, std::uint32_t size,
                                            std::uint32_t flag) = 0;

    // --- archive text emit helpers (shared globals 0109CEE0/0109CEE4/0109CECC) ---
    virtual void emit_indent_00bd6990(std::uint32_t depth) = 0;          // ECX = depth
    virtual void emit_terminator_00bd6ac0(bool root) = 0;                // CL = root
    virtual void emit_string_00bd6bd0(NativeStr owned) = 0;              // by value, RET 8
    virtual void emit_compact_encoded_00bd6100(NativeStr owned) = 0;     // by value, RET 8
    virtual void emit_key_00bd7b30(bool root, TaggedValue key) = 0;      // CL = root, RET 8
    virtual void emit_value_00bd7f50(TaggedValue value) = 0;             // RET 8
    virtual void save_storage_append_00bd49b0(void* storage, const NativeStr* bytes) = 0;  // ECX storage, RET 4
    // archive vtable slot 1 (+4h), __thiscall(archive; key tag, key value) RET 8: open the section.
    virtual void archive_open_vslot1(void* archive, TaggedValue key) = 0;

    // --- other engine calls ---
    // CALL [0109CEDC] with ECX = the payload object: the installed object-to-id function.
    virtual std::int32_t object_id_fn_0109cedc(void* object) = 0;
    virtual bool vec3_exact_equals_00453f40(const float* a, const float* b) = 0;  // ECX a, RET 4
    virtual bool vec2_equal_00533d30(const float* a, const float* b) = 0;
    virtual bool vec4_exact_equals_00475df0(const float* a, const float* b) = 0;
    // mount+8 provider vtable +10h, __thiscall(provider; name) RET 4 -> AL.
    virtual std::uint8_t vfs_provider_exists_vslot10(void* provider, const void* name) = 0;

    // --- CRT ---
    virtual void* operator_new_00bf681b(std::uint32_t size) = 0;
    virtual void operator_delete_00bf65ac(void* block) = 0;
    virtual std::FILE* fopen_00bf838e(const char* path, const char* mode) = 0;
    virtual int fclose_00bf8081(std::FILE* file) = 0;
    virtual std::size_t fwrite_00bfb08b(const void* data, std::size_t size, std::size_t count,
                                        std::FILE* file) = 0;
    virtual void invalid_parameter_00bf6713() = 0;  // checked-iterator report; returns
};

// 00BD2F90 BSP_RandomThreads_NormalSample. ECX = stream, stack (mean, sigma), RET 8, ST0.
float random_normal_sample_00bd2f90(RandomThreads& threads, RandomStream stream, float mean, float sigma);

// 00BD3810 / 00BD38B0: register / unregister the 0109CECC singleton, vtable 00D682D0.
inline constexpr std::uint32_t kSingletonClientVtable_00d682d0 = 0x00D682D0u;
std::uint32_t* singleton_client_construct_00bd3810(std::uint32_t* self, void*& global_0109cecc,
                                                   cc13_seg41::SingletonLifetimeHost& lifetime);
void singleton_client_destruct_00bd38b0(std::uint32_t* self, void*& global_0109cecc,
                                        cc13_seg41::SingletonLifetimeHost& lifetime);

// 00BD47B0 / 00BD4270: the 0109CECC object's base constructor and destructor
// (vtable 00D682F0, a Lua state owner at +38h). __fastcall(ECX = this).
inline constexpr std::uint32_t kStateOwnerVtable_00d682f0 = 0x00D682F0u;
std::uint8_t* state_owner_construct_base_00bd47b0(std::uint8_t* self, void*& global_0109cecc,
                                                  cc13_seg41::SingletonLifetimeHost& lifetime, Host& host);
void state_owner_destruct_00bd4270(std::uint8_t* self, void*& global_0109cecc,
                                   cc13_seg41::SingletonLifetimeHost& lifetime, Host& host);

// 00BD4BF0 BSP_Matrix4x4_Equals, __thiscall(ECX a, b) RET 4 -> EAX.
bool matrix4x4_equals_00bd4bf0(const float* a, const float* b);
// 00BD5680 BSP_TaggedValue_Equals, __thiscall(ECX a, b) RET 4 -> AL.
bool tagged_value_equals_00bd5680(const TaggedValue& a, const TaggedValue& b, Host& host);

// Archive reader / writer references: {vtable, refcount block*, archive}.
struct ArchiveRef {
    std::uint32_t vtable;
    std::uint32_t* refcount_04;
    void* archive_08;
};
// 00BD4EB0 / 00BD4F30, __thiscall(ECX ref; archive, key tag, key value) RET 0Ch.
ArchiveRef* archive_reader_ref_construct_00bd4eb0(ArchiveRef* self, void* archive, TaggedValue key, Host& host);
ArchiveRef* archive_writer_ref_construct_00bd4f30(ArchiveRef* self, void* archive, TaggedValue key, Host& host);
// 00BD4FE0 / 00BD5010, __thiscall(ECX archive; out ref, key tag, key value) RET 0Ch -> out.
ArchiveRef* archive_reader_ref_open_00bd4fe0(void* archive, ArchiveRef* out, TaggedValue key, Host& host);
ArchiveRef* archive_writer_ref_open_00bd5010(void* archive, ArchiveRef* out, TaggedValue key, Host& host);

// LuaObject helpers taking the LuaObject by value on the stack (callee destroys
// it, RET 14h or 18h). `arg` is that by-value storage.
void lua_read_vector3_xyz_00bd5040(float* out, LuaObjectBytes& arg, Host& host);           // ECX out
void lua_write_matrix4x4_00bd5140(const float* matrix, LuaObjectBytes& arg, Host& host);   // ECX matrix
void lua_read_vector2_xy_00bd51e0(float* out, LuaObjectBytes& arg, Host& host);            // ECX out
void lua_read_integer_string_00bd5330(const NativeStr* key, std::int32_t* out, LuaObjectBytes& arg,
                                      Host& host);                                          // ECX key, EDX out
void lua_set_integer_as_string_field_00bd7aa0(const NativeStr* key, LuaObjectBytes& arg, std::int32_t value,
                                              Host& host);                                  // ECX key

// The Lua archive writer: 00BD6300 SetNewTableNamed (ECX table; key) RET 8,
// 00BD7180 SetTypedValue (ECX table; key, value) RET 10h, 00BD8EC0
// EnterNewTable (ECX writer; key) RET 8 over the LuaObject vector at writer+4.
void lua_set_new_table_named_00bd6300(LuaObjectBytes* table, TaggedValue key, Host& host);
void lua_set_typed_value_00bd7180(LuaObjectBytes* table, TaggedValue key, TaggedValue value, Host& host);
struct LuaObjectVectorProxy {
    std::uint32_t proxy_00;
    LuaObjectBytes* first_04;
    LuaObjectBytes* last_08;
    LuaObjectBytes* end_0c;
};
void lua_writer_enter_new_table_00bd8ec0(std::uint8_t* writer, TaggedValue key, Host& host);

// The stdio archive text stream: {vtable, depth +4, +8, FILE* +0Ch}.
struct StdioFileStream {
    std::uint32_t vtable;
    std::uint32_t depth_04;
    std::uint32_t field_08;
    std::FILE* file_0c;
};
static_assert(sizeof(void*) != 4 || offsetof(StdioFileStream, file_0c) == 0x0C);
inline constexpr std::uint32_t kStdioStreamVtable_00d68340 = 0x00D68340u;
inline constexpr std::uint32_t kStdioStreamCompactVtable_00d68350 = 0x00D68350u;
inline constexpr std::uint32_t kArchiveWriterBaseVtable_00ce3784 = 0x00CE3784u;
StdioFileStream* stdio_stream_construct_00bd5ce0(StdioFileStream* self, const NativeStr* path,
                                                 const ArchiveWriterGlobals& globals, Host& host);
StdioFileStream* stdio_stream_construct_compact_00bd5d60(StdioFileStream* self, const NativeStr* path,
                                                         const ArchiveWriterGlobals& globals, Host& host);
void stdio_stream_destroy_00bd5860(StdioFileStream* self, Host& host);
void stdio_stream_open_block_00bd8d10(StdioFileStream* self, TaggedValue key, ArchiveWriterGlobals& g, Host& host);
void stdio_stream_close_block_00bd6d00(StdioFileStream* self, ArchiveWriterGlobals& g, Host& host);
void stdio_stream_close_block_compact_00bd6d50(StdioFileStream* self, ArchiveWriterGlobals& g, Host& host);
void stdio_stream_write_key_value_00bd8d70(StdioFileStream* self, TaggedValue key, TaggedValue value,
                                           ArchiveWriterGlobals& g, Host& host);
void stdio_stream_open_block_compact_00bd8dd0(StdioFileStream* self, TaggedValue key, ArchiveWriterGlobals& g,
                                              Host& host);
void stdio_stream_write_key_value_compact_00bd8df0(StdioFileStream* self, TaggedValue key, TaggedValue value,
                                                   ArchiveWriterGlobals& g, Host& host);
void stdio_stream_write_string_00bd6db0(StdioFileStream* self, const NativeStr* text, ArchiveWriterGlobals& g,
                                        Host& host);

// VFS flagged-exists visitor (vtable 00D683A4) slots 1 and 2.
void vfs_flagged_exists_mount_callback_00bd9130(std::uint8_t* visitor, const std::uint8_t* mount,
                                                const void* name, Host& host);
std::uint8_t vfs_flagged_exists_read_result_00bd9160(const std::uint8_t* visitor);

}  // namespace bsp::cc13_seg95
