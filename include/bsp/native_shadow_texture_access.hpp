#pragma once
#include "bsp/system_lighting_constants.hpp"

namespace bsp {
// Companions borrow these exact native fields. They neither allocate owners nor
// create textures, retain objects, or substitute nominal COM dimensions.
struct NativeShadowDepthTargetFields {
    void* actual_owner;
    const std::uint32_t& vtable_00;
    std::uint32_t& width_04;
    std::uint32_t& height_08;
    std::uint8_t& enabled_0c;
    std::uint8_t& created_0d;
    void*& color_texture_10;
    void*& depth_texture_18;
};

class NativeD3D9ShadowTexture final : public SystemShadowTexture {
public:
    NativeD3D9ShadowTexture(void* identity, const std::uint32_t& actual_vtable,
        std::uint32_t& actual_width28, std::uint32_t& actual_height2c)
        : actual_owner(identity), vtable_00(actual_vtable),
          reported_width_28(actual_width28), reported_height_2c(actual_height2c) {}
    bool width_3c(std::uint32_t&, std::string&) override;
    bool height_40(std::uint32_t&, std::string&) override;
    void* const actual_owner;
    const std::uint32_t& vtable_00;
    std::uint32_t& reported_width_28;
    std::uint32_t& reported_height_2c;
};

// Binding lookups must not mutate native state, dispatch callbacks, or retain.
// Return the companion for this exact identity, or nullptr when unavailable.
// Bindings and actual owners must survive the immediately following operation.
class NativeShadowTextureResolver {
public:
    virtual ~NativeShadowTextureResolver() = default;
    virtual NativeShadowDepthTargetFields* resolve_target(void* actual_owner) = 0;
    virtual NativeD3D9ShadowTexture* resolve_texture(void* actual_owner) = 0;
};

class NativeShadowMapView final : public SystemShadowMapOwner {
public:
    NativeShadowMapView(const std::array<SystemLightingMatrixWords, 4>& matrices,
        const SystemLightingWords3& direction, const SystemLightingWords4& limits,
        void* const& actual_global_slot, void* const& actual_fallback384,
        NativeShadowTextureResolver& bindings)
        : SystemShadowMapOwner(matrices, direction, limits),
          global_target_f8bbf0(actual_global_slot), fallback_texture_384(actual_fallback384),
          resolver(bindings) {}
    bool texture_08(SystemShadowTexture*&, std::string&) override;
    bool texture_0c(SystemShadowTexture*&, std::string&);
    void* const& global_target_f8bbf0;
    void* const& fallback_texture_384;
    NativeShadowTextureResolver& resolver;
};

// Native ECX=this, borrowed EAX result, RET. These host interfaces have a new ABI.
void* get_native_shadow_color_texture_00a8fd90(const NativeShadowDepthTargetFields&);
void* get_native_shadow_depth_texture_00a8fdb0(const NativeShadowDepthTargetFields&);
bool get_native_shadow_depth_texture_00a8fcf0(
    const NativeShadowMapView&, SystemShadowTexture*&, std::string&);
bool get_native_shadow_color_texture_00a8fd10(
    const NativeShadowMapView&, SystemShadowTexture*&, std::string&);
bool get_native_texture_width_00b3ce50(
    const NativeD3D9ShadowTexture&, std::uint32_t&, std::string&);
bool get_native_texture_height_00b3ce60(
    const NativeD3D9ShadowTexture&, std::uint32_t&, std::string&);
// B3CEB0: ECX=wrapper, stack width then height, RET8. Captures both before stores.
bool set_native_texture_reported_dimensions_00b3ceb0(
    NativeD3D9ShadowTexture&, std::uint32_t width, std::uint32_t height, std::string&);
// A8FDD0: ECX=target, stack low byte is preserved verbatim, RET4.
bool toggle_native_shadow_target_00a8fdd0(NativeShadowDepthTargetFields&,
    std::uint8_t enabled, NativeShadowTextureResolver&, std::string&);
} // namespace bsp
