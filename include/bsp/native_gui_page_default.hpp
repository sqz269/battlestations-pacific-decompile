#pragma once
#include <cstdint>

namespace bsp {
struct NativeGuiPageDefaultBindings {
    const volatile std::uint32_t& scalar_00d7a2f0;
    const volatile std::uint32_t& scalar_00ce3804;
    const volatile std::uint32_t& one_00d7a24c;
};
// Complete normal AA3840[105] body. Native ECX actual fresh124h page storage,
// EAX same pointer, RET. Calls genuine raw AA9390(type1), then stamps D5BE38
// and applies ordered CURRENT scalar reads/stores. This new explicit binding
// ABI is not a factory, a logical GuiLayerImage, or script constructor AC6600.
// Does not allocate/publish the page or initialize EC..FC/120..123. Existing
// base unwritten bytes and allocator metadata at+124 retain their preimage.
// Base owns its source allocation-failure cleanup; this routine adds no EH,
// rollback, destructor or slot return. Native FH3/ABI/game parity is unclaimed.
void* construct_native_gui_page_default_00aa3840(void* actual_page,
    NativeGuiPageDefaultBindings&);
} // namespace bsp
