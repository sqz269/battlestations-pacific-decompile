#pragma once
#include <cstdint>

namespace bsp {

// Stateless source adapter for the actual 12h signed-int/float map used by
// 4D6900. Borrows its sole header/nodes and returns the actual mapped address.
// Requires an initialized, consistent native tree on entry; key may alias its
// live header/node storage. Reuses existing lower-bound/unique/link mechanics.
// This is not the original STL ABI or a general arbitrary-hint insertion API.
// Allocation callbacks and returning CRT diagnostics retain native ordering;
// callbacks must keep every subsequently accessed allocation alive.
float* subscript_input_deadline_map_storage(void* actual_tree, const std::int32_t* key);

} // namespace bsp
