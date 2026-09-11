#pragma once
#include <cstddef>
#include <cstdint>

namespace bsp {
// Borrow all nine immutable original DWORDs of each reached physical-buffer
// profile. Native addresses identify data, not callable host vtables. Existing
// two-argument construction admits pooled profiles only; each private table is
// required only when its corresponding current identity is actually reached.
struct NativeRendererResetReadinessProfiles final {
    const volatile std::uint32_t* const pooled_index_00d61e58;
    const volatile std::uint32_t* const pooled_vertex_00d61e7c;
    const volatile std::uint32_t* const private_index_00d61e10;
    const volatile std::uint32_t* const private_vertex_00d61e34;
    NativeRendererResetReadinessProfiles(const volatile std::uint32_t* index,
        const volatile std::uint32_t* vertex,
        const volatile std::uint32_t* private_index = nullptr,
        const volatile std::uint32_t* private_vertex = nullptr) noexcept
        : pooled_index_00d61e58(index), pooled_vertex_00d61e7c(vertex),
          private_index_00d61e10(private_index), private_vertex_00d61e34(private_vertex) {}
};
static_assert(sizeof(NativeRendererResetReadinessProfiles) == 16);

// Complete B492B0/B49180, each 276 bytes plus its four-entry pool jump table.
// ECX actual 2Ch wrapper, unused EDX, stacked actual device, RET4. The actual
// device callee argument word is also the CreateBuffer output cell. No output
// initialization, HRESULT branch or unsupported-pool policy is introduced.
// Raw fields +14 flags/+18 capacity/+28 owned COM; exact COM AddRef/Release and
// current callee-slot reloads. No semantic D3D9BufferBinding layout is accepted.
void __fastcall recreate_native_physical_vertex_buffer_00b492b0(
    void* actual_wrapper, void* unused_edx, void* actual_device);
void __fastcall recreate_native_physical_index_buffer_00b49180(
    void* actual_wrapper, void* unused_edx, void* actual_device);

// Complete B4B810/B4B9C0: each is exactly RET4. No receiver/device dereference,
// COM call or lifetime operation; general registers, flags and FP state remain
// untouched. Explicit unused EDX keeps the unused device argument stacked.
void __fastcall recreate_native_private_index_buffer_00b4b810(
    void* actual_wrapper, void* unused_edx, void* actual_device);
void __fastcall recreate_native_private_vertex_buffer_00b4b9c0(
    void* actual_wrapper, void* unused_edx, void* actual_device);

// Complete B1FD90[70] behavior for four proven pooled/private buffer profiles.
// Native ECX renderer/RET; this new interface adds profiles in EDX. Current
// ready+1D8C/lost+1D8A gate; ready=1 publishes before the first profile call. Reload
// second wrapper+1978/profile/current device+1A10/slot+20 in native order.
// No guard, rollback, extra HRESULT or semantic return. Other virtual profiles
// remain outside the explicit source boundary, not a general native restriction.
void __fastcall restore_native_renderer_dynamic_buffers_00b1fd90(
    void* actual_renderer, const NativeRendererResetReadinessProfiles&);

// Complete B20C50[31]. ECX raw platform; byte+41 gates real GetFocus(), then
// compare its HWND with the CURRENT raw HWND+30. EAX exactly 0/1, plain RET.
// No context, injected OS callback, focus change, window creation or UI action.
std::uint32_t __fastcall native_platform_has_focus_00b20c50(
    const void* actual_platform) noexcept;
} // namespace bsp
