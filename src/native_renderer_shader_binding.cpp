#include "bsp/native_renderer_shader_binding.hpp"

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);
Word load(const void* base, Word offset = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(reinterpret_cast<Word>(base) + offset);
}
void store(void* base, Word offset, Word value) noexcept {
    *reinterpret_cast<volatile Word*>(reinterpret_cast<Word>(base) + offset) = value;
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
struct GuardCleanup {
    const NativeRendererOptionalGuardStorage& guard;
    NativeRendererSynchronizationGlobals& globals;
    bool armed{true};
    ~GuardCleanup() noexcept {
        if (armed) destroy_native_renderer_optional_guard_00b21110(guard, globals);
    }
};
using BindShader = std::int32_t (__stdcall*)(void*, void*);

template<Word Cache, Word Slot, Word Counter>
void bind_shader(void* renderer, void* incoming,
    NativeRendererSynchronizationGlobals& globals) {
    NativeRendererOptionalGuardStorage guard;
    if (globals.mode_00 != 0) {
        guard.renderer_04 = renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, globals);
    }
    void* const old = pointer(load(renderer, Cache));
    GuardCleanup cleanup{guard, globals};
    bool changed = true;
    if (old && incoming) {
        const Word old_com = load(old, 8);
        changed = old_com != load(incoming, 8);
    }
    if (changed) {
        store(renderer, Cache, reinterpret_cast<Word>(incoming));
        if (incoming) {
            void* const device = pointer(load(renderer, 0x1a10));
            void* const shader = pointer(load(incoming, 8));
            void* const table = pointer(load(device));
            const auto bind = reinterpret_cast<BindShader>(load(table, Slot));
            (void)bind(device, shader);
            store(renderer, Counter, load(renderer, Counter) + 1u);
        } else if (old) {
            void* const device = pointer(load(renderer, 0x1a10));
            void* const table = pointer(load(device));
            const auto bind = reinterpret_cast<BindShader>(load(table, Slot));
            (void)bind(device, nullptr);
            store(renderer, Counter, load(renderer, Counter) + 1u);
        }
    }
    const auto current_mode = globals.mode_00;
    cleanup.armed = false;
    if (current_mode != 0) {
        const std::uint32_t ignored = guard.entered_00;
        const void* const saved_renderer = guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(saved_renderer, ignored, globals);
    }
}
} // namespace

void bind_native_renderer_vertex_shader_00b21d10(void* renderer, void* logical,
    NativeRendererSynchronizationGlobals& globals) {
    bind_shader<0x1770, 0x170, 0x1bc0>(renderer, logical, globals);
}
void bind_native_renderer_pixel_shader_00b21c20(void* renderer, void* logical,
    NativeRendererSynchronizationGlobals& globals) {
    bind_shader<0x176c, 0x1ac, 0x1bbc>(renderer, logical, globals);
}
} // namespace bsp
