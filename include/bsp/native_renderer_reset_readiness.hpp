#pragma once
#include <cstddef>
#include <cstdint>

namespace bsp {
// Borrow all nine immutable original DWORDs of the two pooled physical buffer
// profiles. These native addresses are data identities, not callable host
// vtables. The private physical profiles have different +20 selectors and are
// outside this renderer dynamic-wrapper source contract.
struct NativeRendererResetReadinessProfiles final {
    const volatile std::uint32_t* const pooled_index_00d61e58;
    const volatile std::uint32_t* const pooled_vertex_00d61e7c;
    NativeRendererResetReadinessProfiles(const volatile std::uint32_t* index,
        const volatile std::uint32_t* vertex) noexcept
        : pooled_index_00d61e58(index), pooled_vertex_00d61e7c(vertex) {}
};
static_assert(sizeof(NativeRendererResetReadinessProfiles) == 8);

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

// Complete B1FD90[70] behavior for the proven pooled dynamic-wrapper profiles.
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
