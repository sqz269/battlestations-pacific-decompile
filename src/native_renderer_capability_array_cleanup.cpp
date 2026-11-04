#include "bsp/native_renderer_capability_array_cleanup.hpp"
#include "bsp/native_renderer_pointer_array_destroy.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/wire_goal4_chain351_switches.hpp"

namespace bsp {
// Complete native reserve95 / resize80 / destructor23 groups are identical
// after masking only CALL displacements. The native leaf targets are the same
// BF55BE allocator and BF6989 free; share the substantive raw resize body.
void __fastcall resize_native_capability_dwords_00b260b0(
    void* header, std::uint32_t, std::int32_t count) {
    resize_native_renderer_vertex_pointers_00b25940(header, 0, count);
}
void __fastcall destroy_native_capability_dwords_00b29e40(void* header) {
    if constexpr (kWireGoal4Chain351Bound) {
        // B29E45 calls B260B0(header, 0), B29E4A reloads data, B29E4D frees
        // it through BF6989. This is the actual renderer+1B18h+44h member.
        resize_native_capability_dwords_00b260b0(header, 0, 0);
        singleton_lifetime_free(reinterpret_cast<void*>(
            *static_cast<volatile std::uint32_t*>(header)));
    } else {
        destroy_native_renderer_vertex_pointers_00b29b20(header);
    }
}
} // namespace bsp
