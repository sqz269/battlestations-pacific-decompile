#pragma once
#include "bsp/native_scene_registry_registration.hpp"
#include <cstdint>

namespace bsp {
class LightTypeBootstrap;
class CameraTypeBootstrap;

// Required only for a reached additional profile/target. Resolution is pure
// metadata lookup: no native reads beyond the supplied identity, stores,
// callbacks, initialization or ownership changes. Return the borrowed actual
// numeric table, never a host vtable. invoke_type_0c must bind the genuine
// reached target; it returns AL, not a bool conversion of an entire register.
class NativeSceneRegistrationDerivedTypes {
public:
    virtual ~NativeSceneRegistrationDerivedTypes() = default;
    virtual const volatile std::uint32_t* resolve_profile(
        std::uint32_t captured_profile) const = 0;
    virtual std::uint8_t invoke_type_0c(std::uint32_t current_target,
        void* actual_node, std::uint32_t captured_token) = 0;
};
struct NativeSceneRegistrationGateContext {
    NativeSceneRegistryRegistrationContext& registry;
    LightTypeBootstrap& light_types;
    CameraTypeBootstrap* camera_types{};
    const volatile std::uint32_t* actual_node_profile_00d62c88{};
    const volatile std::uint32_t* actual_camera_profile_00d62cf0{};
    const volatile std::uint32_t* actual_light_profile_00d62f58{};
    const volatile std::uint32_t* actual_directional_profile_00d62fb0{};
    NativeSceneRegistrationDerivedTypes* derived{};
};
struct NativeSceneRegistrationGateInsertFrame {
    volatile std::uint32_t result_00[3]; // native local12B, including padding
    volatile std::uint32_t node_argument; // original incoming word becomes key
    NativeSceneRegistryRegistrationInsertFrame registry;
};
struct NativeSceneRegistrationGateEraseFrame {
    volatile std::uint32_t node_argument;
    NativeSceneRegistryEraseKeyFrame registry;
};

// B7AA70[6], MOV EAX,[0109018C]; RET. Borrow the actual light descriptor's
// current own_id cell, without running initialization or reading cached IDs.
std::uint32_t current_native_light_type_token_00b7aa70(
    const volatile std::uint32_t& actual_token_0109018c) noexcept;

// Complete B83D50[62]/B83EC0[51]. Native ECX is actual3Ch resource, stack+4
// node, RET4. Capture node/profile, read CURRENT type token, then captured
// table's CURRENT0C. On nonzero AL overwrite the actual incoming argument
// cell with CAPTURED node and call registry at captured resource+14h.
void register_native_scene_node_if_light_00b83d50(void* actual_resource,
    NativeSceneRegistrationGateInsertFrame&, NativeSceneRegistrationGateContext&,
    NativeSceneRegistryRegistrationAcquired&);
void unregister_native_scene_node_if_light_00b83ec0(void* actual_resource,
    NativeSceneRegistrationGateEraseFrame&, NativeSceneRegistrationGateContext&);

// Caller supplies initialized CURRENT token cells and profile views from the
// SAME actual process domain. camera_types, when reached, must share those
// light/node/root cells. Known numeric profile identities are checked before
// table use; unknown profiles/targets require explicit genuine derived
// bindings. No unknown-profile base predicate or logical scene fallback.
// Frames are address-stable initialized preimages. Nested frames preserve
// opaque words; result padding is untouched. Acquired is fresh, disjoint and
// survives any insertion failure; a rejected predicate leaves it untouched.
// All reached callback-modified pointers/cells must remain valid. No native
// retain/release, admission or rollback is added. Registry keys are actual
// node pointers, never host companion addresses. Source invalid-domain errors,
// private-stack aliases, saved registers, native FH3/ABI/application closure,
// node/root attachment and terminal ownership are outside this interface.
} // namespace bsp
