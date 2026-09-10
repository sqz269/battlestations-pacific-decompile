#include "bsp/native_shadow_texture_access.hpp"

namespace bsp {
namespace {
bool fail(std::string& error, const char* message) {
    error = message;
    return false;
}
bool target_profile(const NativeShadowDepthTargetFields& target, std::string& error) {
    if (!target.actual_owner || target.vtable_00 != 0x00d5b5e8)
        return fail(error, "Shadow target requires the actual D5B5E8 profile");
    return true;
}
bool texture_profile(const NativeD3D9ShadowTexture& texture, std::string& error) {
    if (!texture.actual_owner || texture.vtable_00 != 0x00d61948)
        return fail(error, "Shadow texture requires the actual D61948 profile");
    return true;
}
NativeD3D9ShadowTexture* resolve_texture(void* actual,
    NativeShadowTextureResolver& resolver, std::string& error) {
    auto* texture = resolver.resolve_texture(actual);
    if (!texture || texture->actual_owner != actual) {
        fail(error, "Actual shadow texture identity has no concrete binding");
        return nullptr;
    }
    return texture_profile(*texture, error) ? texture : nullptr;
}
bool get_texture(const NativeShadowMapView& shadow, bool depth,
    SystemShadowTexture*& result, std::string& error) {
    // A8FCF0/A8FD10 capture the global once, then test +0C before +0D.
    void* const actual_target = shadow.global_target_f8bbf0;
    if (!actual_target)
        return fail(error, "Native shadow texture getter requires the global target");
    auto* target = shadow.resolver.resolve_target(actual_target);
    if (!target || target->actual_owner != actual_target)
        return fail(error, "Actual global shadow target identity has no concrete binding");
    if (!target_profile(*target, error)) return false;
    void* actual_texture;
    if (target->enabled_0c != 0 && target->created_0d != 0)
        actual_texture = depth ? get_native_shadow_depth_texture_00a8fdb0(*target)
                               : get_native_shadow_color_texture_00a8fd90(*target);
    else
        actual_texture = shadow.fallback_texture_384;
    // The native getter may return null; its caller decides whether it can use it.
    if (!actual_texture) {
        result = nullptr;
        return true;
    }
    auto* texture = resolve_texture(actual_texture, shadow.resolver, error);
    if (!texture) return false;
    result = texture;
    return true;
}
bool write_dimensions(void* actual_texture, std::uint32_t width, std::uint32_t height,
    NativeShadowTextureResolver& resolver, std::string& error) {
    if (!actual_texture)
        return fail(error, "Native shadow toggle requires its actual texture wrapper");
    auto* texture = resolve_texture(actual_texture, resolver, error);
    return texture && set_native_texture_reported_dimensions_00b3ceb0(
        *texture, width, height, error);
}
} // namespace

void* get_native_shadow_color_texture_00a8fd90(const NativeShadowDepthTargetFields& target) {
    return target.color_texture_10;
}
void* get_native_shadow_depth_texture_00a8fdb0(const NativeShadowDepthTargetFields& target) {
    return target.depth_texture_18;
}
bool get_native_shadow_depth_texture_00a8fcf0(const NativeShadowMapView& shadow,
    SystemShadowTexture*& result, std::string& error) {
    return get_texture(shadow, true, result, error);
}
bool get_native_shadow_color_texture_00a8fd10(const NativeShadowMapView& shadow,
    SystemShadowTexture*& result, std::string& error) {
    return get_texture(shadow, false, result, error);
}
bool get_native_texture_width_00b3ce50(const NativeD3D9ShadowTexture& texture,
    std::uint32_t& result, std::string& error) {
    if (!texture_profile(texture, error)) return false;
    result = texture.reported_width_28;
    return true;
}
bool get_native_texture_height_00b3ce60(const NativeD3D9ShadowTexture& texture,
    std::uint32_t& result, std::string& error) {
    if (!texture_profile(texture, error)) return false;
    result = texture.reported_height_2c;
    return true;
}
bool NativeD3D9ShadowTexture::width_3c(std::uint32_t& result, std::string& error) {
    return get_native_texture_width_00b3ce50(*this, result, error);
}
bool NativeD3D9ShadowTexture::height_40(std::uint32_t& result, std::string& error) {
    return get_native_texture_height_00b3ce60(*this, result, error);
}
bool NativeShadowMapView::texture_08(SystemShadowTexture*& result, std::string& error) {
    return get_native_shadow_depth_texture_00a8fcf0(*this, result, error);
}
bool NativeShadowMapView::texture_0c(SystemShadowTexture*& result, std::string& error) {
    return get_native_shadow_color_texture_00a8fd10(*this, result, error);
}
bool set_native_texture_reported_dimensions_00b3ceb0(NativeD3D9ShadowTexture& texture,
    std::uint32_t width, std::uint32_t height, std::string& error) {
    if (!texture_profile(texture, error)) return false;
    texture.reported_width_28 = width;
    texture.reported_height_2c = height;
    return true;
}
bool toggle_native_shadow_target_00a8fdd0(NativeShadowDepthTargetFields& target,
    std::uint8_t enabled, NativeShadowTextureResolver& resolver, std::string& error) {
    if (!target_profile(target, error)) return false;
    const bool created = target.created_0d != 0; // CMP precedes the +0C write.
    target.enabled_0c = enabled;
    if (!created) return true;
    if (enabled != 0) {
        const std::uint32_t height = target.height_08;
        const std::uint32_t width = target.width_04;
        void* const color = target.color_texture_10;
        if (!write_dimensions(color, width, height, resolver, error)) return false;
        const std::uint32_t next_height = target.height_08;
        const std::uint32_t next_width = target.width_04;
        void* const depth = target.depth_texture_18;
        return write_dimensions(depth, next_width, next_height, resolver, error);
    }
    void* const color = target.color_texture_10;
    if (!write_dimensions(color, 8, 8, resolver, error)) return false;
    void* const depth = target.depth_texture_18;
    return write_dimensions(depth, 8, 8, resolver, error);
}
} // namespace bsp
