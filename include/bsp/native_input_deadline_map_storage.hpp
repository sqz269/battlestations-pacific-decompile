#pragma once
#include <cstdint>

namespace bsp {
struct NativeInputDeadlineMapIterator { void* owner; void* node; };
static_assert(sizeof(NativeInputDeadlineMapIterator) == 8);

// Source storage adapter over canonical12h headers and18h nodes. These are
// reused source contracts, not original STL implementations or ABI wrappers.
// Pair is two raw DWORDs: signed key at0 and mapped float bits at4. No payload
// retention or conversion; actual malloc/new-handler allocation and free domain.
void* allocate_input_deadline_map_node(void* left, void* parent, void* right,
    const void* actual_pair, std::uint8_t color);

// Shared predecessor mechanics specialized only for native nil byte+15.
// Both calls use the real returning-capable CRT invalid-parameter service.
// Equality captures owner comparison before validation, then reloads nodes.
void decrement_input_deadline_map_iterator(NativeInputDeadlineMapIterator&);
bool equal_input_deadline_map_iterators(const NativeInputDeadlineMapIterator&,
    const NativeInputDeadlineMapIterator&);

// Same reviewed link/rebalance mechanics as the hardware adapter, with18h
// allocation, color/nil14/15 and unsigned limit1FFFFFFE. Caller supplies an
// already selected parent and side. Link count is incremented after allocation;
// output NODE precedes OWNER. No comparator, duplicate check or hint is added.
// At limit, throws the existing owning NativeHardwareLayoutTreeLengthError.
NativeInputDeadlineMapIterator* link_input_deadline_map_node(void* actual_tree,
    NativeInputDeadlineMapIterator* output, std::uint8_t insert_left,
    void* selected_parent, const void* actual_pair);

// Actual signed-key subscript4D6900/hinted insertion4D3CD0 remain required.
// There is no ready get-or-insert call, successful stub, or second tree owner.
// Valid raw storage, selected insertion site and caller-controlled aliasing are
// required. Native FH3/register-spill alias identity and game ABI are not added.
} // namespace bsp
