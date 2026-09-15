#pragma once

namespace bsp::detail {
// Extracted from the existing 4CEC60 string-subtree cleanup. The payload
// capture precedes the left-link capture, both precede release/free. Policies
// preserve each native node layout and actual payload/allocation services.
template<class Access, class Capture, class Release, class Free>
void erase_tree_subtree_right_first(void* node, Capture capture,
    Release release, Free free_node) {
    if (Access::sentinel(node)) return;
    do {
        erase_tree_subtree_right_first<Access>(Access::right(node),
            capture, release, free_node);
        const auto payload = capture(node);
        auto* const next = Access::left(node);
        release(node, payload);
        free_node(node);
        node = next;
    } while (!Access::sentinel(node));
}
} // namespace bsp::detail
