#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace bsp {
struct InstanceRenderEntry;

// Native00b1dce0: ECX first pointer slot, EDX one-past-last slot; stack signed
// ideal budget, comparator; RET8. This entry point supplies budget=entry count
// and the actual category1 predicate00b51ab0 (signed material ascending, float
// depth descending). New vector/value interface, not the original pointer ABI.
//
// Reconstructs native insertion<=32, median/ninther pivot choice, three-way
// partition, shrinking ideal budget and heap fallback, including equivalent-
// entry pointer permutation above32. Input order of ties is NOT generally
// stable. Duplicate entry pointers are allowed and retain native behavior.
//
// Every entry and section must exist and every depth must be finite. Count*4
// must fit signed32-bit native pointer-distance arithmetic. Invalid inputs
// are rejected before mutation. Entry/section fields and vector storage must
// remain stable during the sort. Successful sorting allocates no new storage.
// Evidence: docs/INSTANCE_CATEGORY_SORT.md.
bool sort_instance_entries_00b1dce0(std::vector<InstanceRenderEntry*>& entries,
    std::string& error);

// Queue+4+index*8 byte and queue+8+index*8 DWORD. The copied DWORD is retained
// for callers, but00b51df0 selects its comparator using the ORIGINAL index.
struct RenderBatchSortConfiguration {
    std::uint8_t enabled{};
    std::uint32_t value{};
};
// Native00b1cb30: ECX queue, stack index/byte-output/DWORD-output, RET0Ch;
// AL=1. This checked projection rejects an absent configuration index.
bool get_render_batch_sort_configuration_00b1cb30(
    const std::vector<RenderBatchSortConfiguration>& configurations,
    std::uint32_t batch_index, RenderBatchSortConfiguration& output,
    std::string& error);

// Borrowed projection of entry+4 -> section+20 -> material. effect is
// material+7C; texture slot0 is material+10. These are values, not an overlay.
struct RenderBatchMaterialKeyFields {
    std::int32_t effect_b0{};
    std::int16_t material_count34{};
    bool texture0_present{};
    std::uint32_t texture20{};
    std::uint8_t effect_c0{};
};
struct RenderBatchMaterialKeySource {
    void* context{};
    // Read the actual retained material/effect for this entry. Read texture20
    // only when signed material_count34>0 and slot0 exists. Native has no
    // callback here: this host adapter must not mutate entries, queue storage,
    // configuration, material bindings or count. Failure is explicit.
    bool (*read)(void*, const InstanceRenderEntry&,
        RenderBatchMaterialKeyFields&, std::string&){};
};

// Numerical EAX result for a binary32 ST0 input to00bf7456 with x87 exceptions
// masked: truncate toward zero to signed64, then keep low32. NaN, infinities
// and signed64 overflow yield zero (integer-indefinite's low word). No out-of-
// range C++ float-to-integer cast; does not emulate x87 flags/traps or general
// extended-precision ST0 inputs. Original helper pops ST0 and returns EDX:EAX.
std::uint32_t render_batch_depth_word_00bf7456(float depth) noexcept;
std::uint64_t make_render_batch_key_00b51df0(float depth,
    const RenderBatchMaterialKeyFields&) noexcept;

// Native00b51df0: ECX batch, stack ORIGINAL batch index, RET4, virtual+0C.
// Disabled: no entry access/mutation. Enabled index0: read each entry's fields,
// write key in list order, then sort by unsigned64 key. Every other index:
// existing signed-material/depth sort, leaving keys unchanged. Callback/entry
// failure retains earlier key writes, but sorting starts only after all writes.
// Index0 accepts every binary32 depth under the masked-x87 numerical contract;
// other indices retain the category1 finite-depth guard. Count/storage/bindings
// must remain stable throughout. No claim of native ABI or whole-frame parity.
bool prepare_render_batch_00b51df0(std::vector<InstanceRenderEntry*>& entries,
    std::uint32_t batch_index,
    const std::vector<RenderBatchSortConfiguration>& configurations,
    const RenderBatchMaterialKeySource&, std::string& error);
}
