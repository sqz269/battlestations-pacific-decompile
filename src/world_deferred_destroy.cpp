#include "bsp/world_deferred_destroy.hpp"

namespace bsp {

WorldDeferredDestroyResult drain_entity_chain_009041a0(WorldDeferredDestroyHost& host,
                                                       DeferredDestroyChain& chain)
{
    WorldDeferredDestroyResult result{};
    // 009041A6: the count, not the head, is the entry test. An empty chain
    // falls straight through to the epilogue at 00904201.
    if (chain.count == 0) return result;

    do {
        // 009041B0. The head is re-read on every iteration; the previous
        // destructor may have replaced it.
        const std::uint32_t node = chain.head;
        const std::uint32_t prev = host.prev_sibling(node); // 009041B2
        const std::uint32_t next = host.next_sibling(node); // 009041B9 / 009041D5

        // 009041B5..009041C2. Present on the chain when it has either link, or
        // when it is the only node the header claims to hold.
        const bool linked = prev != 0 || next != 0 || chain.count <= 1;
        if (linked) {
            if (prev != 0) {
                host.set_next_sibling(prev, next); // 009041C8, prev->next = next
            } else {
                chain.head = next; // 009041D0, header head = next
            }
            if (next != 0) {
                host.set_prev_sibling(next, prev); // 009041DC, next->prev = prev
            } else {
                chain.tail = prev; // 009041E4, header tail = prev
            }
            host.set_next_sibling(node, 0); // 009041EA
            host.set_prev_sibling(node, 0); // 009041ED
            chain.count -= 1;               // 009041F0
            ++result.unlinked;
        } else {
            ++result.skipped_unlink;
        }

        const std::int32_t count_before = chain.count;
        const std::uint32_t head_before = chain.head;
        // 009041F4..009041FA: node->vtable[0](1). ECX is still the node.
        host.destroy_node(node);
        ++result.destroyed;

        if (chain.count == count_before && chain.head == head_before && !linked) {
            // Not a native exit. See WorldDeferredDestroyResult::stalled.
            result.stalled = true;
            return result;
        }
        // 009041FC: repeat while the count is nonzero.
    } while (chain.count != 0);

    return result;
}

} // namespace bsp
