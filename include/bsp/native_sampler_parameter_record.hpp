#pragma once

#include "bsp/native_lua_objects.hpp"

namespace bsp {

// Caller-retained diagnostics and two actual14h locals. Their native fields
// start with the caller's byte preimage; the concrete providers initialize
// them. The globals slot is reused after the first explicit destruction.
struct NativeSamplerParameterRecordOperation final {
    enum class Phase { fresh, running, complete, failed, diagnostic_retired };
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    NativeLuaObjectStorage result;
    NativeLuaObjectStorage globals;
    bool result_obligation{};
    bool globals_obligation{};
    std::int32_t native_state{-1};

    NativeSamplerParameterRecordOperation() noexcept {}
    ~NativeSamplerParameterRecordOperation();
    NativeSamplerParameterRecordOperation(const NativeSamplerParameterRecordOperation&) = delete;
    NativeSamplerParameterRecordOperation& operator=(const NativeSamplerParameterRecordOperation&) = delete;
    void acknowledge_diagnostic_cleanup() noexcept;
};

// Complete B1B890, ECX complete receiver, owner at receiver+4, stack key and
// four-binary32 record pointer, RET8. Source EDX adds persistent diagnostics.
// Key must satisfy the existing actual NativeString provider contract. Both
// original public pointer slots remain live until their native capture sites.
void __fastcall set_native_sampler_parameter_record_00b1b890(
    void* actual_complete_receiver, NativeSamplerParameterRecordOperation&,
    const NativeString* actual_key_header, const void* actual_record);

} // namespace bsp
