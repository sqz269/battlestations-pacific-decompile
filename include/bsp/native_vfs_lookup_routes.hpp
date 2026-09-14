#pragma once

#include <cstdint>

namespace bsp {
struct NativePhysicalFileDateContext;
class ActualNativeStringPoolStorage;
struct NativeVfsDeviceRouteContext;
struct NativeVfsPendingRouteContext;

// Bind actual profile storage for the original stack-visitor identities.
// Each reached virtual call rereads the current visitor identity and table word.
// The physical context supplies current publication, actual strings and CRT.
struct NativeVfsLookupRouteContext {
    NativePhysicalFileDateContext& physical;
    const void* actual_exists_profile_00d68398;
    const void* actual_name_probe_profile_00d683e8;
    NativeVfsDeviceRouteContext* device{}; // Optional D683F4 profile/source binding.
    NativeVfsPendingRouteContext* pending{}; // Optional D68478 profile/source binding.
};

// Invoke the already captured original provider+10h target on actual storage.
// Shared by BD90D0 and BF0FB0; includes actual MPAK BB4B20. Unknown entries
// retain the existing explicit invalid_argument source boundary.
std::uint8_t invoke_native_vfs_provider_contains(std::uintptr_t captured_entry,
    void* actual_provider, const void* actual_name, NativeVfsLookupRouteContext&);

// Full five-byte +18 leaves: ECX and stack name unused, RET4, AL0.
// No provider/profile/header read and no mutation, even for unusable arguments.
std::uint8_t probe_native_mpkg_name_00bb8640(void* unused_provider,
    void* unused_name) noexcept;
std::uint8_t probe_native_msar_name_00bba080(void* unused_provider,
    void* unused_name) noexcept;

// Full BD90D0: ECX visitor; stack mount payload/name; RET8; no result.
// Read current payload+8 provider/current table+10, store raw returned AL at+4.
void read_native_vfs_exists_provider_00bd90d0(void* actual_visitor,
    const void* actual_mount_payload, const void* actual_name,
    NativeVfsLookupRouteContext&);
// Full four-byte getters: ECX visitor; RET; AL=current byte+4, not normalized.
std::uint8_t read_native_vfs_exists_result_00bd90f0(const void* actual_visitor) noexcept;
std::uint8_t read_native_vfs_name_probe_result_00bdb5d0(const void* actual_visitor) noexcept;

// Full BDBC00: ECX visitor; stack payload/name; RET8; no result.
// Assign into CURRENT owned string+8, then reread payload+8/current provider+18
// with that mutable header. Store returned raw AL at visitor+4 after the call.
void read_native_vfs_name_probe_provider_00bdbc00(void* actual_visitor,
    const void* actual_mount_payload, const void* actual_name,
    NativeVfsLookupRouteContext&);
// Full BD8FE0: ECX visitor, RET. Separate seven-byte base reset reached by
// BDB5E0's FH3 cleanup CC6010; writes D68380 only. BD90B0 remains separate.
void reset_native_vfs_name_probe_base_00bd8fe0(void* actual_visitor) noexcept;
// Full BDB5E0: ECX visitor, RET. Release current name data+0C using length+8,
// then reset identity; header fields and result byte remain unchanged.
void destroy_native_vfs_name_probe_visitor_00bdb5e0(void* actual_visitor,
    ActualNativeStringPoolStorage&);

// Full BDD0A0 (672 bytes), qualified implementation for D68398/E8 and F4/478
// when the corresponding optional device/pending route binding is supplied.
// ECX captured manager; stack name/visitor; RET8; no specified result.
// Existing actual tree layout/traversal/EH ordering; no tree population.
// Unknown identities/current slots are explicit invalid_argument SOURCE
// boundaries, not original native failures or general visitor reconstruction.
void visit_native_vfs_lookup_mounts_00bdd0a0(void* actual_manager,
    const void* actual_name, void* actual_visitor, NativeVfsLookupRouteContext&);

// Full BDD440: ECX captured manager; stack name; RET4; raw AL result byte.
// Copy then normalize temporary name, construct D68398 visitor, traverse,
// capture result, reset visitor, release temporary. Caller name is unchanged.
std::uint8_t exists_native_vfs_file_00bdd440(void* actual_manager,
    const void* actual_name, NativeVfsLookupRouteContext&);
// Full BDD600: ECX captured manager; stack mutable name; RET4; AL0/1.
// Normalize caller name BEFORE constructing D683E8 visitor. On success assign
// current visitor string to caller; both exits destroy the owned visitor name.
bool replace_native_vfs_file_name_00bdd600(void* actual_manager,
    void* actual_name, NativeVfsLookupRouteContext&);

// Source APIs are not original binary ABI/SEH replacements. Caller supplies
// current native storage and actual pool; no archive/stream owner is invented.
} // namespace bsp
