#include "bsp/native_tick_subnode_requeue.hpp"

#include "bsp/native_pending_registry_getter.hpp"
#include "bsp/native_tick_sublist_unlink.hpp"
#include "bsp/native_tick_subnode_reparent.hpp"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <cstddef>
#include <cstdint>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::int32_t) == 4);
static_assert(sizeof(std::uint32_t) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

template<class T>
volatile T& actual_word(void* base, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile T*>(static_cast<std::byte*>(base) + offset);
}

} // namespace

void requeue_native_tick_subnode_00876120(
    void* actual_receiver,
    void* volatile& actual_incoming_node_word,
    void* actual_guard_prefix,
    void* volatile& actual_registry_publication_00f878cc,
    void* volatile& actual_manager_publication_01090aa0) {
    if (actual_word<std::int32_t>(actual_receiver, 0x24) <= 1) { // 00876129..2C
        return;
    }

    void* const registry = get_native_pending_registry_00875280( // 0087612F
        actual_registry_publication_00f878cc, actual_manager_publication_01090aa0);
    void* const captured_section = actual_word<void*>(registry, 0x04); // 00876134
    if (captured_section != nullptr) { // 00876137..39
        EnterCriticalSection(static_cast<CRITICAL_SECTION*>(captured_section));
        auto& depth = actual_word<std::uint32_t>(captured_section, 0x18);
        depth = depth + 1u; // 00876142: after actual Enter
    }

    void* const node = actual_incoming_node_word; // 00876145: sole late read
    (void)unlink_native_tick_sublist_node_00874e60(
        static_cast<std::byte*>(actual_receiver) + 0x1c, 0u, node); // 00876149..4D
    const bool release_section = captured_section != nullptr; // 00876152
    actual_word<void*>(node, 0x04) = nullptr; // 00876154: also when K is null
    if (release_section) { // 0087615B
        auto& depth = actual_word<std::uint32_t>(captured_section, 0x18);
        depth = depth - 1u; // 0087615D: current word, before actual Leave
        LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(captured_section));
    }

    // Native PUSH EDI supplies a value copy after Leave. This is the actual
    // Source child's new input cell; do not reread/forward the incoming cell.
    void* volatile reparent_node_word = node; // 00876168
    reparent_native_tick_subnode_00876020(actual_receiver, reparent_node_word,
        actual_guard_prefix, actual_registry_publication_00f878cc,
        actual_manager_publication_01090aa0); // 00876169..6B
}

} // namespace bsp
