#include "bsp/native_renderer_remaining_array_cleanup.hpp"
#include "bsp/native_procedural_resource_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
void free_current_data(void* header) noexcept {
    const auto current = *static_cast<const volatile std::uint32_t*>(header);
    singleton_lifetime_free(reinterpret_cast<void*>(current));
}
} // namespace

void __fastcall resize_native_renderer_array_00b22ee0(
    void* header, std::uint32_t, std::int32_t requested) {
    resize_native_procedural_pointer_array_00737390(header, requested);
}
void __fastcall resize_native_renderer_array_00b22f30(
    void* header, std::uint32_t, std::int32_t requested) {
    resize_native_procedural_pointer_array_00737390(header, requested);
}
void __fastcall resize_native_renderer_array_00b22f80(
    void* header, std::uint32_t, std::int32_t requested) {
    resize_native_procedural_pointer_array_00737390(header, requested);
}
void __fastcall destroy_native_renderer_array_00b280b0(void* header) {
    resize_native_renderer_array_00b22ee0(header, 0, 0);
    free_current_data(header);
}
void __fastcall destroy_native_renderer_array_00b280d0(void* header) {
    resize_native_renderer_array_00b22f30(header, 0, 0);
    free_current_data(header);
}
void __fastcall destroy_native_renderer_array_00737bf0(void* header) {
    resize_native_procedural_pointer_array_00737390(header, 0);
    free_current_data(header);
}
void __fastcall destroy_native_renderer_array_00b280f0(void* header) {
    resize_native_renderer_array_00b22f80(header, 0, 0);
    free_current_data(header);
}
} // namespace bsp
