#include "bsp/native_renderer_record_array_cleanup.hpp"
#include "bsp/native_renderer_pointer_array_destroy.hpp"

namespace bsp {
// Whole-chain native instruction equivalence: destructors23, resizes80,
// reserves95 bytes, masking only rel32 CALL operands. All leaf allocator/free
// targets match. Share the actual implementation, including its failure path.
void __fastcall resize_native_renderer_vertex_shader_registry_00b25cf0(
    void* header, std::uint32_t, std::int32_t count) {
    resize_native_renderer_vertex_pointers_00b25940(header, 0, count);
}
void __fastcall resize_native_renderer_pixel_shader_registry_00b25d40(
    void* header, std::uint32_t, std::int32_t count) {
    resize_native_renderer_vertex_pointers_00b25940(header, 0, count);
}
void __fastcall resize_native_renderer_third_state_cache_00b22e90(
    void* header, std::uint32_t, std::int32_t count) {
    resize_native_renderer_vertex_pointers_00b25940(header, 0, count);
}

void __fastcall destroy_native_renderer_vertex_shader_registry_00b29b60(void* header) {
    destroy_native_renderer_vertex_pointers_00b29b20(header);
}
void __fastcall destroy_native_renderer_pixel_shader_registry_00b29b80(void* header) {
    destroy_native_renderer_vertex_pointers_00b29b20(header);
}
void __fastcall destroy_native_renderer_third_state_cache_00b28090(void* header) {
    destroy_native_renderer_vertex_pointers_00b29b20(header);
}
} // namespace bsp
