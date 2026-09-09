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
    auto* node = sentinel_->next;
    sentinel_->next = sentinel_;
    sentinel_->previous = sentinel_;
    count_ = 0;
    while (node != sentinel_) {
        auto* next = node->next;
        delete node;
        node = next;
    }
    delete sentinel_;
    sentinel_ = nullptr;
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
}
