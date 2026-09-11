#include "bsp/bot_scheduler_output.hpp"

namespace bsp {

std::int32_t aggregate_bot_side_scores_00914390(const BotSideAssetScores& scores) noexcept
{
    // 00914EB7..00914EC8. The decompiler renders the sum as
    //   [0x76] + [0x74] + [0x72] + [0x77] + [0x75] + [0x73]
    // which in record offsets is +1D8h, +1D0h, +1C8h, +1DCh, +1D4h, +1CCh, that
    // is categories 4, 2, 0, 5, 3, 1. Signed 32-bit wrap-around is preserved by
    // going through the unsigned type, which is what the native ADD does.
    std::uint32_t total = static_cast<std::uint32_t>(scores.category[4]);
    total += static_cast<std::uint32_t>(scores.category[2]);
    total += static_cast<std::uint32_t>(scores.category[0]);
    total += static_cast<std::uint32_t>(scores.category[5]);
    total += static_cast<std::uint32_t>(scores.category[3]);
    total += static_cast<std::uint32_t>(scores.category[1]);
    return static_cast<std::int32_t>(total);
}

std::int32_t accumulate_weighted_bot_side_term(std::int32_t running, std::int32_t weight,
                                               std::int32_t count) noexcept
{
    // 00914BD3: IMUL of the 00910570 result by the node's +10h, added to the
    // running total. Both the product and the sum are 32-bit and wrap.
    const std::uint32_t product =
        static_cast<std::uint32_t>(weight) * static_cast<std::uint32_t>(count);
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(running) + product);
}

std::int32_t clamp_bot_side_term_at_zero_00914ea0(std::int32_t value) noexcept
{
    // 00914EA0: `value & ((value < 0) - 1)`. The mask is zero for a negative
    // value and all ones otherwise, so the term contributes nothing when the
    // lookup fails. Reproduced as the arithmetic it is, not as std::max, because
    // the native form is branchless and defined for the whole signed range.
    const std::uint32_t mask =
        static_cast<std::uint32_t>(static_cast<std::int32_t>(value < 0) - 1);
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(value) & mask);
}

bool bot_demand_reaches_threshold(std::int32_t count, std::int32_t threshold) noexcept
{
    // The publication gate is `*configVector.front() <= entry->count`, a signed
    // comparison; the JLE that skips publication is the negation.
    return threshold <= count;
}

} // namespace bsp
