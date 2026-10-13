// Packet cc13-ver13 (docs/CC13_VER_13.md). Hypotheses, not recovered symbols.
// Evidence: Capstone over the PE on disk (tools/bsp.py disasm-raw); see the header.
#include "bsp/target_validity_ver_cc13.hpp"

namespace bsp {

bool squadron_land_target_latches_009f8295(const SquadronLandTargetFacts& f) noexcept {
    if (!f.present || f.destroyed_5e || f.torn_down_5d) return true;   // 009F829F..009F82AB -> 009F82CE
    if (!f.slot0_valid_00927c90) return false;                          // 009F82B9 JE 009F8416
    return f.owner_side_54 != f.target_side_54;                         // 009F82C8 JE 009F8416
}

}  // namespace bsp
