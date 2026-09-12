#pragma once

namespace bsp {
class ActualNativeStringPoolStorage;

// Complete actual-storage bodies; native ECX owner/payload, no stack arguments,
// RET0, no semantic result. These new C++ interfaces do not reproduce native
// SEH/exception ABI. See docs/NATIVE_VFS_REQUEST_LIST_LIFETIME.md.
// Lists preserve owner+0, hold their actual sentinel at+4 and count at+8.
// Every nonnull string return repeats the canonical actual pool getter.
// Its inherited noexcept release excludes native throwing-getter identity.

// 004D2640..004D265C: existing actual4D05E0 clear, free current sentinel,
// then null owner+4. Requires a valid sentinel, even for an empty list.
void destroy_native_vfs_string_list_004d2640(
    void* actual_list_owner, ActualNativeStringPoolStorage&);

// 00BE1220..00BE12B9: actual20h payload produced by00BE10A0/00BE13E0.
// Destroy string lists at+14 then+8, followed by its base8h string. Preserve
// string header and list owner+0 words. Do not free the payload itself.
// Native states: 1 cleans+8 then base; 0 cleans base; -1 cleans neither.
void destroy_native_vfs_request_payload_00be1220(
    void* actual_payload, ActualNativeStringPoolStorage&);

// 00BE19E0..00BE1A1D: detach/self-link sentinel and zero count before walking
// captured nodes. Each actual28h node holds next/previous then the20h payload.
// Capture next before destroying payload+8 and freeing the node; compare next
// against the current owner sentinel after calls. Retain the sentinel.
void clear_native_vfs_request_list_00be19e0(
    void* actual_list_owner, ActualNativeStringPoolStorage&);

// 00BE1D60..00BE1D7C: clear request list, free current sentinel, null owner+4.
void destroy_native_vfs_request_list_00be1d60(
    void* actual_list_owner, ActualNativeStringPoolStorage&);
} // namespace bsp
