#pragma once

#include "bsp/native_gui_named_value_array_reserve_storage.hpp"

namespace bsp {
struct NativeGuiIconDelayedLoadsStorage;
struct NativeStringRawPoolContext;

// Ordinary logical Native homes, not an Original FH3 stack layout. No member
// initializers: caller default-initializes retained storage without seeding its
// preimage. State/saved-owner stores occur only at the reached Native epochs.
// State is operand evidence, never construction/cleanup/Origin authority.
struct NativeGuiIconDelayedLoadsDestroyStorageFrame {
    std::uint32_t native_state_home;
    std::uint32_t saved_owner_home;
    NativeGuiNamedValueArrayReserveStorageFrame reserve;
};
static_assert(sizeof(NativeGuiIconDelayedLoadsDestroyStorageFrame) == 32);

class NativeGuiIconDelayedLoadsDestroyStorageAcquired final {
public:
    NativeGuiIconDelayedLoadsDestroyStorageAcquired() = default;
    NativeGuiIconDelayedLoadsDestroyStorageAcquired(
        const NativeGuiIconDelayedLoadsDestroyStorageAcquired&) = delete;
    NativeGuiIconDelayedLoadsDestroyStorageAcquired& operator=(
        const NativeGuiIconDelayedLoadsDestroyStorageAcquired&) = delete;
    NativeGuiIconDelayedLoadsDestroyStorageAcquired(
        NativeGuiIconDelayedLoadsDestroyStorageAcquired&&) = delete;
    NativeGuiIconDelayedLoadsDestroyStorageAcquired& operator=(
        NativeGuiIconDelayedLoadsDestroyStorageAcquired&&) = delete;

    std::uint32_t captured_identity_bits() const { return captured_identity_bits_; }
    bool backing_free_entered() const { return backing_free_entered_; }
    bool backing_free_returned() const { return backing_free_returned_; }
private:
    friend void destroy_native_gui_icon_delayed_loads_storage_00ab5560(
        void*, NativeGuiIconDelayedLoadsDestroyStorageFrame&,
        NativeGuiIconDelayedLoadsDestroyStorageAcquired&, NativeStringRawPoolContext&,
        NativeGuiIconDelayedLoadsStorage* volatile&, const volatile std::uint32_t&);
    std::uint32_t captured_identity_bits_{};
    bool backing_free_entered_{};
    bool backing_free_returned_{};
};

class NativeGuiIconDelayedLoadsScalarStorageAcquired final {
public:
    NativeGuiIconDelayedLoadsScalarStorageAcquired() = default;
    NativeGuiIconDelayedLoadsScalarStorageAcquired(
        const NativeGuiIconDelayedLoadsScalarStorageAcquired&) = delete;
    NativeGuiIconDelayedLoadsScalarStorageAcquired& operator=(
        const NativeGuiIconDelayedLoadsScalarStorageAcquired&) = delete;
    NativeGuiIconDelayedLoadsScalarStorageAcquired(
        NativeGuiIconDelayedLoadsScalarStorageAcquired&&) = delete;
    NativeGuiIconDelayedLoadsScalarStorageAcquired& operator=(
        NativeGuiIconDelayedLoadsScalarStorageAcquired&&) = delete;

    std::uint32_t captured_identity_bits() const { return captured_identity_bits_; }
    const NativeGuiIconDelayedLoadsDestroyStorageAcquired& destruction() const {
        return destruction_;
    }
    bool flags_test_reached() const { return flags_test_reached_; }
    // Read this observation only when flags_test_reached() is true.
    std::uint8_t tested_flags_byte() const { return tested_flags_byte_; }
    bool owner_free_entered() const { return owner_free_entered_; }
    bool owner_free_returned() const { return owner_free_returned_; }
private:
    friend void* scalar_delete_native_gui_icon_delayed_loads_storage_00ab57c0(
        void*, const volatile std::uint32_t&,
        NativeGuiIconDelayedLoadsDestroyStorageFrame&,
        NativeGuiIconDelayedLoadsScalarStorageAcquired&, NativeStringRawPoolContext&,
        NativeGuiIconDelayedLoadsStorage* volatile&, const volatile std::uint32_t&);
    std::uint32_t captured_identity_bits_{};
    NativeGuiIconDelayedLoadsDestroyStorageAcquired destruction_{};
    bool flags_test_reached_{};
    std::uint8_t tested_flags_byte_{};
    bool owner_free_entered_{};
    bool owner_free_returned_{};
};

// Acquired objects are separate passive Source observations, with fresh DMIs.
// Only the actual raw friends write them. Caller prepares a fresh, distinct,
// nonmoving retained acquisition for each invocation; there is no freshness
// guard/reset/setter, observer callback, owner, Native issuer or cleanup permit.
// Const getters require a live retained record, not a general reader-NoThrow or
// concurrency promise. Identity is captured address bits, never a live reference.
// A missing normal-return observation is entered-only progress, not permission
// to retry, destroy, retire, compensate or infer Native exception provenance.
//
// Whole NORMAL AB5560[94B/27ops], not interior address AB5580: whole resize58 of
// captured owner+4 with ORIGINAL literal0, CURRENT data free, unconditional SAME
// publication clear even if replaced, then captured owner CE3818 stamp. No normal
// base-helper call, unregister, owner free or additional count/data/capacity clear.
// Native EH CB7CB0 -> AB21C0 remains evidence only; no Source catch/action exists.
void destroy_native_gui_icon_delayed_loads_storage_00ab5560(
    void* actual_owner16, NativeGuiIconDelayedLoadsDestroyStorageFrame&,
    NativeGuiIconDelayedLoadsDestroyStorageAcquired&, NativeStringRawPoolContext&,
    NativeGuiIconDelayedLoadsStorage* volatile& publication_00f8bd98,
    const volatile std::uint32_t& actual_feature_word_0109eea4);

// Necessary NORMAL Source ABI composition for existing HIGH-tagged CG57C0.
// It is NOT a new game-owned/CG body reconstruction or an Original callable ABI.
// Borrow a genuine caller-staged full DWORD HOME, separate from Frame/Acquired.
// Only its CURRENT LOWBYTE is read, ONCE after whole destroy returns. That same
// byte supplies the observation and bit0 branch. If set, free captured owner;
// return captured identity bits even after free, with no later pointee access.
void* scalar_delete_native_gui_icon_delayed_loads_storage_00ab57c0(
    void* captured_owner16, const volatile std::uint32_t& actual_flags_home,
    NativeGuiIconDelayedLoadsDestroyStorageFrame&,
    NativeGuiIconDelayedLoadsScalarStorageAcquired&, NativeStringRawPoolContext&,
    NativeGuiIconDelayedLoadsStorage* volatile& publication_00f8bd98,
    const volatile std::uint32_t& actual_feature_word_0109eea4);

// Both APIs require genuine16B C++ storage/current producer-manager custody,
// individually valid array/string/payload lifetimes and SAME actual raw pool,
// canonical feature CELL and typed volatile publication ref. No pointer/profile
// creates ownership and no void*&/typed-owner cast or new bank is supplied.
// Retain graph, Frame/HOMEs and observations through entered failures. All are
// distinct per reentry and disjoint from Native-written owner/records/payload
// and active Source argument/provider extents. Inherit58/55/54/BF domains and
// private-home/argument/47CD/compare/fault/EFLAGS/FH3/SEH/Origin exclusions.
// No full-DWORD flag snapshot, count/capacity/null/overflow gate, flags1/count0
// subset, fake default, new noexcept/NoThrow, RAII, catch, compensation or replay.
// Existing service/Host/default-initialization/feature preparation is unchanged;
// no current caller, retirement, ABI, Native unwind or runtime activation follows.
} // namespace bsp
