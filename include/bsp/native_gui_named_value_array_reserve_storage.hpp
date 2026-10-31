#pragma once

#include "bsp/native_gui_named_value_copy_storage.hpp"

namespace bsp {

// Ordinary caller-retained Native argument/local homes. No initializers: the
// caller stages requested_capacity_home at its actual PUSH epoch; other homes
// keep their preimages until the corresponding Native store. The index is a
// saved loop index (even a null destination advances), not construction credit.
// native_state_home records -1/0 only; it grants no cleanup/origin authority.
struct NativeGuiNamedValueArrayReserveStorageFrame {
    std::uint32_t requested_capacity_home;
    std::uint32_t replacement_home;
    std::uint32_t copy_index_home;
    std::uint32_t destination_home;
    std::uint32_t native_state_home;
    NativeGuiNamedValueCopyStorageFrame copy;
};
static_assert(sizeof(NativeGuiNamedValueArrayReserveStorageFrame) == 24);

// Whole normal AB4720[298B/106ops], actual12B {data,count,capacity} descriptor.
// Signed request<1 writes/reloads its actual HOME; signed capacity>=request
// returns. Allocate wrap32(12*request), copy CURRENT count/data via whole54,
// return old headers forward via CURRENT count/data, free CURRENT data, then
// publish the captured replacement and selected capacity. Count is untouched.
// Request-HOME reloads occur only after an entered copy/release loop; mutable
// homes are separate from the register captures used across callbacks/free.
//
// Reached nonnull copy slots materialize individual existing trivial raw12B
// EntryStorage objects with default placement initialization and no field
// writes. No T[n] array, NativeString, owning array or typed-queue admission is
// established. The body uses DWORD addresses rather than typed-array indexing.
// Caller proves actual descriptor/source/backing/subobject/preimage validity,
// genuine allocation/free provenance and retains all captured old/new buffers.
//
// This entire Frame/HOMEs and active Source argument/provider backing must be
// disjoint from descriptor, records, payload and all Native write extents.
// Whole54 performs its late source+8 read/store before returning; the otherwise
// intervening 47CD private replacement-HOME reload follows that return here.
// Normal commutation relies on that separation and no callback in between;
// original private aliases, load/fault order and frame addresses are excluded.
// No interposition hook or fence is added to whole54.
//
// Borrow SAME raw pool cells/providers and authenticated live0109EEA4 feature
// reference. Inherit copy/BF extents, DF0/CPU, pool/new-handler/deletion and
// serialization/lifetime domains. Reentry needs distinct retained homes; only
// normal nested returns allow reuse. No cap, overflow/null guard or fake owner.
// No RAII, compensation, Native EH/filter/Origin permission or new noexcept.
// An escape retains all reached writes, allocations and debt for explicit
// caller disposition. Existing typed arrays/consumers remain separate.
// New C++ ABI, not Native ECX/RET4/private-HOME/flags/fault/FH3/runtime proof.
void reserve_native_gui_named_value_array_storage_00ab4720(
    void* actual_header12, NativeGuiNamedValueArrayReserveStorageFrame&,
    NativeStringRawPoolContext&,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
