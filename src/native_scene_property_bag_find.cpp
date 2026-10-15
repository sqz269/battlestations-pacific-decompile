#include "bsp/native_scene_property_bag_find.hpp"

#include "bsp/native_enum_dictionary_lookup.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <cstdint>
#include <cstring>

namespace bsp {
namespace {
using U = std::uint32_t;
static_assert(sizeof(void*) == sizeof(U));
U address(const void* p) noexcept { return reinterpret_cast<U>(p); }
void* pointer(U p) noexcept { return reinterpret_cast<void*>(p); }
const char* text(U p) noexcept { return reinterpret_cast<const char*>(p); }
U read(U p, U offset = 0) noexcept {
    return *reinterpret_cast<const volatile U*>(p + offset);
}
std::uint8_t read_byte(U p) noexcept {
    return *reinterpret_cast<const volatile std::uint8_t*>(p);
}
void write(U p, U value) noexcept { *reinterpret_cast<volatile U*>(p) = value; }
void write_byte(U p, std::uint8_t value) noexcept {
    *reinterpret_cast<volatile std::uint8_t*>(p) = value;
}

// States0 and1 have identical actual-header cleanup actions. This host guard
// has no native private-frame ABI, and is deliberately unarmed during ctor.
class CompletedKey {
public:
    CompletedKey(void* header, NativeStringRawPoolContext& pool) noexcept
        : header_(header), pool_(pool) {}
    ~CompletedKey() noexcept(false) {
        if (armed_) {
            armed_ = false;
            destroy_native_string_header_0041dd20(header_, pool_);
        }
    }
    void arm() noexcept { armed_ = true; }
    void release_normally() {
        // Native captures data BEFORE disarming, length AFTER; keep nonnull
        // block/length captures across the potentially throwing pool getter.
        const U block = read(address(header_), 4);
        armed_ = false;
        if (block != 0) {
            const U bytes = read(address(header_)) + 1u;
            auto* const owner = native_string_pool_get_or_create_00419cc0(
                pool_.actual_published_01090aa8,
                pool_.actual_manager_publication_01090aa0);
            return_native_string_pool_00bd1510(owner, pointer(block), bytes,
                pool_.actual_small_returns_disabled_01090aa4);
        }
    }
private:
    void* header_;
    NativeStringRawPoolContext& pool_;
    bool armed_ = false;
};
} // namespace

void* find_native_scene_property_bag_008f2260(void* actual_bag,
    const NativeScenePropertyBagFindFrame& frame,
    NativeScenePropertyBagFindContext& context) {
    const U parameters = address(frame.actual_parameter4);
    const U key = read(parameters);
    const U bag = address(actual_bag);
    const char* const dot_pointer = std::strchr(text(key), '.');
    const U dot = address(dot_pointer);
    void* const header = frame.actual_key_header8;
    CompletedKey completed(header, context.actual_string_pool);
    if (dot != 0) {
        const U head_bytes = dot - key;
        const U shared_head = address(context.actual_shared_head_00f89658);
        // BF7680 explicitly chooses backward copy when dst lies inside src.
        // Use the real overlap-capable CRT service, including zero-byte copy.
        std::memmove(pointer(shared_head), pointer(key), head_bytes);
        write_byte(shared_head + head_bytes, 0);
        construct_native_string_header_0041e870(header, context.actual_string_pool,
            text(shared_head));
        completed.arm();
        void* const node = find_native_property_map_node_0043b8b0(pointer(bag + 4u),
            header, *reinterpret_cast<U*>(parameters),
            context.actual_node_empty_00e177e4, context.actual_query_empty_00e17654);
        U record;
        if (node != nullptr) {
            record = read(address(node), 8);
            write_byte(parameters, 1);
        } else {
            write_byte(parameters, 0);
            record = read(parameters);
        }
        completed.release_normally();
        if (read_byte(parameters) != 0 && read(record, 4) == 6) {
            const U nested_bag = read(record, 12);
            const U tail = dot + 1u;
            const auto& child = *frame.recursive_frame;
            write(address(child.actual_parameter4), tail);
            return find_native_scene_property_bag_008f2260(pointer(nested_bag),
                child, context);
        }
        return nullptr;
    }

    construct_native_string_header_0041e870(header, context.actual_string_pool,
        text(key));
    completed.arm();
    void* const node = find_native_property_map_node_0043b8b0(pointer(bag + 4u),
        header, *reinterpret_cast<U*>(parameters),
        context.actual_node_empty_00e177e4, context.actual_query_empty_00e17654);
    U result;
    if (node != nullptr) {
        result = read(address(node), 8);
    } else {
        // The original loads the current bucket word even though its AL/SBB
        // mask returns0. Preserve that actual read, not just the final result.
        (void)read(parameters);
        result = 0;
    }
    completed.release_normally();
    return pointer(result);
}

} // namespace bsp
