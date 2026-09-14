#pragma once

#include <cstdint>

namespace bsp {
struct NativePhysicalFileDateContext;
class ActualNativeStringPoolStorage;
class NativeVfsRuntimeBindings;

// Borrow initialized actual storage. Without an explicit native binding,
// provider/stream tables contain CALLABLE original-ABI functions. The manager
// failure field+90 is also dispatched by the explicit native binding; without
// that binding it retains its required callable original-ABI contract.
// This context creates no provider, stream, table, manager or pool owner.
// The visitor profile is the actual three-word D6838C identity table, whose
// numeric +04/+08 targets select the reconstructed BDA690/BD9040 bodies.
using NativeVfsOpenedResourceLog = void (__fastcall*)(void* manager,
    void* unused_edx, const void* name, void* stream, std::uint32_t mount_byte);
struct NativeVfsOpenRouteContext {
    NativePhysicalFileDateContext& physical;
    const void* actual_open_profile_00d6838c;
    NativeVfsOpenedResourceLog log_opened_resource_00bde9c0;
    // Optional explicit native method composition. Constructor/destructor of
    // the binding install/restore this source context pointer, never owner data.
    NativeVfsRuntimeBindings* native_bindings=nullptr;
};

// Complete BDCA80: ECX manager, stack mutable raw name header, RET4.
// Current alias pointer+94/count+98; 16-byte pairs; first equal-length
// case-insensitive match only. Copy current replacement+8; no renormalization.
void apply_native_vfs_first_alias_00bdca80(void* actual_manager,
    void* actual_name, ActualNativeStringPoolStorage&);

// Complete BDA690 consumer: ECX visitor; stack payload/name; RET8.
// Current provider+08 receives name and full flags. Publish stream+04, then
// current payload byte+0C only when nonnull. Verified 4254B0 RET diagnostics.
void open_native_vfs_mount_00bda690(void* actual_visitor,
    const void* actual_mount_payload, const void* actual_name,
    NativeVfsOpenRouteContext&);
// Complete nine-byte raw leaf: ECX visitor, RET, EAX 0/1 from stream DWORD+4.
std::uint32_t has_native_vfs_open_stream_00bd9040(const void* actual_visitor) noexcept;

// Complete BDD0A0 control flow, qualified for the D6838C open visitor.
// Current visitor identity and table slots are read on every dispatch.
// Other identities/slots are explicit source boundaries, not native failures.
void visit_native_vfs_open_mounts_00bdd0a0(void* actual_manager,
    const void* actual_name, void* actual_visitor, NativeVfsOpenRouteContext&);

// Complete BDF310 CONSUMER control flow: ECX captured manager, name/flags
// stack, RET8, EAX stream. Normalize copied name, apply first alias, visit;
// handle failure and counters, then release the temporary. Flags are retained.
// Required external BDE9C0 and callable provider/stream/failure services remain
// unresolved owners. No retain, release, cache, buffering or fallback is added.
// New source API, not an original binary ABI/SEH replacement or game validation.
void* open_native_vfs_resource_00bdf310(void* actual_manager,
    const void* actual_name, std::uint32_t flags, NativeVfsOpenRouteContext&);
} // namespace bsp
