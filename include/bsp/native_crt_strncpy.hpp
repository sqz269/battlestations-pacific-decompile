#pragma once

namespace bsp {

// Whole Original _strncpy at 00BF9280..00BF93A3, ordinary raw-storage domain.
// New potentially throwing interface; no physical Original ABI/home identity.
// count0 returns actual_destination without payload access or cursor advances.
// Positive calls require the writable count-byte destination region and every
// reached BYTE/full DWORD source window; actual overlap and later self-induced
// source changes are admitted. Every terminal advance must remain nonwrapping
// within/one-past its same allocation. No unused source NUL/padding promise.
void* __cdecl copy_native_crt_padded_bytes_00bf9280(
    void* actual_destination, const void* actual_source, unsigned int count);

} // namespace bsp
