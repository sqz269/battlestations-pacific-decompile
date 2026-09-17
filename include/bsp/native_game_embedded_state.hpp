#pragma once
#include <cstdint>

namespace bsp {
// Actual game+1EF0 owner; this constructor accesses +0..29C. This is an
// accessed prefix, not a recovered full class/allocation-size declaration.
struct NativeGameEmbeddedStateConstants {
    const volatile std::uint32_t& bits_00d7a24c;
    const volatile std::uint32_t& bits_00d042f8;
    const volatile std::uint32_t& bits_00d7a260;
    const volatile std::uint32_t& bits_00d7a208;
    const volatile std::uint32_t& bits_00d0de84;
};
struct NativeGameEmbeddedStateCalls {
    virtual ~NativeGameEmbeddedStateCalls()=default;
    virtual void* call_00bd1860();
    virtual void* allocate_00bf55be(std::uint32_t bytes);
    virtual void free_00bf6989(void* allocation);
};
struct NativeGameEmbeddedStateOperation final {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    void* owner{};
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
    void* current_allocation{};
    std::uint32_t copy_index{};
    NativeGameEmbeddedStateOperation() noexcept=default;
    ~NativeGameEmbeddedStateOperation();
    NativeGameEmbeddedStateOperation(const NativeGameEmbeddedStateOperation&)=delete;
    NativeGameEmbeddedStateOperation& operator=(const NativeGameEmbeddedStateOperation&)=delete;
    // Acknowledges external diagnostic cleanup; it does not free the graph.
    void acknowledge_diagnostic_cleanup() noexcept;
};
// Eight1Ch records, then EC/E0 gate/time reset. Sparse byte padding preserved.
void reset_native_game_peer_rows_007868c0(void*,std::uint8_t,
    const NativeGameEmbeddedStateConstants&) noexcept;
void* construct_native_game_peer_rows_00786a80(void*,
    const NativeGameEmbeddedStateConstants&) noexcept;
// Eight ordered pairs at0 and40, with one flag per pair at80..87.
void reset_native_game_peer_pairs_00778590(void*) noexcept;
void* construct_native_game_peer_pairs_00778610(void*) noexcept;
// Full normal76EDE0, ECX owner/EAX owner/RET. SSE subtraction, real lock and
// malloc/new-handler buffer defaults. Native FH3/ABI/destructor remain open.
// Exceptions retain stage, replacement allocation and partial owner; no replay.
void* construct_native_game_embedded_state_0076ede0(void*,
    const NativeGameEmbeddedStateConstants&,NativeGameEmbeddedStateCalls&,
    NativeGameEmbeddedStateOperation&);
} // namespace bsp
