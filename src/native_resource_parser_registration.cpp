#include "bsp/native_resource_parser_registration.hpp"
#include "bsp/native_resource_cache_links.hpp"
#include "bsp/native_resource_cache_node.hpp"
#include "bsp/native_resource_cache_pair.hpp"
#include "bsp/native_resource_cache_insert.hpp"
#include "bsp/native_resource_manager_trees.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <exception>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == 4);
void* at(const void* p, U n = 0) noexcept { return reinterpret_cast<void*>(reinterpret_cast<U>(p) + n); }
U word(const void* p, U n = 0) noexcept { return *static_cast<const volatile U*>(at(p,n)); }
void put(void* p, U n, U v) noexcept { *static_cast<volatile U*>(at(p,n)) = v; }
void* pointer(const void* p, U n = 0) noexcept { return reinterpret_cast<void*>(word(p,n)); }
void return_captured(void* data, U length, NativeStringRawPoolContext& strings) {
    if (!data) return;
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8,strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool,data,length + 1u,
        strings.actual_small_returns_disabled_01090aa4);
}
} // namespace

std::uint8_t register_native_resource_type_parser_00b80a50(void* manager, void* parser,
    NativeStringRawPoolContext& strings, const SingletonLifetimeCallbacks& callbacks,
    NativeResourceParserNameCalls& calls) {
    // Native offsets after saved registers: name10h, iterator then pair1Ch,
    // completed pair28h. The insertion result reuses the original name slot.
    alignas(4) unsigned char locals[0x34];
    U owned_name[2];
    void* const first_name = locals + 0x10;
    void* const insertion_pair = locals + 0x1c;
    void* const completed_pair = locals + 0x28;
    int state = -1;
    try {
        U target = word(pointer(parser),4);
        void* const query = calls.type_name(target,parser,first_name);
        void* const captured_head = pointer(manager,0x0c); // B80A80, before find.
        void* const tree = at(manager,8);
        void* const found = find_native_resource_parser_name_00b7e740(
            tree,insertion_pair,query,callbacks);
        void* const owner = pointer(found);
        if (!owner || owner != tree) callbacks.invalid_parameter(callbacks.context);
        const bool exists = pointer(found,4) != captured_head;
        void* const first_data = pointer(first_name,4);
        // The first local has NO outer cleanup state, even while find/getter
        // can throw. Its normal return precedes the duplicate branch.
        return_captured(first_data,word(first_name),strings);
        if (exists) return 0;

        target = word(pointer(parser),4); // B80AE4/AE6: reload table and target.
        (void)calls.type_name(target,parser,owned_name);
        void* const pair = construct_native_resource_parser_pair_00b7f340(
            completed_pair,parser,owned_name,strings);
        const bool identical = insertion_pair == pair;
        state = 0; // B80B0E: only the completed pair is now armed.
        put(insertion_pair,0,0);
        put(insertion_pair,4,0);
        void* captured_data = nullptr;
        if (!identical) {
            resize_native_string_header_0041dd40(insertion_pair,strings,word(pair),true);
            const bool nonempty = word(pair) != 0;
            captured_data = pointer(insertion_pair,4); // B80B2C before branch.
            if (nonempty) {
                const U count = word(insertion_pair);
                void* const source = pointer(pair,4);
                std::memmove(captured_data,source,count);
            }
        }
        put(insertion_pair,8,word(pair,8));
        state = 1;
        insert_native_resource_parser_unique_00b80290(tree,first_name,insertion_pair,strings,callbacks);
        state = 0; // Disarm before normal insertion-pair cleanup.
        return_captured(captured_data,word(insertion_pair),strings);
        void* const pair_data = pointer(completed_pair,4);
        state = -1;
        return_captured(pair_data,word(completed_pair),strings);
        return 1; // B80BAF; deliberately ignores inserted BYTE in result+8.
    } catch (...) {
        try {
            if (state >= 1) { state = 0; destroy_native_resource_parser_insert_pair_00b7e950(insertion_pair,strings); }
            if (state >= 0) { state = -1; destroy_native_resource_parser_pair_00b7e930(completed_pair,strings); }
        } catch (...) { std::terminate(); }
        throw;
    }
}

// Equivalence is established over all instructions, allocator catch code and
// FuncInfo/unwind/try/catch records. The report retains both original spans;
// these are concrete source reuse calls, not invented native call-graph edges.
void* rotate_native_resource_parser_right_00b7ca80(void* tree, void* node) noexcept {
    return rotate_native_resource_cache_right_00b7cbd0(tree,node);
}
void* rotate_native_resource_parser_left_00b7d590(void* tree, void* node) noexcept {
    return rotate_native_resource_cache_left_00b7d5f0(tree,node);
}
void* construct_native_resource_parser_node_00b7f1b0(void* node, void* left,
    void* parent, void* right, const void* pair, U color, NativeStringRawPoolContext& strings) {
    return construct_native_resource_cache_node_00b7f220(node,left,parent,right,pair,color,strings);
}
void* allocate_native_resource_parser_node_00b7f610(void* left, void* parent,
    void* right, const void* pair, U color, NativeStringRawPoolContext& strings) {
    return allocate_native_resource_cache_node_00b7f6a0(left,parent,right,pair,color,strings);
}
void* construct_native_resource_parser_pair_00b7f340(void* pair, void* parser,
    void* name, NativeStringRawPoolContext& strings) {
    return construct_native_resource_cache_pair_00b7f290(pair,parser,name,strings);
}
void* insert_native_resource_parser_at_00b7fd30(void* tree, void* output,
    U side, void* parent, const void* pair, NativeStringRawPoolContext& strings) {
    return insert_native_resource_cache_at_00b7ff80(tree,output,side,parent,pair,strings);
}
void* insert_native_resource_parser_unique_00b80290(void* tree, void* output,
    const void* pair, NativeStringRawPoolContext& strings, const SingletonLifetimeCallbacks& callbacks) {
    return insert_native_resource_cache_unique_00b803b0(tree,output,pair,strings,callbacks);
}
void destroy_native_resource_parser_pair_00b7e930(void* pair, NativeStringRawPoolContext& strings) {
    destroy_native_string_header_0041dd20(pair,strings);
}
void destroy_native_resource_parser_insert_pair_00b7e950(void* pair, NativeStringRawPoolContext& strings) {
    destroy_native_string_header_0041dd20(pair,strings);
}
} // namespace bsp
