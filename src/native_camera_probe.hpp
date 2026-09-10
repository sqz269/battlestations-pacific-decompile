#pragma once
#include "bsp/native_camera_owner.hpp"
#include <memory>

// Isolated installed-asset probe composition, not a reconstructed game factory.
// Runtime, shared type counter/descriptors, strings, allocator list and renderer
// publication must outlive this object and its final explicit close.
struct NativeCameraProbeEvidence {
    DWORD constructor_width{}, constructor_height{};
    bool same_backing{}, creator_one{}, context_two{}, logical_one{};
    bool camera_retired{}, context_retired{}, pool_returned{}, pool_destroyed{};
    bool checked() const noexcept;
};
class NativeCameraProbe final {
public:
    NativeCameraProbe(bsp::NativeNodeDestructionRuntime&, bsp::TypeIdCounterLifetime&,
        bsp::LightTypeBootstrap&, bsp::AllocatorListDomain&,
        bsp::D3D9StateCache* const volatile& current_renderer,
        bsp::NativeViewportRendererAccess&, NativeCameraProbeEvidence&);
    ~NativeCameraProbe();
    NativeCameraProbe(const NativeCameraProbe&) = delete;
    NativeCameraProbe& operator=(const NativeCameraProbe&) = delete;
    bsp::NativeCameraOwner& owner() noexcept;
    const bsp::CameraViewport& viewport() const noexcept;
    void close() noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
