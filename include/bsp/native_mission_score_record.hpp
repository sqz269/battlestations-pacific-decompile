#pragma once
#include "bsp/native_profile_collections.hpp"
#include <cstddef>
#include <type_traits>

namespace bsp {
// Native base record284h plus the extended count at284h. No blanket initialization.
struct alignas(4) NativeMissionScoreRecordStorage final {std::byte bytes[0x288];};
static_assert(sizeof(NativeMissionScoreRecordStorage)==0x288);
static_assert(std::is_trivially_default_constructible_v<NativeMissionScoreRecordStorage>);
// 593570..593C9F: ECX actual record, RET. Complete normal owned-field cleanup:
// three strings,36 trees (including nested counter maps),three plain vectors.
// Preserve scalar/opaque words and string headers; free CURRENT tree sentinels,
// clear head/count, and do not free the record. Uses existing source string/CRT
// storage contracts. General partial-range erase and native FH3 rollback are
// outside this interface. On failure the caller owns the partially retired graph.
void destroy_native_mission_score_record_00593570(void* actual_record,
    NativeStringStorage&,NativeProfileCollectionCalls&);
} // namespace bsp
