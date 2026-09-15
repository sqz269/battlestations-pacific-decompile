#include "bsp/native_render_resources_surfaces.hpp"
#include "bsp/native_renderer_device_recreation_actual.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native render-resource surface factories require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4 && sizeof(long) == 4);
void* at(const void* p, Word offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + offset);
}
Word word(const void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, offset));
}
void put(void* p, Word offset, Word value) noexcept {
    *static_cast<volatile Word*>(at(p, offset)) = value;
}
Word bits(const void* p) noexcept { return static_cast<Word>(reinterpret_cast<std::uintptr_t>(p)); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
template<class Method> Method method(const void* p, Word slot) noexcept {
    return reinterpret_cast<Method>(word(pointer(word(p)), slot));
}
using CreateSurface = HRESULT (WINAPI *)(void*, UINT, UINT, D3DFORMAT,
    D3DMULTISAMPLE_TYPE, DWORD, BOOL, IDirect3DSurface9**, HANDLE*);
using Release = ULONG (WINAPI *)(void*);

void begin(void* renderer, NativeRendererSurfaceFactoryContext& c,
    NativeRendererSurfaceFactoryAcquired& a) {
    if (a.phase != NativeRendererSurfaceFactoryAcquired::Phase::fresh)
        throw std::logic_error("surface factory requires a fresh persistent acquired operation");
    a.phase = NativeRendererSurfaceFactoryAcquired::Phase::guard;
    if (c.synchronization.mode_00 != 0) {
        a.guard.renderer_04 = renderer;
        a.guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(renderer, c.synchronization);
        a.guard_initialized = true;
    }
}
void unwind(NativeRendererSurfaceFactoryContext& c, NativeRendererSurfaceFactoryAcquired& a) noexcept {
    try {
        if (a.unwind_state == 1) {
            a.unwind_state = 0;
            return_d3d9_surface_slot_00b3dcc0(a.raw_slot);
            a.raw_slot_returned = true;
        }
        if (a.unwind_state == 0) {
            a.unwind_state = -1;
            destroy_native_renderer_optional_guard_00b21110(a.guard, c.synchronization);
        }
    } catch (...) { std::terminate(); }
}
void finish_guard(bool enabled, NativeRendererSurfaceFactoryContext& c,
    NativeRendererSurfaceFactoryAcquired& a) {
    if (enabled)
        leave_native_renderer_optional_guard_00b33b00(a.guard.renderer_04, a.guard.entered_00, c.synchronization);
}
bool needs_retry(const NativeRendererSurfaceFactoryAcquired& a) noexcept {
    return a.com_output == nullptr && a.create_result != 0 &&
        static_cast<Word>(a.create_result) != 0x8876017cu &&
        static_cast<Word>(a.create_result) != 0x8007000eu;
}
void recreate(void* renderer, NativeRendererSurfaceFactoryContext& c) {
    if (!c.recreation)
        throw std::invalid_argument("surface factory reached unbound actual B29670 recreation domain");
    recreate_native_renderer_device_00b29670(renderer, *c.recreation);
}
void construct_owner(Word flags, std::uint8_t kind, NativeRendererSurfaceFactoryContext& c,
    NativeRendererSurfaceFactoryAcquired& a) {
    a.phase = NativeRendererSurfaceFactoryAcquired::Phase::construct;
    a.raw_slot = allocate_d3d9_surface_slot_00b3f2a0();
    a.unwind_state = 1;
    a.owner = a.raw_slot ? construct_native_surface_00b3f630(
        a.raw_slot, a.com_output, flags, kind, c.surfaces) : nullptr;
    a.unwind_state = 0;
    a.phase = NativeRendererSurfaceFactoryAcquired::Phase::bookkeeping;
}
void append(void* renderer, NativeSurfaceOwnerStorage* owner) {
    void* const header = at(renderer, 0x1b0c);
    const Word capacity = word(header, 8);
    if (word(header, 4) == capacity) {
        const auto doubled = static_cast<std::int32_t>(capacity + capacity);
        reserve_native_renderer_surface_slots_00b22850(header, doubled > 1 ? doubled : 1);
    }
    void* const slot = pointer(word(header) + word(header, 4) * 4u);
    if (slot) put(slot, 0, bits(owner));
    put(header, 4, word(header, 4) + 1u);
}
void release_output(NativeRendererSurfaceFactoryAcquired& a) {
    void* const captured = a.com_output;
    const auto release = method<Release>(captured, 8);
    (void)release(captured); // Native is unconditional. No null-COM success fallback.
}
Word truncate_quality(float quality) noexcept {
    unsigned short saved, truncating;
    __int64 converted;
    __asm {
        fld quality
        fnstcw saved
        mov ax, saved
        or ax, 0c00h
        mov truncating, ax
        fldcw truncating
        fistp qword ptr converted
        fldcw saved
    }
    return static_cast<Word>(converted); // Original LOW32, including FISTP indefinite.
}
bool account(Word width, Word height, Word format, bool color,
    NativeRendererSurfaceFactoryContext& c, NativeRendererSurfaceFactoryAcquired& a) noexcept {
    const Word storage_bits = native_format_storage_bits_00b21210(format);
    Word bytes = storage_bits >> 3;
    float bytes_float = 0.0f;
    const volatile float* const unsigned_fix = &c.unsigned_dword_fix_00ce3978;
    const volatile double* const counter_fix = &c.unsigned_counter_fix_00d57da0;
    volatile Word* const counter = &c.allocation_bytes_0108d4c0;
    const volatile std::uint8_t* const mode = &c.synchronization.mode_00;
    auto* const unwind_state = &a.unwind_state;
    if (storage_bits) {
        __asm {
            mov eax, bytes
            test eax, eax
            fild dword ptr bytes
            jns converted_bytes
            mov edx, unsigned_fix
            fadd dword ptr [edx]
        converted_bytes:
            fstp dword ptr bytes_float
        }
    }
    Word area = width * height;
    unsigned short saved, truncating;
    __int64 converted;
    std::uint8_t leave_enabled = 0;
    __asm {
        mov eax, area
        test eax, eax
        fild dword ptr area
        jns converted_area
        mov edx, unsigned_fix
        fadd dword ptr [edx]
    converted_area:
        mov ecx, counter
        mov eax, dword ptr [ecx]
        fmul dword ptr bytes_float
        test eax, eax
        fild dword ptr [ecx]
        jns converted_counter
        mov edx, counter_fix
        fadd qword ptr [edx]
    converted_counter:
        fnstcw saved
        cmp color, 0
        je keep_guard_armed
        mov edx, unwind_state
        mov dword ptr [edx], -1
    keep_guard_armed:
        faddp st(1), st(0)
        mov ax, saved
        or ax, 0c00h
        mov truncating, ax
        cmp color, 0
        je mode_not_captured
        mov edx, mode
        cmp byte ptr [edx], 0
        setne leave_enabled
    mode_not_captured:
        fldcw truncating
        fistp qword ptr converted
        mov eax, dword ptr converted
        mov dword ptr [ecx], eax
        fldcw saved
    }
    return leave_enabled != 0;
}
} // namespace

Word native_format_storage_bits_00b21210(Word format) noexcept {
    static constexpr std::uint8_t format_values[98] = { 24,32,32,64,64,64,64,8,8,64,64,32,32,32,32,32,64,0,0,0,64,8,0,0,0,0,0,0,0,0,8,64,8,0,0,0,0,0,0,0,64,64,32,32,32,0,0,32,0,0,64,32,0,64,0,32,0,32,0,32,16,16,32,32,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,64,16,32,64,32,64,128,16 };
    if (format - 0x14u <= 0x61u) return format_values[format - 0x14u];
    switch (format) {
    case 0x3154454du: return 64;
    case 0x31545844u: return 4;
    case 0x32545844u: case 0x33545844u: case 0x34545844u: case 0x35545844u:
    case 0x32595559u: case 0x59565955u: return 8;
    case 0x47424752u: case 0x42475247u: return 32;
    default: return 0;
    }
}
void reserve_native_renderer_surface_slots_00b22850(void* header, std::int32_t requested) {
    if (requested < 1) requested = 1;
    if (static_cast<std::int32_t>(word(header, 8)) >= requested) return;
    const Word bytes = static_cast<Word>(requested) * 4u;
    void* const replacement = singleton_lifetime_allocate({SingletonAllocationKind::pointer_slots, bytes, bytes});
    Word index = 0;
    void* destination = replacement;
    while (static_cast<std::int32_t>(index) < static_cast<std::int32_t>(word(header, 4))) {
        if (destination) put(destination, 0, word(pointer(word(header) + index * 4u)));
        ++index;
        destination = at(destination, 4);
    }
    singleton_lifetime_free(pointer(word(header)));
    put(header, 0, bits(replacement));
    put(header, 8, static_cast<Word>(requested));
}
NativeSurfaceOwnerStorage* create_native_renderer_render_target_00b2a7c0(void* renderer,
    Word width, Word height, Word format, Word multisample, NativeRendererSurfaceFactoryContext& c,
    NativeRendererSurfaceFactoryAcquired& a) {
    begin(renderer, c, a);
    try {
        a.phase = NativeRendererSurfaceFactoryAcquired::Phase::create;
        void* device = pointer(word(renderer, 0x1a10));
        a.com_output = nullptr;
        a.unwind_state = 0;
        a.native_site = 0x00b2a826;
        a.create_result = method<CreateSurface>(device, 0x70)(device, width, height,
            static_cast<D3DFORMAT>(format), static_cast<D3DMULTISAMPLE_TYPE>(multisample),
            0, FALSE, const_cast<IDirect3DSurface9**>(&a.com_output), nullptr);
        if (needs_retry(a)) {
            a.native_site = 0x00b2a843; recreate(renderer, c);
            device = pointer(word(renderer, 0x1a10));
            a.native_site = 0x00b2a867;
            a.create_result = method<CreateSurface>(device, 0x70)(device, width, height,
                static_cast<D3DFORMAT>(format), static_cast<D3DMULTISAMPLE_TYPE>(multisample),
                0, FALSE, const_cast<IDirect3DSurface9**>(&a.com_output), nullptr);
        }
        a.native_site = 0x00b2a86e; construct_owner(0x10, 0, c, a);
        append(renderer, a.owner);
        a.native_site = 0x00b2a8dc; release_output(a);
        const bool leave = account(width, height, format, true, c, a);
        finish_guard(leave, c, a);
        a.phase = NativeRendererSurfaceFactoryAcquired::Phase::complete;
        return a.owner;
    } catch (...) { a.phase = NativeRendererSurfaceFactoryAcquired::Phase::failed; unwind(c, a); throw; }
}
NativeSurfaceOwnerStorage* create_native_renderer_depth_surface_00b2a9a0(void* renderer,
    Word width, Word height, Word format, Word multisample, float quality, std::uint8_t discard,
    NativeRendererSurfaceFactoryContext& c, NativeRendererSurfaceFactoryAcquired& a) {
    begin(renderer, c, a);
    try {
        a.phase = NativeRendererSurfaceFactoryAcquired::Phase::create;
        void* device = pointer(word(renderer, 0x1a10));
        const Word integer_quality = truncate_quality(quality);
        a.com_output = nullptr;
        a.unwind_state = 0;
        a.native_site = 0x00b2aa31;
        a.create_result = method<CreateSurface>(device, 0x74)(device, width, height,
            static_cast<D3DFORMAT>(format), static_cast<D3DMULTISAMPLE_TYPE>(multisample),
            integer_quality, discard, const_cast<IDirect3DSurface9**>(&a.com_output), nullptr);
        if (needs_retry(a)) {
            a.native_site = 0x00b2aa4e; recreate(renderer, c);
            device = pointer(word(renderer, 0x1a10));
            a.native_site = 0x00b2aa78;
            a.create_result = method<CreateSurface>(device, 0x74)(device, width, height,
                static_cast<D3DFORMAT>(format), static_cast<D3DMULTISAMPLE_TYPE>(multisample),
                integer_quality, discard, const_cast<IDirect3DSurface9**>(&a.com_output), nullptr);
        }
        a.native_site = 0x00b2aa7f; construct_owner(0x100, 1, c, a);
        (void)account(width, height, format, false, c, a);
        append(renderer, a.owner);
        a.native_site = 0x00b2ab76; release_output(a);
        const bool leave = c.synchronization.mode_00 != 0;
        a.unwind_state = -1;
        finish_guard(leave, c, a);
        a.phase = NativeRendererSurfaceFactoryAcquired::Phase::complete;
        return a.owner;
    } catch (...) { a.phase = NativeRendererSurfaceFactoryAcquired::Phase::failed; unwind(c, a); throw; }
}
void resize_native_render_resources_depth_00b0fc10(void* service, Word width, Word height,
    Word unused_argument, NativeRenderResourcesDepthContext& c, NativeRendererSurfaceFactoryAcquired& a) {
    (void)unused_argument;
    auto* frame = static_cast<NativeFrameTargetOwnerStorage*>(pointer(word(service, 0x1c8)));
    if (!frame) return;
    NativeFrameTargetOwnerContext frames{c.factory.surfaces, c.surface_profile_00d619a0};
    set_native_frame_target_depth_00b1fb00(*frame, nullptr, frames);
    void* const renderer = c.factory.surfaces.actual_renderer_00f8d394;
    if (!renderer || word(renderer) != 0x00d5f0a8u || !c.renderer_profile_00d5f0a8 ||
        c.renderer_profile_00d5f0a8[0x94 / 4] != 0x00b2a9a0u)
        throw std::invalid_argument("unsupported current depth factory renderer slot94");
    NativeSurfaceOwnerStorage* const created = create_native_renderer_depth_surface_00b2a9a0(
        renderer, width, height, 0x4b, 0, 0.0f, 1, c.factory, a);
    frame = static_cast<NativeFrameTargetOwnerStorage*>(pointer(word(service, 0x1c8)));
    set_native_frame_target_depth_00b1fb00(*frame, created, frames);
    if (created) {
        a.creator_release_started = true;
        const auto decrement = c.decrement_iat_00ce2220;
        if (!decrement) throw std::invalid_argument("unbound current depth creator decrement target");
        if (decrement(reinterpret_cast<volatile long*>(at(created, 4))) == 0) {
            if (word(created) != 0x00d619a0u || !c.surface_profile_00d619a0 ||
                c.surface_profile_00d619a0[0] != 0x00bd30e0u ||
                word(created) != 0x00d619a0u || c.surface_profile_00d619a0[1] != 0x00b3f5b0u)
                throw std::invalid_argument("unsupported current depth creator terminal");
            delete_native_surface_00b3f5b0(*created, 1, c.factory.surfaces);
        }
    }
}
} // namespace bsp
