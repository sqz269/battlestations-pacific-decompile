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
// Native allocator and platform object ABI remain separate dependencies.
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
    friend struct PlatformTextInput;
    void clear_nodes() noexcept;
    struct Node;
    static Node* create_sentinel_00bec710();
    static Node* create_node_00bec7b0(Node* next, Node* previous, TextInputEvent event);
    void increase_count_00bed290(std::uint32_t increment);
    void destroy_00bec730() noexcept;
    Node* sentinel_;
    std::uint32_t count_{};
};

struct PlatformTextInput {
    bool enabled{}; // platform+170h
    bool clipboard_requested{}; // platform+44h; consumption remains unported
    TextInputQueue queue;
    // ECX platform, stack enabled byte, RET4. Always clears, including same-state calls.
    void enable_00a965a0(bool value) noexcept;
    // Only the text branches of00bed3b0. Does not consume the window message:
    // the native procedure still calls DefWindowProcA after these effects.
    void enqueue_message_00bed3b0_fragment(std::uint32_t message, std::uint32_t wparam);
};

// Required owner callbacks, not default text-editing implementations.
struct TextInputCallbacks {
    bool enabled{}; // text owner+4, distinct from platform input-enabled flag
    virtual ~TextInputCallbacks() = default;
    virtual bool on_event(TextInputEvent event) = 0; // original virtual slot0
    virtual void fallback(TextInputEvent event) = 0; // original00a96750 dependency
};
// Native ECX owner, unused stack argument, RET4. Typed dispatch projection;
// callback implementations and original owner/UI allocation are not reconstructed.
void dispatch_text_input_00a96f40_fragment(TextInputQueue& queue, TextInputCallbacks& owner);
}
