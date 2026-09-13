#include "bsp/native_hardware_layout_factory.hpp"
#include "bsp/native_hardware_layout_pool_allocate.hpp"
#include "bsp/native_hardware_layout_tree_insert.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(NativeHardwareLayoutTreePair) == 24);

void* address(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
__forceinline std::uint32_t word(const void* location) noexcept {
    std::uint32_t value;
    __asm { mov eax, location }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }

int terminate_cpp_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_raw_slot(void* raw_slot, void* actual_pool) noexcept {
    __try {
        return_native_hardware_layout_00b60260(raw_slot, actual_pool);
    } __except (terminate_cpp_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
struct RawSlotCleanup {
    void* raw_slot;
    void* actual_pool;
    bool armed;
    ~RawSlotCleanup() noexcept {
        if (armed) unwind_raw_slot(raw_slot, actual_pool);
    }
};
} // namespace

void* get_or_create_native_hardware_layout_00b2f710(
    const void* key, NativeHardwareLayoutConstructContext& context,
    void** acquired_before_insertion) {
    if (acquired_before_insertion && *acquired_before_insertion)
        throw std::logic_error("hardware layout acquisition output must start empty");
    auto& owner_context = context.actual_owner;
    auto* const actual_tree = owner_context.actual_tree_0108d530;
    auto* const actual_pool = owner_context.actual_hardware_layout_pool_0108fe9c;
    const auto& invalid = owner_context.invalid_parameters;
    NativeHardwareLayoutTreeIterator found;
    find_native_hardware_layout_key_00b28220(actual_tree, &found, key, invalid);
    auto* const captured_iterator_owner = pointer(word(&found));
    auto* const captured_head = pointer(word(address(actual_tree, 4)));
    if (!captured_iterator_owner || captured_iterator_owner != actual_tree)
        invalid.invalid_parameter(invalid.context);
    auto* const selected_node = pointer(word(address(&found, 4)));
    if (selected_node != captured_head) {
        if (!captured_iterator_owner) invalid.invalid_parameter(invalid.context);
        if (selected_node == pointer(word(address(captured_iterator_owner, 4))))
            invalid.invalid_parameter(invalid.context);
        auto* const selected_owner = pointer(word(address(selected_node, 0x20)));
        InterlockedIncrement(static_cast<volatile LONG*>(address(selected_owner, 4)));
        if (acquired_before_insertion) *acquired_before_insertion = selected_owner;
        return selected_owner;
    }

    auto* const raw_slot = allocate_native_hardware_layout_00b606f0(actual_pool);
    RawSlotCleanup cleanup{raw_slot, actual_pool, true};
    void* result = nullptr;
    if (raw_slot) result = construct_native_hardware_layout_00b60cb0(raw_slot, key, context);
    cleanup.armed = false;
    if (acquired_before_insertion) *acquired_before_insertion = result;

    // Original scratch pairs are uninitialized: only reached key words,
    // current count and value are written by the existing complete helpers.
    NativeHardwareLayoutTreePair first_pair;
    NativeHardwareLayoutTreePair copied_pair;
    void* value = result;
    construct_native_hardware_layout_pair_00b282b0(&first_pair, key, &value);
    copy_native_hardware_layout_pair_00b25ef0(&copied_pair, &first_pair);
    NativeHardwareLayoutTreeInsertResult inserted;
    insert_native_hardware_layout_pair_00b2f540(actual_tree, &inserted, &copied_pair, invalid);
    return result;
}

} // namespace bsp
