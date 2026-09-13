#pragma once

#include "bsp/native_render_resource_container_removal.hpp"
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {
class NativeStringStorage;
struct SingletonLifetimeCallbacks;

// Borrowed application domains. The six-argument constructor preserves the
// semantic pool interface; the five-argument constructor binds the SAME
// ActualNativeStringPoolStorage for both temporary names and record removal.
// No renderer, string, guard, resource or lifetime owner is created here.
struct NativeRendererTextureNameNotificationContext {
    NativeRendererTextureNameNotificationContext(const void* volatile&,
        NativeRendererSynchronizationGlobals&, NativeStringStorage&, SizedStoragePool&,
        const SingletonLifetimeCallbacks&, const NativeRenderResourceAccountingTables&) noexcept;
    NativeRendererTextureNameNotificationContext(const void* volatile&,
        NativeRendererSynchronizationGlobals&, ActualNativeStringPoolStorage&,
        const SingletonLifetimeCallbacks&, const NativeRenderResourceAccountingTables&) noexcept;
    const void* volatile& actual_renderer_00f8d394;
    NativeRendererSynchronizationGlobals& synchronization;
    NativeStringStorage& actual_string_storage;
    SizedStoragePool* actual_string_pool;
    const SingletonLifetimeCallbacks& callbacks;
    const NativeRenderResourceAccountingTables& accounting_tables;
    ActualNativeStringPoolStorage* actual_native_string_pool;
};

// Complete 00B32250..00B32340. Native ECX receiver, stack original eight-byte
// name header, RET4; EAX has no semantic result. Capture the receiver separately
// from the optional guard's current global renderer. The nested removal receives
// the original name after a real temporary name is copied and lowercased.
//
// The existing initialized-guard domain applies: cleanup must not become enabled
// after entry was skipped. Nested removal retains its supported current texture
// profile/accounting-table requirements. Native storage must remain readable at
// its observed accesses. This is a new C++ interface, not a native ABI entry.
void notify_native_renderer_texture_name_removal_00b32250(
    void* actual_receiver, const void* actual_name_header,
    NativeRendererTextureNameNotificationContext&);

} // namespace bsp
