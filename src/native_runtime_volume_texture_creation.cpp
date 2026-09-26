#include "bsp/native_runtime_volume_texture_creation.hpp"

#include "bsp/native_cube_texture_owner_array_reserve.hpp"
#include "bsp/native_renderer_device_recreation_actual.hpp"
#include "bsp/native_texture_loading_cache.hpp"

#include <cstring>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native runtime volume creation requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == 4);

void* at(const void* base, Word offset = 0) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<Word>(base) + offset);
}
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
Word word(const volatile void* location) noexcept {
    Word value;
    __asm { mov eax, location }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}
void put(void* location, Word value) noexcept {
    __asm { mov eax, location }
    __asm { mov edx, value }
    __asm { mov dword ptr [eax], edx }
}
std::int32_t signed_bits(Word value) noexcept {
    std::int32_t result;
    std::memcpy(&result, &value, sizeof result);
    return result;
}
void state_byte(NativeRuntimeVolumeTextureCreationAcquired& a,
    std::uint8_t value) noexcept {
    *reinterpret_cast<volatile std::uint8_t*>(&a.unwind_state) = value;
}
// Recovered MSVC x86 COM ABI boundary, not an ISO-compatible SDK function
// type. Preserve every format/pool DWORD bit; require the genuine current
// callable native +60 target and inspect its complete emitted call schedule.
static_assert(sizeof(DWORD) == 4 && sizeof(UINT) == 4 && sizeof(HRESULT) == 4);
using Create = HRESULT (__stdcall*)(void*, UINT, UINT, UINT, UINT, DWORD,
    DWORD, DWORD, IDirect3DVolumeTexture9**, HANDLE*);

int cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_factory(volatile NativeRuntimeVolumeTextureCreationArguments& args,
    NativeRuntimeVolumeTextureCreationContext& c,
    NativeRuntimeVolumeTextureCreationAcquired& a) noexcept {
    __try {
        if (word(&a.unwind_state) == 1u) {
            put(&a.unwind_state, 0);
            // CBD3A8 reads CURRENT overwritten caller word, not a saved owner.
            void* const current_slot = pointer(args.height_04);
            a.returned_slot = current_slot;
            return_native_volume_texture_slot_00b3dcf0(
                current_slot, c.owner.actual_volume_pool_0108dba8);
            a.raw_slot_returned = true;
        }
        if (word(&a.unwind_state) == 0u) {
            put(&a.unwind_state, 0xffffffffu);
            destroy_native_renderer_optional_guard_00b21110(
                a.locals.guard_0c, c.synchronization);
        }
    } __except (cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
struct FactoryCleanup {
    volatile NativeRuntimeVolumeTextureCreationArguments& arguments;
    NativeRuntimeVolumeTextureCreationContext& context;
    NativeRuntimeVolumeTextureCreationAcquired& acquired;
    ~FactoryCleanup() noexcept {
        if (acquired.phase != NativeRuntimeVolumeTextureCreationAcquired::Phase::complete) {
            acquired.phase = NativeRuntimeVolumeTextureCreationAcquired::Phase::failed;
            unwind_factory(arguments, context, acquired);
        }
    }
};
} // namespace

void return_native_volume_texture_slot_00b3dcf0(
    void* current_slot, D3D9SurfacePool& pool) {
    pool.return_raw_slot_00b3d860(current_slot);
}

void* create_native_runtime_volume_texture_00b2a5a0(void* renderer,
    volatile NativeRuntimeVolumeTextureCreationArguments& args,
    NativeRuntimeVolumeTextureCreationContext& c,
    NativeRuntimeVolumeTextureCreationAcquired& a) {
    using Phase = NativeRuntimeVolumeTextureCreationAcquired::Phase;
    if (a.phase != Phase::fresh)
        throw std::invalid_argument("runtime volume factory frame must be fresh");
    a.phase = Phase::running;
    put(&a.unwind_state, 0xffffffffu);
    volatile auto& locals = a.locals;
    const auto entry_mode = c.synchronization.mode_00;
    locals.renderer_08 = renderer;
    FactoryCleanup cleanup{args, c, a};
    if (entry_mode != 0) {
        a.locals.guard_0c.renderer_04 = renderer;
        a.locals.guard_0c.entered_00 = enter_native_renderer_optional_guard_00b33ad0(
            renderer, c.synchronization);
    }
    const Word flags = args.flags_14;
    const Word pool_nibble = flags & 0xfu;
    Word usage = 0;
    put(&a.unwind_state, 0);
    if (pool_nibble <= 3u) locals.pool_00 = pool_nibble;
    if ((flags & 0x10u) != 0) usage = 1;
    switch (flags & 0xf00u) {
    case 0x100: usage |= 2; break;
    case 0x200: usage |= 0x4000; break;
    case 0x300: usage |= 0x40; break;
    case 0x400: usage |= 0x100; break;
    case 0x500: usage |= 0x80; break;
    default: break;
    }
    if ((flags & 0xf000u) == 0x1000u) usage |= 0x200;

    void* const first_device = pointer(word(at(renderer, 0x1a10)));
    const Word format = args.format_10;
    const Word depth = args.depth_08;
    const Word levels = args.levels_0c;
    locals.com_output_04 = nullptr;
    void* const first_table = pointer(word(first_device));
    const Word first_pool = locals.pool_00;
    const Word first_height = args.height_04;
    const Word first_width = args.width_00;
    const auto first_create = reinterpret_cast<Create>(word(at(first_table, 0x60)));
    const HRESULT first_result = first_create(first_device, first_width, first_height,
        depth, levels, usage, format,
        first_pool, &a.locals.com_output_04, nullptr);
    a.last_create_result = first_result;
    if (!locals.com_output_04 && first_result != 0 &&
        static_cast<Word>(first_result) != 0x8876017cu &&
        static_cast<Word>(first_result) != 0x8007000eu) {
        if (!c.recreation)
            throw std::invalid_argument("runtime volume reached unbound actual B29670 domain");
        recreate_native_renderer_device_00b29670(locals.renderer_08, *c.recreation);
        void* const current_renderer = locals.renderer_08;
        void* const device = pointer(word(at(current_renderer, 0x1a10)));
        void* const table = pointer(word(device));
        const auto create = reinterpret_cast<Create>(word(at(table, 0x60)));
        const Word pool = locals.pool_00;
        const Word height = args.height_04;
        const Word width = args.width_00;
        a.last_create_result = create(device, width, height, depth, levels, usage,
            format, pool,
            &a.locals.com_output_04, nullptr);
    }

    void* const raw_slot = allocate_native_volume_texture_00b3f2d0(
        c.owner.actual_volume_pool_0108dba8);
    args.height_04 = reinterpret_cast<Word>(raw_slot);
    state_byte(a, 1);
    void* owner = nullptr;
    if (raw_slot) {
        const Word current_flags = args.flags_14;
        IDirect3DVolumeTexture9* const current_com = locals.com_output_04;
        owner = construct_native_runtime_volume_texture_00b3d720(raw_slot,
            current_com, current_flags, c.actual_shared_serial_0108d6e8, c.owner);
    }
    a.owner = owner; // Diagnostic only; native EDI retains the captured return.
    void* const saved_renderer = locals.renderer_08;
    void* const header = at(saved_renderer, 0x1b00);
    const Word capacity = word(at(header, 8));
    const Word count_before_reserve = word(at(header, 4));
    const bool grow = count_before_reserve == capacity;
    state_byte(a, 0);
    if (grow) {
        const Word doubled = capacity + capacity;
        const std::int32_t requested = signed_bits(doubled) > 1 ? signed_bits(doubled) : 1;
        reserve_native_cube_texture_owner_array_00735ff0(header, requested);
    }
    const Word current_count = word(at(header, 4));
    const Word current_data = word(header);
    const Word destination = current_data + current_count * 4u;
    if (destination != 0) put(pointer(destination), reinterpret_cast<Word>(owner));
    put(at(header, 4), word(at(header, 4)) + 1u);

    const auto exit_mode = c.synchronization.mode_00;
    put(&a.unwind_state, 0xffffffffu);
    if (exit_mode != 0) {
        const Word saved_word = word(&a.locals.guard_0c);
        const void* const saved_guard_renderer = a.locals.guard_0c.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(
            saved_guard_renderer, saved_word, c.synchronization);
    }
    a.phase = Phase::complete;
    return owner;
}
} // namespace bsp
