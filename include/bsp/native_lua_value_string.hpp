#pragma once

#include "bsp/native_lua_type_name.hpp"
#include "bsp/native_string_from_int_raw.hpp"

#include <array>
#include <cstdint>

namespace bsp {
struct NativeLuaObjectStorage;

enum class NativeLuaValueStringRole : std::uint32_t {
    true_value, false_value, pointer_value, integer_format
};
struct NativeLuaValueStringSpan {
    const char* data;
    std::uintptr_t original_address;
    std::uint32_t extent_including_nul;
    NativeLuaValueStringRole role;
};

// Immutable metadata over actual RO spans, SAME raw cells and the ACTUAL
// current uint32 0109EEA4 conversion word. No literal/feature payload read or
// interpreter/owner/format callback is created. Mapping provenance and all
// reached object/output/provider lifetimes remain the real caller's proof.
class NativeLuaValueStringContext final {
public:
    NativeLuaValueStringContext(NativeStringRawPoolContext&,
        const volatile std::uint32_t& actual_conversion_0109eea4,
        const std::array<NativeLuaTypeNameSpan, 11>& actual_type_spans,
        const std::array<NativeLuaValueStringSpan, 4>& actual_value_spans);
    NativeLuaValueStringContext(const NativeLuaValueStringContext&) = delete;
    NativeLuaValueStringContext& operator=(const NativeLuaValueStringContext&) = delete;
    NativeLuaValueStringContext(NativeLuaValueStringContext&&) = delete;
    NativeLuaValueStringContext& operator=(NativeLuaValueStringContext&&) = delete;
private:
    friend void* construct_native_lua_value_string_00b69130(
        const NativeLuaObjectStorage&, void*, const NativeLuaValueStringContext&);
    NativeStringRawPoolContext& strings_;
    const volatile std::uint32_t& conversion_;
    const std::array<NativeLuaTypeNameSpan, 11> type_spans_;
    const std::array<NativeLuaValueStringSpan, 4> value_spans_;
    const NativeLuaTypeNameContext type_names_;
    const NativeStringFromIntRawContext integer_;
};

// Distinct whole B69130 ordinary Source boundary. Original ECX=actual14,
// hidden fresh8 result on stack, EAX=SAME result, RET4. All branches retain
// current predicate/C-API epochs. Integer conversion consumes the real Lua
// ST0 return AFTER FLOAT32 spill/reload using the actual current word address.
// No B68630/semantic-string substitute, local Frame/EH/output cleanup or
// rollback. Caller owns normal output and retains every partial failure debt.
void* construct_native_lua_value_string_00b69130(const NativeLuaObjectStorage&,
    void* actual_fresh8, const NativeLuaValueStringContext&);
} // namespace bsp
