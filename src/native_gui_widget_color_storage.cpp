#include "bsp/native_gui_widget_color_storage.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/gui_widget_owner.hpp"
#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "Raw GUI color uses DWORD pointers.");

void* current_pointer(const void* storage, std::size_t offset) noexcept {
    const auto bits = *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<const std::byte*>(storage) + offset);
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(bits));
}

// Raw projection of the already-established whole B74650[12B/4 starts].
// The older public helper requires a NativeModelOwner; no such facade is
// introduced over this actual receiver. No separate B7 body/ABI credit.
bool has_geometry_00b74650(const void* actual_model) noexcept {
    return current_pointer(actual_model, 0x180) != nullptr;
}

// Whole B74640 field access; its native stacked DWORD is unused.
// Read the captured Model's data word without forming a typed tail view.
void* raw_model_geometry_00b74640(const void* actual_model) noexcept {
    void* geometry;
    __asm {
        mov ecx, actual_model
        mov eax, dword ptr [ecx + 180h]
        mov geometry, eax
    }
    return geometry;
}
} // namespace

void set_native_gui_widget_color_storage_00aa6870(void* actual_widget,
    const void* original_rgba) noexcept {
    const auto* const source = static_cast<const volatile std::uint32_t*>(original_rgba);
    auto* const color = reinterpret_cast<volatile std::uint32_t*>(
        static_cast<std::byte*>(actual_widget) + 0x50);
    std::uint32_t bits = source[0];
    color[0] = bits;
    bits = source[1];
    color[1] = bits;
    bits = source[2];
    void* model = current_pointer(actual_widget, 0x4c);
    color[2] = bits;
    bits = source[3];
    color[3] = bits;

    if (!has_geometry_00b74650(model)) return;
    model = current_pointer(actual_widget, 0x4c);
    void* mesh = raw_model_geometry_00b74640(model);
    if (gui_mesh_element_count_00b72b40(mesh) == 0) return;
    model = current_pointer(actual_widget, 0x4c);
    mesh = raw_model_geometry_00b74640(model);
    void* const element = gui_geometry_element_00b732c0(mesh, 0);
    void* const material = current_pointer(element, 0x20);
    auto* const diffuse = reinterpret_cast<volatile std::uint32_t*>(
        native_material_diffuse_00b179f0(*static_cast<NativeMaterialStorage*>(material), 0));
    bits = source[0];
    diffuse[0] = bits;
    bits = source[1];
    diffuse[1] = bits;
    bits = source[2];
    diffuse[2] = bits;
    bits = source[3];
    diffuse[3] = bits;
}
} // namespace bsp
