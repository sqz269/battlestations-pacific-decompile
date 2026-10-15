#include "bsp/native_dword_uninitialized_fill.hpp"

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

void __fastcall fill_native_dword_uninitialized_006970d0(
    void* destination, std::uint32_t count, const void* value,
    void* unused_owner, const void* unused_value_duplicate,
    std::uint32_t unused_tag) {
    (void)unused_owner;
    (void)unused_value_duplicate;
    (void)unused_tag;
    if (count == 0)
        return;
    auto cursor = address(destination);
    auto remaining = count;
    const void* const captured_value_address = value;
    while (remaining != 0) {
        if (cursor != 0) {
            // Native EAX captures the source ADDRESS, not its contents.
            // Keep each current DWORD load immediately before its store.
            const auto current_value =
                *static_cast<const volatile std::uint32_t*>(captured_value_address);
            *reinterpret_cast<volatile std::uint32_t*>(
                static_cast<std::uintptr_t>(cursor)) = current_value;
        }
        --remaining;
        cursor += 4U;
    }
}

void* __fastcall fill_native_dword_uninitialized_n_dispatch_00697760(
    void* owner, std::uint32_t unused_edx, void* destination,
    std::uint32_t count, const void* value) {
    (void)unused_edx;
    const auto captured_destination = address(destination);
    const auto captured_count = count;
    const auto unused_tag = address(owner) & 0xffffff00U;
    fill_native_dword_uninitialized_006970d0(
        destination, count, value, owner, value, unused_tag);
    return pointer(captured_destination + captured_count * 4U);
}

} // namespace bsp
