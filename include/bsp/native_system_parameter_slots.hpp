#pragma once

#include <cstdint>

namespace bsp {

// Complete native bodies, exposed through new Win32 C++ interfaces. Borrow the
// actual sixteen-word global domain at 0108FC30 and actual raw owner storage;
// no owner or bank is constructed here. Arguments point to the current native
// DWORD argument slots, not a float snapshot. The divisor/one pointers borrow
// the current eight/four bytes at CE47A0/D7A24C (normally double1000/float1).
// The pointer values remain stable during a call; their pointed-to storage may
// overlap. All resulting addresses must be valid in the native access domain.
// x87 environment, binary32 spills, MOVSS bit copies and alias order survive.
// Original-caller ABI, unrestricted fault recovery and gameplay are unproved.

// 00B40860..00B4087F, 32 bytes. Native ECX index, EDX source; no stack args,
// RET, no semantic result. DWORD (index<<4)+base wraps; there is no index guard.
// Source lane0 is loaded before destination calculation; four forward x87
// FLD/FSTP pairs can convert NaNs/denormals and cascade through overlapping data.
void publish_native_system_parameter_slot_00b40860(
    void* parameters_0108fc30, std::uint32_t index, const void* source) noexcept;

// 00B9A030..00B9A078, 73 bytes. Native ECX owner, four DWORD stack slots,
// RET10h. Owner+3BC: argument0/current divisor, arguments1..3; publish slot0.
// The quotient is explicitly spilled to binary32 and reloaded before +3BC.
void set_native_system_parameter_slot0_00b9a030(
    void* parameters_0108fc30, void* owner, const void* argument_slots,
    const void* divisor_00ce47a0) noexcept;

// 00B9A080..00B9A0C8, 73 bytes. Native ECX owner, three DWORD stack slots,
// RET0Ch. Owner+5D0: arguments0..2, current one bits; publish slot2.
void set_native_system_parameter_slot2_00b9a080(
    void* parameters_0108fc30, void* owner, const void* argument_slots,
    const void* one_00d7a24c) noexcept;

// 00B9A0D0..00B9A126, 87 bytes. Native ECX owner, three DWORD stack slots,
// RET0Ch. Owner+5E0: arguments0..2, current one bits; store x87 1/argument0
// separately at owner+5F0 before publishing slot3. +5F0 is not published.
void set_native_system_parameter_slot3_00b9a0d0(
    void* parameters_0108fc30, void* owner, const void* argument_slots,
    const void* one_00d7a24c) noexcept;

// 00B9A130..00B9A17A, 75 bytes. Native ECX owner, two DWORD stack slots,
// RET8. Owner+3CC: argument0/current divisor, argument1, positive zero,
// current one bits; binary32 quotient spill/reload; publish slot1.
void set_native_system_parameter_slot1_00b9a130(
    void* parameters_0108fc30, void* owner, const void* argument_slots,
    const void* divisor_00ce47a0, const void* one_00d7a24c) noexcept;

} // namespace bsp
