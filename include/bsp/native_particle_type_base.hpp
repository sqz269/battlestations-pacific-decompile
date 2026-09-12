#pragma once
#include "bsp/native_particle_definition.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace bsp {
// Borrow the actual d3dx9_40 module, kept loaded by the caller. Resolves the
// named library export; no replacement half converter or implicit DLL load.
class NativeD3dx9Float32To16Import final {
public:
    explicit NativeD3dx9Float32To16Import(HMODULE actual_d3dx9_40);
    NativeD3dx9Float32To16Import(const NativeD3dx9Float32To16Import&) = delete;
    NativeD3dx9Float32To16Import& operator=(const NativeD3dx9Float32To16Import&) = delete;
    std::uint16_t* convert(std::uint16_t*, const float*, UINT) const;
private:
    using Function = std::uint16_t* (WINAPI*)(std::uint16_t*, const float*, UINT);
    Function function_;
};

struct NativeParticleTypeBaseBindings {
    NativeParticleDefinitionBindings& owners; // SAME strings/F8D344/native free domains
    const volatile std::uint32_t* base_profile_00d5ddc0; // actual 11-DWORD table
    void* (__cdecl* allocate_array_00bf55be)(std::size_t);
    const NativeD3dx9Float32To16Import& half_import;
    const volatile std::uint32_t* one_00d7a24c;
    const volatile std::uint32_t* scalar_00ce3804;
    const volatile std::uint32_t* record_scalar_00ce6650;
    // B01150 copies one never-initialized native stack DWORD into record+18.
    // Caller explicitly supplies that incoming machine residue for exact
    // replay. It has no established default or semantic meaning.
    std::uint32_t initial_record_stack_word18;
};

// Complete native bodies, new C++ interfaces. These operate on actual raw
// PARTICLE definitions; the AFA280 emitter-definition type is distinct.
// B01150 ECX actual owner, stack(name8h,word14,parent18), RET0C/EAX owner.
// Initializes sparse fields in its 80h base and appends one 1Ch record to the
// actual 0Ch {pointer,count,capacity} descriptor at +68. Untouched bytes stay.
// Original FH3/SEH ABI and arbitrary invalid-pointer faults are not emulated.
void* construct_native_particle_type_base_00b01150(void* actual_owner,
    const void* actual_name, std::uint32_t word14, void* actual_parent,
    NativeParticleTypeBaseBindings&);
// B00C20 ECX actual descriptor, stack(signed requested capacity), RET4.
// Signed comparisons and wrapping 1Ch allocation/address arithmetic retained.
void reserve_native_particle_type_records_00b00c20(void* actual_descriptor,
    std::int32_t capacity, NativeParticleTypeBaseBindings&);
// B00FB0 ECX actual owner, RET. Releases ten actual parameter slots in native
// order, clears only +48, destroys +68 descriptor/string and restores CEB130.
void destroy_native_particle_type_base_00b00fb0(void*, NativeParticleTypeBaseBindings&);
// B01130 ECX owner, stack(flags), RET4/EAX captured owner; free iff flags&1.
void* delete_native_particle_type_base_00b01130(void*, std::uint32_t flags,
    NativeParticleTypeBaseBindings&);
// B00920..B00940: ECX owner, RET/EAX new+54. Signed sequential minima of
// count-1, old+54, then old+50. Empty count produces -1; no lower clamp.
std::int32_t clamp_native_particle_type_record_range_00b00920(void*) noexcept;
// B00740..B00742: RET4, ignores ECX and its sole stack word; no EAX contract.
void native_particle_type_noop_00b00740(void*, std::uint32_t) noexcept;
} // namespace bsp
