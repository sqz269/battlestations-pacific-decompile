#pragma once

#include "bsp/native_hardware_layout_owner.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

// Borrow actual globals, ownership providers and the immutable original-token
// D62AF4 table. Supported incoming/retained layouts have that hardware profile:
// +00 BD30E0, +04 B60770, +08 B5FF00. These DWORDs are evidence, never callable
// host pointers. Other profiles require a separately established binding.
struct NativeRendererVertexLayoutBindingContext {
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc;
    NativeHardwareLayoutOwnerContext& actual_hardware_layout_owner;
    const volatile std::uint32_t* actual_layout_profile_00d62af4;
};

// Complete B23F20..B24002: native ECX renderer, stack layout, RET4; no stable
// EAX result. Borrow raw renderer+17B4 retained layout, +1A10 real D3D9 device,
// and +1BAC wrapping counter. Outer identity bypasses all work. Guard entry
// precedes a fresh retained-owner capture; replacement publishes/retains the
// incoming owner before releasing the captured old one through real providers.
// A nonnull original input binds its borrowed COM declaration; null does not
// unbind the device. HRESULT is ignored. Counter/normal leave follow returning
// work only. Exceptions invoke the actual optional guard, without rollback.
//
// This is a new MSVC Win32 C++ interface, not a native ABI replacement. It
// inherits the actual optional guard's uninitialized-record/current-mode domain.
void bind_native_renderer_vertex_layout_00b23f20(void* actual_renderer,
    void* actual_input_layout, NativeRendererVertexLayoutBindingContext&);

} // namespace bsp
