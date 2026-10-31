#pragma once

namespace bsp {
struct NativeSceneResourceStorageContext;
struct NativeSceneResourceConstructFrame;
struct NativeSceneResourceAcquired;
struct NativeAmbientIdentityContext;
namespace detail { struct NativeSceneTypedStorageAccess; }

// Source extent facts, not Native fields, reference credits or retry permits.
// Only the actual constructor/lifetime implementation can issue/change them.
// Keep these nonmoving records in the same retained construction acquisition.
class NativeSceneResourceTypedLifetime final {
public:
    NativeSceneResourceTypedLifetime() noexcept = default;
    NativeSceneResourceTypedLifetime(const NativeSceneResourceTypedLifetime&) = delete;
    NativeSceneResourceTypedLifetime& operator=(const NativeSceneResourceTypedLifetime&) = delete;
private:
    friend void* construct_native_scene_resource_00b83c50(void*,
        NativeSceneResourceConstructFrame&, NativeSceneResourceStorageContext&,
        NativeSceneResourceAcquired&);
    friend struct detail::NativeSceneTypedStorageAccess;
    void* actual_{};
    NativeSceneResourceStorageContext* context_{};
    bool born_{}, returned_{}, destroying_{}, ended_{}, failed_{};
};
class NativeAmbientTypedLifetime final {
public:
    NativeAmbientTypedLifetime() noexcept = default;
    NativeAmbientTypedLifetime(const NativeAmbientTypedLifetime&) = delete;
    NativeAmbientTypedLifetime& operator=(const NativeAmbientTypedLifetime&) = delete;
private:
    friend void* construct_native_ambient_00b7c290(void*, const void*,
        NativeAmbientTypedLifetime&, NativeAmbientIdentityContext&) noexcept;
    friend struct detail::NativeSceneTypedStorageAccess;
    void* actual_{};
    NativeAmbientIdentityContext* context_{};
    bool born_{}, returned_{}, destroying_{}, ended_{}, failed_{};
};
} // namespace bsp
