#pragma once
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
struct TrackedCriticalSection;

// Actual 0Ch headers. Entries are weak raw pointers, without reference changes.
// No initialization hides preimages or the stale backing pointer after cleanup.
struct NativeParticleManagerPointerArray {
    void** volatile data_00;
    volatile std::uint32_t count_04;
    volatile std::uint32_t capacity_08;
};
// The SAME 34h manager published at F8C274 by AF06A0, historically named
// FoliageGroupManager. AF0B10 preserves +1C/+20 and writes +30 only on success.
struct NativeParticleModelManagerStorage {
    volatile std::uint32_t native_table_00;
    NativeParticleManagerPointerArray models_04;
    NativeParticleManagerPointerArray entries_10;
    std::byte untouched_1c[8];
    volatile std::uint32_t word_24;
    volatile std::uint32_t word_28;
    volatile std::uint32_t scalar_bits_2c;
    TrackedCriticalSection* section_30;
};
struct NativeParticleModelManagerAccess {
    NativeParticleModelManagerStorage* volatile& manager_00f8c274;
    SingletonLifetimeDomain& lifetime_01090aa0;
    const volatile std::uint32_t& one_00d7a24c;
};

// Complete AF0630..068E, AF07E0..0818, AF0900..094F, AF0A60..0AC6,
// AF0AF0..0B06. Native ECX header, one stack argument/RET4 except cleanup RET.
// These C++ interfaces retain the actual header/storage and wrapped DWORD math.
void reserve_native_particle_manager_pointers_00af0630(NativeParticleManagerPointerArray&, std::int32_t);
void append_native_particle_manager_pointer_00af07e0(NativeParticleManagerPointerArray&, void* const*);
void resize_native_particle_manager_pointers_00af0900(NativeParticleManagerPointerArray&, std::int32_t);
std::uint8_t remove_native_particle_manager_pointer_00af0a60(NativeParticleManagerPointerArray&, void* const*) noexcept;
void destroy_native_particle_manager_pointers_00af0af0(NativeParticleManagerPointerArray&) noexcept;

// AF0950 / AF0AE0: native ECX actual manager, stack model value, RET4.
// Removal returns AL and swaps the last entry over the first match. No lock,
// retain, release, deduplication, substitute registry, or backing-pointer clear.
void register_native_particle_model_00af0950(void* actual_manager, void* actual_model);
std::uint8_t unregister_native_particle_model_00af0ae0(void* actual_manager, void* actual_model) noexcept;

// Full base AF06A0..0730 / AF0740..07D8 and derived AF0B10..0B81 /
// AF0B90..0C40. Native ECX manager, RET, constructor EAX original pointer.
// Added access borrows the application's SAME publication/lifetime domain.
NativeParticleModelManagerStorage* construct_native_particle_manager_base_00af06a0(NativeParticleModelManagerStorage*, NativeParticleModelManagerAccess&);
void destroy_native_particle_manager_base_00af0740(NativeParticleModelManagerStorage*, NativeParticleModelManagerAccess&);
NativeParticleModelManagerStorage* construct_native_particle_model_manager_00af0b10(NativeParticleModelManagerStorage*, NativeParticleModelManagerAccess&);
void destroy_native_particle_model_manager_00af0b90(NativeParticleModelManagerStorage*, NativeParticleModelManagerAccess&);
// Full scalar destructors AF0870..088D / AF1080..109D: ECX manager,
// stack flags, RET4/EAX original; free iff flags bit0 after destruction.
NativeParticleModelManagerStorage* delete_native_particle_manager_base_00af0870(NativeParticleModelManagerStorage*, std::uint32_t, NativeParticleModelManagerAccess&);
NativeParticleModelManagerStorage* delete_native_particle_model_manager_00af1080(NativeParticleModelManagerStorage*, std::uint32_t, NativeParticleModelManagerAccess&);
// New C++ interfaces; original ABI, FH3/SEH and gameplay are not established.
} // namespace bsp
