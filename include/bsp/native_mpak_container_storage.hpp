#pragma once

#include "bsp/native_mpak_provider.hpp"
#include "bsp/native_mpak_record_copy.hpp"

namespace bsp {
class NativePathCanonicalizerServices;
class NativeAdoptedSubstreamDispatch;

// Concrete source specialization for the parser's actual vector headers and
// end-insertion paths. Every owner and allocation service is borrowed.
class NativeMpakContainerStorage final : public NativeMpakContainerLibrary {
public:
    NativeMpakContainerStorage(NativeStringStorage& strings,
        NativeAdoptedSubstreamDispatch& streams,
        NativePathCanonicalizerServices& allocation_and_pool,
        NativeMpakOffsetVectorCopyLibrary& offsets) noexcept;
    void append_file_00bb7a20(void*, const void*) override;
    void append_directory_00bb7ba0(void*, const void*) override;
    void insert_offset_00a40d60(void*, void*, void*, void*, const void*) override;
    void invalid_parameter_00bf6713() override;
    void destroy_file_range_00bb6220(void*, void*, void*, void*) override;
    void destroy_file_vector_00bb6e60(void*) override;
    void destroy_directory_vector_00bb71f0(void*) override;
private:
    NativeMpakDirectoryContext directory_context() noexcept;
    NativeStringStorage& strings_;
    NativeAdoptedSubstreamDispatch& streams_;
    NativePathCanonicalizerServices& allocation_and_pool_;
    NativeMpakOffsetVectorCopyLibrary& offsets_;
};
} // namespace bsp
