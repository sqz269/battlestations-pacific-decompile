#include "bsp/native_mpkg_runtime.hpp"
#include "bsp/native_vfs_runtime_bindings.hpp"
#include "bsp/native_path_canonicalizer.hpp"
#include "bsp/native_retained_memory_owners.hpp"
#include "bsp/oct18_tail_c.hpp"
#include "bsp/native_filestore_open.hpp"
#include "bsp/native_raw_inflate_owner.hpp"
#include "bsp/native_stream_copy_range.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <cstring>
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
void* NativeMpkgArchiveRuntimeOperations::open_named(void* archive,
    const void* name,std::uint32_t flags) {
    oct18_tail_c::MpkgBindings bindings{};
    bindings.allocate_header_00bf55be = [this](std::uint32_t bytes) {
        return context_.directory.services.allocate_00bf55be(bytes);
    };
    bindings.free_header_00bf65ac = [this](void* bytes) {
        context_.directory.services.free_00bf65ac(bytes);
    };
    bindings.allocate_owner_00bf681b = [](std::uint32_t bytes) {
        return singleton_lifetime_allocate({SingletonAllocationKind::object,bytes,bytes});
    };
    bindings.inflate_00bbc1d0 = [this](void* owner,void* source,
        const std::uint32_t* descriptor,std::uint32_t input,std::uint32_t output) {
        try {
            return construct_native_raw_inflate_owner_00bbc1d0(owner,source,
                descriptor,input,output,context_.directory.streams);
        } catch (...) {
            // BB8C33..BB8C68 state0 / DFE370 -> CC4780: failure frees
            // only the raw34h allocation, after its own nested unwind.
            singleton_lifetime_free(owner);
            throw;
        }
    };
    bindings.memory_00bef750 = [this](void* stream) {
        return convert_native_stored_stream_00bef750(stream,context_.conversion);
    };
    bindings.copy_range_00bef840 = [this](void* source,std::uint32_t low,
        std::uint32_t high,std::uint32_t length_low,std::uint32_t length_high) {
        return copy_native_stream_range_00bef840(source,low,high,length_low,
            length_high,context_.conversion.memory_owners,context_.directory.streams);
    };
    bindings.adopted_00bf1130 = [this](void* owner,void* source,
        std::uint32_t low,std::uint32_t high,std::uint32_t length_low,std::uint32_t length_high) {
        try {
            return construct_native_adopted_substream_00bf1130(owner,source,
                low,high,length_low,length_high,context_.directory.streams);
        } catch (...) {
            // BB8D06..BB8D21 state2 / DFE380 -> CC478B: release raw28h only.
            // The reopened stream is deliberately not retired here.
            singleton_lifetime_free(owner);
            throw;
        }
    };
    bindings.stricmp_00bf7fbf = &_stricmp;
    bindings.native_context = &context_;
    return oct18_tail_c::mpkg_open_named_00bb8d60(archive,name,flags,
        context_.actual_manager_publication_0109ceec,bindings);
}
} // namespace bsp
