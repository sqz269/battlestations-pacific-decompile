#pragma once
#include "bsp/native_lua_objects.hpp"
#include "bsp/native_shader_descriptor_owner.hpp"
namespace bsp {
// Actual1Ch field, first member is a native8h string. Descriptor D0/DC destroys
// that string and frees the record directly; there is no vtable/refcount.
struct NativeShaderFieldStorage {
    NativeString name_00;
    std::int32_t scalar_type_08,component_count_0c;
    std::uint32_t component_mask_10;
    std::int32_t semantic_14,semantic_index_18;
};
static_assert(sizeof(NativeShaderFieldStorage)==0x1c);
static_assert(offsetof(NativeShaderFieldStorage,scalar_type_08)==8);
static_assert(offsetof(NativeShaderFieldStorage,component_mask_10)==0x10);
static_assert(offsetof(NativeShaderFieldStorage,semantic_index_18)==0x18);
// B573F0 leaves three stack locals unwritten if their accepted ordinal is
// missing. Explicit inputs preserve that uncertainty without invented defaults
// or undefined C++ reads. Fully populated fields overwrite all three values.
struct NativeShaderFieldStackPreimage {
    std::int32_t scalar_type,component_count,semantic;
};
// Supplies the original uninitialized stack inputs independently for each
// outer entry. This is a host input boundary, not a recovered game callback.
struct NativeShaderFieldStackInputs {
    void* context;
    NativeShaderFieldStackPreimage (*for_entry)(void*,NativeLuaObjectStorage&);
};
// FullB34680: ECX0Ch pointer header, signed stack request, RET4; minimum10.
void reserve_native_shader_field_pointers_00b34680(NativeShaderDescriptorArray&,std::int32_t request);
// FullB573F0: incoming ECX unused, one stack Lua table, EAX allocated1Ch field,
// RET4. Iterate numeric integral keys by accepted ORDINAL; retain Lua order.
// Mask/index start0; name empty until ordinal0, where nonstrings use "undef".
// Raw allocation is freed if output-name copying fails, without a name dtor.
NativeShaderFieldStorage* read_native_shader_field_00b573f0(NativeLuaObjectStorage&,
    NativeStringStorage&,const bool& crt_sse2_conversion,NativeShaderFieldStackPreimage);
// FullB419B0: ECX unused; stack Shader/output-header/NativeString field-name;
// RET0C. Fresh lookup after table gate; EVERY outer value becomes a field,
// regardless of its key. Preserve existing rows, grow max(capacity+5,10).
// All entry values must support native Lua iteration; no added type rejection.
void append_native_shader_field_table_00b419b0(NativeLuaObjectStorage& shader,
    NativeShaderDescriptorArray&,const NativeString& field,NativeStringStorage&,
    const bool& crt_sse2_conversion,const NativeShaderFieldStackInputs&);
} // namespace bsp
