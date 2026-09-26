#pragma once

#include "bsp/light_type_bootstrap.hpp"

#include <cstdint>

namespace bsp {

// Actual live descriptors, without member initializers or storage adoption.
// Name words are original image addresses, not host pointers or callables.
struct NativeShaderDataSourceTypeDescriptor {
    std::uint32_t own_id;
    std::uint32_t root_id;
    std::uint32_t native_name_address;
};
struct NativeShaderTextureSourceTypeDescriptor {
    std::uint32_t own_id;
    std::uint32_t data_source_id;
    std::uint32_t root_id;
    std::uint32_t native_name_address;
};
struct NativeShaderAnimatedTextureSourceTypeDescriptor {
    std::uint32_t own_id;
    std::uint32_t texture_source_id;
    std::uint32_t data_source_id;
    std::uint32_t root_id;
    std::uint32_t native_name_address;
};

struct NativeTextureSourceTypeIdStorage {
    volatile std::uint8_t& data_guard_00f8d424;
    volatile NativeShaderDataSourceTypeDescriptor& data_00f8d428;
    volatile std::uint8_t& texture_guard_0109e9c0;
    volatile NativeShaderTextureSourceTypeDescriptor& texture_0109e9c4;
    volatile std::uint8_t& animated_guard_0109e9c1;
};

class NativeTextureSourceTypeIds final {
public:
    // All borrowed references are live and stable through every call and
    // reentrant provider callback. Counter/root/storage belong to ONE process
    // domain; construction does not initialize/reset guards or descriptors.
    NativeTextureSourceTypeIds(TypeIdCounterLifetime&, LightTypeBootstrap&,
        NativeTextureSourceTypeIdStorage) noexcept;

    // ECX is a live aligned target, no stack arguments, RET. Guards remain
    // process-wide even when the target differs from the parent global cell.
    // Existing provider failures retain every preceding write, without replay.
    void initialize_data_source_00b19c70(volatile NativeShaderDataSourceTypeDescriptor&);
    void initialize_texture_source_00c30310(volatile NativeShaderTextureSourceTypeDescriptor&);
    void initialize_animated_texture_source_00c30360(
        volatile NativeShaderAnimatedTextureSourceTypeDescriptor&);

private:
    std::uint32_t consume_type_id();
    TypeIdCounterLifetime& counter_;
    LightTypeBootstrap& shared_types_;
    NativeTextureSourceTypeIdStorage storage_;
};

// Complete native C30460[12], captures and returns the same ECX target after
// C30360 returns normally. Source ABI differs from the original register ABI.
volatile NativeShaderAnimatedTextureSourceTypeDescriptor*
initialize_native_animated_texture_source_type_00c30460(
    volatile NativeShaderAnimatedTextureSourceTypeDescriptor&,
    NativeTextureSourceTypeIds&);

} // namespace bsp
