#include "bsp/native_mpkg_runtime.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include "bsp/native_path_canonicalizer.hpp"
#include "bsp/native_retained_memory_owners.hpp"

#include <stdexcept>

namespace bsp {
NativeMpkgRuntimeServices::NativeMpkgRuntimeServices(NativeVfsRuntimeBindings& vfs,
    NativePathCanonicalizerServices& pool,NativeRetainedMemoryOwnerContext& memory) noexcept
    :vfs_(vfs),pool_(pool),memory_(memory) {}
void* NativeMpkgRuntimeServices::open_manager_00bb99a0(std::uintptr_t entry,void* manager,
    const void* name,std::uint32_t flags) {
    return vfs_.open_manager_entry(entry,manager,name,flags);
}
void NativeMpkgRuntimeServices::delete_stream_00bb9aae(std::uintptr_t entry,void* stream,
    std::uint32_t flags) {
    if(entry!=0x00bb8f90)throw std::invalid_argument("Unimplemented MPKG converted-stream deletion");
    (void)delete_native_memory_stream_00bb8f90(stream,flags,memory_);
}
std::uint64_t NativeMpkgRuntimeServices::source_length(std::uintptr_t entry,void* stream) {
    return vfs_.stream_length_entry(entry,stream);
}
void* NativeMpkgRuntimeServices::allocate_00bf55be(std::uint32_t bytes) {
    return pool_.allocate_scratch_00bf55be(bytes);
}
void NativeMpkgRuntimeServices::free_00bf65ac(void* pointer) {
    pool_.free_scratch_00bf65ac(pointer);
}
NativeStringPoolStorage* NativeMpkgRuntimeServices::string_pool_00419cc0() {
    return pool_.string_pool_00419cc0();
}
void NativeMpkgRuntimeServices::return_string_00bd1510(NativeStringPoolStorage* pool,
    void* pointer,std::uint32_t bytes,std::uint32_t one) {
    pool_.return_string_00bd1510(pool,pointer,bytes,one);
}
NativeMpkgArchiveRuntimeOperations::NativeMpkgArchiveRuntimeOperations(NativeMpkgArchiveContext& context) noexcept
    :context_(context) {}
void* NativeMpkgArchiveRuntimeOperations::construct_00bb9920(void* archive,const void* name) {
    return construct_native_mpkg_archive_00bb9920(archive,name,context_);
}
void NativeMpkgArchiveRuntimeOperations::destroy_00bb9c10(void* archive) {
    destroy_native_mpkg_archive_00bb9c10(archive,context_);
}
} // namespace bsp
