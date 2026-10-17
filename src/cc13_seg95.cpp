// cc13_seg95: segment 95 leftovers. See include/bsp/cc13_seg95.hpp and
// docs/CC13_SEG95.md. Not wired into the game.
#include "bsp/cc13_seg95.hpp"

#include <cstdlib>
#include <cstring>

#include "bsp/ocean_spectrum.hpp"  // ocean_normal_draw_00bf0df0 (00BF0DF0)

namespace bsp::cc13_seg95 {
namespace {

// The inlined ~NativeString used throughout the segment: when data is set,
// 00419CC0 gives the pool and 00BD1510(data, length + 1, 1) takes the block.
void release_inline(NativeStr& s, Host& host) {
    if (s.data != nullptr) {
        void* pool = host.sized_pool_00419cc0();
        host.sized_pool_return_00bd1510(pool, s.data, s.length + 1, 1);
    }
}

// The inlined NativeString(const char*) of a one-character literal:
// {0, 0}, 0041DD40(1, 1), then memcpy(data, text, length + 1) when data is set.
void construct_literal1(NativeStr& s, const char* text, Host& host) {
    s.length = 0;
    s.data = nullptr;
    host.native_string_resize_0041dd40(&s, 1, 1);
    if (s.data != nullptr) std::memcpy(s.data, text, s.length + 1);
}

const char* payload_text(std::uint32_t payload) {
    return reinterpret_cast<const char*>(static_cast<std::uintptr_t>(payload));
}
float payload_float(std::uint32_t payload) {
    float f;
    std::memcpy(&f, &payload, sizeof f);
    return f;
}
const float* payload_floats(std::uint32_t payload) {
    return reinterpret_cast<const float*>(static_cast<std::uintptr_t>(payload));
}
// CVTTSS2SI on the key's float bits (truncation; out-of-range inputs, which
// give 80000000h in hardware, are not modelled).
std::int32_t truncate_key(std::uint32_t payload) {
    return static_cast<std::int32_t>(payload_float(payload));
}

// The literals the segment reads.
constexpr char kX_00ceb488[] = "x";
constexpr char kY_00d045f8[] = "y";
constexpr char kZ_00cfd718[] = "z";
constexpr char kOpenBrace_00ce439c[] = "{\n";
constexpr char kCloseBrace_00ce4cd4[] = "}";
constexpr char kWriteBinary_00d1d430[] = "wb";

}  // namespace

// 00BD2F90: copies (mean, sigma) into a new frame, 00BD2ED0 picks the thread's
// state for the stream in ECX, and 00BF0DF0 (ECX = state) draws the polar
// Box-Muller sample with the same two arguments; RET 8, the result in ST0.
// Draws 2k words from the state, k >= 1 (one pair per rejection round, about
// 1.27 rounds on average).
float random_normal_sample_00bd2f90(RandomThreads& threads, RandomStream stream, float mean, float sigma) {
    return ocean_normal_draw_00bf0df0(threads.state_00bd2ed0(stream), mean, sigma);
}

std::uint32_t* singleton_client_construct_00bd3810(std::uint32_t* self, void*& global_0109cecc,
                                                   cc13_seg41::SingletonLifetimeHost& lifetime) {
    // [this] = 00D682D0; lock; 0109CECC = this; 00BD0C30(0109CECC); unlock.
    return cc13_seg41::singleton_register(self, kSingletonClientVtable_00d682d0, global_0109cecc, lifetime);
}

void singleton_client_destruct_00bd38b0(std::uint32_t* self, void*& global_0109cecc,
                                        cc13_seg41::SingletonLifetimeHost& lifetime) {
    // [this] = 00D682D0; lock; 00BCFCA0(0109CECC); 0109CECC = 0; unlock; [this] = 00CE3818.
    cc13_seg41::singleton_unregister(self, kSingletonClientVtable_00d682d0, global_0109cecc, lifetime);
}

namespace {
template <typename T>
T& at(std::uint8_t* base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}
}  // namespace

std::uint8_t* state_owner_construct_base_00bd47b0(std::uint8_t* self, void*& global_0109cecc,
                                                  cc13_seg41::SingletonLifetimeHost& lifetime, Host& host) {
    singleton_client_construct_00bd3810(reinterpret_cast<std::uint32_t*>(self), global_0109cecc, lifetime);
    at<std::uint32_t>(self, 0x00) = kStateOwnerVtable_00d682f0;
    at<std::uint32_t>(self, 0x04) = 0;
    at<std::uint32_t>(self, 0x08) = 0;
    at<std::uint8_t>(self, 0x0C) = 0;
    at<std::uint8_t>(self, 0x10) = 0;
    at<std::uint32_t>(self, 0x14) = 0;  // native string {+14h length, +18h data}
    at<std::uint32_t>(self, 0x18) = 0;
    at<std::uint32_t>(self, 0x1C) = 0;
    at<std::uint8_t>(self, 0x20) = 0;
    at<std::uint8_t>(self, 0x21) = 0;
    at<std::uint32_t>(self, 0x24) = 0;  // native string {+24h, +28h}
    at<std::uint32_t>(self, 0x28) = 0;
    at<std::uint32_t>(self, 0x30) = 0;  // +2Ch is not written
    host.lua_state_owner_construct_00b66bd0(self + 0x38);
    at<std::uint32_t>(self, 0x500) = 0;  // native string {+500h, +504h}
    at<std::uint32_t>(self, 0x504) = 0;
    at<std::uint32_t>(self, 0x514) = 0;  // vector begin/end/capacity
    at<std::uint32_t>(self, 0x518) = 0;
    at<std::uint32_t>(self, 0x51C) = 0;
    at<std::uint32_t>(self, 0x524) = 0;
    at<std::uint32_t>(self, 0x528) = 0;  // native string {+528h, +52Ch}
    at<std::uint32_t>(self, 0x52C) = 0;
    return self;
}

void state_owner_destruct_00bd4270(std::uint8_t* self, void*& global_0109cecc,
                                   cc13_seg41::SingletonLifetimeHost& lifetime, Host& host) {
    at<std::uint32_t>(self, 0x00) = kStateOwnerVtable_00d682f0;
    release_inline(at<NativeStr>(self, 0x528), host);
    if (void* block = at<void*>(self, 0x514)) host.operator_delete_00bf65ac(block);
    at<std::uint32_t>(self, 0x514) = 0;
    at<std::uint32_t>(self, 0x518) = 0;
    at<std::uint32_t>(self, 0x51C) = 0;
    release_inline(at<NativeStr>(self, 0x500), host);
    host.lua_state_owner_close_00b669a0(self + 0x38);
    release_inline(at<NativeStr>(self, 0x24), host);
    release_inline(at<NativeStr>(self, 0x14), host);
    singleton_client_destruct_00bd38b0(reinterpret_cast<std::uint32_t*>(self), global_0109cecc, lifetime);
}

bool matrix4x4_equals_00bd4bf0(const float* a, const float* b) {
    // Sixteen FUCOMIP / TEST AH,44h / JP tests in index order; NaN is unequal.
    for (int i = 0; i < 16; ++i)
        if (!(a[i] == b[i])) return false;
    return true;
}

bool tagged_value_equals_00bd5680(const TaggedValue& a, const TaggedValue& b, Host& host) {
    switch (a.tag) {  // only the left tag is read
    case 0: return _stricmp(payload_text(a.payload), payload_text(b.payload)) == 0;  // 00BF7FBF
    case 1:
    case 4: return a.payload == b.payload;
    case 2:
    case 10: return payload_float(a.payload) == payload_float(b.payload);
    case 3: return static_cast<std::uint8_t>(a.payload) == static_cast<std::uint8_t>(b.payload);
    case 5: return host.vec3_exact_equals_00453f40(payload_floats(a.payload), payload_floats(b.payload));
    case 6: return host.vec2_equal_00533d30(payload_floats(a.payload), payload_floats(b.payload));
    case 7: return matrix4x4_equals_00bd4bf0(payload_floats(a.payload), payload_floats(b.payload));
    case 8: return host.vec4_exact_equals_00475df0(payload_floats(a.payload), payload_floats(b.payload));
    case 9: return true;
    default: return false;
    }
}

namespace {
ArchiveRef* archive_ref_construct(ArchiveRef* self, std::uint32_t vtable, void* archive, TaggedValue key,
                                  Host& host) {
    self->vtable = vtable;
    auto* count = static_cast<std::uint32_t*>(host.operator_new_00bf681b(4));
    if (count != nullptr) *count = 1;
    self->refcount_04 = count;
    self->archive_08 = archive;
    host.archive_open_vslot1(archive, key);
    return self;
}
}  // namespace

ArchiveRef* archive_reader_ref_construct_00bd4eb0(ArchiveRef* self, void* archive, TaggedValue key, Host& host) {
    return archive_ref_construct(self, 0x00CE3768u, archive, key, host);
}
ArchiveRef* archive_writer_ref_construct_00bd4f30(ArchiveRef* self, void* archive, TaggedValue key, Host& host) {
    return archive_ref_construct(self, 0x00CE3794u, archive, key, host);
}
ArchiveRef* archive_reader_ref_open_00bd4fe0(void* archive, ArchiveRef* out, TaggedValue key, Host& host) {
    // ECX is the archive; the out reference is the first stack argument.
    archive_reader_ref_construct_00bd4eb0(out, archive, key, host);
    return out;
}
ArchiveRef* archive_writer_ref_open_00bd5010(void* archive, ArchiveRef* out, TaggedValue key, Host& host) {
    archive_writer_ref_construct_00bd4f30(out, archive, key, host);
    return out;
}

void lua_read_vector3_xyz_00bd5040(float* out, LuaObjectBytes& arg, Host& host) {
    LuaObjectBytes tz, ty, tx;
    LuaObjectBytes* z = host.lua_get_by_name_00b67800(&arg, &tz, kZ_00cfd718);
    LuaObjectBytes* y = host.lua_get_by_name_00b67800(&arg, &ty, kY_00d045f8);
    LuaObjectBytes* x = host.lua_get_by_name_00b67800(&arg, &tx, kX_00ceb488);
    const float fx = host.lua_get_number_00b66270(x);
    const float fy = host.lua_get_number_00b66270(y);
    const float fz = host.lua_get_number_00b66270(z);
    out[0] = fx;
    out[1] = fy;
    out[2] = fz;
    host.lua_destruct_00b67700(&tx);
    host.lua_destruct_00b67700(&ty);
    host.lua_destruct_00b67700(&tz);
    host.lua_destruct_00b67700(&arg);
}

void lua_write_matrix4x4_00bd5140(const float* matrix, LuaObjectBytes& arg, Host& host) {
    for (std::int32_t row = 0; row < 4; ++row) {
        host.lua_set_new_table_at_00b66670(&arg, row);
        LuaObjectBytes sub;
        host.lua_get_by_index_00b67720(&arg, &sub, row);
        for (std::int32_t col = 0; col < 4; ++col)
            host.lua_set_numeric_element_00b66580(&sub, col, *matrix++);
        host.lua_destruct_00b67700(&sub);
    }
    host.lua_destruct_00b67700(&arg);
}

void lua_read_vector2_xy_00bd51e0(float* out, LuaObjectBytes& arg, Host& host) {
    LuaObjectBytes ty, tx;
    LuaObjectBytes* y = host.lua_get_by_name_00b67800(&arg, &ty, kY_00d045f8);
    LuaObjectBytes* x = host.lua_get_by_name_00b67800(&arg, &tx, kX_00ceb488);
    const float fx = host.lua_get_number_00b66270(x);
    const float fy = host.lua_get_number_00b66270(y);
    out[0] = fx;
    out[1] = fy;
    host.lua_destruct_00b67700(&tx);
    host.lua_destruct_00b67700(&ty);
    host.lua_destruct_00b67700(&arg);
}

void lua_read_integer_string_00bd5330(const NativeStr* key, std::int32_t* out, LuaObjectBytes& arg,
                                      Host& host) {
    LuaObjectBytes field;
    LuaObjectBytes* found = host.lua_get_by_native_string_00b68100(&arg, &field, key);
    const std::int32_t value = std::atol(host.lua_get_string_00b662b0(found));  // 00BF8417
    host.lua_destruct_00b67700(&field);
    *out = value;  // stored after the field object is destroyed
    host.lua_destruct_00b67700(&arg);
}

void lua_set_integer_as_string_field_00bd7aa0(const NativeStr* key, LuaObjectBytes& arg, std::int32_t value,
                                              Host& host) {
    NativeStr text;
    NativeStr* formatted = host.native_string_from_int_004260b0(&text, value);
    host.lua_set_string_field_00b674c0(&arg, key, formatted);
    release_inline(text, host);
    host.lua_destruct_00b67700(&arg);
}

void lua_set_new_table_named_00bd6300(LuaObjectBytes* table, TaggedValue key, Host& host) {
    if (key.tag == 0) {
        // The binary builds the string in place over the key's stack slots.
        NativeStr name;
        host.native_string_assign_0041e870(&name, payload_text(key.payload));
        host.lua_set_new_table_00b67580(table, &name);
        release_inline(name, host);
    } else if (key.tag == 1) {
        host.lua_set_new_table_at_00b66670(table, static_cast<std::int32_t>(key.payload));
    } else if (key.tag == 2) {
        host.lua_set_new_table_at_00b66670(table, truncate_key(key.payload));
    }
}

namespace {
// The key dispatch of value types 5..8: create the sub-table under the key and
// assign it to `sub` (00B67690), destroying the looked-up temporary. Key tags
// above 2 leave `sub` unbound.
void bind_new_subtable(LuaObjectBytes* table, TaggedValue key, LuaObjectBytes& sub, Host& host) {
    LuaObjectBytes found;
    LuaObjectBytes* result = nullptr;
    if (key.tag == 0) {
        const char* text = payload_text(key.payload);
        NativeStr name;
        host.native_string_assign_0041e870(&name, text);
        host.lua_set_new_table_00b67580(table, &name);
        host.native_string_destroy_0041dd20(&name);
        result = host.lua_get_by_name_00b67800(table, &found, text);
    } else if (key.tag == 1 || key.tag == 2) {
        const std::int32_t index =
            key.tag == 1 ? static_cast<std::int32_t>(key.payload) : truncate_key(key.payload);
        host.lua_set_new_table_at_00b66670(table, index);
        result = host.lua_get_by_index_00b67720(table, &found, index);
    } else {
        return;
    }
    host.lua_assign_00b67690(&sub, result);
    host.lua_destruct_00b67700(&found);
}

void write_elements(LuaObjectBytes& sub, const float* values, int count, Host& host) {
    for (int i = 0; i < count; ++i) host.lua_set_numeric_element_00b66580(&sub, i + 1, values[i]);
}
}  // namespace

void lua_set_typed_value_00bd7180(LuaObjectBytes* table, TaggedValue key, TaggedValue value, Host& host) {
    switch (value.tag) {
    case 0: {  // C string
        const char* text = payload_text(value.payload);
        if (key.tag == 0) host.lua_set_cstring_00b66790(table, payload_text(key.payload), text);
        else if (key.tag == 1) host.lua_set_string_at_00b66740(table, static_cast<std::int32_t>(key.payload), text);
        else if (key.tag == 2) host.lua_set_string_at_00b66740(table, truncate_key(key.payload), text);
        return;
    }
    case 1: {  // integer
        const auto v = static_cast<std::int32_t>(value.payload);
        if (key.tag == 0) {
            NativeStr name;
            host.native_string_assign_0041e870(&name, payload_text(key.payload));
            host.lua_set_integer_00b67460(table, &name, v);
            release_inline(name, host);
        } else if (key.tag == 1) {
            host.lua_set_integer_at_00b665d0(table, static_cast<std::int32_t>(key.payload), v);
        } else if (key.tag == 2) {
            host.lua_set_integer_at_00b665d0(table, truncate_key(key.payload), v);
        }
        return;
    }
    case 2: {  // float
        const float v = payload_float(value.payload);
        if (key.tag == 0) {
            NativeStr name;
            host.native_string_assign_0041e870(&name, payload_text(key.payload));
            host.lua_set_number_field_00b67400(table, &name, v);
            release_inline(name, host);
        } else if (key.tag == 1) {
            host.lua_set_numeric_element_00b66580(table, static_cast<std::int32_t>(key.payload), v);
        } else if (key.tag == 2) {
            host.lua_set_numeric_element_00b66580(table, truncate_key(key.payload), v);
        }
        return;
    }
    case 3: {  // boolean: the whole payload dword is passed on
        if (key.tag == 0) {
            NativeStr name;
            host.native_string_assign_0041e870(&name, payload_text(key.payload));
            host.lua_set_boolean_00b673a0(table, &name, value.payload);
            host.native_string_destroy_0041dd20(&name);
        } else if (key.tag == 1) {
            host.lua_set_boolean_at_00b66530(table, static_cast<std::int32_t>(key.payload), value.payload);
        } else if (key.tag == 2) {
            host.lua_set_boolean_at_00b66530(table, truncate_key(key.payload), value.payload);
        }
        return;
    }
    case 4: {  // object: written as the id from the function at 0109CEDC (called for every key tag)
        const std::int32_t id =
            host.object_id_fn_0109cedc(reinterpret_cast<void*>(static_cast<std::uintptr_t>(value.payload)));
        if (key.tag == 0) {
            NativeStr name;
            host.native_string_assign_0041e870(&name, payload_text(key.payload));
            host.lua_set_integer_00b67460(table, &name, id);
            host.native_string_destroy_0041dd20(&name);
        } else if (key.tag == 1) {
            host.lua_set_integer_at_00b665d0(table, static_cast<std::int32_t>(key.payload), id);
        } else if (key.tag == 2) {
            host.lua_set_integer_at_00b665d0(table, truncate_key(key.payload), id);
        }
        return;
    }
    case 5:    // vector3 -> {1, 2, 3}
    case 6:    // vector2 -> {1, 2}
    case 8: {  // vector4 -> {1, 2, 3, 4}
        LuaObjectBytes sub;
        host.lua_construct_00b65f50(&sub);
        bind_new_subtable(table, key, sub, host);
        write_elements(sub, payload_floats(value.payload), value.tag == 5 ? 3 : value.tag == 6 ? 2 : 4, host);
        host.lua_destruct_00b67700(&sub);
        return;
    }
    case 7: {  // matrix -> rows 1..4, each {1, 2, 3, 4}, floats in memory order
        LuaObjectBytes sub;
        host.lua_construct_00b65f50(&sub);
        bind_new_subtable(table, key, sub, host);
        const float* m = payload_floats(value.payload);
        for (std::int32_t row = 1; row <= 4; ++row) {
            host.lua_set_new_table_at_00b66670(&sub, row);
            LuaObjectBytes line;
            host.lua_get_by_index_00b67720(&sub, &line, row);
            for (std::int32_t col = 1; col <= 4; ++col) host.lua_set_numeric_element_00b66580(&line, col, *m++);
            host.lua_destruct_00b67700(&line);
        }
        host.lua_destruct_00b67700(&sub);
        return;
    }
    case 10: {  // integer written as its decimal string
        NativeStr text;
        host.native_string_from_int_004260b0(&text, static_cast<std::int32_t>(value.payload));
        if (key.tag == 0) host.lua_set_string_00b67630(table, payload_text(key.payload), &text);
        else if (key.tag == 1) host.lua_set_lstring_at_00b672f0(table, static_cast<std::int32_t>(key.payload), &text);
        else if (key.tag == 2) host.lua_set_lstring_at_00b672f0(table, truncate_key(key.payload), &text);
        host.native_string_destroy_0041dd20(&text);
        return;
    }
    default:  // 9 and anything else: nothing
        return;
    }
}

namespace {
// The checked back() of the LuaObject vector, with the binary's three
// _invalid_parameter reports (00BF6713 returns; execution continues).
LuaObjectBytes* checked_back(LuaObjectVectorProxy& v, Host& host) {
    const auto last = reinterpret_cast<std::uintptr_t>(v.last_08);
    if (reinterpret_cast<std::uintptr_t>(v.first_04) > last) host.invalid_parameter_00bf6713();
    const std::uintptr_t back = last - sizeof(LuaObjectBytes);
    if (back > reinterpret_cast<std::uintptr_t>(v.last_08) || back < reinterpret_cast<std::uintptr_t>(v.first_04))
        host.invalid_parameter_00bf6713();
    if (!(back < reinterpret_cast<std::uintptr_t>(v.last_08))) host.invalid_parameter_00bf6713();
    return reinterpret_cast<LuaObjectBytes*>(back);
}
}  // namespace

void lua_writer_enter_new_table_00bd8ec0(std::uint8_t* writer, TaggedValue key, Host& host) {
    auto& stack = *reinterpret_cast<LuaObjectVectorProxy*>(writer + 4);
    lua_set_new_table_named_00bd6300(checked_back(stack, host), key, host);
    LuaObjectBytes child;
    LuaObjectBytes* found = host.lua_lookup_child_00bd5790(&child, checked_back(stack, host), key);
    host.lua_object_vector_push_back_00442220(&stack, found);
    host.lua_destruct_00b67700(&child);
}

namespace {
StdioFileStream* stdio_stream_open(StdioFileStream* self, const NativeStr* path, const ArchiveWriterGlobals& g,
                                   Host& host) {
    self->depth_04 = 0;
    self->vtable = kStdioStreamVtable_00d68340;
    self->field_08 = 0;
    const char* name = path->data != nullptr ? path->data : g.empty_0109cee5;
    self->file_0c = host.fopen_00bf838e(name, kWriteBinary_00d1d430);
    return self;
}
}  // namespace

StdioFileStream* stdio_stream_construct_00bd5ce0(StdioFileStream* self, const NativeStr* path,
                                                 const ArchiveWriterGlobals& globals, Host& host) {
    return stdio_stream_open(self, path, globals, host);
}

StdioFileStream* stdio_stream_construct_compact_00bd5d60(StdioFileStream* self, const NativeStr* path,
                                                         const ArchiveWriterGlobals& globals, Host& host) {
    stdio_stream_open(self, path, globals, host);
    self->vtable = kStdioStreamCompactVtable_00d68350;
    return self;
}

void stdio_stream_destroy_00bd5860(StdioFileStream* self, Host& host) {
    self->vtable = kStdioStreamVtable_00d68340;
    host.fclose_00bf8081(self->file_0c);  // no null test
    self->vtable = kArchiveWriterBaseVtable_00ce3784;
}

void stdio_stream_open_block_00bd8d10(StdioFileStream* self, TaggedValue key, ArchiveWriterGlobals& g, Host& host) {
    g.file_0109cee0 = self->file_0c;
    host.emit_indent_00bd6990(self->depth_04);
    host.emit_key_00bd7b30(self->depth_04 == 0, key);
    NativeStr brace;
    host.native_string_assign_0041e870(&brace, kOpenBrace_00ce439c);
    host.emit_string_00bd6bd0(brace);  // the callee owns and releases it
    self->depth_04 += 1;
    g.file_0109cee0 = nullptr;
}

void stdio_stream_close_block_00bd6d00(StdioFileStream* self, ArchiveWriterGlobals& g, Host& host) {
    g.file_0109cee0 = self->file_0c;
    self->depth_04 -= 1;
    host.emit_indent_00bd6990(self->depth_04);
    NativeStr brace;
    host.native_string_assign_0041e870(&brace, kCloseBrace_00ce4cd4);
    host.emit_string_00bd6bd0(brace);
    host.emit_terminator_00bd6ac0(self->depth_04 == 0);
    g.file_0109cee0 = nullptr;
}

void stdio_stream_close_block_compact_00bd6d50(StdioFileStream* self, ArchiveWriterGlobals& g, Host& host) {
    // The same body as 00BD6D00 between "flag = 1" and "flag = 0".
    g.compact_0109cee4 = 1;
    stdio_stream_close_block_00bd6d00(self, g, host);
    g.compact_0109cee4 = 0;
}

void stdio_stream_write_key_value_00bd8d70(StdioFileStream* self, TaggedValue key, TaggedValue value,
                                           ArchiveWriterGlobals& g, Host& host) {
    g.file_0109cee0 = self->file_0c;
    host.emit_indent_00bd6990(self->depth_04);
    host.emit_key_00bd7b30(self->depth_04 == 0, key);
    host.emit_value_00bd7f50(value);
    host.emit_terminator_00bd6ac0(self->depth_04 == 0);
    g.file_0109cee0 = nullptr;
}

void stdio_stream_open_block_compact_00bd8dd0(StdioFileStream* self, TaggedValue key, ArchiveWriterGlobals& g,
                                              Host& host) {
    g.compact_0109cee4 = 1;
    stdio_stream_open_block_00bd8d10(self, key, g, host);
    g.compact_0109cee4 = 0;
}

void stdio_stream_write_key_value_compact_00bd8df0(StdioFileStream* self, TaggedValue key, TaggedValue value,
                                                   ArchiveWriterGlobals& g, Host& host) {
    g.compact_0109cee4 = 1;
    stdio_stream_write_key_value_00bd8d70(self, key, value, g, host);
    g.compact_0109cee4 = 0;
}

void stdio_stream_write_string_00bd6db0(StdioFileStream* self, const NativeStr* text, ArchiveWriterGlobals& g,
                                        Host& host) {
    g.file_0109cee0 = self->file_0c;
    if (g.compact_0109cee4 != 0) {
        // Inlined copy constructor into the by-value argument of 00BD6100.
        NativeStr copy{0, nullptr};
        if (&copy != text) {
            host.native_string_resize_0041dd40(&copy, text->length, 1);
            if (text->length != 0) std::memcpy(copy.data, text->data, copy.length);  // 00BF7680
        }
        host.emit_compact_encoded_00bd6100(copy);
    } else if (g.file_0109cee0 != nullptr) {
        const char* data = text->data != nullptr ? text->data : g.empty_0109cee5;
        host.fwrite_00bfb08b(data, text->length, 1, g.file_0109cee0);
    } else {
        host.save_storage_append_00bd49b0(g.storage_0109cecc, text);
    }
    g.file_0109cee0 = nullptr;
}

void vfs_flagged_exists_mount_callback_00bd9130(std::uint8_t* visitor, const std::uint8_t* mount,
                                                const void* name, Host& host) {
    if (mount[0x0C] != 0) {
        void* provider = *reinterpret_cast<void* const*>(mount + 0x08);
        visitor[4] = host.vfs_provider_exists_vslot10(provider, name);
    }
}

std::uint8_t vfs_flagged_exists_read_result_00bd9160(const std::uint8_t* visitor) {
    return visitor[4];
}

}  // namespace bsp::cc13_seg95
