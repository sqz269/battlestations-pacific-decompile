#include "bsp/text_input.hpp"
#include "bsp/engine_services_cc13.hpp"
#include "bsp/native_platform_construction.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
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
TextInputQueue::TextInputQueue(void* header, const SingletonLifetimeCallbacks& invalid) noexcept
    : sentinel_(nullptr), native_header_(header), native_invalid_(&invalid) {
    if (!header || !invalid.invalid_parameter) std::terminate();
}
TextInputQueue::~TextInputQueue() { if (!native_header_) destroy_00bec730(); }
TextInputQueue::Node* TextInputQueue::current_sentinel() const noexcept {
    return native_header_ ? *reinterpret_cast<Node* const volatile*>(
        static_cast<unsigned char*>(native_header_) + 4) : sentinel_;
}
std::uint32_t TextInputQueue::size() const noexcept {
    return native_header_ ? *reinterpret_cast<const volatile std::uint32_t*>(
        static_cast<unsigned char*>(native_header_) + 8) : count_;
}

void TextInputQueue::increase_count_00bed290(std::uint32_t increment) {
    if (std::uint32_t{0x7fffffff} - count_ < increment)
        throw std::length_error("list<T> too long");
    count_ += increment;
}

void TextInputQueue::append_00bed370(TextInputEvent event) {
    if (native_header_) {
        append_native_text_queue_00bed370(native_header_, nullptr, &event);
        return;
    }
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
    if (native_header_) {
        // A965A0: preserve captures and current-sentinel reloads around free.
        auto* const first = current_sentinel();
        auto* node = first->next;
        first->next = first;
        auto* const current = current_sentinel();
        current->previous = current_sentinel();
        *reinterpret_cast<volatile std::uint32_t*>(static_cast<unsigned char*>(native_header_) + 8) = 0;
        while (node != current_sentinel()) {
            auto* const next = node->next;
            singleton_lifetime_free(node);
            node = next;
        }
        return;
    }
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
    auto* const head = current_sentinel();
    return head->next == head ? nullptr : &head->next->event;
}

bool TextInputQueue::pop_00bece90(TextInputEvent& event) noexcept {
    auto* const head = current_sentinel();
    auto* node = head->next;
    if (node == head) return false;
    event = node->event;
    if (native_header_) {
        struct NativeInvalid final : engine_services_cc13::InvalidParameterSink {
            const SingletonLifetimeCallbacks& calls;
            explicit NativeInvalid(const SingletonLifetimeCallbacks& c) noexcept : calls(c) {}
            void invalid_parameter_00bf6713() override { calls.invalid_parameter(calls.context); }
        } invalid(*native_invalid_);
        engine_services_cc13::NativeIterator next;
        engine_services_cc13::erase_text_queue_node_00bec990(
            *static_cast<engine_services_cc13::NativeContainerHeader*>(native_header_),
            next, {native_header_, node}, invalid);
        return true;
    }
    // Valid-node erase path00bec9bf..00bec9da. Native iterator/debug checks and
    // returned iterator ABI are outside this queue's private-node interface.
    node->previous->next = node->next;
    node->next->previous = node->previous;
    delete node;
    --count_;
    return true;
}

PlatformTextInput::PlatformTextInput(void* platform, const SingletonLifetimeCallbacks& invalid) noexcept
    : enabled(*reinterpret_cast<bool*>(static_cast<unsigned char*>(platform) + 0x170)),
      clipboard_requested(*reinterpret_cast<bool*>(static_cast<unsigned char*>(platform) + 0x44)),
      queue(static_cast<unsigned char*>(platform) + 0x174, invalid) {}

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
