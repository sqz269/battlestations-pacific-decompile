#pragma once

#include <cstdint>

namespace bsp {

// Complete 00B21EC0..00B21F0A, translated to a new MSVC Win32 C++ interface.
// Native ECX is the renderer; stack arguments are adapter format, engine
// resource flags, resource kind, and check format, in that order; RET 10h.
// Native AL contains the boolean result. This API is not a binary replacement.
//
// Borrow actual raw renderer storage through offset 1993h. Its current +1990h
// publication must point to a valid IDirect3D9 object with the live COM table
// and CheckDeviceFormat method at +28h. All reached DWORD cells are readable
// and four-byte aligned. The fixed original adapter index 0 and HAL kind 1
// are passed with the caller's format values and original resource kind.
//
// Full 00B20A80 translates flags into private usage/pool cells before the
// current factory/table/method loads. The pool result is unused. Success is
// the signed HRESULT >= 0, including nonzero success values. No renderer
// writes, null handling, validation, or result substitution is introduced.
//
// Private native/C++ argument-slot aliases, incidental registers, exact fault
// and asynchronous mutation behavior, and original unwind ABI are excluded.
bool check_native_renderer_device_format_00b21ec0(
    void* actual_renderer, std::uint32_t adapter_format,
    std::uint32_t engine_resource_flags, std::uint32_t resource_kind,
    std::uint32_t check_format);

} // namespace bsp
