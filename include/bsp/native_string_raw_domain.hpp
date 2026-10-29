#pragma once
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"

namespace bsp {
// Context objects are borrowed metadata. Distinct real views may name the
// SAME three native publication/control cells. Compare every cell address;
// never compare current values or snapshot them before native entry.
// Each context object and all referenced cells still require full caller
// lifetime/custody. This predicate does not extend either view's lifetime.
inline bool same_native_string_raw_pool_cells(const NativeStringRawPoolContext& left,
    const NativeStringRawPoolContext& right) noexcept {
    return &left.actual_published_01090aa8 == &right.actual_published_01090aa8 &&
        &left.actual_small_returns_disabled_01090aa4 == &right.actual_small_returns_disabled_01090aa4 &&
        &left.actual_manager_publication_01090aa0 == &right.actual_manager_publication_01090aa0;
}
// Admit only the existing final actual-pool implementation over those SAME
// cells and its actual manager route. No virtual allocator/getter is invoked.
inline bool same_native_string_storage_raw_pool_cells(const NativeStringStorage& storage,
    const NativeStringRawPoolContext& raw) noexcept {
    const auto* const actual = dynamic_cast<const ActualNativeStringPoolStorage*>(&storage);
    return actual && actual->uses_raw_pool_cells(raw);
}
} // namespace bsp
