#include "bsp/native_volume_texture_owner.hpp"
#include "bsp/native_logical_texture_named_base.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native volume texture owner requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(D3D9SurfacePool::slot_bytes == 0x38);
static_assert(D3D9SurfacePool::slot_slab_index_offset == native_volume_texture_owner_bytes);

void* address(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
__forceinline std::uint32_t word(const void* storage) noexcept {
    std::uint32_t value;
    __asm { mov eax, storage }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}
__forceinline void put(void* storage, std::uint32_t value) noexcept {
    __asm { mov eax, storage }
    __asm { mov edx, value }
    __asm { mov dword ptr [eax], edx }
}
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }
std::uint32_t table_word(const volatile std::uint32_t* table,
    std::uint32_t offset = 0) noexcept {
    return word(address(const_cast<const std::uint32_t*>(table), offset));
}
const volatile std::uint32_t* memory_table(void* owner,
    NativeRetainedMemoryOwnerContext& context) noexcept {
    const auto profile = word(owner);
    if (profile == 0x00d15ad8) return context.actual_backing_profile_00d15ad8;
    if (profile == 0x00d642c0) return context.actual_stream_profile_00d642c0;
    __assume(0);
}
void release_memory_at_zero(void* captured, NativeRetainedMemoryOwnerContext& context) {
    const auto invoker = table_word(memory_table(captured, context));
    __assume(invoker == 0x00bd30e0);
    // BD30E0 reloads the current profile before invoking deleting slot4 with 1.
    const auto terminal = table_word(memory_table(captured, context), 4);
    if (terminal == 0x008d4470) {
        delete_native_memory_backing_008d4470(captured, 1, context);
        return;
    }
    if (terminal == 0x00bb8f90) {
        delete_native_memory_stream_00bb8f90(captured, 1, context);
        return;
    }
    __assume(0);
}
using ComWordCall = std::uint32_t (__stdcall*)(void*);
ComWordCall current_com_call(void* actual_com, std::uint32_t offset) noexcept {
    return reinterpret_cast<ComWordCall>(word(address(pointer(word(actual_com)), offset)));
}
int terminate_cpp_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_owner(void* owner, NativeStringStorage& strings) noexcept {
    // Original B340F0 is a five-byte tail to full B33F50. FH3 has one
    // cleanup state, no catches; terminate a second C++ throw during search.
    __try {
        unwind_native_volume_texture_base_00b340f0(owner, strings);
    } __except (terminate_cpp_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
struct OwnerCleanup {
    void* owner;
    NativeStringStorage& strings;
    bool armed{true};
    ~OwnerCleanup() noexcept { if (armed) unwind_owner(owner, strings); }
};
} // namespace

void unwind_native_volume_texture_base_00b340f0(
    void* owner, NativeStringStorage& strings) {
    destroy_native_logical_texture_named_base_00b33f50(owner, strings);
}

void destroy_native_volume_texture_00b3eb70(void* owner,
    NativeVolumeTextureOwnerContext& context) {
    put(owner, 0x00d618b0);
    const auto* const captured_renderer = context.renderer_notification.actual_renderer_00f8d394;
    const auto renderer_profile = word(captured_renderer);
    __assume(renderer_profile == 0x00d5f0a8);
    const auto* const captured_table = context.actual_renderer_profile_00d5f0a8;
    OwnerCleanup cleanup{owner, context.renderer_notification.actual_string_storage};
    const auto* const name = native_logical_texture_name_address_00b33e40(owner);
    const auto notification_entry = table_word(captured_table, 0x6c);
    __assume(notification_entry == 0x00b32250);
    notify_native_renderer_texture_name_removal_00b32250(
        const_cast<void*>(captured_renderer), name, context.renderer_notification);
    if (auto* const captured_source = pointer(word(address(owner, 0x30)))) {
        if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(address(captured_source, 4))) == 0)
            release_memory_at_zero(captured_source, context.retained_memory);
        put(address(owner, 0x30), 0);
    }
    if (auto* const captured_com = pointer(word(address(owner, 0x10)))) {
        current_com_call(captured_com, 4)(captured_com);
        current_com_call(captured_com, 8)(captured_com);
    }
    if (auto* const current_com = pointer(word(address(owner, 0x10)))) {
        current_com_call(current_com, 8)(current_com);
        put(address(owner, 0x10), 0);
    }
    cleanup.armed = false;
    destroy_native_logical_texture_named_base_00b33f50(
        owner, context.renderer_notification.actual_string_storage);
}

void* delete_native_volume_texture_00b3f430(void* owner,
    std::uint32_t flags, NativeVolumeTextureOwnerContext& context) {
    destroy_native_volume_texture_00b3eb70(owner, context);
    if ((flags & 1u) != 0)
        context.actual_volume_pool_0108dba8.return_raw_slot_00b3d860(owner);
    return owner;
}
} // namespace bsp
