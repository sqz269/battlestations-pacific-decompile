#pragma once
#include "bsp/native_lua_vfs_dispatch.hpp"
#include "bsp/native_adopted_substream.hpp"
namespace bsp {
struct NativeVfsOpenRouteContext;
struct NativeVfsLookupRouteContext;
struct NativePhysicalStreamOpenContext;
struct NativeStoredStreamConversionContext;
struct NativeVfsOpenLoggingContext;
struct NativeRetainedMemoryOwnerContext;
// Concrete source dispatch for verified native manager/provider/stream methods.
// Owners retain their original numeric vtable words. The corresponding native
// table bytes must be readable at those addresses, as required by the existing
// physical and mount-lookup consumers; no original code is called through them.
// Supports D685B4 manager, D69168 physical and D689E8 FileStore providers,
// D691B0 physical, D642C0 memory and D68DB0 adopted-source streams. Other profiles/slots are
// explicit source boundaries. This does not construct the manager/mount tree.
class NativeVfsRuntimeBindings final : public NativeLuaVfsDispatch,
    public NativeAdoptedSubstreamDispatch {
public:
    NativeVfsRuntimeBindings(NativeVfsOpenRouteContext&,NativeVfsLookupRouteContext&,
        NativePhysicalStreamOpenContext&,NativeStoredStreamConversionContext&,
        NativeVfsOpenLoggingContext&,NativeRetainedMemoryOwnerContext&);
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
    std::uint32_t stream_size_low(void* stream,std::uint32_t argument);
    void log_open(void* manager,const void* name,void* stream,std::uint32_t mount_byte);
private:
    NativeVfsOpenRouteContext& route_;
    NativeVfsLookupRouteContext& lookup_;
    NativePhysicalStreamOpenContext& physical_;
    NativeStoredStreamConversionContext& store_;
    NativeVfsOpenLoggingContext& logging_;
    NativeRetainedMemoryOwnerContext& memory_;
    NativeVfsRuntimeBindings* previous_;
    NativeAdoptedSubstreamDispatch* previous_substreams_;
};
} // namespace bsp
