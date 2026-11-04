#pragma once

#include "bsp/oct18_tail_a.hpp"

#include <cstddef>
#include <cstdint>
#include <new>
#include <vector>

namespace bsp::game {

// The current host census is a C++ vector. Borrow its actual element backing
// through a native control view, then publish the reconstructed body's end to
// that same owner before any alias builder or subsequent preload can read it.
class GamePendingClassCensusView final {
public:
    explicit GamePendingClassCensusView(std::vector<std::int32_t>& census)
        : census_(census) {
        auto* const data = census.data();
        control_ = {0, data, census.empty() ? data : data + census.size(),
            census.capacity() == 0 ? data : data + census.capacity()};
    }
    void* control() noexcept { return &control_; }
    void publish_size() {
        const auto size = control_.last == control_.first ? 0u
            : static_cast<std::size_t>(control_.last - control_.first);
        census_.resize(size); // metadata handoff; int32_t has no element cleanup
    }

private:
    struct Control {
        // +0 is not read by 004C8AA0 or its checked DWORD-range erase child.
        std::uint32_t unused_proxy;
        std::int32_t* first;
        std::int32_t* last;
        std::int32_t* capacity_end;
    };
    static_assert(sizeof(void*) == 4 && sizeof(Control) == 0x10);
    static_assert(offsetof(Control, first) == 4 && offsetof(Control, last) == 8);
    std::vector<std::int32_t>& census_;
    Control control_{};
};

// The scene loader's stock queue (00F8A0B0/00F8A0B4). These are the actual
// nodes consumed by VehicleClassPreloadBinding, not an auxiliary reset list.
// bsp.py show 0095C4D0: next+0, previous+4, mapped class index+8; append at tail.
class GamePendingClassStorage final {
public:
    GamePendingClassStorage() noexcept : head_(&sentinel_) {
        sentinel_.next = &sentinel_;
        sentinel_.previous = &sentinel_;
    }
    GamePendingClassStorage(const GamePendingClassStorage&) = delete;
    GamePendingClassStorage& operator=(const GamePendingClassStorage&) = delete;
    ~GamePendingClassStorage() {
        std::int32_t unused;
        while (pop_stock_queue(unused)) {}
    }

    void push_stock_queue(std::int32_t mapped_index) {
        for (Node* node = sentinel_.next; node != &sentinel_; node = node->next) {
            if (node->class_index == mapped_index) return;
        }
        // Match the reset body's singleton_lifetime_free allocation domain.
        void* memory = singleton_lifetime_allocate(
            {SingletonAllocationKind::object, sizeof(Node), sizeof(Node)});
        auto* node = new (memory) Node{&sentinel_, sentinel_.previous, mapped_index};
        sentinel_.previous->next = node;
        sentinel_.previous = node;
        ++count_;
    }

    bool pop_stock_queue(std::int32_t& index) noexcept {
        Node* const node = sentinel_.next;
        if (node == &sentinel_) return false;
        index = node->class_index;
        sentinel_.next = node->next;
        node->next->previous = &sentinel_;
        --count_;
        singleton_lifetime_free(node);
        return true;
    }

    oct18_tail_a::PendingClassState reset_state(GamePendingClassCensusView& census) noexcept {
        return {census.control(), head_, count_};
    }

private:
    struct Node {
        Node* next;
        Node* previous;
        std::int32_t class_index;
    };
    static_assert(sizeof(Node) == 0x0c && offsetof(Node, class_index) == 8);
    Node sentinel_{};
    void* volatile head_;
    std::uint32_t count_{};
};

} // namespace bsp::game
