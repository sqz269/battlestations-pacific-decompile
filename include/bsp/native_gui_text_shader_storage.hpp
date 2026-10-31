#pragma once
#include "bsp/native_gui_text_reference.hpp"
#include "bsp/native_font_resources.hpp"
#include "bsp/native_platform_load_messages.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Text shader storage requires MSVC Win32.
#endif

namespace bsp {
struct NativeGuiTextRawFontNameStorageContext;
class NativeGuiTextRawFontNameStorageFrame;
struct NativeGuiTextRawFontNameStorageAcquired;
class NativeGuiTextRawShaderNameStorageInvocation;
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
    // Pure immutable raw preparation. No Font export/lookup, native cell/name
    // read or acquisition; the legacy default constructor keeps this null.
    explicit NativeGuiTextShaderStorageFrame(const NativeGuiTextShaderStorageContext&) noexcept;
    // Credited raw preparation additionally pins the SAME canonical122 frame.
    // This constructor is pure; only a genuine normal raw AB8CE0 invocation
    // can issue the private credit. The old constructors issue no authority.
    NativeGuiTextShaderStorageFrame(const NativeGuiTextShaderStorageContext&,
        const NativeGuiTextReferenceFrame&) noexcept;
    const NativeGuiTextShaderStorageContext* prepared_raw_context() const noexcept {
        return prepared_raw_context_;
    }
    const NativeGuiTextReferenceFrame* prepared_reference_frame() const noexcept {
        return prepared_reference_frame_;
    }

    enum class RawPhase { fresh, entered, complete, failed };
    enum class CreditPhase { none, outstanding, reserved, returned };
    enum class Consumer { none, font_name, retirement, shader_name };
    // A value snapshot of private metadata, never an ownership capability.
    // None of these values may be supplied back to authorize an operation.
    struct CreditProgress {
        RawPhase producer{RawPhase::fresh};
        CreditPhase credit{CreditPhase::none};
        Consumer consumer{Consumer::none};
        bool failed{}, active{}, capture_entered{}, decrement_entered{};
        bool decrement_returned{}, zero_entered{}, zero_returned{}, slot_cleared{};
        bool consumer_returned{};
        bool retirement_started{}; // snapshot of private one-shot entry, including null/no-producer
        // CURRENT invocation, separate from the historical nonnull consumer
        // below. Authority requires active && !failed. On failure these are
        // retained diagnostic identities only; snapshots never follow a token.
        Consumer active_consumer{Consumer::none};
        const NativeGuiTextRawFontNameStorageContext* active_font_context{};
        const NativeGuiTextRawFontNameStorageFrame* active_font_frame{};
        const NativeGuiTextRawFontNameStorageAcquired* active_font_acquired{};
        const NativeGuiTextRawModelRetirementContext* active_retirement_context{};
        const NativeGuiTextRetirementAcquired* active_retirement_acquired{};
        const NativeGuiTextRetirementShader* active_retirement_shader{};
        void* actual_text{};
        void* effect{}; // diagnostic after terminal; never dereference it
        const NativeGuiTextRawModelReference* reference{};
        const NativeGuiTextRawModelReferenceContext* reference_context{};
        const NativeGuiTextReferenceFrame* reference_frame{};
        const NativeMaterialEffectCacheAcquired* cache{};
        const NativeGuiTextRawFontNameStorageContext* font_context{};
        const NativeGuiTextRawFontNameStorageFrame* font_frame{};
        const NativeGuiTextRawFontNameStorageAcquired* font_acquired{};
        const NativeGuiTextRawModelRetirementContext* retirement_context{};
        const NativeGuiTextRetirementAcquired* retirement_acquired{};
        const NativeGuiTextRetirementShader* retirement_shader{};
        NativeTextureSurfaceReferenceIncrement captured_decrement{};
        long decrement_result{};
        const NativeGuiTextRawShaderNameStorageInvocation* active_shader_invocation{};
        const NativeGuiTextRawShaderNameStorageInvocation* shader_invocation{};
    };
    CreditProgress credit_progress() const noexcept;
    bool matches_credit_backing(const NativeGuiTextShaderStorageContext&,
        const NativeGuiTextReferenceFrame&) const noexcept;

    // Only the exact genuine outer friends below can construct this token. Their
    // typed private-body policy carries it to Original's reached epochs; no
    // public entry can construct a token or manufacture issuance/completion.
    // State is retained IN ShaderFrame. A local token owns no native credit.
    class CreditConsumer final {
    public:
        ~CreditConsumer() noexcept;
        CreditConsumer(const CreditConsumer&) = delete;
        CreditConsumer& operator=(const CreditConsumer&) = delete;
        CreditConsumer(CreditConsumer&&) = delete;
        CreditConsumer& operator=(CreditConsumer&&) = delete;
        // AFTER Original CURRENT1EC capture (and raw118's state6 store), before
        // the reached CURRENT CE2220 sample/call. Null never returns a credit.
        void capture(void* captured_current_shader);
        void decrement_entered(NativeTextureSurfaceReferenceIncrement);
        void decrement_returned(long actual_result);
        void zero_entered();
        void zero_returned();
        void slot_cleared(); // AFTER Original post-callback Text1EC store
        // ONLY the real equal-name branch may report no capture. It keeps any
        // outstanding credit. Resize/copy/lookup escapes cannot use this path.
        void finish_without_capture();
        // AFTER the entire outer native body returns normally, including all
        // later raw118 members/base work. Never polls an ended native payload.
        void complete();
        void fail() noexcept;
    private:
        friend void set_native_gui_text_font_name_storage_00ab8c30(
            NativeGuiTextRawModelReference&, const void*,
            NativeGuiTextRawFontNameStorageContext&,
            NativeGuiTextRawFontNameStorageFrame&,
            NativeGuiTextRawFontNameStorageAcquired&);
        friend void destroy_native_gui_text_storage_00ab8250(void*,
            NativeGuiTextRawModelRetirementContext&, NativeGuiTextRetirementAcquired&);
        CreditConsumer(NativeGuiTextShaderStorageFrame&,
            NativeGuiTextRawModelReference&,
            const NativeGuiTextRawFontNameStorageContext&,
            const NativeGuiTextRawFontNameStorageFrame&,
            const NativeGuiTextRawFontNameStorageAcquired&);
        CreditConsumer(NativeGuiTextShaderStorageFrame&, void* actual_text,
            NativeGuiTextRawModelRetirementContext&, NativeGuiTextRetirementAcquired&);
        friend void set_native_gui_text_shader_name_storage_00ab8e70(
            NativeGuiTextRawModelReference&, const void*, NativeGuiTextRawShaderNameStorageInvocation&);
        CreditConsumer(NativeGuiTextShaderStorageFrame&, NativeGuiTextRawModelReference&,
            const NativeGuiTextRawShaderNameStorageInvocation&);
        enum class Stage { opened, no_capture, null_capture, captured,
            decrement_entered, decremented, zero_entered, zero_returned, cleared, complete, failed };
        NativeGuiTextShaderStorageFrame& frame_;
        const Consumer kind_;
        const NativeGuiTextRawFontNameStorageContext* font_context_{};
        const NativeGuiTextRawFontNameStorageFrame* font_frame_{};
        const NativeGuiTextRawFontNameStorageAcquired* font_acquired_{};
        NativeGuiTextRawModelRetirementContext* retirement_context_{};
        NativeGuiTextRetirementAcquired* retirement_acquired_{};
        Stage stage_{Stage::opened};
        bool active_{};
        const NativeGuiTextRawShaderNameStorageInvocation* shader_invocation_{};
        void require_stage(Stage);
    };
    ~NativeGuiTextShaderStorageFrame() noexcept;
    NativeGuiTextShaderStorageFrame(const NativeGuiTextShaderStorageFrame&) = delete;
    NativeGuiTextShaderStorageFrame& operator=(const NativeGuiTextShaderStorageFrame&) = delete;
private:
    friend bool ensure_native_gui_text_font_shader_storage_00ab8ce0(
        NativeGuiTextRawModelReference&, NativeGuiTextShaderStorageContext&,
        NativeGuiTextShaderStorageFrame&);
    const NativeGuiTextShaderStorageContext* const prepared_raw_context_{};
    const NativeGuiTextReferenceFrame* const prepared_reference_frame_{};
    CreditProgress credit_;
    class RawProducer;
    struct RawEvidence {
        void* renderer{};
        std::uint32_t profile{}, target{};
        void* result{};
        bool entered{}, returned{}, published{};
    } raw_evidence_;
    // An active no-credit/equal-name invocation is distinct from the single
    // irreversible nonnull credit consumer recorded in credit_.
    const CreditConsumer* active_consumer_{};
    bool retirement_started_{};
    void authenticate_bound_reference(const NativeGuiTextRawModelReference&);
    void begin_raw(NativeGuiTextRawModelReference&, NativeGuiTextShaderStorageContext&);
    void complete_raw(NativeGuiTextRawModelReference&, NativeGuiTextShaderStorageContext&,
        bool actual_attempted);
    void fail_raw() noexcept;
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
// Qualified raw-Model93/122 transport over the SAME body. Reserve this exact
// immutably prepared Frame/context in raw122 BEFORE binding. CURRENT48 must be
// genuine B318B0; an unknown reached target fails after prior effects.
// A cached fast return or attempted-null is NOT a new Text effect credit.
// Preserve this frame/real Font/cache/Lua/platform/provider graph through any
// failure. No raw Font export, cached1EC retirement or production admission.
bool ensure_native_gui_text_font_shader_storage_00ab8ce0(NativeGuiTextRawModelReference&,
    NativeGuiTextShaderStorageContext&, NativeGuiTextShaderStorageFrame&);
} // namespace bsp
