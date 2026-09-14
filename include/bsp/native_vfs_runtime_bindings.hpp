#pragma once
#include "bsp/native_lua_vfs_dispatch.hpp"
#include "bsp/native_adopted_substream.hpp"
#include "bsp/native_vfs_factory_selection.hpp"
#include "bsp/native_vfs_manager_lifetime.hpp"
#include "bsp/native_vfs_pending_routes.hpp"
namespace bsp {
struct NativeVfsOpenRouteContext;
struct NativeVfsLookupRouteContext;
struct NativePhysicalStreamOpenContext;
struct NativeStoredStreamConversionContext;
struct NativeVfsOpenLoggingContext;
struct NativeRetainedMemoryOwnerContext;
struct NativePhysicalProviderContext;
struct NativeFileAccessLogLifetimeBindings;
struct NativeMpkgProviderContext;
struct NativeMpakRuntimeContext;
struct NativePhysicalPendingIoContext;
// Concrete source dispatch for verified native manager/provider/stream methods.
// Owners retain their original numeric vtable words. The corresponding native
// table bytes must be readable at those addresses, as required by the existing
// physical and mount-lookup consumers; no original code is called through them.
// Supports D685B4/D68D04 managers, D69168 physical and D689E8 FileStore providers,
// D641F8 MPAK providers, and D691B0 physical, D642C0 memory, D68DB0 adopted-source
// and D64400 raw inflater streams. Other profiles/slots are
// explicit source boundaries. Factory creation and manager destruction reuse
// the actual provider owners. The optional borrowed contexts are required for
// physical factory/deletion and logger deletion respectively. No private pool,
// publication, callback or lifetime domain is allocated by this binding.
class NativeVfsRuntimeBindings final : public NativeLuaVfsDispatch,
    public NativeAdoptedSubstreamDispatch, public NativeVfsFactoryCreateDispatch,
    public NativeVfsManagerVirtualCalls, public NativeVfsProviderPendingDispatch {
public:
    NativeVfsRuntimeBindings(NativeVfsOpenRouteContext&,NativeVfsLookupRouteContext&,
        NativePhysicalStreamOpenContext&,NativeStoredStreamConversionContext&,
        NativeVfsOpenLoggingContext&,NativeRetainedMemoryOwnerContext&,
        NativePhysicalProviderContext* = nullptr,
        NativeFileAccessLogLifetimeBindings* = nullptr,
        NativePhysicalPendingIoContext* = nullptr);
    ~NativeVfsRuntimeBindings() override;
    NativeVfsRuntimeBindings(const NativeVfsRuntimeBindings&)=delete;
    NativeVfsRuntimeBindings& operator=(const NativeVfsRuntimeBindings&)=delete;
    void* open(std::uintptr_t,void*,const NativeString&,std::uint32_t) override;
    std::uint8_t exists(std::uintptr_t,void*,NativeString&) override;
    std::uint8_t is_open(std::uintptr_t,void*) override;
    std::uint64_t length(std::uintptr_t,void*) override;
    void read(std::uintptr_t,void*,void*,std::uint32_t,std::uint32_t*) override;
    void zero_reference(std::uintptr_t,void*) override;
    std::uint8_t source_is_open(std::uintptr_t,void*) override;
    std::uint32_t source_seek(std::uintptr_t,void*,std::uint32_t,std::uint32_t,std::uint32_t) override;
    void source_read(std::uintptr_t,void*,void*,std::uint32_t,std::uint32_t*) override;
    // Numeric memory/physical write methods remain outside this binding;
    // the complete substream forwarding body accepts a separate write dispatcher.
    void source_write(std::uintptr_t,void*,const void*,std::uint32_t,std::uint32_t*) override;
    void source_zero_reference(std::uintptr_t,void*,std::uintptr_t) override;
    void* provider_open(void* provider,const void* name,std::uint32_t flags);
    // Consume already captured entries without re-reading the owner's table.
    void* open_manager_entry(std::uintptr_t entry,void* manager,
        const void* actual_name,std::uint32_t flags);
    // BDF432 consumes the captured manager+90 code identity. Only the
    // reconstructed startup no-op target is admitted; never execute an identity.
    void open_failure_entry(std::uintptr_t entry,void* captured_manager);
    std::uint8_t invoke_submit(std::uintptr_t captured_entry, void* actual_provider,
        const void* first_header, const void* second_header,
        std::uintptr_t opaque_callback, std::uint32_t flags) override;
    void invoke_tick(std::uintptr_t captured_entry, void* actual_provider) override;
    std::uint64_t stream_length_entry(std::uintptr_t entry,void* stream);
    std::uint64_t stream_position_entry(std::uintptr_t entry,void* stream);
    // Explicit borrowed connection permits constructing the actual recursive
    // manager/archive service graph. Return the previous binding for disposal.
    NativeMpkgProviderContext* bind_mpkg_provider(NativeMpkgProviderContext*) noexcept;
    NativeMpakRuntimeContext* bind_mpak_provider(NativeMpakRuntimeContext*) noexcept;
    void restore_mpak_provider(NativeMpakRuntimeContext* installed,
        NativeMpakRuntimeContext* previous) noexcept;
    std::uint32_t stream_size_low(void* stream,std::uint32_t argument);
    void log_open(void* manager,const void* name,void* stream,std::uint32_t mount_byte);
    void* factory_create(std::uintptr_t captured_entry, void* actual_factory,
        const void* system_header, const void* virtual_header) override;
    void invoke_log_virtual0_00be1fa1(std::uint32_t captured_target,
        void* captured_owner, std::uint32_t flags) override;
    void invoke_provider_virtual4_00be1ffd(std::uint32_t captured_target,
        void* captured_owner, std::uint32_t flags) override;
private:
    NativeVfsOpenRouteContext& route_;
    NativeVfsLookupRouteContext& lookup_;
    NativePhysicalStreamOpenContext& physical_;
    NativeStoredStreamConversionContext& store_;
    NativeVfsOpenLoggingContext& logging_;
    NativeRetainedMemoryOwnerContext& memory_;
    NativePhysicalProviderContext* provider_;
    NativeFileAccessLogLifetimeBindings* log_lifetime_;
    NativePhysicalPendingIoContext* pending_;
    NativeMpkgProviderContext* mpkg_{};
    NativeMpakRuntimeContext* mpak_{};
    NativeVfsRuntimeBindings* previous_;
    NativeAdoptedSubstreamDispatch* previous_substreams_;
};
} // namespace bsp
