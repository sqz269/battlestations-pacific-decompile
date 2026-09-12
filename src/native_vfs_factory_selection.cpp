#include "bsp/native_vfs_factory_selection.hpp"
#include "bsp/singleton_lifetime.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS factory selection requires MSVC Win32.
#endif

namespace bsp {
namespace {
void* link(const void* base, std::uint32_t offset = 0) noexcept {
    static_assert(sizeof(void*) == 4);
    return *reinterpret_cast<void* const volatile*>(
        reinterpret_cast<std::uintptr_t>(base) + offset);
}
} // namespace

void* select_native_vfs_factory_00bdb040(void* manager,
    const void* system_header, const void* virtual_header,
    NativeVfsFactoryCreateDispatch& dispatch,
    const SingletonLifetimeCallbacks& callbacks) {
    auto* const initial_head = link(manager, 0x34);
    auto* cursor = link(initial_head);
    auto* const list = reinterpret_cast<void*>(
        reinterpret_cast<std::uintptr_t>(manager) + 0x30u);
    for (;;) {
        // BDB050 compares EDI to itself; BDB057 cannot execute. MOV EBX at
        // BDB052 and the second head load at BDB060 remain distinct reads.
        auto* const current_head = link(list, 4);
        if (cursor == current_head) return nullptr;
        if (cursor == link(list, 4))
            callbacks.invalid_parameter(callbacks.context);
        auto* const factory = link(cursor, 8);
        auto* const table = link(factory);
        const auto entry = reinterpret_cast<std::uintptr_t>(link(table, 4));
        auto* const result = dispatch.factory_create(
            entry, factory, system_header, virtual_header);
        if (result) return result;
        if (cursor == link(list, 4))
            callbacks.invalid_parameter(callbacks.context);
        cursor = link(cursor);
    }
}
} // namespace bsp
