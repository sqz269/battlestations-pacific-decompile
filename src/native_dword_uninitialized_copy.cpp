#include "bsp/native_dword_uninitialized_copy.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uintptr_t) == sizeof(std::uint32_t));

std::uint32_t address(const void* pointer) noexcept {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer));
}

void* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(value));
}
} // namespace

void* __fastcall copy_native_dword_uninitialized_00696d40(
    const void* first, const void* last, void* destination,
    void* unused_owner, std::uint32_t unused_metadata,
    std::uint32_t unused_tag) {
    (void)unused_owner;
    (void)unused_metadata;
    (void)unused_tag;
    auto source = address(first);
    const auto captured_last = address(last);
    auto output = address(destination);
    while (source != captured_last) {
        if (output != 0) {
            const auto current_value = *reinterpret_cast<const volatile std::uint32_t*>(
                static_cast<std::uintptr_t>(source));
            *reinterpret_cast<volatile std::uint32_t*>(
                static_cast<std::uintptr_t>(output)) = current_value;
        }
        source += 4U;
        output += 4U;
    }
    return pointer(output);
}

void* __fastcall copy_native_dword_uninitialized_dispatch_00697860(
    void* owner, std::uint32_t unused_edx, const void* first,
    const void* last, void* destination) {
    (void)unused_edx;
    const auto unused_tag = address(owner) & 0xffffff00U;
    return copy_native_dword_uninitialized_00696d40(
        first, last, destination, owner, address(destination), unused_tag);
}

} // namespace bsp
