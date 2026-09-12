#include "bsp/mission_picture_owner.hpp"
#include "bsp/native_resource_cache_leaves.hpp"
#include <stdexcept>

namespace bsp {

void validate_mission_picture_services(const MissionPictureTextureServices& services) {
    const auto& resolve = services.resolve;
    if (!resolve.find_atlas_item || !resolve.load_texture || !resolve.width ||
        !resolve.height || !resolve.retain || !services.get_gui_manager_004c12b0 ||
        !services.size_preimage)
        throw std::invalid_argument("mission picture requires actual atlas/renderer, native reference and frame-size bindings");
}

MissionPictureOwner::MissionPictureOwner(NativeRenderActualOwners& owners) noexcept
    : owners_(&owners) {}

MissionPictureOwner::MissionPictureOwner(const MissionPictureOwner& source) noexcept
    : owners_(source.owners_), texture_(source.texture_) {
    // Same actual InterlockedIncrement(+04) as 005C85C7. This shared leaf
    // neither resolves a companion nor creates a second reference count.
    if (texture_) retain_native_cache_resource_004ddb20(texture_);
    uv_ = source.uv_;
}

MissionPictureOwner& MissionPictureOwner::operator=(const MissionPictureOwner& source) {
    void* const old = texture_;
    if (old != source.texture_) {
        auto* const old_owners = owners_;
        texture_ = source.texture_;
        owners_ = source.owners_;
        if (texture_) retain_native_cache_resource_004ddb20(texture_);
        if (old) release_native_render_actual_owner(*old_owners, old);
    }
    uv_ = source.uv_;
    return *this;
}

MissionPictureOwner::~MissionPictureOwner() { release_then_clear(); }

void MissionPictureOwner::release_then_clear() {
    void* const old = texture_;
    if (old) {
        release_native_render_actual_owner(*owners_, old);
        texture_ = nullptr;
    }
}

void MissionPictureOwner::read_005c6a70(
    std::string_view name, const MissionPictureTextureServices& services) {
    validate_mission_picture_services(services);
    if (owners_ != &services.actual_owners)
        throw std::invalid_argument("mission picture must use its canonical native owner domain");
    if (name.empty()) {
        release_then_clear();
        texture_ = nullptr;
        return;
    }
    services.get_gui_manager_004c12b0();
    auto size = services.size_preimage();
    void* const acquired = resolve_gui_texture_00aa2660(name, uv_, size, 1.0f, services.resolve);
    // AA2660 publishes UV before either the old zero callback or +A0 store.
    // Native valid-domain release is nonthrowing; no host rollback is added.
    release_then_clear();
    texture_ = acquired;
}

} // namespace bsp
