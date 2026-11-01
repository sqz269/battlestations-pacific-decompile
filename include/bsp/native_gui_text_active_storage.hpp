#pragma once
#include <cstdint>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual Text active storage requires MSVC Win32.
#endif

namespace bsp {
namespace game { class GameNativeReadOnlyData; }
struct NativeStringRawPoolContext;

// Required genuine child transport, not a numeric-code or semantic widget
// adapter. Authenticate the SAME captured actual child/producer record and
// captured current profile, then call existing raw AA8530 with its SAME live
// NativeGuiWidgetVisibilityContext. Only that body's documented byte-valued
// request boundary may narrow flags. This call receives the FULL captured
// DWORD; it must not reload the caller's home or the child's profile/slot.
// All captured children/ancestors/nodes and provider backing survive callbacks.
// The captured table belongs to the SAME verified RO owner as this context;
// metadata authentication must not reenter or mutate the call's Source records.
// No extra class query, retain, owner, fallback or readiness answer is added.
class NativeGuiTextActiveChild34Bindings {
public:
    virtual ~NativeGuiTextActiveChild34Bindings() = default;
    virtual void call_visible_34_00aa8530(void* actual_child,
        std::uint32_t captured_profile, std::uint32_t captured_target,
        std::uint32_t captured_flags_dword) = 0;
};

// Cold borrowed metadata only. The actual pool/manager/return-gate cells, live
// feature word and verified RO mappings are the caller's existing domains.
// Construct/retain before the first active call; no Text/pool/type is created.
class NativeGuiTextActiveStorageContext final {
public:
    NativeGuiTextActiveStorageContext(NativeStringRawPoolContext&,
        const volatile std::uint32_t& actual_feature_word_0109eea4,
        const game::GameNativeReadOnlyData&, NativeGuiTextActiveChild34Bindings&);
    NativeStringRawPoolContext& strings;
    const volatile std::uint32_t& feature;
    const game::GameNativeReadOnlyData& data;
    NativeGuiTextActiveChild34Bindings& children;
    const void* const left_literal_00d5c764;
    const void* const right_literal_00d5c750;
};

struct NativeGuiTextActiveNameStorage final {
    std::uint32_t header[2]{}; // actual8B LEN/DATA, not a semantic NativeString
    volatile std::uint32_t preserve_home{};
    volatile std::uint32_t lookup_homes[2]{}; // header pointer; unread zero HOME1
    void* captured_data{}; // diagnostic only after its normal header return
    void* captured_child{}; // unretained, caller-held through both dispatches
    bool resize_entered{}, resize_returned{}, copy_entered{}, copy_returned{};
    bool lookup_entered{}, lookup_returned{};
    // These describe the Source normal-return capability, including DATA=null
    // bypass. They do not assert that its internal getter/return both executed.
    bool header_return_entered{}, header_returned{};
};
class NativeGuiTextActiveStorageAcquired;
class NativeGuiTextActiveStorageFrame final {
public:
    NativeGuiTextActiveStorageFrame() = default;
    NativeGuiTextActiveStorageFrame(const NativeGuiTextActiveStorageFrame&) = delete;
    NativeGuiTextActiveStorageFrame& operator=(const NativeGuiTextActiveStorageFrame&) = delete;
    NativeGuiTextActiveNameStorage left, right;
private:
    friend void propagate_native_gui_text_active_storage_00ab87d0(void*,
        const volatile std::uint32_t&, NativeGuiTextActiveStorageContext&,
        NativeGuiTextActiveStorageFrame&, NativeGuiTextActiveStorageAcquired&);
    const NativeGuiTextActiveStorageAcquired* invocation_{};
};
struct NativeGuiTextActiveChild34Call final {
    void* receiver{}; // opaque after an escaping call/retirement
    std::uint32_t profile{}, target{}, flags_dword{};
    bool entered{}, returned{};
};
class NativeGuiTextActiveStorageAcquired final {
public:
    enum class Phase { fresh, entered, complete, failed };
    NativeGuiTextActiveStorageAcquired() = default;
    NativeGuiTextActiveStorageAcquired(const NativeGuiTextActiveStorageAcquired&) = delete;
    NativeGuiTextActiveStorageAcquired& operator=(const NativeGuiTextActiveStorageAcquired&) = delete;
    Phase phase() const noexcept { return phase_; }
    void* captured_text{};
    const volatile std::uint32_t* flags_home{}; // address only before late read
    std::uint32_t active_call_site{}, captured_flags_dword{};
    std::int32_t native_state{-1}; // scheduling observation, never origin/cleanup authority
    bool flags_captured{};
    NativeGuiTextActiveChild34Call left, right;
    std::exception_ptr failure;
private:
    friend void propagate_native_gui_text_active_storage_00ab87d0(void*,
        const volatile std::uint32_t&, NativeGuiTextActiveStorageContext&,
        NativeGuiTextActiveStorageFrame&, NativeGuiTextActiveStorageAcquired&);
    Phase phase_{Phase::fresh};
};

// Complete normal AB87D0..AB8901: native ECX captured Text, stack DWORD, RET4.
// Explicit Source ABI, not Native private frames/register/FH3/SEH/fault ABI.
// No Text active-byte store. Build/find/return left, then right. Only BOTH
// nonnull: capture left profile, read full CURRENT flag DWORD ONCE, then slot34;
// call left, then reload right profile/slot34 and forward the SAME captured DWORD.
// Only genuine AA8530 is bound by this interface; other reached targets are
// explicit unresolved Source failures, never numeric calls or successful no-ops.
// AA7E00 searches direct children; its second physical HOME is unread.
//
// Supply genuine live actual Text/list/child/node/name/pool/RO/provider backing
// and a live aligned flags HOME, externally serialized. Keep frame/acquired,
// context/derived providers and all transitive borrowed backing mutually
// disjoint and alive through callbacks/escape. Each invocation uses fresh
// retained nonmoving frame/acquired storage. Callbacks must not mutate these
// scheduling/diagnostic records or retire either captured child before its
// remaining use. Genuine callees still write their supplied raw header/homes.
// These caller proofs are not inferred from profiles, public diagnostics or
// successful lookup. No new pool/Model/count/retain/selector/host is supplied.
//
// Entered Source failures record/rethrow and RETAIN partial headers/provider
// debt; neither destructor performs Native cleanup, refund, retry or termination.
// This does not classify a Native exception origin or implement Original EH
// header-cleanup actions. The actual owner must retain the entire graph on an
// escape; no dead header/DATA/child rediscovery follows. Normal helper return
// ends that header buffer's usable lifetime even though its stale bits remain.
void propagate_native_gui_text_active_storage_00ab87d0(void* captured_actual_text,
    const volatile std::uint32_t& actual_flag_dword_home,
    NativeGuiTextActiveStorageContext&, NativeGuiTextActiveStorageFrame&,
    NativeGuiTextActiveStorageAcquired&);
} // namespace bsp
