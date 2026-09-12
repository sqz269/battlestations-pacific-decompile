// The objective unit list at objective+20h and the two Lua bindings that push and pop it.
//
// Packet cc2_mission_objectives, worktree agent/cc2-mission-objectives. Ghidra was read-only
// for this packet. Every name here is a hypothesis, not a recovered symbol.
//
// docs/OBJECTIVE_UNIT_LIST.md carries the evidence. docs/MISSION_RESULT_DECISION.md already
// established the Objective record (name at +4h/+8h, kind at +18h, state at +1Ch, the list at
// +20h..+28h) and the per-slot ObjectiveSet at game+21A4h + slot*4; this header adds the
// element type of that list, its two producers and the single routine that walks it.
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "bsp/lua_binding_core.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The list and its element
// ---------------------------------------------------------------------------
// The std::list object begins at objective+20h; 008dee50 loads [this+24h] as the head and
// then does ADD ESI,0x20 to get the list itself, so +24h is _Myhead and +28h is _Mysize.
inline constexpr std::size_t kObjectiveUnitListOffset = 0x20;      // 008dee70, 008deee8
inline constexpr std::size_t kObjectiveUnitListHeadOffset = 0x24;  // 008dee6d, 008dfe8c
inline constexpr std::size_t kObjectiveUnitListSizeOffset = 0x28;  // 008de200 increments it

// The set's own list of objectives, for contrast: 008df2e5 reads [this+28h] as the head next
// to 008df2ec LEA EDI,[ECX+0x24]. include/bsp/local_player_unit_lists.hpp declares +18h/+1Ch/
// +20h under kObjectiveSet* names; those belong to the SzurkeNyil marker object read by
// 008ddf90, not to this set, and are left untouched.
inline constexpr std::size_t kObjectiveSetListOffset = 0x24;      // 008df2ec, 008df425
inline constexpr std::size_t kObjectiveSetListHeadOffset = 0x28;  // 008df2e5, 008df418
// The player slot the set belongs to, read before every replication send (008df9b0).
inline constexpr std::size_t kObjectiveSetPlayerSlotOffset = 0x14;

// The element is a pointer to a 16-byte heap record, not a unit pointer.
inline constexpr std::size_t kObjectiveUnitEntrySize = 0x10;     // operator new at 008dee53
inline constexpr std::size_t kObjectiveUnitEntryUnitOffset = 0x0;  // 008dee67, 008deec9
inline constexpr std::size_t kObjectiveUnitEntryXOffset = 0x4;     // 008deecf
inline constexpr std::size_t kObjectiveUnitEntryYOffset = 0x8;     // 008deed5
inline constexpr std::size_t kObjectiveUnitEntryZOffset = 0xc;     // 008deedb

// A marker point in world space, the three floats at entry+4h.
struct ObjectiveWorldPoint {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// One entry of the list. The two producers initialise disjoint halves: 008dee50 writes the
// unit and leaves the floats alone, 008deeb0 writes a null unit and the three floats. The
// `position_valid` flag records which producer made the entry so the projection never
// presents an uninitialised position as a zero vector.
struct ObjectiveUnitEntry {
    void* unit{nullptr};             // +0h, null for a position entry
    ObjectiveWorldPoint position{};  // +4h..+Fh, meaningful only when unit is null
    bool position_valid{false};
};

// 008dee50: allocate, store the unit, push_back. The position half stays unwritten.
ObjectiveUnitEntry objective_unit_entry_008dee50(void* unit) noexcept;
// 008deeb0: allocate, zero the unit word, store the three floats, push_back.
ObjectiveUnitEntry objective_position_entry_008deeb0(const ObjectiveWorldPoint& point) noexcept;

// ---------------------------------------------------------------------------
// The objective and the set
// ---------------------------------------------------------------------------
// The fields of the native Objective this packet reads. kind and state are the peer packet's
// (+18h, +1Ch); the list is the subject here.
struct ObjectiveUnitList {
    std::string name;                        // objective+4h length, +8h pointer
    int kind{0};                             // objective+18h, 2 is hidden
    int state{0};                            // objective+1Ch, 1 completed, 2 failed
    std::vector<ObjectiveUnitEntry> entries; // objective+20h
};

// The objective kind that suppresses both the announcement and the marker refresh (008dfe6e
// CMP dword ptr [EBP+0x18],0x2).
inline constexpr int kObjectiveKindHidden = 2;
// The state 008df2b0 and 008df5d0 accept on a hidden objective before binding a marker
// (008df3?? CMP dword ptr [...+0x1c],0x1).
inline constexpr int kObjectiveStateCompleted = 1;

// The gate both add paths apply after the push: a hidden objective that is not completed
// stops before the marker call.
constexpr bool objective_marker_call_runs(int kind, int state) noexcept
{
    return !(kind == kObjectiveKindHidden && state != kObjectiveStateCompleted);
}

// The two unit bytes 008df2b0 tests before it pushes anything (unit+5Dh, unit+5Eh).
struct ObjectiveUnitLiveness {
    bool removed_5d{false};
    bool destroyed_5e{false};
};
constexpr bool objective_accepts_unit(const ObjectiveUnitLiveness& live) noexcept
{
    return !live.removed_5d && !live.destroyed_5e;
}

// ---------------------------------------------------------------------------
// The binding argument contract
// ---------------------------------------------------------------------------
// Objectives_AddUnit(party, playerSlot, objectiveName, target, ...). The eight player objects
// live at game+18CCh + k*4 and their party id at player+28h; the eight objective sets at
// game+21A4h + k*4. Argument 0 builds a mask of every slot in that party; argument 1, when it
// names an active player, replaces the mask with that one slot.
inline constexpr int kObjectiveSetSlotCount = 8;              // 008ce44b CMP EBP,0x21c4
inline constexpr int kObjectiveFirstTargetArgument = 3;       // 008ce075 MOV ESI,0x3
inline constexpr int kObjectiveNameArgument = 2;              // 008cdffa PUSH 0x2

// 008cdef2's loop: bit k is set when player k's party matches the argument.
unsigned int objective_party_slot_mask(const int* player_party, int slot_count,
                                       int party) noexcept;
// 008cdfe0: mask = (uint16)(1 << slot), applied only for an active player slot.
unsigned int objective_explicit_slot_mask(int slot) noexcept;
// 008cdf58: player+8h set and player+9h clear.
constexpr bool objective_slot_is_active(bool player_byte_08, bool player_byte_09) noexcept
{
    return player_byte_08 && !player_byte_09;
}

// ---------------------------------------------------------------------------
// Host
// ---------------------------------------------------------------------------
// One virtual per native call site. No defaults.
struct ObjectiveUnitHost {
    virtual ~ObjectiveUnitHost() = default;

    // [00e188a8]+18CCh + slot*4, then +28h: the player's party id (008cdead).
    virtual int player_party(int slot) = 0;
    // The same player, bytes +8h and +9h (008cdf58, 008cdf5e).
    virtual bool player_slot_active(int slot) = 0;

    // The objective named `name` in the set of player `slot`, or null. The walk is
    // 008df2b0's over the set's list at +24h with a case-insensitive name compare.
    virtual ObjectiveUnitList* objective_in_slot(int slot, const std::string& name) = 0;

    // unit+5Dh and unit+5Eh (008df2b0's entry test).
    virtual ObjectiveUnitLiveness unit_liveness(void* unit) = 0;

    // 00694af0 then 00694a60. Neither body was read; both are named by their call site.
    virtual bool observer_pair_registered(ObjectiveUnitList& objective, void* unit) = 0;
    virtual void observer_register_pair(ObjectiveUnitList& objective, void* unit) = 0;

    // 006de3d0 at 008df39e and 006de3e0 at 008df5d0's tail. Both native routines are a bare
    // RET 8: the add path binds no marker. Kept as a host call so the call site stays visible.
    virtual void marker_bind_on_add_stub(const std::string& marker_class, void* unit) = 0;

    // 006de3f0 at 008dff3b, the unit marker rebind. Its insert half past 008de42c was not
    // read, so only the clear-and-replace contract is claimed.
    virtual void marker_rebind_unit(const std::string& marker_class, void* unit) = 0;
    // 006de4c0 at 008dffd2, RET 10h: build the key from (name, position), resolve the slot
    // and destroy its occupant.
    virtual void marker_clear_position(const std::string& marker_class,
                                       const ObjectiveWorldPoint& point) = 0;

    // 008dfc00 at 008e0035 and 008ce510's own dispatch: hand a live unit back to the
    // set-level removal. Only the dispatch of 008dfc00 and 008df400 was read, so the erase
    // itself (008dc3e0) is a contract.
    virtual void set_remove_unit(const std::string& objective_name, void* unit) = 0;
    // 008ddd30, the position twin of the removal, reached from 008ce510.
    virtual void set_remove_position(const std::string& objective_name,
                                     const ObjectiveWorldPoint& point) = 0;

    // 008de6a0 at 008df3d7, the second call of the add path. Contract unread.
    virtual void objective_add_tail(const std::string& marker_class, void* unit) = 0;
};

// The marker class string at 00D16100, immediately before its vtable at 00D1610C.
inline constexpr const char* kObjectiveMarkerClass = "SzurkeNyil";

// ---------------------------------------------------------------------------
// The routines
// ---------------------------------------------------------------------------
// 008df2b0, the per-unit push: liveness test, name lookup, observer pair, push, marker call.
void objective_set_add_unit_008df2b0(ObjectiveUnitHost& host, int slot,
                                     const std::string& objective_name, void* unit);
// 008df5d0, the per-position push.
void objective_set_add_position_008df5d0(ObjectiveUnitHost& host, int slot,
                                          const std::string& objective_name,
                                          const ObjectiveWorldPoint& point);

// 008dfe50, the walk the completed and failed transitions run. Its ABI is
// __thiscall(ObjectiveSet*, Objective*), RET 4: ECX is stored at 008dfe74 and read back at
// 008e002d as the `this` of the removal call, which the ledger's __stdcall note predates.
void objective_walk_unit_markers_008dfe50(ObjectiveUnitHost& host, ObjectiveUnitList& objective);

// One trailing argument of either binding, after 008889C0 and 0088B840 have classified it.
// Reading a Vector3 table (00888760) and flattening a table argument into several targets
// (00B67080 / 00B67190) are Lua-side steps LuaBindingArgumentReader does not model, so the
// classified targets are an input here rather than something these functions read.
struct ObjectiveTarget {
    bool is_unit{false};
    void* unit{nullptr};
    ObjectiveWorldPoint point{};
};

// 008cdd60 and 008ce510. Arguments 0 to 2 come from the reader; the targets are arguments 3
// upward, already classified and flattened.
int lua_binding_objectives_add_unit_008cdd60(LuaBindingArgumentReader& args,
                                              ObjectiveUnitHost& host,
                                              const std::vector<ObjectiveTarget>& targets);
int lua_binding_objectives_remove_unit_008ce510(LuaBindingArgumentReader& args,
                                                 ObjectiveUnitHost& host,
                                                 const std::vector<ObjectiveTarget>& targets);

// The mask both bindings compute from arguments 0 and 1, exposed for the doc's table and for
// callers that already hold the two arguments.
unsigned int objective_target_slot_mask(ObjectiveUnitHost& host, bool have_party, int party,
                                        bool have_slot, int slot);

} // namespace bsp
