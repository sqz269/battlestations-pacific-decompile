#pragma once

#include "bsp/native_lua_objects.hpp"
#include <cstdint>

namespace bsp {

// Caller-retained source diagnostic frame. The actual 14h temporary starts
// with its caller-supplied byte preimage; B67980 writes only its native fields.
struct NativeSamplerParameterWriteOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    NativeLuaObjectStorage globals;
    bool globals_obligation{};
    bool cleanup_armed{};

    NativeSamplerParameterWriteOperation() noexcept {}
    ~NativeSamplerParameterWriteOperation();
    NativeSamplerParameterWriteOperation(const NativeSamplerParameterWriteOperation&) = delete;
    NativeSamplerParameterWriteOperation& operator=(const NativeSamplerParameterWriteOperation&) = delete;
    void acknowledge_diagnostic_cleanup() noexcept;
};

struct NativeSamplerParameterWriteContext {
    const char* actual_empty_0108ff2c;
    NativeSamplerParameterWriteOperation& operation;
};

// Full B1B830: original thiscall ECX complete receiver, actual Lua owner at
// receiver+4, stack key/value header pointers, RET8. Source EDX is additional.
void __fastcall set_native_sampler_parameter_00b1b830(
    void* actual_complete_receiver, NativeSamplerParameterWriteContext&,
    const void* actual_key_header, const void* actual_value_header);

} // namespace bsp
