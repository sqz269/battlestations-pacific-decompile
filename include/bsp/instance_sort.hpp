#pragma once
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
}
