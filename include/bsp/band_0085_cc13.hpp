#pragma once

// Gameplay leaves from 00850000-0087FFFF and 00890000-008FFFFF reconstructed by
// cc13-gp85: box and vector tests, two unit predicates, the objective state
// tests, the power-up slot cooldown record, the landscape mask dilation and the
// voice-language selection. Names are hypotheses, not recovered symbols.
// Evidence, ABIs and open questions are in docs/CC13_GP_0085_008F.md. None is
// wired into a host and none is a drop-in binary replacement: these are C++
// interfaces over models of the native data. Build-tested only.
//
// x87 model: the image runs with 53-bit precision; every FSTP to a float slot is
// a float cast, and the intermediates in between are modelled as double. An
// unordered compare (a NaN input) is noted per function.

#include <cstdint>
#include <optional>
#include <string_view>

namespace bsp::cc13_band0085 {

// 00855C20, __thiscall(ECX = const float v[3], float eps), RET 4 -> EAX.
// 1 when eps > |v[i]| for all three components, else 0. |x| is x when x > 0,
// otherwise -0.0f - x (the constant at 00D7A208 is -0.0f). The tests run x, y,
// z and stop at the first failure; a NaN component or eps gives 0 (JBE).
int vector3_all_abs_less_than_00855c20(const float v[3], float eps) noexcept;

// 0085BF20, __fastcall(ECX = minA, EDX = maxA, [esp+4] = minB, [esp+8] = maxB),
// RET 8 -> AL. False when minA > maxB or minB > maxA on any axis (tested in
// the order x, z, y); touching boxes overlap. Every test is FCOMIP + JA/JBE, so
// an unordered compare does not separate: a NaN coordinate counts as overlap.
bool aabb_overlaps_0085bf20(const float min_a[3], const float max_a[3], const float min_b[3],
                            const float max_b[3]) noexcept;

// 0085BF90, __fastcall(ECX = point, EDX = box min, [esp+4] = box max), RET 4
// -> ST0. Squared distance from the point to the box. Per axis d = float(min -
// p); when d > 0 the term is float(d * d), otherwise e = float(p - max) and the
// term is float(e * e) when e > 0, else 0. A NaN axis contributes 0. The
// terms are summed as (y + x) + z in extended precision and returned as float.
float aabb_point_distance_squared_0085bf90(const float point[3], const float box_min[3],
                                           const float box_max[3]) noexcept;

// 00859C60, __fastcall(ECX = unit) -> EAX. 1 when the float at unit+36Ch (the
// maximum health, docs/CC13_GP_0085_008F.md) is greater than the float at
// unit+370h (the current health); NaN gives 0.
int unit_is_damaged_00859c60(float max_health_36c, float health_370) noexcept;

// 00851E40, __fastcall(ECX = unit) -> pointer. Which hangar block a unit
// carries: unit+1188h when IsKindOf(09h, MMothership), unit+72Ch when
// IsKindOf(45h, MAirfield) or IsKindOf(46h, MShipyard), else null. The tests run
// in that order through vtable +5Ch; a null unit gives null. The model returns
// the byte offset (0 for "none"); the IsKindOf answers are inputs.
std::uint32_t unit_hangar_block_offset_00851e40(bool is_mothership, bool is_airfield,
                                                bool is_shipyard) noexcept;

// 008DA130, __fastcall(ECX = objective) -> EAX: state (+1Ch) == 0, active.
bool objective_is_active_008da130(std::int32_t state_1c) noexcept;

// 008DA140, __fastcall(ECX = objective) -> EAX. 1 unless kind (+18h) is 2
// ("hidden" in the kind table 00E0C948); a hidden objective gives 1 only when
// its state (+1Ch) is 1 (completed).
bool objective_is_shown_008da140(std::int32_t kind_18, std::int32_t state_1c) noexcept;

// The power-up slot cooldown record (008E4B00 / 008E4BB0). Offsets in the image
// record; class_* are the power-up class fields the record points at (+8).
struct PowerupSlotRecord {
    bool pending_0 = false;          // +0, cleared by 008E4B00
    std::uint32_t subject_4 = 0;     // +4, cleared by 008E4B00
    float ready_time_0c = 0.0f;      // +0Ch, now + class+7Ch
    float end_time_10 = 0.0f;        // +10h, now + class+80h
    bool deferred_14 = false;        // +14h, class kind (+8) == 1 (air support)
};

struct PowerupClassTimes {
    std::int32_t kind_8 = 0;         // class+8
    float delay_7c = 0.0f;           // class+7Ch
    float cooldown_80 = 0.0f;        // class+80h
};

// 008E4B00, __thiscall(ECX = record, class), RET 4. Stores the class pointer
// (+8, not modelled), sets deferred = (kind == 1); when not deferred,
// end = float(cooldown + now) and ready = float(delay + now), with now the
// float at 00F876A4. Clears +4 and +0 in both cases. A deferred record keeps
// its old times.
void powerup_slot_start_cooldown_008e4b00(PowerupSlotRecord& record, const PowerupClassTimes& cls,
                                          float now_f876a4) noexcept;

// 008E4BB0, __fastcall(ECX = record) -> ST0. 1.0 when deferred; 0 when now >
// end; else float((end - now) / class cooldown). An unordered now/end compare
// takes the division path.
float powerup_slot_cooldown_fraction_008e4bb0(const PowerupSlotRecord& record, float cooldown_80,
                                              float now_f876a4) noexcept;

// 0087FA90, __stdcall(rows, row_count, column_count, block), RET 10h. For
// block > 1, every whole block x block tile (row and column starts step by
// block while start <= count - block; partial edge tiles are skipped) holding
// any non-zero byte is filled with 1. block <= 1 does nothing.
void landscape_dilate_mask_to_blocks_0087fa90(std::uint8_t* const* rows, std::int32_t row_count,
                                              std::int32_t column_count, std::int32_t block) noexcept;

// One row of the language table at [00F88974] (stride 20h, count [00F88978]).
struct VoiceLanguageRow {
    std::uint32_t has_voice_10 = 0;          // +10h
    const char* voice_folder_14 = nullptr;   // +14h
};

// 008D57A0, __fastcall(ECX = settings) -> const char*. index is settings+4.
// "english" when count < index (signed JL) or the row's +10h is 0; else
// "authentic" when the profile owns DL_Content_0000062 (00706760, an input
// here); else the row's +14h, or "" (00F88A3C) when that is null.
// The bound admits index == count and any negative index, which read outside
// the table; the model returns nullopt for those instead of reading.
std::optional<std::string_view> voice_language_name_008d57a0(const VoiceLanguageRow* table,
                                                             std::int32_t count, std::int32_t index,
                                                             bool owns_dl_content_62) noexcept;

}  // namespace bsp::cc13_band0085
