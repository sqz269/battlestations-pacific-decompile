#include "bsp/native_texture_2d_owner.hpp"
#include "bsp/native_logical_texture_named_base.hpp"
#include "bsp/native_physical_buffer_owner.hpp"
#include "bsp/native_render_buffer_unregistration.hpp"
#include "bsp/native_texture_surface_cache_storage.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstring>
#include <exception>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(D3DSURFACE_DESC) == 32);

void* address(const void* base, std::uint32_t offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
__forceinline std::uint32_t word(const void* location) noexcept {
    std::uint32_t value;
    __asm { mov eax, location }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}
__forceinline std::uint8_t byte(const void* location) noexcept {
    std::uint8_t value;
    __asm { mov eax, location }
    __asm { mov al, byte ptr [eax] }
    __asm { mov value, al }
    return value;
}
__forceinline void put(void* location, std::uint32_t value) noexcept {
    __asm { mov eax, location }
    __asm { mov edx, value }
    __asm { mov dword ptr [eax], edx }
}
void* pointer(std::uint32_t value) noexcept { return reinterpret_cast<void*>(value); }
volatile LONG* references(void* owner) noexcept {
    return reinterpret_cast<volatile LONG*>(address(owner, 4));
}
NativeTextureSurfaceCacheStorage& cache(void* owner) noexcept {
    return *static_cast<NativeTextureSurfaceCacheStorage*>(address(owner, 0x40));
}

const volatile std::uint32_t* memory_table(void* owner,
    NativeRetainedMemoryOwnerContext& context) noexcept {
    const auto current_profile = word(owner);
    if (current_profile == 0x00d15ad8) return context.actual_backing_profile_00d15ad8;
    if (current_profile == 0x00d642c0) return context.actual_stream_profile_00d642c0;
    __assume(0);
}
std::uint32_t table_word(const volatile std::uint32_t* table,
    std::uint32_t offset = 0) noexcept {
    return word(address(const_cast<const std::uint32_t*>(table), offset));
}
void release_memory_at_zero(void* captured, NativeRetainedMemoryOwnerContext& context) {
    const auto invoker = table_word(memory_table(captured, context));
    __assume(invoker == 0x00bd30e0);
    // Full BD30E0 reloads the current owner table before its flag1 dispatch.
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
const volatile std::uint32_t* surface_table(void* owner,
    NativeTexture2DOwnerContext& context) noexcept {
    const auto current_profile = word(owner);
    __assume(current_profile == 0x00d619a0);
    return context.actual_surface_profile_00d619a0;
}
void release_surface_at_zero(void* captured, NativeTexture2DOwnerContext& context) {
    const auto invoker = table_word(surface_table(captured, context));
    __assume(invoker == 0x00bd30e0);
    const auto terminal = table_word(surface_table(captured, context), 4);
    __assume(terminal == 0x00b3f5b0);
    delete_native_surface_00b3f5b0(
        *static_cast<NativeSurfaceOwnerStorage*>(captured), 1, context.surfaces);
}
const volatile std::uint32_t* renderer_table(const void* renderer,
    NativeTexture2DOwnerContext& context) noexcept {
    const auto current_profile = word(renderer);
    __assume(current_profile == 0x00d5f0a8);
    return context.actual_renderer_profile_00d5f0a8;
}

using ComWordCall = std::uint32_t (__stdcall*)(void*);
using ComDescCall = HRESULT (__stdcall*)(void*, UINT, D3DSURFACE_DESC*);
ComWordCall current_com_word_call(void* actual_com, std::uint32_t offset) noexcept {
    return reinterpret_cast<ComWordCall>(word(address(pointer(word(actual_com)), offset)));
}
void balanced_com_pair(void* captured) {
    current_com_word_call(captured, 4)(captured);
    current_com_word_call(captured, 8)(captured);
}
void support(NativeTexture2DOwnerContext& context) {
    resource_support_singleton_00b3e730(context.surfaces.actual_resource_support_0108fedc,
        context.surfaces.actual_lifetime_01090aa0);
}

int terminate_cpp_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_owner(void* owner, void* diagnostic, int state,
    NativeTexture2DOwnerContext& context) noexcept {
    // Native FH3 has cleanup maps and no catches. A second C++ exception must
    // terminate during search, before any nested destructor starts unwinding.
    __try {
        if (state >= 2)
            destroy_native_buffer_diagnostic_record_00b3f4c0(
                diagnostic, context.renderer_notification.actual_string_storage);
        if (state >= 1) destroy_native_texture_surface_cache_00b3ec40(cache(owner));
        unwind_native_logical_texture_named_base_00b34010(
            owner, context.renderer_notification.actual_string_storage);
    } __except (terminate_cpp_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
struct OwnerCleanup {
    void* owner;
    NativeTexture2DOwnerContext& context;
    void* diagnostic;
    int state;
    ~OwnerCleanup() noexcept {
        if (state >= 0) unwind_owner(owner, diagnostic, state, context);
    }
};
} // namespace

void* construct_native_texture_2d_00b3f930(void* owner, const void* name,
    IDirect3DTexture9* input_texture, std::uint32_t saved_width,
    std::uint32_t saved_height, std::uint32_t flags, NativeTexture2DOwnerContext& context) {
    auto& strings = context.renderer_notification.actual_string_storage;
    construct_native_logical_texture_named_profile_00b34230(
        owner, name, input_texture, flags, strings, context.actual_shared_serial_0108d6e8);
    put(owner, 0x00d61948);
    put(address(owner, 0x24), 0);
    put(address(owner, 0x3c), 0);
    OwnerCleanup cleanup{owner, context, nullptr, 0};
    put(address(owner, 0x40), 0);
    put(address(owner, 0x44), 0);
    put(address(owner, 0x48), 0);
    auto* const current_com = pointer(word(address(owner, 0x10)));
    put(address(owner, 0x4c), 0);
    const auto get_level_count = current_com_word_call(current_com, 0x34);
    cleanup.state = 1;
    put(address(owner, 0x14), get_level_count(current_com));
    put(address(owner, 0x1c), flags);
    if (input_texture) balanced_com_pair(input_texture);

    alignas(4) unsigned char descriptor[sizeof(D3DSURFACE_DESC)];
    const auto get_level_desc = reinterpret_cast<ComDescCall>(
        word(address(pointer(word(input_texture)), 0x44)));
    get_level_desc(input_texture, 0, reinterpret_cast<D3DSURFACE_DESC*>(descriptor));
    // HRESULT is ignored and no zero-filled descriptor fallback is introduced.
    const auto actual_width = word(descriptor + 0x18);
    const auto actual_height = word(descriptor + 0x1c);
    put(address(owner, 0x28), actual_width);
    put(address(owner, 0x2c), actual_height);
    balanced_com_pair(input_texture);
    const auto actual_format = word(descriptor);
    put(address(owner, 0x34), saved_width);

    alignas(4) unsigned char diagnostic[12];
    put(diagnostic, reinterpret_cast<std::uintptr_t>(input_texture));
    char* captured_data = nullptr;
    auto* const diagnostic_name = diagnostic + 4;
    const bool distinct = static_cast<const void*>(diagnostic_name) != name;
    put(address(owner, 0x38), saved_height);
    put(address(owner, 0x30), 0);
    put(address(owner, 0x18), actual_format);
    put(diagnostic_name, 0);
    put(diagnostic_name + 4, 0);
    if (distinct) {
        resize_native_string_header_0041dd40(diagnostic_name, strings, word(name), true);
        const auto current_source_length = word(name);
        captured_data = static_cast<char*>(pointer(word(diagnostic_name + 4)));
        if (current_source_length != 0) {
            const auto count = word(diagnostic_name);
            const auto* source = pointer(word(address(name, 4)));
            if (count != 0) std::memmove(captured_data, source, count);
        }
    }
    cleanup.diagnostic = diagnostic;
    cleanup.state = 2;
    support(context);
    cleanup.state = 1;
    if (captured_data) strings.release(captured_data, word(diagnostic_name) + 1u);
    if ((flags & 0x10u) != 0)
        context.actual_tracking_counter_0108daf8 = context.actual_tracking_counter_0108daf8 + 1u;
    cleanup.state = -1;
    return owner;
}

void destroy_native_texture_2d_00b3f2e0(void* owner, NativeTexture2DOwnerContext& context) {
    put(owner, 0x00d61948);
    auto* const captured_source = pointer(word(address(owner, 0x4c)));
    OwnerCleanup cleanup{owner, context, nullptr, 1};
    if (captured_source) {
        if (InterlockedDecrement(references(captured_source)) == 0)
            release_memory_at_zero(captured_source, context.retained_memory);
        put(address(owner, 0x4c), 0);
    }
    const auto* const captured_renderer = context.renderer_notification.actual_renderer_00f8d394;
    const auto* const captured_notification_slot = address(const_cast<const std::uint32_t*>(
        renderer_table(captured_renderer, context)), 0x6c);
    const auto* const owner_name = native_logical_texture_name_address_00b33e40(owner);
    const auto notification_entry = word(captured_notification_slot);
    __assume(notification_entry == 0x00b32250);
    notify_native_renderer_texture_name_removal_00b32250(
        const_cast<void*>(captured_renderer), owner_name, context.renderer_notification);
    unregister_native_renderer_texture_00b27d40(
        const_cast<void*>(context.renderer_notification.actual_renderer_00f8d394),
        reinterpret_cast<std::uintptr_t>(owner));

    if (auto* const captured_com = pointer(word(address(owner, 0x10))))
        balanced_com_pair(captured_com);
    support(context);
    if (auto* const current_com = pointer(word(address(owner, 0x10)))) {
        current_com_word_call(current_com, 8)(current_com);
        put(address(owner, 0x10), 0);
    }
    std::uint32_t index = 0;
    if (static_cast<std::int32_t>(word(address(owner, 0x44))) > 0) {
        do {
            auto* const data = pointer(word(address(owner, 0x40)));
            auto* const captured_slot = address(data, index * 8u + 4u);
            auto* const captured_surface = pointer(word(captured_slot));
            if (captured_surface) {
                if (InterlockedDecrement(references(captured_surface)) == 0)
                    release_surface_at_zero(captured_surface, context);
                put(captured_slot, 0);
            }
            index += 1u;
        } while (static_cast<std::int32_t>(index) <
                 static_cast<std::int32_t>(word(address(owner, 0x44))));
    }
    if ((byte(address(owner, 0x1c)) & 0x10u) != 0)
        context.actual_tracking_counter_0108daf8 = context.actual_tracking_counter_0108daf8 - 1u;
    cleanup.state = 0;
    destroy_native_texture_surface_cache_00b3ec40(cache(owner));
    cleanup.state = -1;
    destroy_native_logical_texture_named_base_00b33f50(
        owner, context.renderer_notification.actual_string_storage);
}

void* delete_native_texture_2d_00b3f590(void* owner,
    std::uint32_t flags, NativeTexture2DOwnerContext& context) {
    destroy_native_texture_2d_00b3f2e0(owner, context);
    if ((flags & 1u) != 0) context.actual_texture_pool_0108db38.return_raw_slot_00b3d8d0(owner);
    return owner;
}
} // namespace bsp
