#include "bsp/native_camera_resource_initializer_fragment.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native camera resource fragment requires MSVC Win32 pointer widths.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(offsetof(TypeIdCounterStorage, next_id_04) == 4);

void initialize_native_camera_resource_fragment_00cd8390(
    const NativeCameraResourceInitializerFragmentContext& context) {
    if (context.guard_01090266 != 0) return;
    context.guard_01090266 = 1;
    context.name_address_01090294 = context.literal_address_00d6327c;

    context.scene_types.initialize_scene_resource_00b869c0(context.scene_01090210);
    const std::uint32_t scene = context.scene_01090210[0];
    const std::uint32_t root = context.scene_01090210[1];
    context.scene_id_0109028c = scene;
    context.root_id_01090290 = root;

    volatile auto* const counter = context.counter.get_006fac20();
    const std::uint32_t id = counter->next_id_04;
    counter->next_id_04 = id + 1u;
    context.own_id_01090288 = id;
}

} // namespace bsp
