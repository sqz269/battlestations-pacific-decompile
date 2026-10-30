#pragma once
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Text failure-origin transport requires the qualified MSVC Win32 Source runtime.
#endif

namespace bsp {
class NativeGuiTextFailureOrigin;
class NativeGuiTextFailureOriginRuntime;
class NativeGuiTextFailureHeaderArm;
class NativeGuiTextFailureConstructorSlotArm;
class NativeGuiTextFailureConstructorCall;
struct NativeGuiTextGlyphQuadStorageContext;
struct NativeGuiTextGlyphQuadStorageAcquired;
struct NativeGuiTextRawModelDefaultStorageFrame;
struct NativeGuiTextRawModelDefaultStorageContext;
struct NativeGuiTextRawModelDefaultStorageAcquired;

// Identity only. No exception_ptr capture/copy, payload ownership or Native credit.
// Default invalid is not an authenticated current cause.
class NativeGuiTextFailureCause final {
public:
    NativeGuiTextFailureCause() noexcept = default;
    bool valid() const noexcept { return runtime_ && object_ && descriptor_; }
private:
    friend class NativeGuiTextFailureOriginRuntime;
    friend class NativeGuiTextFailureOrigin;
    friend class NativeGuiTextFailureHeaderArm;
    friend class NativeGuiTextFailureConstructorSlotArm;
    friend class NativeGuiTextFailureConstructorCall;
    const NativeGuiTextFailureOriginRuntime* runtime_{};
    const void* object_{};
    const void* descriptor_{};
    bool same(const NativeGuiTextFailureCause& r) const noexcept {
        return valid() && runtime_ == r.runtime_ && object_ == r.object_ && descriptor_ == r.descriptor_;
    }
};

// Pure preparation, NOT verification or a Native capability. Caller must bind
// this SAME stable object only to the own MSVC14.51 Source C++ runtime/catches
// qualified by receipts423/874. Do not query another CRT, outside active catch,
// on another fiber/runtime epoch, or to discover whether its PTD exists.
class NativeGuiTextFailureOriginRuntime final {
public:
    enum class Contract { qualified_own_msvc_14_51_source_catch };
    explicit NativeGuiTextFailureOriginRuntime(Contract) noexcept {}
    NativeGuiTextFailureOriginRuntime(const NativeGuiTextFailureOriginRuntime&) = delete;
    NativeGuiTextFailureOriginRuntime& operator=(const NativeGuiTextFailureOriginRuntime&) = delete;
    NativeGuiTextFailureCause read_current_catch() const; // SDK call; no general noexcept promise
};

// Appended caller-retained metadata only. Edge identity is invocation + exact
// static Source site/native site + SAME supplier + stage. Normal edges hold no
// escaping permission; failed invocation/denial is never reset/replayed.
class NativeGuiTextFailureOrigin final {
public:
    enum class Verdict : std::uint8_t { unset, host, unclassified, proven_native };
    enum class Stage : std::uint8_t { ordinary, cleanup_secondary };
    struct Site final { std::uint32_t native{}, source{}; };
    struct Edge final { Site site; const void* supplier{}; Stage stage{Stage::ordinary};
        Site parent_site{}; const void* parent_supplier{}; };
    NativeGuiTextFailureOrigin() noexcept = default;
    NativeGuiTextFailureOrigin(const NativeGuiTextFailureOrigin&) = delete;
    NativeGuiTextFailureOrigin& operator=(const NativeGuiTextFailureOrigin&) = delete;
    void begin(const NativeGuiTextFailureOriginRuntime&, const void* supplier, std::uint32_t entry) noexcept;
    void complete() noexcept;
    void retain_current(const NativeGuiTextFailureOriginRuntime&);
    Verdict verdict() const noexcept { return verdict_; } // diagnostics only
    bool committed() const noexcept { return verdict_ != Verdict::unset; }
    bool requires_retention() const noexcept { return phase_ == Phase::entered || phase_ == Phase::failed; }

    template<class F> decltype(auto) unknown(const NativeGuiTextFailureOriginRuntime& r,
        Site site, const void* supplier, F&& f, Stage stage = Stage::ordinary) {
        const Edge previous = enter({site, supplier, stage});
        try {
            if constexpr (std::is_void_v<std::invoke_result_t<F>>) {
                std::forward<F>(f)(); leave(previous);
            } else {
                decltype(auto) result = std::forward<F>(f)(); leave(previous); return result;
            }
        } catch (...) {
            const auto cause = r.read_current_catch();
            if (stage == Stage::cleanup_secondary) secondary(cause, {site, supplier, stage});
            else commit(Verdict::unclassified, cause);
            throw;
        }
    }
    // Used ONLY around concrete added Source admission/check/authentication work,
    // never around mixed/native callbacks or inferred from public diagnostics.
    template<class F> decltype(auto) host(const NativeGuiTextFailureOriginRuntime& r,
        Site site, const void* supplier, F&& f) {
        const Edge previous = enter({site, supplier, Stage::ordinary});
        try {
            if constexpr (std::is_void_v<std::invoke_result_t<F>>) {
                std::forward<F>(f)(); leave(previous);
            } else {
                decltype(auto) result = std::forward<F>(f)(); leave(previous); return result;
            }
        } catch (...) { commit(Verdict::host, r.read_current_catch()); throw; }
    }
    template<class F> decltype(auto) child(const NativeGuiTextFailureOriginRuntime& r,
        Site site, const void* supplier, std::uint32_t entry,
        const NativeGuiTextFailureOrigin& child, F&& f) {
        const Edge previous = enter({site, supplier, Stage::ordinary});
        try {
            if constexpr (std::is_void_v<std::invoke_result_t<F>>) {
                std::forward<F>(f)(); leave(previous);
            } else {
                decltype(auto) result = std::forward<F>(f)(); leave(previous); return result;
            }
        } catch (...) {
            const auto cause = r.read_current_catch();
            adopt(cause, child, supplier, entry); throw;
        }
    }
private:
    friend class NativeGuiTextFailureHeaderArm;
    friend class NativeGuiTextFailureConstructorSlotArm;
    friend class NativeGuiTextFailureConstructorCall;
    enum class Phase : std::uint8_t { fresh, entered, complete, failed };
    Phase phase_{Phase::fresh};
    Verdict verdict_{Verdict::unset}; // fresh unset differs from committed unknown
    const NativeGuiTextFailureOriginRuntime* runtime_{};
    const void* supplier_{};
    std::uint32_t entry_{};
    Edge active_{};
    Edge escaped_{};
    NativeGuiTextFailureCause cause_;
    bool valid_{true};
    bool secondary_committed_{};
    NativeGuiTextFailureCause secondary_cause_;
    Edge secondary_edge_{};
    const NativeGuiTextFailureOrigin* forwarded_child_{};
    Edge forwarded_child_edge_{};
    const void* native_issuer_{}; // NO issuer or Native mint operation exists in this slice
    Edge enter(Edge) noexcept;
    void leave(Edge) noexcept;
    void commit(Verdict, NativeGuiTextFailureCause) noexcept;
    void secondary(NativeGuiTextFailureCause, Edge) noexcept;
    void adopt(NativeGuiTextFailureCause, const NativeGuiTextFailureOrigin&,
        const void*, std::uint32_t) noexcept;
};

// Separate one-shot arm per actual header lifetime. Normal return is consumed
// BEFORE the raw returning getter. Destruction has no action/policy.
class NativeGuiTextFailureHeaderArm final {
public:
    NativeGuiTextFailureHeaderArm() noexcept = default;
    NativeGuiTextFailureHeaderArm(const NativeGuiTextFailureHeaderArm&) = delete;
    NativeGuiTextFailureHeaderArm& operator=(const NativeGuiTextFailureHeaderArm&) = delete;
    void arm(const NativeGuiTextFailureOrigin&, const void* header, std::uint32_t state) noexcept;
    void normal_return_entered() noexcept;
    void returned() noexcept;
    void return_failed() noexcept;
    bool consume_proven_primary(const NativeGuiTextFailureOrigin&,
        const NativeGuiTextFailureCause&, const void* actual_header, std::uint32_t state) noexcept;
private:
    enum class Phase : std::uint8_t { fresh, armed, normal_return, unwind_return, returned, failed };
    Phase phase_{Phase::fresh};
    const NativeGuiTextFailureOrigin* invocation_{};
    const void* header_{};
    std::uint32_t state_{};
};
// DISTINCT from the eight-byte HeaderArm. This retained metadata binds the
// address of Glyph home7; it never captures a replacement cleanup operand.
// All operations are private to the actual Glyph entry. There is no issuer.
class NativeGuiTextFailureConstructorSlotArm final {
public:
    NativeGuiTextFailureConstructorSlotArm() noexcept = default;
    NativeGuiTextFailureConstructorSlotArm(const NativeGuiTextFailureConstructorSlotArm&) = delete;
    NativeGuiTextFailureConstructorSlotArm& operator=(const NativeGuiTextFailureConstructorSlotArm&) = delete;
private:
    friend class NativeGuiTextFailureConstructorCall;
    friend void write_native_gui_text_glyph_quad_storage_00ab98f0(void*,
        volatile std::uint32_t (&)[10], NativeGuiTextGlyphQuadStorageContext&,
        NativeGuiTextGlyphQuadStorageAcquired&);
    enum class Phase : std::uint8_t { fresh, armed, disarmed, consumed, denied };
    Phase phase_{Phase::fresh};
    const NativeGuiTextFailureOrigin* invocation_{};
    const NativeGuiTextFailureOriginRuntime* runtime_{};
    volatile std::uint32_t* home_{};
    void* receiver_{};
    const void* provider_{};
    NativeGuiTextFailureOrigin::Site caller_edge_{};
    NativeGuiTextFailureCause escaped_cause_;
    void arm(NativeGuiTextFailureOrigin&, const NativeGuiTextFailureOriginRuntime&,
        volatile std::uint32_t*, void*, const void*, NativeGuiTextFailureOrigin::Site,
        NativeGuiTextFailureConstructorCall&) noexcept;
    void normal_disarm(NativeGuiTextFailureConstructorCall&) noexcept;
    bool consume_proven_primary(const NativeGuiTextFailureOrigin&,
        const NativeGuiTextFailureOriginRuntime&, const NativeGuiTextFailureConstructorCall&,
        volatile std::uint32_t* actual_home);
};

// Caller-retained noncopyable envelope, NEVER an escaping automatic local.
// Genuine raw93 privately binds its actual record once at entry. This is
// callee-entry binding, not proof that the caller preselected that record.
class NativeGuiTextFailureConstructorCall final {
public:
    NativeGuiTextFailureConstructorCall() noexcept = default;
    NativeGuiTextFailureConstructorCall(const NativeGuiTextFailureConstructorCall&) = delete;
    NativeGuiTextFailureConstructorCall& operator=(const NativeGuiTextFailureConstructorCall&) = delete;
private:
    friend class NativeGuiTextFailureConstructorSlotArm;
    friend void write_native_gui_text_glyph_quad_storage_00ab98f0(void*,
        volatile std::uint32_t (&)[10], NativeGuiTextGlyphQuadStorageContext&,
        NativeGuiTextGlyphQuadStorageAcquired&);
    friend void* construct_native_gui_text_default_storage_00ab9650(void*,
        NativeGuiTextRawModelDefaultStorageFrame&, NativeGuiTextRawModelDefaultStorageContext&,
        NativeGuiTextRawModelDefaultStorageAcquired&, NativeGuiTextFailureConstructorCall&);
    enum class Phase : std::uint8_t { fresh, prepared, calling, failed, returned, disarmed };
    Phase phase_{Phase::fresh};
    bool valid_{true}, bound_{}, native_body_returned_{}, failure_committed_{};
    NativeGuiTextFailureOrigin* invocation_{};
    NativeGuiTextFailureConstructorSlotArm* slot_{};
    const NativeGuiTextFailureOriginRuntime* runtime_{};
    volatile std::uint32_t* home_{};
    void* receiver_{};
    const void* provider_{};
    const void* callee_record_{};
    NativeGuiTextFailureOrigin* callee_origin_{};
    std::uint32_t callee_entry_{};
    NativeGuiTextFailureOrigin::Site caller_edge_{};
    NativeGuiTextFailureCause escaped_cause_;
    bool bind_once(void*, NativeGuiTextFailureOrigin&, const void*) noexcept;
    void body_returned(const NativeGuiTextFailureOrigin&, const void*) noexcept;
    void failed();
    template<class F> void* invoke(F&& f) {
        if (phase_ != Phase::prepared) valid_ = false;
        else phase_ = Phase::calling; // never reset a failed/reentered envelope
        const auto previous = invocation_->enter({caller_edge_, provider_,
            NativeGuiTextFailureOrigin::Stage::ordinary});
        try {
            void* const result = std::forward<F>(f)();
            invocation_->leave(previous);
            phase_ = Phase::returned; // Source provider return ONLY; no Native credit
            return result;
        } catch (...) { failed(); throw; }
    }
};
} // namespace bsp
