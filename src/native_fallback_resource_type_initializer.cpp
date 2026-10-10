#include "bsp/native_fallback_resource_type_initializer.hpp"

#include <cstddef>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native fallback resource type initializer requires MSVC Win32 pointer widths.
#endif

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(offsetof(TypeIdCounterStorage, next_id_04) == 4);

void initialize_native_fallback_resource_type_00b86a00(
    const NativeFallbackResourceTypeInitializerContext& context) {
    if (context.guard_0109020d != 0) return;
    context.guard_0109020d = 1;
    context.receiver[3] = context.literal_address_00d631f4;

    context.scene_types.initialize_scene_resource_00b869c0(context.scene_01090210);
    const std::uint32_t scene = context.scene_01090210[0];
    context.receiver[1] = scene;
    const std::uint32_t root = context.scene_01090210[1];
    context.receiver[2] = root;

    volatile auto* const counter = context.counter.get_006fac20();
    const std::uint32_t id = counter->next_id_04;
    counter->next_id_04 = id + 1u;
    context.receiver[0] = id;
}

} // namespace bsp
