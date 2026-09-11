#include "bsp/native_cube_texture_owner.hpp"
#include "bsp/native_cube_texture_base.hpp"
#include "bsp/native_cube_texture_pool_allocate.hpp"
#include "bsp/native_logical_texture_named_base.hpp"

#include <exception>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native cube texture owner requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(D3DSURFACE_DESC) == 32);
static_assert(offsetof(D3DSURFACE_DESC, Format) == 0);
static_assert(offsetof(D3DSURFACE_DESC, Width) == 0x18);

void* address(const void* base, std::uint32_t byte_offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + byte_offset);
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
    std::uint32_t byte_offset = 0) noexcept {
    return word(address(const_cast<const std::uint32_t*>(table), byte_offset));
}

const volatile std::uint32_t* memory_table(void* owner,
    NativeRetainedMemoryOwnerContext& context) noexcept {
    const auto current_profile = word(owner);
    if (current_profile == 0x00d15ad8) return context.actual_backing_profile_00d15ad8;
    if (current_profile == 0x00d642c0) return context.actual_stream_profile_00d642c0;
    __assume(0);
}
void release_memory_at_zero(void* captured, NativeRetainedMemoryOwnerContext& context) {
    const auto invoker = table_word(memory_table(captured, context));
    __assume(invoker == 0x00bd30e0);
    // BD30E0 reloads the current table and invokes its deleting slot with 1.
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
const volatile std::uint32_t* renderer_table(const void* renderer,
    NativeCubeTextureOwnerContext& context) noexcept {
    const auto current_profile = word(renderer);
    __assume(current_profile == 0x00d5f0a8);
    return context.actual_renderer_profile_00d5f0a8;
}

using ComWordCall = std::uint32_t (__stdcall*)(void*);
using ComDescCall = HRESULT (__stdcall*)(void*, UINT, D3DSURFACE_DESC*);
ComWordCall current_com_word_call(void* actual_com, std::uint32_t byte_offset) noexcept {
    return reinterpret_cast<ComWordCall>(word(address(pointer(word(actual_com)), byte_offset)));
}

int terminate_cpp_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_owner(void* owner, NativeStringStorage& strings) noexcept {
    // Original FH3 has one cleanup state and no catches. A second C++ throw
    // must terminate during search, before a nested destructor can unwind.
    __try {
        unwind_native_logical_texture_unnamed_base_00b34090(owner, strings);
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

void* construct_native_cube_texture_00b3d650(void* owner,
    IDirect3DCubeTexture9* input_texture, std::uint32_t flags,
    NativeCubeTextureOwnerContext& context) {
    construct_native_logical_texture_unnamed_base_00b34020(
        owner, input_texture, flags, context.actual_shared_serial_0108d6e8);
    put(address(owner, 0x2c), 0);
    put(owner, 0x00d61870);
    OwnerCleanup cleanup{owner, context.renderer_notification.actual_string_storage};
    alignas(4) unsigned char descriptor[sizeof(D3DSURFACE_DESC)];
    auto get_level_desc = reinterpret_cast<ComDescCall>(
        word(address(pointer(word(input_texture)), 0x44)));
    get_level_desc(input_texture, 0, reinterpret_cast<D3DSURFACE_DESC*>(descriptor));
    put(address(owner, 0x24), word(descriptor + 0x18));
    const auto level_count = current_com_word_call(input_texture, 0x34)(input_texture);
    put(address(owner, 0x18), word(descriptor));
    put(address(owner, 0x14), level_count);
    cleanup.armed = false;
    return owner;
}

void destroy_native_cube_texture_00b3ead0(void* owner,
    NativeCubeTextureOwnerContext& context) {
    put(owner, 0x00d61870);
    auto* const captured_source = pointer(word(address(owner, 0x2c)));
    OwnerCleanup cleanup{owner, context.renderer_notification.actual_string_storage};
    if (captured_source) {
        if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(address(captured_source, 4))) == 0)
            release_memory_at_zero(captured_source, context.retained_memory);
        put(address(owner, 0x2c), 0);
    }
    const auto* const captured_renderer = context.renderer_notification.actual_renderer_00f8d394;
    const auto* const captured_table = renderer_table(captured_renderer, context);
    const auto* const name = native_logical_texture_name_address_00b33e40(owner);
    const auto notification_entry = table_word(captured_table, 0x6c);
    __assume(notification_entry == 0x00b32250);
    notify_native_renderer_texture_name_removal_00b32250(
        const_cast<void*>(captured_renderer), name, context.renderer_notification);
    if (auto* const current_com = pointer(word(address(owner, 0x10)))) {
        current_com_word_call(current_com, 8)(current_com);
        put(address(owner, 0x10), 0);
    }
    cleanup.armed = false;
    destroy_native_logical_texture_named_base_00b33f50(
        owner, context.renderer_notification.actual_string_storage);
}

void* delete_native_cube_texture_00b3f410(void* owner,
    std::uint32_t flags, NativeCubeTextureOwnerContext& context) {
    destroy_native_cube_texture_00b3ead0(owner, context);
    if ((flags & 1u) != 0)
        return_native_cube_texture_slot_00b3d940(context.actual_cube_pool_0108db70, owner);
    return owner;
}
} // namespace bsp
