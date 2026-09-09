#pragma once
#include <cstdint>

namespace bsp {
// Native queue record: low character/virtual-key byte, then 0=character/1=key.
struct TextInputEvent {
    std::uint8_t value;
    std::uint8_t key_event;
};
static_assert(sizeof(TextInputEvent) == 2);

// Owns the platform+174h list dependency. Typed ownership, not native object ABI.
// Input enabling, clipboard policy and UI consumption are separate dependencies.
class TextInputQueue {
public:
    TextInputQueue();
    ~TextInputQueue();
    TextInputQueue(const TextInputQueue&) = delete;
    TextInputQueue& operator=(const TextInputQueue&) = delete;
    void append_00bed370(TextInputEvent event);
    // Native platform method takes two byte-output pointers, RET 8. Host returns
    // false on empty instead of invoking the CRT invalid-parameter handler.
    bool pop_00bece90(TextInputEvent& event) noexcept;
    std::uint32_t size() const noexcept { return count_; }
    const TextInputEvent* front() const noexcept;
private:
    struct Node;
    static Node* create_sentinel_00bec710();
    static Node* create_node_00bec7b0(Node* next, Node* previous, TextInputEvent event);
    void increase_count_00bed290(std::uint32_t increment);
    void destroy_00bec730() noexcept;
    Node* sentinel_;
    std::uint32_t count_{};
};
}
