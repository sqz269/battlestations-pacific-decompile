#pragma once
#include "bsp/native_gui_text_reference.hpp"
#include "bsp/native_font_resources.hpp"
#include "bsp/native_platform_load_messages.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Text shader storage requires MSVC Win32.
#endif

namespace bsp {
// Reached unknown current renderer48 targets require a genuine body. Numeric
// profile/target values are selectors, never callable host addresses. The
// provider receives retained acquisition backing BEFORE any nested effects.
// It must return one owned actual effect credit (or native null), in the SAME
// canonical owner domain. It cannot substitute a semantic effect or no-op.
class NativeGuiTextShaderRendererCalls {
public:
    virtual ~NativeGuiTextShaderRendererCalls() = default;
    virtual void* call_renderer_48(void* actual_renderer,
        std::uint32_t captured_profile, std::uint32_t captured_target,
        const void* actual_name_header8, NativeMaterialEffectCacheContext&,
        NativeMaterialEffectCacheAcquired&) = 0;
};
struct NativeGuiTextShaderStorageContext {
    NativeFontResourceOwners& fonts; // SAME existing image custody; no name lookup here
    // SAME already-existing actual0109CF04 pointer cell used for native +28.
    // This is not the effect loader's ResourceLoadEventHost adapter cell.
    NativePlatformLoadPublication configuration_0109cf04;
    NativeGuiTextShaderRendererCalls* unknown_renderer48{};
};

// One independently retained invocation, not a native resource owner or an
// extra cache/count. Both actual8B header addresses and nested acquisition
// backing exist BEFORE entry. names[] deliberately has no initialization;
// each native41E870 constructor writes its reached header. Header0 is the
// empty comparison wrapper, reused for GuiFont; header1 is GuiFontBilinear.
struct NativeGuiTextShaderStorageFrame final {
    enum class Phase { fresh, entered, complete, failed };
    enum class Boundary { none, metadata, string_construct, string_return, renderer48 };
    Phase phase{Phase::fresh};
    Boundary boundary{Boundary::none};
    Boundary escaped_boundary{Boundary::none};
    volatile std::uint32_t names[4];
    NativeMaterialEffectCacheAcquired cache;
    void* destination{};
    const void* selected_name{};
    void* captured_configuration{};
    void* captured_font{}; // borrowed identity, never another retain
    void* captured_renderer{};
    void* result{}; // may already belong to Text1EC; diagnostic only
    std::uint32_t renderer_profile{}, renderer_target{}, active_call_site{}, escaped_call_site{};
    std::int32_t native_eh_state{-1};
    bool cached{}, override_selected{}, height_matches{}, font_scale_matches{};
    bool empty_constructed{}, empty_compared{}, empty_returned{}, default_constructed{};
    bool renderer_entered{}, renderer_returned{}, shader_published{}, cleanup_entered{};
    bool cleanup_returned{}, exception_cleanup_started{}, attempted{};
    void* captured_name_data{};
    std::uint32_t captured_name_bytes{};

    NativeGuiTextShaderStorageFrame() noexcept = default;
    ~NativeGuiTextShaderStorageFrame() noexcept;
    NativeGuiTextShaderStorageFrame(const NativeGuiTextShaderStorageFrame&) = delete;
    NativeGuiTextShaderStorageFrame& operator=(const NativeGuiTextShaderStorageFrame&) = delete;
};

// Complete normal AB8CE0..AB8E6C[397B/118 instructions]. Native ECX actual
// Text/no arguments/RET; AL means selection ATTEMPTED, even a null result.
// New C++ interface, not native private-stack/thiscall/FH3/SEH/FP-exception ABI.
//
// Requires SAME completed full93/canonical122 Text. Cache1EC nonnull returns
// false before name/config/font/renderer reads. Otherwise actual1C0 comparison
// and pooled default headers feed CURRENT renderer48; known B318B0 executes
// the existing real cache body using this reference's SAME effects context.
// Other targets require unknown_renderer48; absent is an explicit boundary.
// Default selection uses native ordered SSE CVTSI2SS(actual config+28)==720,
// then ONLY on match raw Font108+18==liveone. No typed Text/descriptor metrics.
// Publish returned owned credit into actual1EC before selected-name cleanup;
// no retain/release/second cache or registry is added.
//
// Caller keeps the genuine93/122/renderer/cache/Font/actual config, raw strings,
// VFS/compiler/Lua binding and complete nested provider graph alive through
// return/failure disposition. Existing effect-loader activation obligations
// are unchanged. Serialize native lifetime/publications, exclude receiver
// retirement during remaining reads, and keep this frame disjoint from ALL
// context/provider/resource/input backing. Known checks cannot prove opaque
// transitive provenance or disjointness. Nested invocations need distinct frames.
//
// C++ escape projects only observed outer native name states0/1: consume state
// before raw41DD20; a cleanup escape terminates. This is not proof of native
// exception origin or complete native unwind equivalence. Never roll back a
// published shader or release an unpublished acquired credit implicitly. Keep
// the failed frame, SAME raw backing and whole independent graph; no reset,
// retry, dependency drain or owner rediscovery from diagnostic identities.
// A failed/entered frame destructor terminates rather than abandoning it.
// No raw content/glyph/property/factory admission or host activation is added.
bool ensure_native_gui_text_font_shader_storage_00ab8ce0(NativeGuiTextReference&,
    NativeGuiTextShaderStorageContext&, NativeGuiTextShaderStorageFrame&);
} // namespace bsp
