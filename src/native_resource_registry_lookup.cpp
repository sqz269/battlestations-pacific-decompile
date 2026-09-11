#include "bsp/native_resource_registry_lookup.hpp"

#include "bsp/native_resource_cache_leaves.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstdint>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native resource registry lookup requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

void* at_offset(void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(storage) + offset);
}
std::uint32_t read_word(const void* storage, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(
        reinterpret_cast<std::uintptr_t>(storage) + offset);
}
void* read_pointer(const void* storage, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(read_word(storage, offset));
}

std::uint32_t current_factory_selector(const void* factory,
    const NativeResourceRegistryLookupContext& context) {
    const void* profile;
    switch (read_word(factory, 0)) {
    case 0x00d64470: profile = context.actual_factory_profile_00d64470; break;
    case 0x00d644ac: profile = context.actual_factory_profile_00d644ac; break;
    case 0x00d644e8: profile = context.actual_factory_profile_00d644e8; break;
    case 0x00d644f0: profile = context.actual_factory_profile_00d644f0; break;
    default: throw std::invalid_argument("Unimplemented native resource factory identity");
    }
    return read_word(profile, 4);
}
} // namespace

void* create_native_registered_resource_00b19e90(void* actual_registry,
    const void* actual_key, const NativeResourceRegistryLookupContext& context) {
    auto* const tree = at_offset(actual_registry, 4);
    std::uint32_t iterator[2];
    find_native_resource_factory_00b19d60(tree, iterator, actual_key,
        context.invalid_parameters);
    auto* const owner = read_pointer(iterator, 0);
    auto* const captured_head = read_pointer(tree, 4);
    if (!owner || owner != tree) {
        context.invalid_parameters.invalid_parameter(context.invalid_parameters.context);
    }
    auto* const node = read_pointer(iterator, 4);
    if (node == captured_head) return nullptr;
    if (!owner) {
        context.invalid_parameters.invalid_parameter(context.invalid_parameters.context);
    }
    if (node == read_pointer(owner, 4)) {
        context.invalid_parameters.invalid_parameter(context.invalid_parameters.context);
    }
    auto* const factory = read_pointer(node, 0x14);
    switch (current_factory_selector(factory, context)) {
    case 0x00bbc6f0: return create_native_caustics_texture_source_00bbc6f0();
    case 0x00bbc810: return create_native_shore_wave_texture_source_00bbc810();
    default: throw std::invalid_argument("Unimplemented current resource factory create slot");
    }
}
} // namespace bsp
