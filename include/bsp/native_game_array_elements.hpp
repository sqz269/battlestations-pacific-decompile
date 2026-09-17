#pragma once
#include <cstdint>

namespace bsp {
struct NativeGameArrayConstants {const volatile std::uint32_t& bits_00ce4adc;};
struct NativeGameArrayCalls {
    virtual ~NativeGameArrayCalls()=default;
    virtual void* allocate_00bf681b(std::uint32_t bytes);
    // Normal CRT array contract for the actual415270/41DD20 string pair.
    virtual void construct_empty_strings_00bf7cd1(void*,std::uint32_t stride,
        std::uint32_t count,std::uint32_t constructor,std::uint32_t destructor);
};
struct NativeGameArrayElementStage {
    void* owner{};
    std::uint32_t native_site{};
    std::uint32_t storage_site{};
    std::int32_t unwind_state{-1};
    void* pending_node{};
};
struct NativeGameArrayOperation final {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    void* base{};
    std::uint32_t stride{},count{},constructor{},destructor{},completed{};
    NativeGameArrayElementStage element;
    NativeGameArrayOperation() noexcept=default;
    ~NativeGameArrayOperation();
    NativeGameArrayOperation(const NativeGameArrayOperation&)=delete;
    NativeGameArrayOperation& operator=(const NativeGameArrayOperation&)=delete;
    // Acknowledges externally handled resources. No automatic native rollback.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Normal raw storage contracts: untouched allocator words and payload padding.
void* initialize_native_game_vector_004d3730(void*) noexcept;
void* construct_native_game_list_004c8140(void*,NativeGameArrayCalls&,NativeGameArrayElementStage&);
void insert_native_game_participant_pairs_004d2920(void* list,std::uint32_t count,
    const void* pair,NativeGameArrayCalls&,NativeGameArrayElementStage&);
// Complete normal307-byte participant constructor, actual118h array stride.
void* construct_native_game_participant_004d6ba0(void*,const NativeGameArrayConstants&,
    NativeGameArrayCalls&,NativeGameArrayElementStage&);
// Normal CRT iteration over the three callback pairs used by4DDB90. Calls real
// source element constructors. Unsupported callback/stride pairs are rejected.
// Source failure retains the completed prefix/current element and rejects replay;
// original CRT/FH3 reverse destruction and element destructors are not supplied.
void construct_native_game_array_00bf7cd1(void*,std::uint32_t stride,std::uint32_t count,
    std::uint32_t constructor,std::uint32_t destructor,const NativeGameArrayConstants&,
    NativeGameArrayCalls&,NativeGameArrayOperation&);
} // namespace bsp
