#include "bsp/native_tick_subnode_reparent.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_pending_registry_getter.hpp"
#include "bsp/native_tick_subnode_unlink_caller.hpp"

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
static_assert(sizeof(std::uint32_t) == 4);
static_assert(sizeof(std::int32_t) == 4);
static_assert(sizeof(CRITICAL_SECTION) == 0x18);

// Borrow individual compatible live words; no complete node/receiver/guard type.
template<class T>
volatile T& actual_word(void* base, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile T*>(static_cast<std::byte*>(base) + offset);
}

class ReparentFailureCleanup final {
public:
    explicit ReparentFailureCleanup(void* actual_guard) noexcept
        : guard_(actual_guard) {}

    ReparentFailureCleanup(const ReparentFailureCleanup&) = delete;
    ReparentFailureCleanup& operator=(const ReparentFailureCleanup&) = delete;

    ~ReparentFailureCleanup() noexcept {
        if (armed_) destroy_native_singleton_guard_00411ee0(guard_);
    }

    void arm() noexcept { armed_ = true; }
    void disarm() noexcept { armed_ = false; }

private:
    void* const guard_;
    bool armed_{false};
};

} // namespace

void reparent_native_tick_subnode_00876020(
    void* actual_receiver,
    void* volatile& actual_incoming_node_word,
    void* actual_guard_prefix,
    void* volatile& actual_registry_publication_00f878cc,
    void* volatile& actual_manager_publication_01090aa0) {
    ReparentFailureCleanup cleanup(actual_guard_prefix);
    void* const registry = get_native_pending_registry_00875280(
        actual_registry_publication_00f878cc, actual_manager_publication_01090aa0);
    void* const captured_section = actual_word<void*>(registry, 0x04);
    actual_word<std::uint32_t>(actual_guard_prefix, 0x00) = 0x00ce37fcu;
    actual_word<void*>(actual_guard_prefix, 0x04) = captured_section;
    if (captured_section != nullptr) {
        EnterCriticalSection(static_cast<CRITICAL_SECTION*>(captured_section));
        auto& depth = actual_word<std::uint32_t>(captured_section, 0x18);
        depth = depth + 1u;
    }

    void* const node = actual_incoming_node_word;
    void* const old_parent = actual_word<void*>(node, 0x04);
    cleanup.arm();
    if (old_parent == nullptr || old_parent != actual_receiver) {
        if (old_parent != nullptr) {
            // Native PUSH ESI supplies a copied value in the child's argument.
            void* volatile child_node_word = node;
            unlink_native_tick_subnode_00875960(old_parent, 0u, child_node_word,
                actual_registry_publication_00f878cc, actual_manager_publication_01090aa0);
        }
        actual_word<void*>(node, 0x04) = actual_receiver;
        void* position = actual_word<void*>(actual_receiver, 0x1c);
        if (position != nullptr) {
            const std::int32_t key = actual_word<std::int32_t>(node, 0x18);
            do {
                if (actual_word<std::int32_t>(position, 0x18) > key) break;
                position = actual_word<void*>(position, 0x0c);
            } while (position != nullptr);
        }

        if (position != nullptr) {
            void* const previous = actual_word<void*>(position, 0x08);
            actual_word<void*>(node, 0x08) = previous;
            if (previous != nullptr) {
                void* const next = actual_word<void*>(previous, 0x0c);
                actual_word<void*>(node, 0x0c) = next;
                actual_word<void*>(previous, 0x0c) = node;
            } else {
                void* const head = actual_word<void*>(actual_receiver, 0x1c);
                actual_word<void*>(node, 0x0c) = head;
                actual_word<void*>(actual_receiver, 0x1c) = node;
            }
            // Read after both stores above; aliases may have changed node+0Ch.
            void* const next = actual_word<void*>(node, 0x0c);
            if (next != nullptr) {
                actual_word<void*>(next, 0x08) = node;
            } else {
                actual_word<void*>(actual_receiver, 0x20) = node;
            }
        } else {
            if (actual_word<std::uint32_t>(actual_receiver, 0x24) != 0u) {
                void* const tail = actual_word<void*>(actual_receiver, 0x20);
                actual_word<void*>(tail, 0x0c) = node;
            } else {
                actual_word<void*>(actual_receiver, 0x1c) = node;
            }
            // The preceding link/head store can change this actual tail word.
            void* const tail = actual_word<void*>(actual_receiver, 0x20);
            actual_word<void*>(node, 0x08) = tail;
            actual_word<void*>(actual_receiver, 0x20) = node;
            actual_word<void*>(node, 0x0c) = nullptr;
        }
        auto& count = actual_word<std::uint32_t>(actual_receiver, 0x24);
        count = count + 1u;
    }

    if (captured_section != nullptr) {
        auto& depth = actual_word<std::uint32_t>(captured_section, 0x18);
        depth = depth - 1u;
        LeaveCriticalSection(static_cast<CRITICAL_SECTION*>(captured_section));
    }
    // State zero covers normal Leave: do not disarm before the call above.
    cleanup.disarm();
}

} // namespace bsp
