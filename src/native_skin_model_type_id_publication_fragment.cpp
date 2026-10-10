#include "bsp/native_skin_model_type_id_publication_fragment.hpp"

#include "bsp/light_type_bootstrap.hpp"

namespace bsp {

void publish_native_skin_model_type_id_fragment_00cd8516(
    TypeIdCounterLifetime& same_counter_lifetime,
    volatile std::uint32_t& actual_current_01090344) {
    volatile TypeIdCounterStorage* const counter =
        same_counter_lifetime.get_006fac20();
    const std::uint32_t old_id = counter->next_id_04;
    counter->next_id_04 = old_id + std::uint32_t{1};
    actual_current_01090344 = old_id;
}

} // namespace bsp
