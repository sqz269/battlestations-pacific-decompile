#include "bsp/native_render_service_focus.hpp"
#include <cstdint>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(p) + offset);
}
volatile Word& word(void* p, Word offset) noexcept {
    return *static_cast<volatile Word*>(at(p, offset));
}
void* pointer(void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(word(p, offset));
}
volatile unsigned char& byte(void* p, Word offset) noexcept {
    return *static_cast<volatile unsigned char*>(at(p, offset));
}
}

void unlink_native_raw_root_node_00b72220(void* actual_root, void* actual_node) noexcept {
    void* const next = pointer(actual_node, 0x3c);
    if (next) word(next, 0x40) = word(actual_node, 0x40);
    void* const previous = pointer(actual_node, 0x40);
    if (previous) word(previous, 0x3c) = word(actual_node, 0x3c);
    else word(actual_root, 0x0c) = word(actual_node, 0x3c);
}

void clear_native_raw_node_root_00b6d890_null(void* actual_node) noexcept {
    void* const captured_root = pointer(actual_node, 0xa4);
    if (!captured_root && pointer(actual_node, 0x30)) return;
    if (captured_root && !pointer(actual_node, 0x30))
        unlink_native_raw_root_node_00b72220(captured_root, actual_node);
    word(actual_node, 0xa4) = 0;
    void* child = pointer(actual_node, 0x34);
    while (child) {
        clear_native_raw_node_root_00b6d890_null(child);
        child = pointer(child, 0x3c);
    }
}

void mark_native_render_batch_dirty_00b50010(void* actual_batch) noexcept {
    byte(actual_batch, 0x24c) = 1;
}

void invalidate_native_render_root_chain_00b4ecc0(void* actual_owner) noexcept {
    void* captured_root = pointer(actual_owner, 0x3c);
    byte(actual_owner, 0x250) = 1;
    while (pointer(captured_root, 0x0c)) {
        void* const current_root = pointer(actual_owner, 0x3c);
        clear_native_raw_node_root_00b6d890_null(pointer(current_root, 0x0c));
        captured_root = pointer(actual_owner, 0x3c);
    }
    byte(actual_owner, 0x251) = 1;
}

void refresh_native_render_service_focus_00b0d1e0(void* actual_service) noexcept {
    if (byte(actual_service, 0x1c4) == 0) return;
    void* const batch = pointer(actual_service, 0x20);
    if (batch) mark_native_render_batch_dirty_00b50010(batch);
    if (byte(actual_service, 0x1c4) == 0) return;
    void* const owner = pointer(actual_service, 0x30);
    if (owner) invalidate_native_render_root_chain_00b4ecc0(owner);
}
}
