#include "bsp/text_input.hpp"
#include <memory>
#include <stdexcept>

namespace bsp {
struct TextInputQueue::Node {
    Node* next;
    Node* previous;
    TextInputEvent event;
};

TextInputQueue::Node* TextInputQueue::create_sentinel_00bec710() {
    static_assert(sizeof(Node) == 12);
    auto* node = new Node;
    node->next = node;
    node->previous = node;
    return node;
}

TextInputQueue::Node* TextInputQueue::create_node_00bec7b0(
    Node* next, Node* previous, TextInputEvent event) {
    auto* node = new Node;
    node->next = next;
    node->previous = previous;
    node->event = event;
    return node;
}

TextInputQueue::TextInputQueue() : sentinel_(create_sentinel_00bec710()) {}
TextInputQueue::~TextInputQueue() { destroy_00bec730(); }

void TextInputQueue::increase_count_00bed290(std::uint32_t increment) {
    if (std::uint32_t{0x7fffffff} - count_ < increment)
        throw std::length_error("list<T> too long");
    count_ += increment;
}

void TextInputQueue::append_00bed370(TextInputEvent event) {
    // Native allocates before the length check. Host RAII frees the unlinked node
    // if that check throws; original allocator/SEH failure behavior is not ported.
    std::unique_ptr<Node> node(create_node_00bec7b0(sentinel_, sentinel_->previous, event));
    increase_count_00bed290(1);
    sentinel_->previous = node.get();
    node->previous->next = node.get();
    node.release();
}

void TextInputQueue::destroy_00bec730() noexcept {
    clear_nodes();
    delete sentinel_;
    sentinel_ = nullptr;
}

void TextInputQueue::clear_nodes() noexcept {
    auto* node = sentinel_->next;
    sentinel_->next = sentinel_;
    sentinel_->previous = sentinel_;
    count_ = 0;
    while (node != sentinel_) {
        auto* next = node->next;
        delete node;
        node = next;
    }
}

const TextInputEvent* TextInputQueue::front() const noexcept {
    return sentinel_->next == sentinel_ ? nullptr : &sentinel_->next->event;
}

bool TextInputQueue::pop_00bece90(TextInputEvent& event) noexcept {
    auto* node = sentinel_->next;
    if (node == sentinel_) return false;
    event = node->event;
    // Valid-node erase path00bec9bf..00bec9da. Native iterator/debug checks and
    // returned iterator ABI are outside this queue's private-node interface.
    node->previous->next = node->next;
    node->next->previous = node->previous;
    delete node;
    --count_;
    return true;
}

void PlatformTextInput::enable_00a965a0(bool value) noexcept {
    enabled = value;
    queue.clear_nodes();
}

void PlatformTextInput::enqueue_message_00bed3b0_fragment(
    std::uint32_t message, std::uint32_t wparam) {
    if (!enabled) return;
    if (message == 0x102) { // WM_CHAR
        if (wparam == 0x16) clipboard_requested = true;
        queue.append_00bed370({static_cast<std::uint8_t>(wparam), 0});
    } else if (message == 0x100) { // WM_KEYDOWN: compare the full DWORD first
        switch (wparam) {
        case 0x26: case 0x28: case 0x25: case 0x27: case 0x24: case 0x09:
        case 0x23: case 0x2e: case 0x2d: case 0x14: case 0x21: case 0x22:
            queue.append_00bed370({static_cast<std::uint8_t>(wparam), 1});
            break;
        default: break;
        }
    }
}

void dispatch_text_input_00a96f40_fragment(TextInputQueue& queue, TextInputCallbacks& owner) {
    while (owner.enabled && queue.size() != 0) {
        TextInputEvent event;
        queue.pop_00bece90(event);
        if (!owner.on_event(event)) owner.fallback(event);
        // A callback may disable the owner or mutate/clear the queue. Re-read
        // both on the next iteration; fallback still runs if on_event disabled it.
    }
}
}
