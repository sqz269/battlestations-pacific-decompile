#include "bsp/native_vfs_request_list_lifetime.hpp"

#include "bsp/native_render_resource_record.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native VFS request lists require MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T>
volatile T& field(void* owner, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile T*>(
        static_cast<unsigned char*>(owner) + offset);
}

void* at(void* owner, std::size_t offset) noexcept {
    return static_cast<unsigned char*>(owner) + offset;
}

// FuncInfoE00ED0/mapE00EC0: state1 -> CC6808 (+8/4D2640), then
// state0 -> CC6800 (base/41DD20). No cleanup for the list currently destroyed.
struct PayloadUnwind {
    void* payload;
    ActualNativeStringPoolStorage& strings;
    int state = 1;
    ~PayloadUnwind() noexcept {
        if (state == 1)
            destroy_native_vfs_string_list_004d2640(at(payload, 8), strings);
        if (state >= 0)
            destroy_native_string_header_0041dd20(payload, strings);
    }
};
} // namespace

void destroy_native_vfs_string_list_004d2640(
    void* owner, ActualNativeStringPoolStorage& strings) {
    clear_native_render_resource_aliases_004d05e0(owner, strings);
    singleton_lifetime_free(field<void*>(owner, 4));
    field<void*>(owner, 4) = nullptr;
}

void destroy_native_vfs_request_payload_00be1220(
    void* payload, ActualNativeStringPoolStorage& strings) {
    PayloadUnwind unwind{payload, strings};
    // The two normal clears are inline in BE1220;4D2640 is its state1 unwind.
    void* list = at(payload, 0x14);
    clear_native_render_resource_aliases_004d05e0(list, strings);
    singleton_lifetime_free(field<void*>(list, 4));
    field<void*>(list, 4) = nullptr;
    list = at(payload, 8);
    unwind.state = 0;
    clear_native_render_resource_aliases_004d05e0(list, strings);
    singleton_lifetime_free(field<void*>(list, 4));
    field<void*>(list, 4) = nullptr;
    // Native captures base data before lowering state and reads length only
    // for nonnull data. Retain the raw header, including callback mutations.
    char* const data = field<char*>(payload, 4);
    unwind.state = -1;
    if (data)
        strings.release(data, field<std::uint32_t>(payload, 0) + 1u);
}

void clear_native_vfs_request_list_00be19e0(
    void* owner, ActualNativeStringPoolStorage& strings) {
    void* head = field<void*>(owner, 4);
    void* cursor = field<void*>(head, 0);
    field<void*>(head, 0) = head;
    head = field<void*>(owner, 4);
    field<void*>(head, 4) = head;
    const bool initially_empty = cursor == field<void*>(owner, 4);
    field<std::uint32_t>(owner, 8) = 0;
    if (initially_empty) return;
    do {
        void* const next = field<void*>(cursor, 0);
        destroy_native_vfs_request_payload_00be1220(at(cursor, 8), strings);
        singleton_lifetime_free(cursor);
        cursor = next;
    } while (cursor != field<void*>(owner, 4));
}

void destroy_native_vfs_request_list_00be1d60(
    void* owner, ActualNativeStringPoolStorage& strings) {
    clear_native_vfs_request_list_00be19e0(owner, strings);
    singleton_lifetime_free(field<void*>(owner, 4));
    field<void*>(owner, 4) = nullptr;
}
} // namespace bsp
