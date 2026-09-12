#include "bsp/scene_landscape_class.hpp"

#include "bsp/entity_class_ids.hpp"
#include "bsp/scene_file.hpp"

namespace bsp {

std::uint32_t scene_landscape_is_kind_of_004f1360(
    std::int32_t dynamic_class_id, std::int32_t requested_class) noexcept {
    // 004F1364,004F1369,004F136E,004F1372, in native compare order.
    return requested_class == kSceneLandscapeClassId || requested_class == 1
        || requested_class == kEntityClassIdRoot || requested_class == dynamic_class_id
        ? 1u : 0u;
}

} // namespace bsp
