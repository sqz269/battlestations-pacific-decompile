// Segment 59 leftovers (008EE670..00922240), packet cc13-seg59b (marker cc13_seg59c).
//
// What segment 59 still lacked after cc13-seg59A / cc13-seg59b: the damage-control slot list
// 0090E6C0 (tagged stl_probable, but it is game code), three code-referenced trivial bodies,
// and three bodies with no code or data reference in the image. The rest of the segment is
// either reconstructed or library code that is bound, not rebuilt. docs/CC13_SEG59C.md has the
// evidence and the census.
//
// Model: the routines work on the original objects as raw bytes (Win32 layout, image offsets)
// and reach STL helpers and the CRT through Seg59cHost, whose methods name the image routine
// they stand for. New C++ interfaces, not drop-in binary replacements; nothing here is bound
// into bsp_game. Names are hypotheses, not recovered symbols.

#ifndef BSP_CC13_SEG59C_HPP
#define BSP_CC13_SEG59C_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_seg59c {

template <class T>
inline T& at(void* base, std::size_t off) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + off);
}
inline void* add(void* base, std::size_t off) noexcept {
    return static_cast<std::uint8_t*>(base) + off;
}

// 0090E6C0 layout: the manager's per-slot record stride and the vector<uint16> at +140h
// (std::vector: +4h first, +8h last), and the unit's id (+174h, a word) and owner slot (+180h).
inline constexpr std::size_t kSlotRecordStride = 0x284;
inline constexpr std::size_t kSlotUnitListOffset = 0x140;
inline constexpr std::uint32_t kSlotCount = 8;          // CMP EAX,7 / JA at 0090E6D2
inline constexpr std::size_t kUnitIdOffset = 0x174;
inline constexpr std::size_t kUnitSlotOffset = 0x180;
// 009080A0 / 00908120: the map node fields they read.
inline constexpr std::size_t kNodeFloat20Offset = 0x20;
inline constexpr std::size_t kNodeInt1F4Offset = 0x1F4;

class Seg59cHost {
public:
    virtual ~Seg59cHost() = default;
    // 00BF6713, the checked-iterator failure report.
    virtual void invalid_parameter_00bf6713() = 0;
    // 007265E0, std::vector<uint16_t>::push_back, __thiscall(ECX = vector, const uint16_t*).
    virtual void vector_u16_push_back_007265e0(void* vector, const std::uint16_t* value) = 0;
    // The three erase(first, last) instantiations 008FACC0 calls, each
    // __thiscall(ECX = vector, iterator* out, vector, first, vector, last).
    virtual void vector_erase_range_008fa1a0(void* vector, void* first, void* last) = 0;
    virtual void vector_erase_range_008fa220(void* vector, void* first, void* last) = 0;
    virtual void vector_erase_range_008f9fd0(void* vector, void* first, void* last) = 0;
    // 00581290, the map iterator's ++ (in-order successor of node under head).
    virtual void* tree_next_00581290(void* head, void* node) = 0;
};

// 0090E6C0, __thiscall(ECX = manager, unit) RET 4. Tail-jumped from 0093A4DD and 0093A55F
// (the repair task's add-fire / add-water seconds, when pending damage exceeds the unit's
// health; ECX = game+21A0h there). Appends the unit's id to its slot's list once; a slot
// above 7 does nothing.
void damage_control_note_unit_in_slot_list_0090e6c0(Seg59cHost& host, void* manager, void* unit);

// 008FB520, __thiscall(ECX = [unit+394h]) -> float 0.0 (FLDZ / RET). The one caller,
// 00959F12 in BSP_Unit_ApplyGunAimMessage, stores the result to a local.
float zero_float_008fb520(const void* self) noexcept;
// 00905370, __thiscall(ECX = [game+21A0h]) RET: no-op. Called at 0076D1CC from
// MMultiPlayer::ServerSendSceneScoring.
void scoring_noop_00905370(void* self) noexcept;
// 00920EC0, __thiscall RET: no-op. Called at 004DF9D9 from BSP_Game_ConstructWorld with
// ECX = the object 004DF9C1 just built with 009221E0 (MOV ECX,EAX at 004DF9CA; 004DF9D3 stores
// it at game+21CCh). ECX is 0 only when the allocation failed (004DF9BD JE 004DF9C8 XOR EAX,EAX).
void construct_world_noop_00920ec0(void* self) noexcept;

// 008FACC0, __fastcall(ECX = object) RET. No code or data reference in the image (rel32
// and absolute scans, docs/CC13_SEG59C.md). Clears the three vectors at +14h, +24h and +04h,
// in that order, each through erase(first, last) after the checked-iterator test first <= last
// (run twice, as the image does).
void clear_three_vectors_008facc0(Seg59cHost& host, void* object);

// 009080A0, __fastcall(ECX = map) -> bool (AL). Unreferenced. True when every node's float
// at +20h compares equal to 0.0f ([00D7A218]); UCOMISS / LAHF / TEST AH,44h / JP, so a NaN
// counts as not equal and returns false.
bool map_all_node_floats_zero_009080a0(Seg59cHost& host, void* map);

// 00908120, __fastcall(ECX = map) -> int. Unreferenced. The sum (wrapping int add) of every
// node's dword at +1F4h.
std::int32_t map_sum_node_int_1f4_00908120(Seg59cHost& host, void* map);

}  // namespace bsp::cc13_seg59c

#endif
