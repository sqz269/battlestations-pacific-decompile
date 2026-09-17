#pragma once
#include "bsp/native_profile_collections.hpp"
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {
struct alignas(4) NativeMissionProgressStorage final {std::byte bytes[0x24];};
static_assert(sizeof(NativeMissionProgressStorage)==0x24);
static_assert(std::is_trivially_default_constructible_v<NativeMissionProgressStorage>);
struct NativeMissionProgressOperation final {
    enum class Phase {fresh,running,complete,failed,diagnostic_retired};
    Phase phase{Phase::fresh};
    void* owner{};
    NativeProfileCollectionCalls* calls{};
    NativeStringStorage* strings{};
    bool destruction{};
    std::uint32_t native_site{};
    std::int32_t unwind_state{-1};
    std::uint32_t iterator_output[2];
    NativeMissionProgressOperation() noexcept=default;
    ~NativeMissionProgressOperation();
    NativeMissionProgressOperation(const NativeMissionProgressOperation&)=delete;
    NativeMissionProgressOperation& operator=(const NativeMissionProgressOperation&)=delete;
    void acknowledge_diagnostic_cleanup() noexcept;
};
// 920E10[171]: ECX24h storage/EAX same/RET. Create three sentinel trees with
// untouched allocator words+0/+C/+18 and otherwise uninitialized payload bytes.
void* construct_native_mission_progress_00920e10(void*,NativeProfileCollectionCalls&,
    NativeMissionProgressOperation&);
// 7FD780[196]: ECX24h owner/RET. Clear full current ranges +18,+C,+0, free
// each CURRENT sentinel, then clear head/count. Does not free the24h owner;
// profile reset separately frees its captured pointer. Populated score-record
// destruction is a required dependency, not a substitute record projection.
void destroy_native_mission_progress_007fd780(void*,NativeStringStorage&,
    NativeProfileCollectionCalls&,NativeMissionProgressOperation&);
// Complete normal owner schedules, not native FH3 rollback or binary entries.
// Failed operations retain the partial graph and reject replay/implicit disposal.
} // namespace bsp
