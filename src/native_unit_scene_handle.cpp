#include "bsp/native_unit_scene_handle.hpp"

#include <stdexcept>

namespace bsp {

void* native_unit_scene_handle_006d1e80(const NativeUnitSceneHandleView& view) noexcept {
    return view.scene_handle_4a4;
}

void publish_native_unit_scene_handle_00955448(NativeUnitSceneHandleView view,
    void* actual_eax) noexcept {
    view.scene_handle_4a4 = actual_eax;
}

void clear_unit_scene_handle_on_killed_00951fb0(NativeUnitSceneHandleView view,
    const NativeUnitKilledSceneHandleProvider& provider) {
    if (!provider.on_killed_00779af0)
        throw std::logic_error("unit killed scene-handle tail requires the actual00779AF0 provider");
    view.scene_handle_4a4 = nullptr;
    provider.on_killed_00779af0(provider.context, view.unit.canonical_unit);
}

} // namespace bsp
