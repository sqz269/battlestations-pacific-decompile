#include "bsp/native_scene_registration_gates.hpp"
#include "bsp/camera_type_bootstrap.hpp"
#include "bsp/light_type_bootstrap.hpp"
#include <cstddef>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
static_assert(offsetof(NativeSceneRegistrationGateInsertFrame, node_argument) == 12);

Word address(const volatile void* value) noexcept {
    return reinterpret_cast<Word>(value);
}
Word word(const void* value) noexcept {
    return *static_cast<const volatile Word*>(value);
}
const volatile Word* profile(Word captured, NativeSceneRegistrationGateContext& c) {
    const volatile Word* result = nullptr;
    switch (captured) {
    case 0x00d62c88: result = c.actual_node_profile_00d62c88; break;
    case 0x00d62cf0: result = c.actual_camera_profile_00d62cf0; break;
    case 0x00d62f58: result = c.actual_light_profile_00d62f58; break;
    case 0x00d62fb0: result = c.actual_directional_profile_00d62fb0; break;
    default:
        if (c.derived) result = c.derived->resolve_profile(captured);
        break;
    }
    if (!result) throw std::logic_error("scene registration requires its captured actual profile");
    return result;
}
std::uint8_t accepts(Word target, void* node, Word token,
    NativeSceneRegistrationGateContext& c) {
    switch (target) {
    case 0x00b6f570: return c.light_types.node_is_type_00b6f570(token);
    case 0x00b7c580: return c.light_types.light_is_type_00b7c580(token);
    case 0x00b7c6d0: return c.light_types.directional_is_type_00b7c6d0(token);
    case 0x00b71ce0:
        if (!c.camera_types)
            throw std::logic_error("scene registration requires current camera type storage");
        return c.camera_types->is_type_00b71ce0(token);
    default:
        if (c.derived) return c.derived->invoke_type_0c(target, node, token);
        throw std::logic_error("scene registration lacks its genuine current type target");
    }
}
bool is_light(void* captured_node, NativeSceneRegistrationGateContext& c) {
    const volatile Word* const captured_profile = profile(word(captured_node), c);
    const Word token = current_native_light_type_token_00b7aa70(
        c.light_types.storage().light_0109018c.own_id);
    const Word current_target = captured_profile[3];
    return accepts(current_target, captured_node, token, c) != 0;
}
} // namespace

std::uint32_t current_native_light_type_token_00b7aa70(
    const volatile std::uint32_t& actual_token_0109018c) noexcept {
    return actual_token_0109018c;
}

void register_native_scene_node_if_light_00b83d50(void* actual_resource,
    NativeSceneRegistrationGateInsertFrame& f, NativeSceneRegistrationGateContext& c,
    NativeSceneRegistryRegistrationAcquired& acquired) {
    void* const captured_node = reinterpret_cast<void*>(f.node_argument); // B83D55
    if (!is_light(captured_node, c)) return; // B83D5A..B83D6D
    f.registry.key_argument = address(&f.node_argument); // B83D6F/B83D73
    f.registry.output_argument = address(f.result_00); // B83D74/B83D78
    void* const registry = static_cast<std::byte*>(actual_resource) + 0x14;
    f.node_argument = address(captured_node); // B83D7C, AFTER both argument pushes
    insert_native_scene_registry_00b83700(registry, f.registry, c.registry, acquired);
}

void unregister_native_scene_node_if_light_00b83ec0(void* actual_resource,
    NativeSceneRegistrationGateEraseFrame& f, NativeSceneRegistrationGateContext& c) {
    void* const captured_node = reinterpret_cast<void*>(f.node_argument); // B83EC2
    if (!is_light(captured_node, c)) return; // B83EC7..B83EDA
    f.registry.key_or_count_argument = address(&f.node_argument); // B83EDC/B83EE0
    void* const registry = static_cast<std::byte*>(actual_resource) + 0x14;
    f.node_argument = address(captured_node); // B83EE4
    erase_native_scene_registry_key_00b83e50(registry, f.registry, c.registry);
}
} // namespace bsp
