#include "bsp/dyn_profile_scopes.hpp"

#include <intrin.h>
#include <cstddef>
#include <stdexcept>

namespace bsp {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(DynProfileScopeStorage) == 0x14);
static_assert(sizeof(DynProfileScopeContext) == 0x10);
static_assert(offsetof(DynProfileScopeContext, memory) == 0
    && offsetof(DynProfileScopeContext, profile_slot_0109e9f8) == 0xc);
namespace {
template<class T> T& at(void* p, std::uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(p) + offset);
}
template<class T> const T& at(const void* p, std::uint32_t offset) {
    return *reinterpret_cast<const T*>(static_cast<const unsigned char*>(p) + offset);
}
std::uint64_t timestamp() noexcept {
    // These compiler barriers preserve the native memory/RDTSC boundary. The
    // CPU instruction remains unserialized, exactly as in the original code.
    _ReadWriteBarrier();
    const auto value = __rdtsc();
    _ReadWriteBarrier();
    return value;
}
} // namespace

namespace {
DynProfileNodeStorage* append_constructed_profile_child(
    DynProfileNodeStorage& parent,DynProfileNodeStorage* child,
    const AvoidZoneDynHullMemory& memory) {
    auto& data = at<DynProfileNodeStorage**>(&parent, 4);
    auto& count = at<std::uint32_t>(&parent, 8);
    auto& capacity = at<std::uint32_t>(&parent, 0xc);
    if (count == capacity) {
        const std::uint32_t requested = capacity * 2u + 2u;
        const std::uint32_t bytes = requested * 4u;
        capacity = requested;
        auto* replacement = static_cast<DynProfileNodeStorage**>(memory.allocate(memory.context, bytes));
        auto destination = reinterpret_cast<std::uint32_t>(replacement);
        for (std::uint32_t i = 0; i < count; ++i, destination += 4u) {
            if (destination) *reinterpret_cast<DynProfileNodeStorage**>(destination) = data[i];
        }
        if (data) memory.release(memory.context, data);
        data = replacement;
    }
    const auto destination = reinterpret_cast<std::uint32_t>(data) + count * 4u;
    if (destination) *reinterpret_cast<DynProfileNodeStorage**>(destination) = child;
    ++count;
    return data[count - 1u]; //C50458 reloads the appended entry, not just EBX.
}
} // namespace

DynProfileNodeStorage* dyn_profile_node_append_child_00c50390(
    DynProfileNodeStorage& parent, const char* name, std::uint32_t id,
    const AvoidZoneDynHullMemory& memory) {
    auto* child = static_cast<DynProfileNodeStorage*>(memory.allocate(memory.context, 0x48));
    if (child) child = dyn_profile_node_construct_00c44000(*child, name, id);
    return append_constructed_profile_child(parent,child,memory);
}

DynProfileNodeStorage* dyn_profile_node_append_child_00c50390(
    DynProfileNodeStorage& parent, const char* name, std::uint32_t id,
    const AvoidZoneDynHullMemory& memory,
    DynProfilePartialNodeDelete delete_partial_00bf65ac) {
    // Native allocation C503A9 precedes state0 C503B7. Do not catch it.
    auto* const allocation = static_cast<DynProfileNodeStorage*>(
        memory.allocate(memory.context,0x48));
    auto* child = allocation;
    if (child) {
        try {
            child = dyn_profile_node_construct_00c44000(*child,name,id,memory);
        } catch (...) {
            // CC8378 / CC837C: captured allocation, distinct BF65AC, rethrow.
            delete_partial_00bf65ac(allocation);
            throw;
        }
    }
    // Native state resets at C503D6. A completed orphan is NOT freed if later
    // parent-vector allocation/copy/release/append fails; no rollback is added.
    return append_constructed_profile_child(parent,child,memory);
}

namespace {
template<bool Qualified>
DynProfileScopeStorage* enter_profile_scope(DynProfileScopeStorage& scope,
    std::uint32_t id, const char* name, const DynProfileScopeContext& context,
    DynProfilePartialNodeDelete delete_partial_00bf65ac) {
    auto* profile = *context.profile_slot_0109e9f8;
    auto& cached = at<DynProfileNodeStorage*>(profile, 0xc + id * 4u);
    if (!cached) {
        auto* parent = at<DynProfileNodeStorage*>(profile, 4);
        DynProfileNodeStorage* child;
        if constexpr(Qualified)
            child=dyn_profile_node_append_child_00c50390(*parent,name,id,context.memory,
                                                       delete_partial_00bf65ac);
        else
            child=dyn_profile_node_append_child_00c50390(*parent,name,id,context.memory);
        profile = *context.profile_slot_0109e9f8; //C57041, before writing the original cache slot.
        cached = child;
    }
    auto* node = cached;
    at<DynProfileNodeStorage*>(&scope, 0x10) = node;
    at<void*>(node, 0) = at<void*>(profile, 4);
    at<void*>(profile, 4) = node;
    const auto start = timestamp();
    at<std::uint32_t>(&scope, 0) = static_cast<std::uint32_t>(start);
    at<std::uint32_t>(&scope, 4) = static_cast<std::uint32_t>(start >> 32u);
    return &scope;
}
} // namespace

DynProfileScopeStorage* dyn_profile_scope_enter_00c57020(DynProfileScopeStorage& scope,
    std::uint32_t id, const char* name, const DynProfileScopeContext& context) {
    return enter_profile_scope<false>(scope,id,name,context,nullptr);
}

DynProfileScopeStorage* dyn_profile_scope_enter_00c57020(DynProfileScopeStorage& scope,
    std::uint32_t id, const char* name, const DynProfileScopeContext& context,
    DynProfilePartialNodeDelete delete_partial_00bf65ac) {
    if(!delete_partial_00bf65ac)
        throw std::invalid_argument("Dyn profile scope requires a matching raw-node partial deleter");
    return enter_profile_scope<true>(scope,id,name,context,delete_partial_00bf65ac);
}

void dyn_profile_scope_leave_00c5bcd7(const DynProfileScopeStorage& scope,
    const DynProfileScopeContext& context) {
    const auto finish = timestamp();
    const std::uint64_t start = at<std::uint32_t>(&scope, 0) |
        (static_cast<std::uint64_t>(at<std::uint32_t>(&scope, 4)) << 32u);
    const std::uint64_t delta = finish - start;
    const auto low = static_cast<std::uint32_t>(delta);
    const auto high = static_cast<std::uint32_t>(delta >> 32u);
    auto* node = at<DynProfileNodeStorage*>(&scope, 0x10);
    auto& sum_low = at<std::uint32_t>(node, 0x38);
    const auto previous_low = sum_low;
    sum_low += low;
    at<std::uint32_t>(node, 0x30) = low;
    at<std::uint32_t>(node, 0x34) = high;
    at<std::uint32_t>(node, 0x3c) += high + (sum_low < previous_low ? 1u : 0u);
    ++at<std::uint32_t>(node, 0x40);
    auto* profile = *context.profile_slot_0109e9f8;
    at<void*>(profile, 4) = at<void*>(at<void*>(profile, 4), 0);
}
} // namespace bsp
