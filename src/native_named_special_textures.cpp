#include "bsp/native_named_special_textures.hpp"
#include "bsp/native_cube_texture_base.hpp"
#include "bsp/native_logical_texture_named_base.hpp"
#include "bsp/native_string_pool_storage.hpp"

#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native named special textures require MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(D3DSURFACE_DESC) == 0x20);
static_assert(offsetof(D3DSURFACE_DESC, Width) == 0x18);
static_assert(sizeof(D3DVOLUME_DESC) == 0x1c);
static_assert(offsetof(D3DVOLUME_DESC, Width) == 0x10);
static_assert(offsetof(D3DVOLUME_DESC, Height) == 0x14);
static_assert(offsetof(D3DVOLUME_DESC, Depth) == 0x18);
void* at(const void* p, std::uint32_t n = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) + n);
}
// Isolated DWORD reads also retain the native unwritten descriptor preimage;
// no C++ initialized-output or failed-HRESULT fallback is substituted.
__forceinline std::uint32_t word(const void* p) noexcept {
    std::uint32_t result;
    __asm { mov eax, p }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov result, eax }
    return result;
}
__forceinline void put(void* p, std::uint32_t value) noexcept {
    __asm { mov eax, p }
    __asm { mov edx, value }
    __asm { mov dword ptr [eax], edx }
}
ActualNativeStringPoolStorage& strings(NativeRendererTextureNameNotificationContext& c) {
    if (!c.actual_native_string_pool || &c.actual_string_storage != c.actual_native_string_pool)
        throw std::invalid_argument("named special textures require the same actual notification string pool");
    return *c.actual_native_string_pool;
}
using CountCall = std::uint32_t (__stdcall*)(void*);
using CubeDescCall = HRESULT (__stdcall*)(void*, UINT, D3DSURFACE_DESC*);
using VolumeDescCall = HRESULT (__stdcall*)(void*, UINT, D3DVOLUME_DESC*);
std::uint32_t current_com_slot(void* input, std::uint32_t offset) noexcept {
    return word(at(reinterpret_cast<void*>(word(input)), offset));
}
struct Operation {
    NativeNamedSpecialTextureAcquired* acquired;
    Operation(NativeNamedSpecialTextureAcquired* a, void* raw, void* com,
        std::uint32_t base_site) : acquired(a) {
        if (!a) return;
        if (a->phase != NativeNamedSpecialTextureAcquired::Phase::fresh)
            throw std::logic_error("named texture constructor operation cannot replay");
        a->raw_owner = raw;
        a->input_com = com;
        a->native_site = base_site;
        a->phase = NativeNamedSpecialTextureAcquired::Phase::named_base;
    }
    ~Operation() {
        if (acquired && acquired->phase != NativeNamedSpecialTextureAcquired::Phase::complete)
            acquired->phase = NativeNamedSpecialTextureAcquired::Phase::failed;
    }
    void site(NativeNamedSpecialTextureAcquired::Phase phase, std::uint32_t address) noexcept {
        if (acquired) { acquired->phase = phase; acquired->native_site = address; }
    }
    void base_complete() noexcept { if (acquired) acquired->named_base_complete = true; }
    void* finish(void* raw) noexcept {
        if (acquired) { acquired->creator = raw; acquired->phase = NativeNamedSpecialTextureAcquired::Phase::complete; }
        return raw;
    }
};
int terminate_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_named_owner(void* raw, ActualNativeStringPoolStorage& storage, bool volume,
    NativeNamedSpecialTextureAcquired* acquired) noexcept {
    // CBED20/40 select the same actual B34090/B340F0 cleanups as established
    // cube/volume owners. Native FH3 has one cleanup state and no catches.
    if (acquired) acquired->cleanup_started = true;
    __try {
        if (volume) unwind_native_volume_texture_base_00b340f0(raw, storage);
        else unwind_native_logical_texture_unnamed_base_00b34090(raw, storage);
        if (acquired) acquired->named_base_unwound = true;
    } __except (terminate_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
struct NameCleanup {
    void* raw;
    ActualNativeStringPoolStorage& storage;
    bool volume;
    NativeNamedSpecialTextureAcquired* acquired;
    bool armed{true};
    ~NameCleanup() noexcept { if (armed) unwind_named_owner(raw, storage, volume, acquired); }
};
} // namespace

void* construct_native_named_cube_texture_base_00b34280(void* raw, const void* name,
    void* com, std::uint32_t flags, ActualNativeStringPoolStorage& storage, std::uint32_t& serial) {
    construct_native_logical_texture_named_base_00b34120(raw, name, com, flags, storage, serial);
    put(raw, 0x00d5f280);
    return raw;
}
void* construct_native_named_volume_texture_base_00b342d0(void* raw, const void* name,
    void* com, std::uint32_t flags, ActualNativeStringPoolStorage& storage, std::uint32_t& serial) {
    construct_native_logical_texture_named_base_00b34120(raw, name, com, flags, storage, serial);
    put(raw, 0x00d5f2c0);
    return raw;
}

void* construct_native_named_cube_texture_00b3ced0(void* raw, const void* name,
    IDirect3DCubeTexture9* input, std::uint32_t flags, NativeCubeTextureOwnerContext& c,
    NativeNamedSpecialTextureAcquired* acquired) {
    Operation operation{acquired, raw, input, 0x00b3cf01};
    auto& storage = strings(c.renderer_notification);
    construct_native_named_cube_texture_base_00b34280(raw, name, input, flags,
        storage, c.actual_shared_serial_0108d6e8);
    operation.base_complete();
    put(raw, 0x00d61870);
    put(at(raw, 0x2c), 0);
    alignas(4) unsigned char descriptor[sizeof(D3DSURFACE_DESC)];
    const auto get_desc = reinterpret_cast<CubeDescCall>(current_com_slot(input, 0x44));
    NameCleanup cleanup{raw, storage, false, acquired}; // State0 before CF21.
    operation.site(NativeNamedSpecialTextureAcquired::Phase::descriptor, 0x00b3cf21);
    get_desc(input, 0, reinterpret_cast<D3DSURFACE_DESC*>(descriptor));
    put(at(raw, 0x24), word(descriptor + 0x18));
    const auto get_count = reinterpret_cast<CountCall>(current_com_slot(input, 0x34));
    operation.site(NativeNamedSpecialTextureAcquired::Phase::level_count, 0x00b3cf30);
    const auto mips = get_count(input);
    put(at(raw, 0x14), mips);
    put(at(raw, 0x18), word(descriptor));
    cleanup.armed = false;
    return operation.finish(raw);
}

void* construct_native_named_volume_texture_00b3cfa0(void* raw, const void* name,
    IDirect3DVolumeTexture9* input, std::uint32_t flags, NativeNamedVolumeTextureContext& c,
    NativeNamedSpecialTextureAcquired* acquired) {
    Operation operation{acquired, raw, input, 0x00b3cfd1};
    auto& storage = strings(c.owner.renderer_notification);
    construct_native_named_volume_texture_base_00b342d0(raw, name, input, flags,
        storage, c.actual_shared_serial_0108d6e8);
    operation.base_complete();
    put(raw, 0x00d618b0);
    put(at(raw, 0x30), 0);
    alignas(4) unsigned char descriptor[sizeof(D3DVOLUME_DESC)];
    const auto get_desc = reinterpret_cast<VolumeDescCall>(current_com_slot(input, 0x44));
    NameCleanup cleanup{raw, storage, true, acquired}; // State0 before CFF1.
    operation.site(NativeNamedSpecialTextureAcquired::Phase::descriptor, 0x00b3cff1);
    get_desc(input, 0, reinterpret_cast<D3DVOLUME_DESC*>(descriptor));
    const auto width = word(descriptor + 0x10);
    const auto height = word(descriptor + 0x14);
    const auto depth = word(descriptor + 0x18);
    put(at(raw, 0x28), height);
    put(at(raw, 0x24), width);
    put(at(raw, 0x2c), depth);
    const auto get_count = reinterpret_cast<CountCall>(current_com_slot(input, 0x34));
    operation.site(NativeNamedSpecialTextureAcquired::Phase::level_count, 0x00b3d00e);
    const auto mips = get_count(input);
    const auto format = word(descriptor);
    put(at(raw, 0x14), mips);
    put(at(raw, 0x18), format);
    cleanup.armed = false;
    return operation.finish(raw);
}
} // namespace bsp
