#pragma once

#include <cstdint>

namespace bsp {
class ActualNativeStringPoolStorage;
struct SingletonLifetimeCallbacks;

// Full BE5C00 and BE5C40: ECX FileStore, stack actual name header, RET4,
// AL Boolean. Both perform a contains query on the primary tree at +14.
// Capture the current head AFTER find but BEFORE returning owner validation;
// compare the current returned node with that captured head. No normalization.
bool contains_native_file_store_file_00be5c00(void* actual_store,
    const void* actual_name_header, const SingletonLifetimeCallbacks&);
// Provider +18 has the same contains behavior: it does not rewrite the name.
bool probe_native_file_store_name_00be5c40(void* actual_store,
    const void* actual_name_header, const SingletonLifetimeCallbacks&);

// Full 80-byte +1C leaves; native ECX unused, stack output/name, RET8,
// EAX output. Clear output before the identity branch, abandoning old storage;
// copy current fields through the actual owning pool. No path transformation,
// provider/root read, or initial-copy cleanup. Null header is not checked.
void* copy_native_file_store_name_00be6040(void* actual_output_header,
    const void* actual_name_header, ActualNativeStringPoolStorage&);
void* copy_native_package_name_00bb5540(void* actual_output_header,
    const void* actual_name_header, ActualNativeStringPoolStorage&);

// Full BB8E00: ECX actual 34h archive state, stack name, RET4, AL Boolean.
// Current array+28 and signed count+2C, 24h rows with name length/data at +0/+4.
// Scan in existing order, equal recorded lengths then current CRT _stricmp.
// Reload array/count/headers as native; no parser, open/read or row allocation.
bool contains_native_mpkg_entry_00bb8e00(const void* actual_archive_state,
    const void* actual_name_header);
// Full 8-byte BB8E80 tail thunk: ECX provider; stack name; inherited RET4.
// Load the actual archive STATE POINTER at provider+14; it is not an inline state.
bool contains_native_mpkg_file_00bb8e80(const void* actual_provider,
    const void* actual_name_header);

// Full BBA650: ECX MSAR provider, stack name, RET4, EAX index or FFFFFFFF.
// Inline begin/end at +1C/+20, 18h rows, name at +0/+4. Count uses signed
// DWORD byte-difference /24, then unsigned index comparison. Repeat current
// bounds for CRT validation, then reread begin before reading a row. A returning
// handler continues the native lookup. No capacity check or population policy.
std::uint32_t find_native_msar_file_index_00bba650(const void* actual_provider,
    const void* actual_name_header, const SingletonLifetimeCallbacks&);
// Full 23-byte BBA710: ECX provider, stack name, RET4, AL index!=FFFFFFFF.
bool contains_native_msar_file_00bba710(const void* actual_provider,
    const void* actual_name_header, const SingletonLifetimeCallbacks&);

// Caller supplies original-compatible current storage. These C++ interfaces
// do not implement archive construction, VFS traversal, original binary ABI,
// native SEH, or startup reachability of the latent MSAR provider.
} // namespace bsp
