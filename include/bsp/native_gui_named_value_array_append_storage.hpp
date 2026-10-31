#pragma once

#include "bsp/native_gui_named_value_array_reserve_storage.hpp"

namespace bsp {

// Caller-retained ordinary Native argument/local backing, no initializers.
// Saved homes remain separate from normal register captures. State -1/0 is
// evidence only, never cleanup/Origin authority. Nested frames need distinct
// per-reentry backing; only normal nested returns permit their reuse.
struct NativeGuiNamedValueArrayAppendStorageFrame {
    std::uint32_t saved_header_home;
    std::uint32_t saved_slot_home;
    std::uint32_t native_state_home;
    NativeGuiNamedValueArrayReserveStorageFrame reserve;
    NativeGuiNamedValueCopyStorageFrame copy;
};
static_assert(sizeof(NativeGuiNamedValueArrayAppendStorageFrame) == 40);

// Whole normal AB4990[116B/35ops], ECX header/source-pointer HOME, RET4.
// CURRENT capacity precedes CURRENT count; equality alone grows by doubled
// captured capacity modulo32, signed>1 or1. Stage only reserve.requested HOME
// then invoke genuine55. After reserve/skip read CURRENT count then data,
// capture wrapping slot, save it and state0 even null. Null skips individual
// entry birth AND the incoming source-HOME read/copy. Nonnull default placement
// birth has no field writes; only THEN read CURRENT source HOME and call whole54.
// Finally read/increment CURRENT count modulo32, including the null skip.
//
// Caller proves real raw12B descriptor/backing/live prefix/spare slot/source
// entry, ordinary C++ subobject/preimage and allocation/string/payload custody.
// Individual existing EntryStorage birth does not establish T[n], an array
// owner, typed pointer arithmetic or the delayed Icon queue's provenance.
// Retain this full Frame, actual volatile source HOME, raw pool/new-handler/
// deletion providers and SAME authenticated live0109EEA4 feature reference.
// No early source value, copied feature flag, semantic owner or facade is used.
//
// Inherit genuine54/55 private Frame/HOME/argument/provider separation from
// descriptor, records, payload and all Native write extents, their qualified
// Q1 qualifications:54's emitted callee argument load after destination zeros,
// and55's private replacement-HOME reload across the late tail store. Inherit
// BF extents/DF0/CPU and serialization/lifetime domains. Original private
// aliases, emitted argument-load/fault order, EFLAGS,
// addresses, ECX/RET4/FH3/SEH and runtime identity remain unproved. No new hook.
// No range/overflow/null filter, default, cleanup, RAII, noexcept, rollback,
// retry or Native-origin issuer. Escapes retain reached effects/debt/graph;
// the Native placement action's operand reads/RET do not authorize disposal.
// AB5F00/delivery/current80/fullIcon/caller activation remain separate.
void append_native_gui_named_value_array_storage_00ab4990(
    void* actual_header12, volatile std::uint32_t& actual_source_argument_home,
    NativeGuiNamedValueArrayAppendStorageFrame&, NativeStringRawPoolContext&,
    const volatile std::uint32_t& actual_feature_word_0109eea4);
} // namespace bsp
