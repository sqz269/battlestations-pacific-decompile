#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

#include "bsp/world_entities.hpp" // kBotSlotCount, kBotSlotStride, kBotSlotDispatchCode

// What the bot scheduler at game+21A0h produces, and where that output lives.
// See docs/BOT_SCHEDULER_OUTPUT.md. The scheduler is a strategy layer: none of
// its three callees performs a virtual call or writes a unit or a controller,
// so nothing here touches a unit command field. Descriptive names are
// hypotheses, not recovered symbols. Semantic interfaces for MSVC Win32.

namespace bsp {

// ---------------------------------------------------------------------------
// The eight per-side records.
// ---------------------------------------------------------------------------

// 00915026, LEA ESI,[EBX+4]. The array starts four bytes into the scheduler,
// not at +1E4h: world_entities.hpp's kBotSlotArrayOffset is the biased pointer
// the retarget loop uses (record[0] + 1E0h), not the array base. Eight records
// of kBotSlotStride end at +1424h, inside the 14B8h allocation, which settles
// the uncertainty recorded in docs/GAME_WORLD_ENTITIES.md.
inline constexpr std::size_t kBotSideRecordBase = 0x4;

// Offsets inside one record, all relative to its own start.
inline constexpr std::size_t kBotSideDemandList = 0x78;     // 00911E80 rebuilds it
inline constexpr std::size_t kBotSideDemandCount = 0x80;    // 00911E80 clears it
inline constexpr std::size_t kBotSideGoalList = 0x88;       // 00912A60 rebuilds it
inline constexpr std::size_t kBotSideGoalCount = 0x8C;      // 00912A60 clears it
inline constexpr std::size_t kBotSideScoreFirst = 0x1C8;    // 00914390, six sums
inline constexpr std::size_t kBotSideScoreStride = 0x4;
inline constexpr std::size_t kBotSideAggregate = 0x1E0;     // 00914390's total
inline constexpr std::size_t kBotSidePlayerSlot = 0x250;    // handed to 0090EDE0

// 00914F88 reads [EDI] with EDI = record + kBotSideAggregate, so the dword the
// retarget pass broadcasts is the aggregate, not "the record's first dword".
inline constexpr std::size_t kBotSideBroadcastField = kBotSideAggregate;

// Byte offset of record `index` from the scheduler's own base.
constexpr std::size_t bot_side_record_offset(std::size_t index) noexcept
{
    return kBotSideRecordBase + index * kBotSlotStride;
}

// ---------------------------------------------------------------------------
// 00914390, the per-side asset accounting. void __fastcall(record).
// ---------------------------------------------------------------------------

inline constexpr std::size_t kBotSideScoreCount = 6;

// The six sums, in memory order. Which asset class feeds each list is not
// proven; the names say where the value lands, not what it counts.
struct BotSideAssetScores {
    std::array<std::int32_t, kBotSideScoreCount> category{}; // +1C8h..+1DCh
    std::int32_t aggregate{0};                               // +1E0h
};

// 00914EB7..00914EC8, the native addition order:
//   +1D8h + +1D0h + +1C8h + +1DCh + +1D4h + +1CCh
// Reproduced exactly, because these are signed 32-bit sums that can wrap and
// the order decides where a wrap shows up.
std::int32_t aggregate_bot_side_scores_00914390(const BotSideAssetScores& scores) noexcept;

// 00914BD3: the only weighted term. The list at record+DCh contributes
// 00910570(node+0Ch) * node+10h to category index 3 (+1D4h).
inline constexpr std::size_t kBotSideWeightedCategory = 3;
std::int32_t accumulate_weighted_bot_side_term(std::int32_t running, std::int32_t weight,
                                               std::int32_t count) noexcept;

// 00914EA0: category index 5 (+1DCh) clamps each term at zero with the native
// idiom `value & ((value < 0) - 1)`, which yields zero for a negative value and
// the value itself otherwise.
inline constexpr std::size_t kBotSideClampedCategory = 5;
std::int32_t clamp_bot_side_term_at_zero_00914ea0(std::int32_t value) noexcept;

// The two flags 00914390 raises at its tail, both only when *record != 0.
// +18Ch when (float)record+14h equals _DAT_00D7A218, +188h when the lists at
// +BCh and +A4h are both empty.
inline constexpr std::size_t kBotSideFlagListsEmpty = 0x188; // [0x62]
inline constexpr std::size_t kBotSideFlagValueMatch = 0x18C; // [99]

// ---------------------------------------------------------------------------
// The demand vocabulary 00911E80 and 00912A60 publish.
// ---------------------------------------------------------------------------

// Every token below is used two ways: as a key into a case-insensitive
// string-to-int map that holds the per-side count, and as the counter name
// handed to the in-mission award grant 0090EDE0 together with record+250h.
// The prefix readings (build unit, bot order, goal action, requested unit
// action) are inference from the switch shape, not from any printed string.
inline constexpr const char* kBotUnitDemandTokens[] = {
    "BU_DM", "BU_PTM", "BU_BM", "BU_CM", "BU_VLB",
    "BU_VDB", "BU_VTB", "BU_FA", "BU_VR", "BU_SM",
};
inline constexpr const char* kBotOrderDemandTokens[] = {
    "BO_CV", "BO_DCM", "BO_KSS", "BO_KSP", "BO_SC", "BO_EE",
};
inline constexpr const char* kBotGoalTokens[] = {
    "GA_SH", "GA_DB", "GA_AO", "GA_PL",
};
inline constexpr const char* kBotRequestedUnitActionTokens[] = {
    "RUA_CU", "RUA_FU", "RUA_SU", "RUA_TBU",
};

// 00911E80's threshold read, scheduler+1470h, compared against each entry's
// +10h float with a strict less-than before the BO_CV family is counted.
inline constexpr std::size_t kBotSchedulerDemandThreshold = 0x1470;

// 009129xx and 00912Fxx: a demand is published only when its count reaches the
// first element of the config vector at [0050FC30(key)]+48h. The comparison is
// `threshold <= count`, taken from the JLE that skips the publication.
bool bot_demand_reaches_threshold(std::int32_t count, std::int32_t threshold) noexcept;

} // namespace bsp
