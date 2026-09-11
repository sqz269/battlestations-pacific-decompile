#pragma once

namespace bsp {

// B1FEF0: ECX actual renderer, EAX borrowed device at +1A10h, RET. No AddRef.
void* get_native_renderer_device_00b1fef0(const void* actual_renderer) noexcept;

// Complete105-byte save/release operations over actual owner fields: COM+08,
// bytecode+0C. Captured AddRef/Release precedes current GetFunction calls;
// native PUSH ECX initializes the size-query DWORD to actual owner-pointer bits.
// Actual CRT allocation, current pointer reloads and publication are retained.
void save_release_native_pixel_shader_00b5e750(void* actual_owner);
void save_release_native_vertex_shader_00b5e810(void* actual_owner);

// Complete68-byte restore operations. The second argument is the address of
// actual four-byte F8D394 global storage, not its current renderer value.
// Read it only after owner COM-null/bytecode-nonnull gates; call the genuine
// getter and actual device COM method. Free current bytecode then clear +0C.
void restore_native_pixel_shader_00b5e890(
    void* actual_owner, const void* actual_renderer_global_f8d394);
void restore_native_vertex_shader_00b5e8e0(
    void* actual_owner, const void* actual_renderer_global_f8d394);

// New MSVC Win32 C++ APIs over borrowed actual storage; valid reached extents
// and COM objects required. No owner construction, HRESULT policy or rollback.
} // namespace bsp
