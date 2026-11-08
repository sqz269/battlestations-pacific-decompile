#include "bsp/native_renderer_record_array_cleanup.hpp"
#include "bsp/native_renderer_pointer_array_destroy.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
// Native instruction equivalence: destructors23, resizes80, reserves95 bytes,
// masking only rel32 CALL operands. Share the resize implementation, including
// its failure path; retain each destructor's address-specific resize operation
// through the equivalent zero-count clear entry below.
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

// Evidence: bsp.py show 00B282F0/00B28300 --asm, entire eight-byte bodies.
// B29B60/B29B80 use the identical resize(0) before reloading/freeing current
// data; retain the real header and failure/free schedule through this call.
__declspec(noinline) void __fastcall clear_native_renderer_vertex_shader_registry_00b282f0(void* header) {
    resize_native_renderer_vertex_shader_registry_00b25cf0(header, 0, 0);
}
__declspec(noinline) void __fastcall clear_native_renderer_pixel_shader_registry_00b28300(void* header) {
    resize_native_renderer_pixel_shader_registry_00b25d40(header, 0, 0);
}

void __fastcall destroy_native_renderer_vertex_shader_registry_00b29b60(void* header) {
    // B29B65 calls B25CF0(header, 0), then B29B6A reloads the current data
    // for BF6989. The renderer owns this actual +1AC4h array header.
    clear_native_renderer_vertex_shader_registry_00b282f0(header);
    singleton_lifetime_free(reinterpret_cast<void*>(
        *static_cast<volatile std::uint32_t*>(header)));
}
void __fastcall destroy_native_renderer_pixel_shader_registry_00b29b80(void* header) {
    // B29B85 -> B25D40, B29B8A reload, B29B8D -> BF6989; renderer+1AD0h.
    clear_native_renderer_pixel_shader_registry_00b28300(header);
    singleton_lifetime_free(reinterpret_cast<void*>(
        *static_cast<volatile std::uint32_t*>(header)));
}
void __fastcall destroy_native_renderer_third_state_cache_00b28090(void* header) {
    // B28095 -> B22E90, B2809A reload, B2809D -> BF6989; renderer+1ADCh.
    resize_native_renderer_third_state_cache_00b22e90(header, 0, 0);
    singleton_lifetime_free(reinterpret_cast<void*>(
        *static_cast<volatile std::uint32_t*>(header)));
}
} // namespace bsp
