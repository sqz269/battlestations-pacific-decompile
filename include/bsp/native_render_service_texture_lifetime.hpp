#pragma once
#include "bsp/gui_native_geometry.hpp"

namespace bsp {

using NativeRenderServiceTextureDecrement = long (__stdcall*)(volatile long*);

// Borrow the SAME canonical registry used by NativeTextureLoadOwners and the
// concrete post-effect owner companions, plus actual current IAT/table views.
// No replacement counter, pool, owner map, native table or callback is created.
struct NativeRenderServiceTextureLifetimeContext {
    NativeRenderActualOwners& actual_owners;
    NativeRenderServiceTextureDecrement const volatile& decrement_iat_00ce2220;
    const volatile std::uint32_t* const actual_profile_00d61948;
    const volatile std::uint32_t* const actual_profile_00d61ec8;
    const volatile std::uint32_t* const actual_profile_00d62074;
};

// Complete original control flow in the admitted concrete domains. B52270:
// ECX actual CCh owner, no stack arguments, RET. Visit +30,+68,+88,+B0 in order;
// capture CE2220 once before the first field. Nonzero auxiliaries are actual24h
// B4E840 post-effect owners (D61EC8 -> BD30E0 -> B4E450), NOT textures.
void release_native_render_service_texture_auxiliaries_00b52270(
    void* actual_owner, NativeRenderServiceTextureLifetimeContext&);

// B52400..B52543,324 bytes, ECX actual CCh owner, RET. Stamp D62074; release
// auxiliaries; capture +18 then a fresh CE2220 epoch; release 2D textures
// +18,+3C,+70,+98; destroy vectors +A4,+8C,+7C,+24; restore CEB130. Five native
// unwind states project to C++ cleanup only. No later raw reference is released
// after an earlier failure. Current-field clears occur only after return.
void destroy_native_render_service_textures_00b52400(
    void* actual_owner, NativeRenderServiceTextureLifetimeContext&);

// B52840..B5285D: original ECX owner, DWORD flags on stack, EAX captured owner,
// RET4. Destroy first; shared CRT free only when flags&1 and destruction returns.
// The returned address may be freed. No implicit recovery or late cleanup.
void* delete_native_render_service_textures_00b52840(void* actual_owner,
    std::uint32_t flags, NativeRenderServiceTextureLifetimeContext&);

// Caller-prepared, address-stable host companion. Bind only after successful
// B52550, before usable publication, in the SAME canonical registry. Binding
// itself does not allocate, initialize the actual +04 atomic, retain or release.
// The registry's transactional bind may allocate host metadata; callers needing
// allocation-free binding must prepare that provider's capacity separately.
// The caller must supply an already live aligned +04 atomic in the established
// MSVC raw-owner domain (B52550 begins it at its original B52578 count1 write).
// Registration is transactional and performs no native
// effects; a diagnostic leaves the completed native allocation caller-owned.
// Once bound, final release owns B52840. Do not call its raw destructor/deleter
// separately. Keep this companion, context and registry alive until retirement;
// then external quiescence permits destruction of the host companion. No raw
// storage read follows native free. Native retirement does not dispose this host.
class NativeRenderServiceTextureReference final : public RenderCommandReference {
public:
    NativeRenderServiceTextureReference(void* actual_owner,
        NativeRenderServiceTextureLifetimeContext&, const GuiNativeGeometryRegistration&);
    ~NativeRenderServiceTextureReference() override;
    NativeRenderServiceTextureReference(const NativeRenderServiceTextureReference&) = delete;
    NativeRenderServiceTextureReference& operator=(const NativeRenderServiceTextureReference&) = delete;
    void release_zero_references() noexcept override;
    bool retired() const noexcept;
private:
    enum class Phase { bound, destroying, retired };
    void* const identity_;
    NativeRenderServiceTextureLifetimeContext& context_;
    const GuiNativeGeometryRegistration& registration_;
    Phase phase_{Phase::bound};
};

// Accessed receiver/vector/owner storage and current table/IAT bindings remain
// valid at each native access; canonical lookup is pure and terminal companions
// borrow exactly captured+04. 2D owners use NativeTextureLoadOwners; post-effect
// owners use the concrete B4E450 companion. The inherited terminal is noexcept:
// unsupported dispatch or an escaping child exception terminates through that
// interface. Raw source entries retain their C++ cleanup projection, not native
// FH3/SEH identity. No arbitrary profiles, binary ABI, concurrency or game proof.
} // namespace bsp
