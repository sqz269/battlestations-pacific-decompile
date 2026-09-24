#include "bsp/native_bright_pass_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
void* delete_native_bright_pass_00b101a0(
    void* actual_owner, std::uint32_t flags, NativeRenderEffectLifetimeContext& context) {
    destroy_native_render_effect_base_00b0f5e0(actual_owner, context); // B101A3
    if ((flags & 1u) != 0) singleton_lifetime_free(actual_owner); // B101B0
    return actual_owner;
}
} // namespace bsp
