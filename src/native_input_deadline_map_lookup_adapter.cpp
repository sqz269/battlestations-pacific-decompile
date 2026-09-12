#include "bsp/native_input_deadline_map_lookup_adapter.hpp"
#include "bsp/native_input_deadline_map_storage.hpp"
#include "bsp/detail/native_tree_insert_storage.hpp"
#include <cstdlib>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native deadline map storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Access = detail::TreeInsertAccess<0x14, 0x15>;

std::int32_t read_key(const void* storage) noexcept {
    return *static_cast<const volatile std::int32_t*>(storage);
}
std::int32_t node_key(void* node) noexcept {
    return read_key(static_cast<unsigned char*>(node) + 0x0c);
}

struct ScalarKeyCapture {
    const void* source;
    bool loaded = false;
    std::int32_t value = 0;
    std::int32_t get() noexcept {
        if (!loaded) {
            value = read_key(source);
            loaded = true;
        }
        return value;
    }
};

struct ScalarPair { std::int32_t key; std::uint32_t mapped_bits; };
static_assert(sizeof(ScalarPair) == 8);

NativeInputDeadlineMapIterator* insert_missing_pair(void* tree,
    NativeInputDeadlineMapIterator* output, const ScalarPair& pair) {
    ScalarKeyCapture captured{&pair};
    return detail::insert_unique_tree_pair<Access, NativeInputDeadlineMapIterator>(
        tree, output, &pair,
        [&captured](void* node) {
            const auto query = captured.get();
            return query < node_key(node);
        },
        [&pair](void* node) {
            const auto current = node_key(node);
            return current < read_key(&pair);
        },
        decrement_input_deadline_map_iterator, link_input_deadline_map_node,
        [](NativeInputDeadlineMapIterator* result, void* owner, void* node, std::uint8_t) {
            Access::word(result, 0) = owner;
            Access::word(result, 4) = node;
        });
}
} // namespace

float* subscript_input_deadline_map_storage(void* tree, const std::int32_t* key) {
    ScalarKeyCapture captured{key};
    auto* node = detail::lower_bound_tree_node<Access>(tree,
        [&captured](void* current) {
            const auto query = captured.get();
            return node_key(current) < query;
        });
    auto* owner = tree;
    // 4D692C / 4D6935 reread the current header and key after traversal.
    bool missing = node == Access::head(tree);
    if (!missing) {
        const auto current_key = read_key(key);
        missing = current_key < node_key(node);
    }
    if (missing) {
        // 4D693C captures a possibly aliased key before any allocation; +0.0
        // is a raw zero DWORD, without a floating-point conversion.
        const ScalarPair pair{read_key(key), 0};
        NativeInputDeadlineMapIterator result;
        // A consistent lower-bound hint selects the same vacant parent/side
        // as the existing unique driver. See the path proof in the report.
        auto* returned = insert_missing_pair(tree, &result, pair);
        owner = Access::word(returned, 0);
        node = Access::word(returned, 4);
    }
    // These diagnostics may return. Keep the captured owner/node, reload the
    // owner's current head after the first call, and perform the final LEA.
    if (owner == nullptr) {
        _invalid_parameter_noinfo();
    }
    if (node == Access::head(owner)) {
        _invalid_parameter_noinfo();
    }
    return reinterpret_cast<float*>(static_cast<unsigned char*>(node) + 0x10);
}

} // namespace bsp
