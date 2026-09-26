#include "bsp/native_texture_source_type_ids.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native texture-source type IDs require MSVC Win32 pointer widths.
#endif

namespace bsp {

static_assert(sizeof(NativeShaderDataSourceTypeDescriptor) == 12);
static_assert(sizeof(NativeShaderTextureSourceTypeDescriptor) == 16);
static_assert(sizeof(NativeShaderAnimatedTextureSourceTypeDescriptor) == 20);
static_assert(alignof(NativeShaderDataSourceTypeDescriptor) == 4);
static_assert(alignof(NativeShaderTextureSourceTypeDescriptor) == 4);
static_assert(alignof(NativeShaderAnimatedTextureSourceTypeDescriptor) == 4);
static_assert(offsetof(NativeShaderDataSourceTypeDescriptor, own_id) == 0);
static_assert(offsetof(NativeShaderDataSourceTypeDescriptor, root_id) == 4);
static_assert(offsetof(NativeShaderDataSourceTypeDescriptor, native_name_address) == 8);
static_assert(offsetof(NativeShaderTextureSourceTypeDescriptor, own_id) == 0);
static_assert(offsetof(NativeShaderTextureSourceTypeDescriptor, data_source_id) == 4);
static_assert(offsetof(NativeShaderTextureSourceTypeDescriptor, root_id) == 8);
static_assert(offsetof(NativeShaderTextureSourceTypeDescriptor, native_name_address) == 12);
static_assert(offsetof(NativeShaderAnimatedTextureSourceTypeDescriptor, own_id) == 0);
static_assert(offsetof(NativeShaderAnimatedTextureSourceTypeDescriptor, texture_source_id) == 4);
static_assert(offsetof(NativeShaderAnimatedTextureSourceTypeDescriptor, data_source_id) == 8);
static_assert(offsetof(NativeShaderAnimatedTextureSourceTypeDescriptor, root_id) == 12);
static_assert(offsetof(NativeShaderAnimatedTextureSourceTypeDescriptor, native_name_address) == 16);

NativeTextureSourceTypeIds::NativeTextureSourceTypeIds(
    TypeIdCounterLifetime& counter, LightTypeBootstrap& shared_types,
    NativeTextureSourceTypeIdStorage storage) noexcept
    : counter_(counter), shared_types_(shared_types), storage_(storage) {}

std::uint32_t NativeTextureSourceTypeIds::consume_type_id() {
    volatile auto* counter = counter_.get_006fac20();
    const auto result = counter->next_id_04;
    counter->next_id_04 = result + 1u;
    return result;
}

void NativeTextureSourceTypeIds::initialize_data_source_00b19c70(
    volatile NativeShaderDataSourceTypeDescriptor& target) {
    if (storage_.data_guard_00f8d424 == 0) {
        storage_.data_guard_00f8d424 = 1;
        target.native_name_address = 0x00d5e580u;
        auto& root = shared_types_.storage().root_0109db84;
        shared_types_.initialize_root_00bea780(root);
        target.root_id = root.own_id;
        target.own_id = consume_type_id();
    }
}

void NativeTextureSourceTypeIds::initialize_texture_source_00c30310(
    volatile NativeShaderTextureSourceTypeDescriptor& target) {
    if (storage_.texture_guard_0109e9c0 == 0) {
        storage_.texture_guard_0109e9c0 = 1;
        target.native_name_address = 0x00d79b20u;
        initialize_data_source_00b19c70(storage_.data_00f8d428);
        target.data_source_id = storage_.data_00f8d428.own_id;
        target.root_id = storage_.data_00f8d428.root_id;
        target.own_id = consume_type_id();
    }
}

void NativeTextureSourceTypeIds::initialize_animated_texture_source_00c30360(
    volatile NativeShaderAnimatedTextureSourceTypeDescriptor& target) {
    if (storage_.animated_guard_0109e9c1 == 0) {
        storage_.animated_guard_0109e9c1 = 1;
        target.native_name_address = 0x00d79b38u;
        initialize_texture_source_00c30310(storage_.texture_0109e9c4);
        target.texture_source_id = storage_.texture_0109e9c4.own_id;
        target.data_source_id = storage_.texture_0109e9c4.data_source_id;
        target.root_id = storage_.texture_0109e9c4.root_id;
        target.own_id = consume_type_id();
    }
}

volatile NativeShaderAnimatedTextureSourceTypeDescriptor*
initialize_native_animated_texture_source_type_00c30460(
    volatile NativeShaderAnimatedTextureSourceTypeDescriptor& target,
    NativeTextureSourceTypeIds& types) {
    auto* captured = &target;
    types.initialize_animated_texture_source_00c30360(target);
    return captured;
}

} // namespace bsp
