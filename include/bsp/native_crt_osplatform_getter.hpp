#pragma once

#include <cstdint>

namespace bsp {
struct NativeCrtPointerDecodeSupportContext;

// Full BFBAB2[55], native cdecl output pointer, EAX status, plain RET.
// This additional-context source interface captures output first. Null output
// skips the platform read. Otherwise load current actual109DD84 once, test it,
// and on success store that same captured value to output and return0.
// Failure uses the existing owning-CRT BFFB8B errno location, writes22, then
// invokes the established BF66EF all-five-zero invalid-parameter service.
// If that service returns, return22 without storing output; no catch or retry.
std::int32_t get_native_crt_osplatform_00bfbab2(
    std::uint32_t* output, const NativeCrtPointerDecodeSupportContext& context);

// Reuses the existing stable borrowed context, not a new allocator/TLS/global
// owner or arbitrary replacement callback. The actual OS word, owning PTD/error
// service and current invalid-handler bindings must already be established.
// winmajor is unused. Services retain their actual side effects/exceptions.
// Native outgoing five-word stack layout, private ABI, registers/EFLAGS,
// asynchronous faults, Watson/termination and gameplay identity are unclaimed.
} // namespace bsp
