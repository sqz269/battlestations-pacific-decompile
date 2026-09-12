#pragma once

#include "bsp/native_mpkg_archive.hpp"
#include "bsp/native_mpkg_directory.hpp"
#include "bsp/native_mpkg_provider.hpp"

namespace bsp {
class NativeVfsRuntimeBindings;
class NativePathCanonicalizerServices;
struct NativeRetainedMemoryOwnerContext;

// Concrete source calls for the original archive/directory boundaries. Borrow
// existing runtime, canonical pool services and actual memory-owner counters.
// No stream, publication, shadow lifetime or successful fallback is created.
class NativeMpkgRuntimeServices final : public NativeMpkgArchiveServices,
    public NativeMpkgDirectoryServices {
public:
    NativeMpkgRuntimeServices(NativeVfsRuntimeBindings&,
        NativePathCanonicalizerServices&,NativeRetainedMemoryOwnerContext&) noexcept;
    void* open_manager_00bb99a0(std::uintptr_t,void*,const void*,std::uint32_t) override;
    void delete_stream_00bb9aae(std::uintptr_t,void*,std::uint32_t) override;
    std::uint64_t source_length(std::uintptr_t,void*) override;
    void* allocate_00bf55be(std::uint32_t) override;
    void free_00bf65ac(void*) override;
    NativeStringPoolStorage* string_pool_00419cc0() override;
    void return_string_00bd1510(NativeStringPoolStorage*,void*,std::uint32_t,std::uint32_t) override;
private:
    NativeVfsRuntimeBindings& vfs_;
    NativePathCanonicalizerServices& pool_;
    NativeRetainedMemoryOwnerContext& memory_;
};

class NativeMpkgArchiveRuntimeOperations final : public NativeMpkgArchiveOperations {
public:
    explicit NativeMpkgArchiveRuntimeOperations(NativeMpkgArchiveContext&) noexcept;
    void* construct_00bb9920(void*,const void*) override;
    void destroy_00bb9c10(void*) override;
private:
    NativeMpkgArchiveContext& context_;
};
} // namespace bsp
