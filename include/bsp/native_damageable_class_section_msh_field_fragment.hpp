#pragma once
#include "bsp/native_damageable_class_section_iterator_setup_fragment.hpp"

namespace bsp {

// Actual enclosing 0087CA80 storage. S is ESP after E4h locals/four saves.
// Both raw regions have at least four-byte alignment.
struct NativeDamageableClassSectionMshFieldFragmentScratch {
    NativeDamageableClassSectionIteratorSetupFragment& live_iterator_owner;
    NativeDamageableClassSectionIteratorSetupFragmentScratch& same_iterator_scratch;
    void* fresh_field_at_parent_e0;  // aligned14h bytes, no live Lua object
    void* fresh_string_at_parent_18; // aligned8h raw header: length0/data4
    const char* actual_msh_category_key_00d0e190; // borrowed actual live key storage
};

// Retained prefix CEBB..CF34 and normal release D181..D19F only. Construct this
// fixed owner BEFORE its single open, within the successful iterator owner's
// lifetime (state13, actual key S58/value S2C). Row-begin state14 and back-row
// selection must already have completed. The witness and SAME iterator scratch
// are borrowed; no iterator API, row/vector/cursor operation or replay is added.
//
// open retains state17 across ALL excluded CF35..D180 fields. Their inner owners
// must have restored17 before explicit close. Exceptional state15 destroys only
// actual Lua E0; exceptional17 destroys the CURRENT raw string header. Both lower
// to13 first. There is no ordinary state16. Normal close instead returns SAVED
// data with SAVED length+1, leaves the current header untouched, and cannot retry
// after a failing getter. Do not let ordinary successful scope exit stand in for
// explicit close: the destructor is the exceptional current-header path.
//
// Borrow canonical actual AA8/AA4/AA0 cells; values remain live per operation.
// All slots, bindings and this owner's private state/captures stay disjoint and
// fixed; callbacks may mutate actual header/buffer but may not alias this owner
// or change bindings/reenter. Actual Lua owner/index/tracking/capacity and error
// handler below all removed slots remain the existing providers' obligations.
// The converting getter must yield a valid nonnull NUL-terminated byte span;
// The borrowed key must have the retained MshCategory identity and remain valid
// throughout lookup; its original bytes are not established by this adapter.
// copy ranges must support the wrapped DWORD count, including zero. No new
// exact-string predicate, default, rollback or typed NativeString is supplied.
//
// Ordinary MSVC Win32 Source ABI. Native register/FH3/SEH/longjmp/fault and host
// CRT formatting/copy identity, actual production/shutdown bindings, excluded
// fields/full-parent composition and runtime proof remain held. Destruction is
// noexcept; a secondary cleanup failure follows ordinary C++ termination.
class NativeDamageableClassSectionMshFieldFragment final {
public:
    NativeDamageableClassSectionMshFieldFragment(
        NativeDamageableClassSectionMshFieldFragmentScratch& actual_scratch,
        NativeStringRawPoolContext& actual_strings) noexcept;
    ~NativeDamageableClassSectionMshFieldFragment() noexcept;
    NativeDamageableClassSectionMshFieldFragment(
        const NativeDamageableClassSectionMshFieldFragment&) = delete;
    NativeDamageableClassSectionMshFieldFragment& operator=(
        const NativeDamageableClassSectionMshFieldFragment&) = delete;
    NativeDamageableClassSectionMshFieldFragment(
        NativeDamageableClassSectionMshFieldFragment&&) = delete;
    NativeDamageableClassSectionMshFieldFragment& operator=(
        NativeDamageableClassSectionMshFieldFragment&&) = delete;

    void open();
    void close();

    // Borrow only the original saved pointer when this exact scratch is live17.
    // Rejection leaves output untouched; successful nullptr is a valid capture.
    // Output must be a live, disjoint pointer object, not private owner storage.
    // This proves no buffer lifetime or NUL termination: the caller keeps the
    // original buffer/current bytes valid and the owner/bindings stable, without
    // reentry. Never treat rejection as an empty category. No current S18 or
    // saved-length read, text copy, ownership/state change, or provider call.
    bool try_borrow_saved_category_data(
        const NativeDamageableClassSectionMshFieldFragmentScratch& expected,
        const char*& output) const noexcept;

private:
    NativeDamageableClassSectionMshFieldFragmentScratch& scratch_;
    NativeStringRawPoolContext& strings_;
    void* saved_data_ = nullptr;
    std::uint32_t saved_length_ = 0;
    unsigned state_ = 13;
};

} // namespace bsp
