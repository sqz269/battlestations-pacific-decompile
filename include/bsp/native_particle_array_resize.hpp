#pragma once
#include "bsp/native_particle_model_construction.hpp"
#include <cstdint>

namespace bsp {
// Complete AFCD90..AFCD95 and AFDAC0..AFDAE0. Native ECX element, EAX same
// address, RET. Names are hypotheses. The record is the actual108h allocation;
// only A4/A0/B4 are written, in that order. D7A24C is captured before writes,
// as raw MOVSS bits, and every other byte remains untouched.
void* construct_native_particle_array_byte_00afcd90(void*) noexcept;
void* construct_native_particle_record_00afdac0(
    void*, const volatile std::uint32_t& one_00d7a24c) noexcept;

// Complete AFD130..AFD1CE / AFD220..AFD2D4 for actual valid cookie-backed
// storage. Native ECX8h header, stack requested count, RET4. Destructively
// replace even for equal/zero count; there is no prefix copy. Free the old
// cookie allocation before attempting the new one, keeping the old header
// unchanged if allocation throws. Allocation bytes saturate at FFFFFFFF.
// Zero count still requests a4h cookie. Existing cookies must have signed
// nonnegative counts and valid allocation extents. This is a new source ABI;
// it reuses the canonical CRT allocation domain and concrete element lifetime.
void resize_native_particle_model_byte_array_00afd130(
    NativeParticleArrayStorage&, std::int32_t count);
void resize_native_particle_model_record_array_00afd220(
    NativeParticleArrayStorage&, std::int32_t count,
    const volatile std::uint32_t& one_00d7a24c);
} // namespace bsp
