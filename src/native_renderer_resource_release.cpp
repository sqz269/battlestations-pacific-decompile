#include "bsp/native_renderer_resource_release.hpp"
#include "bsp/native_occlusion_query_device_reset.hpp"
#include "bsp/native_surface_owner.hpp"
#include "bsp/native_texture_device_reset.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
Word word(const volatile void* p, Word byte_offset = 0) noexcept {
    Word result;
    __asm {
        mov eax, p
        mov edx, byte_offset
        mov eax, dword ptr [eax + edx]
        mov result, eax
    }
    return result;
}
std::uint8_t byte(const void* p, Word byte_offset) noexcept {
    std::uint8_t result;
    __asm {
        mov eax, p
        mov edx, byte_offset
        mov al, byte ptr [eax + edx]
        mov result, al
    }
    return result;
}
void clear_ready(void* p) noexcept {
    __asm {
        mov eax, p
        mov byte ptr [eax + 1d8bh], 0
    }
}
void* pointer(Word bits) noexcept { return reinterpret_cast<void*>(bits); }
Word address(const void* p) noexcept { return reinterpret_cast<Word>(p); }
Word renderer_slot(void* renderer, Word slot,
    const NativeRendererBindingResetContext& context) noexcept {
    const auto token = word(renderer);
    __assume(token == 0x00d5f0a8u);
    return word(context.actual_renderer_profile_00d5f0a8, slot);
}
void release_surface(void* owner, const NativeRendererResourceRestoreProfiles& profiles) {
    const auto token = word(owner);
    __assume(token == 0x00d619a0u);
    const auto target = word(profiles.surface_00d619a0, 0x3c);
    __assume(target == 0x00b3d510u);
    release_native_surface_for_reset_00b3d510(*static_cast<NativeSurfaceOwnerStorage*>(owner));
}
void release_query(void* owner, const NativeRendererResourceRestoreProfiles& profiles) {
    const auto token = word(owner);
    __assume(token == 0x00d62ad0u);
    const auto target = word(profiles.query_00d62ad0, 0x18);
    __assume(target == 0x00b5fe20u);
    release_native_occlusion_query_for_reset_00b5fe20(owner);
}
void release_texture(void* owner, const NativeRendererResourceRestoreProfiles& profiles) {
    const auto token = word(owner);
    const volatile Word* table;
    if (token == 0x00d61948u) table = profiles.texture_2d_00d61948;
    else if (token == 0x00d61870u) table = profiles.cube_00d61870;
    else { __assume(token == 0x00d618b0u); table = profiles.volume_00d618b0; }
    const auto target = word(table, 0x20);
    if (target == 0x00b3dd30u) {
        const NativeTextureResetSurfaceProfile surface{profiles.surface_00d619a0};
        release_native_texture_2d_for_reset_00b3dd30(owner, surface);
    } else {
        __assume(target == 0x00b33f10u);
        native_texture_reset_noop_00b33f10(owner);
    }
}
int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) noexcept {
    __try {
        destroy_native_renderer_optional_guard_00b21110(guard, globals);
    } __except (cleanup_exception(GetExceptionCode())) { __assume(0); }
}
struct GuardCleanup {
    const NativeRendererOptionalGuardStorage& guard;
    NativeRendererSynchronizationGlobals& globals;
    bool armed = true;
    ~GuardCleanup() noexcept { if (armed) unwind_guard(guard, globals); }
};
} // namespace

void release_native_renderer_resources_00b262c0(void* renderer,
    NativeRendererResourceReleaseContext& context) {
    auto& bindings = context.actual_bindings;
    auto& globals = bindings.actual_vertex.actual_logical_owner.actual_synchronization_0108d6dc;
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    const auto ready = byte(renderer, 0x1d8b); // B262F1 before state0 is armed.
    GuardCleanup cleanup{guard, globals};
    if (ready != 0) {
        clear_ready(renderer);
        for (Word sampler = 0; sampler < 20; ++sampler) {
            const auto target = renderer_slot(renderer, 0x130, bindings);
            __assume(target == 0x00b24710u);
            bind_native_renderer_texture_00b24710(renderer, sampler, nullptr, bindings.actual_texture);
        }
        for (Word remaining = 4; remaining != 0; --remaining) {
            const auto target = renderer_slot(renderer, 0x134, bindings);
            __assume(target == 0x00b24840u);
            bind_native_renderer_vertex_stream_00b24840(renderer, 0, nullptr, bindings.actual_vertex);
        }
        const auto target = renderer_slot(renderer, 0x138, bindings);
        __assume(target == 0x00b24b00u);
        bind_native_renderer_index_stream_00b24b00(renderer, nullptr, 0, bindings.actual_index);
        if (auto* const depth = pointer(word(renderer, 0x198c)))
            release_surface(depth, context.actual_resources);
        for (Word offset = 0x197c; offset != 0x198c; offset += 4) {
            if (auto* const color = pointer(word(renderer, offset)))
                release_surface(color, context.actual_resources);
        }
        for (std::int32_t index = 0; index < static_cast<std::int32_t>(word(renderer, 0x19a4)); ++index) {
            auto* const rows = pointer(word(renderer, 0x19a0));
            release_query(pointer(word(rows, static_cast<Word>(index) * 4)), context.actual_resources);
        }
        auto cursor = word(renderer, 0x1b00);
        auto count = word(renderer, 0x1b04);
        auto end = cursor + count * 4;
        while (cursor != end) {
            release_texture(pointer(word(pointer(cursor))), context.actual_resources);
            count = word(renderer, 0x1b04);
            const auto current_base = word(renderer, 0x1b00);
            cursor += 4;
            end = current_base + count * 4;
        }
        cursor = word(renderer, 0x1b0c);
        count = word(renderer, 0x1b10);
        end = cursor + count * 4;
        while (cursor != end) {
            release_surface(pointer(word(pointer(cursor))), context.actual_resources);
            count = word(renderer, 0x1b10);
            const auto current_base = word(renderer, 0x1b0c);
            cursor += 4;
            end = current_base + count * 4;
        }
        count = word(renderer, 0x1a7c); // Record walk has no owner load or callback.
        cursor = word(renderer, 0x1a78);
        if (cursor != cursor + count * 0x2c) {
            count = word(renderer, 0x1a7c);
            end = cursor + count * 0x2c;
            do { cursor += 0x2c; } while (cursor != end);
        }
        clear_native_renderer_binding_cache_00b241c0(pointer(address(renderer) + 0x34), context.actual_cache);
    }
    const auto current_mode = globals.mode_00;
    cleanup.armed = false;
    if (current_mode != 0)
        leave_native_renderer_optional_guard_00b33b00(guard.renderer_04, word(&guard), globals);
}
} // namespace bsp
