#include "bsp/native_registry_list_producers.hpp"

#include "bsp/cc13_avoid1.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp {
namespace {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uintptr_t) == 4);
static_assert(sizeof(Cc13TerrainGridLayerListNode) == 0x0c);
static_assert(offsetof(Cc13TerrainGridLayerListNode, next) == 0);
static_assert(offsetof(Cc13TerrainGridLayerListNode, previous) == 4);
static_assert(offsetof(Cc13TerrainGridLayerListNode, value) == 8);

template<class Value>
void store_word(std::uintptr_t address, const Value& value) noexcept {
    static_assert(sizeof(Value) == 4);
    std::memcpy(reinterpret_cast<void*>(address), &value, sizeof(value));
}

Cc13TerrainGridLayerListNode* allocate_node_bytes() {
    return static_cast<Cc13TerrainGridLayerListNode*>(singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x0c, 0x0c}));
}

} // namespace

Cc13TerrainGridLayerListNode* __cdecl buy_native_registry_list_head_0041bfd0() {
    auto* const node = allocate_node_bytes();
    const auto address = reinterpret_cast<std::uintptr_t>(node);
    if (address != 0) {
        store_word(address, node); // 0041BFDE: next self-link
    }
    const auto previous_address = address + std::uintptr_t{4};
    if (previous_address != 0) {
        store_word(previous_address, node); // 0041BFE7: previous self-link
    }
    return node; // payload+8h remains uninitialized
}

Cc13TerrainGridLayerListNode* __stdcall buy_native_registry_list_node_0041c1f0(
    Cc13TerrainGridLayerListNode* next,
    Cc13TerrainGridLayerListNode* previous,
    Cc13TerrainGridLayerNative* const* value_address) {
    auto* const node = allocate_node_bytes();
    const auto address = reinterpret_cast<std::uintptr_t>(node);
    if (address != 0) {
        store_word(address, next); // 0041C202
    }
    const auto previous_address = address + std::uintptr_t{4};
    if (previous_address != 0) {
        store_word(previous_address, previous); // 0041C20F
    }
    const auto payload_address = address + std::uintptr_t{8};
    if (payload_address != 0) {
        Cc13TerrainGridLayerNative* value;
        std::memcpy(&value, value_address, sizeof(value)); // 0041C21C, after links
        store_word(payload_address, value); // 0041C21E
    }
    return node;
}

void* __fastcall construct_native_registry_list_0041e270(void* actual_list_header) {
    auto* const head = buy_native_registry_list_head_0041bfd0(); // 0041E273
    const auto address = reinterpret_cast<std::uintptr_t>(actual_list_header);
    store_word(address + std::uintptr_t{4}, head); // 0041E278
    store_word(address + std::uintptr_t{8}, std::uint32_t{0}); // 0041E27B
    return actual_list_header; // 0041E282; proxy+0h untouched
}

} // namespace bsp
