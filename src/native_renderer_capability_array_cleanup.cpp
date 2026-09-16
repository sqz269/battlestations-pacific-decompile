#include "bsp/native_renderer_capability_array_cleanup.hpp"
#include "bsp/native_renderer_pointer_array_destroy.hpp"

namespace bsp {
// Complete native reserve95 / resize80 / destructor23 groups are identical
// after masking only CALL displacements. The native leaf targets are the same
// BF55BE allocator and BF6989 free; share their substantive raw implementation.
void __fastcall resize_native_capability_dwords_00b260b0(
    void* header, std::uint32_t, std::int32_t count) {
    resize_native_renderer_vertex_pointers_00b25940(header, 0, count);
}
void __fastcall destroy_native_capability_dwords_00b29e40(void* header) {
    destroy_native_renderer_vertex_pointers_00b29b20(header);
}
} // namespace bsp
