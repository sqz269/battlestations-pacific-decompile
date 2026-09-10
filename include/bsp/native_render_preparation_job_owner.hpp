#pragma once
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Native table words, not callable host vtables. The registered pointer is
// this four-byte secondary subobject at actual owner+4, not a host companion.
struct NativeRenderPreparationJobSecondary {
    volatile std::uint32_t native_vtable_00;
};
struct NativeRenderPreparationJobStorage {
    volatile std::uint32_t native_vtable_00;
    NativeRenderPreparationJobSecondary secondary_04;
};

// One companion borrows the actual F8D444 publication slot and the shared
// 1090AA0 lifetime domain. It has no implicit cleanup or private manager.
// final_secondary_table is the actual immutable D5E15C profile (one word).
class NativeRenderPreparationJobLifetime final {
public:
    NativeRenderPreparationJobLifetime(SingletonLifetimeDomain&,
        NativeRenderPreparationJobStorage* volatile& global_00f8d444,
        const volatile std::uint32_t* final_secondary_table_00d5e15c);

    // B0FFB0: no native inputs, EAX current singleton, RET. Allocate8 bytes,
    // publish, capture secondary+4 before the second manager lookup, register
    // on that manager, unlock the captured section, then reload publication.
    NativeRenderPreparationJobStorage* get_singleton_00b0ffb0();

    // B0F1D0: ECX actual primary, RET. Unconditionally clear publication and
    // reset only secondary+4 to CE3818. No unregister, free or primary reset.
    void destroy_body_00b0f1d0(NativeRenderPreparationJobStorage&) noexcept;
    // B0F210: ECX primary, stack flags, RET4, EAX original primary address.
    // Same reset, then ordinary-free the primary allocation iff flags&1.
    NativeRenderPreparationJobStorage* delete_primary_00b0f210(
        NativeRenderPreparationJobStorage*, std::uint32_t flags) noexcept;
    // B0F1C0: secondary ECX-4, tailcall B0F210. Return primary, not secondary.
    NativeRenderPreparationJobStorage* delete_secondary_00b0f1c0(
        NativeRenderPreparationJobSecondary*, std::uint32_t flags) noexcept;
    // B0D930 is the distinct D5E154 construction-base profile. Reset/free the
    // supplied base address itself; never substitute this for final deletion.
    NativeRenderPreparationJobSecondary* delete_construction_base_00b0d930(
        NativeRenderPreparationJobSecondary*, std::uint32_t flags) noexcept;

    // Concrete shared-manager callback route: read the registered pointer's
    // current table and D5E15C entry0, then execute B0F1C0 -> B0F210. True means
    // that destruction actually ran. False leaves it untouched for the owner's
    // other concrete dispatcher/error handling; it is not successful cleanup.
    bool try_delete_registered(void* actual_registered_secondary,
        std::uint32_t flags) noexcept;

private:
    SingletonLifetimeDomain& domain_;
    NativeRenderPreparationJobStorage* volatile& global_00f8d444_;
    const volatile std::uint32_t* final_secondary_table_00d5e15c_;
};

// Original destructor entries require valid nonnull owners. Native null paths
// write address zero; these APIs do not introduce silent null success or claim
// to reproduce access violations. Allocation/manager/section domains must stay
// valid across their native callbacks. Raw native execution B1BF70 and job
// scheduling are separate; storing D5E160 does not implement its execute slot.
} // namespace bsp
