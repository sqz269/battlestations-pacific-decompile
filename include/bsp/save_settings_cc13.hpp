#pragma once

// Packet cc13-save1: save, profile and settings code in segments 2 and 29 that
// earlier passes named but did not reconstruct. Evidence: docs/CC13_SAVE_1.md.
//
// Every name here is a descriptive hypothesis, not a recovered symbol. None of
// these interfaces is ABI compatible: native code passes LuaObject wrappers,
// NativeString values and raw object pointers, while the host takes lua_State
// stack indices, std::string and explicit field values. Nothing is
// game-validated.

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "bsp/mission_savedata_lua.hpp"

struct lua_State;

namespace bsp::cc13_save {

using bsp::MissionLuaEntityHooks;
using bsp::MissionLuaFunctionNames;

// ---------------------------------------------------------------------------
// Segment 2: shared entity-interface default slots (40-95 vtables each).
// ---------------------------------------------------------------------------

struct Vec3f {
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
};

// 0042B860, no arguments, RET: returns the literal "Entity" (00CE3C7C).
const char* entity_type_name_0042b860();

// 0042B9E0, __fastcall(ECX entity), RET: [00E0CD80 + 4 * entity+C4h]. The
// caller passes the table and the kind; native performs no range check.
const char* entity_class_name_0042b9e0(const char* const* class_name_table, std::int32_t kind);

// 0042BA00, __thiscall(entity, Vec3f* out), RET 4. When byte entity+C8h is
// clear it first runs 00414DB0 (world pose refresh, ECX = entity), then copies
// entity+FCh..+104h. `world_position` is read after the refresh.
Vec3f entity_world_position_0042ba00(bool world_pose_valid, const std::function<void()>& refresh_world,
                                     const Vec3f& world_position);

// 0042D680, __thiscall(entity, Vec3f* out, float t), RET 8. Calls vtable
// +34h (velocity into a stack Vec3f) first, spills t * velocity per axis to
// float, then refreshes the pose as 0042BA00 does and adds entity+FCh..+104h.
Vec3f entity_predict_position_0042d680(const std::function<Vec3f()>& velocity, bool world_pose_valid,
                                       const std::function<void()>& refresh_world,
                                       const Vec3f& world_position, float t);

// 0042D770, __cdecl(float out[16]), RET: identity, 1.0f loaded from 00D7A24C.
void entity_identity_matrix_0042d770(float out[16]);

// 0042E960, __fastcall(ECX entity), RET: entity+158h when dword +15Ch is
// zero, else entity+160h; a null result becomes the empty string 00E1767D.
const char* entity_name_0042e960(const char* name_at_158, std::uint32_t selector_at_15c,
                                 const char* name_at_160);

// 0042DEF0, camera mover vtable +120h, __thiscall(mover, float out[16]),
// RET 4: copies mover+2F8h (16 dwords, REP MOVSD) when both dwords +1BCh and
// +3ACh are nonzero, otherwise writes the identity as 0042D770 does.
void camera_mover_matrix_0042def0(std::uint32_t field_1bc, std::uint32_t field_3ac,
                                  const float matrix_at_2f8[16], float out[16]);

// Camera mover copy slots, all __thiscall(mover, out*), RET 4.
// 0042C040 (slot 34h): copies +1B0h..+1B8h (the mover velocity).
Vec3f camera_mover_velocity_0042c040(const Vec3f& velocity_at_1b0);
// 0042C560 (slot 114h, source +2B8h) and 0042CBE0 (cockpit slot 120h, source
// +3E8h): REP MOVSD of 16 dwords, so the bits are copied unchanged.
void copy_matrix_0042c560(const float source[16], float out[16]);
// 0042CC00 (cockpit slot 114h): 004134F0 (ECX = mover+2B8h, [ESP+4] =
// mover+74h) copies the pose into the chase matrix with FLD/FSTP per float,
// then REP MOVSD copies +2B8h out. The x87 copy quietens a signalling NaN;
// the host copies values, which on SSE2 builds keeps the bits.
void cockpit_chase_matrix_0042cc00(const float pose_at_74[16], float chase_at_2b8[16], float out[16]);
// 0042DA30 (slot 120h): node = +1BCh. With a node, B6DB70 (ECX = node)
// refreshes it when bit 1 of byte node+5Ch is clear, then node+F0h is copied;
// without one the identity (00D7A24C) is written.
void camera_node_world_matrix_0042da30(bool has_node, bool node_world_valid,
                                       const std::function<void()>& refresh_node,
                                       const float node_world_at_f0[16], float out[16]);

// 00439000, __fastcall(ECX key), RET: an 8-byte {char* data; bool owns} key;
// _free(data) only when the owns byte at +4 is set. The host passes the
// deallocator because the native one is the CRT heap.
struct OwnedCStringKey {
    char* data{nullptr};
    bool owns{false};
};
void destroy_cstring_key_00439000(OwnedCStringKey& key, const std::function<void(void*)>& free_block);

// 00437BD0, __thiscall(int* cache = registry+4h, int size), RET 4. Grows the
// capacity through 00436530 when cache[2] < size, null-fills the slots from
// the old size up, and on shrink pops from the end, releasing each non-null
// descriptor (InterlockedDecrement of desc+4h, vtable slot 0 at zero) and
// nulling the slot. The host vector's size plays cache[1]; `release` stands
// for the decrement-and-destroy.
void resize_descriptor_cache_00437bd0(std::vector<void*>& cache, std::int32_t size,
                                      const std::function<void(void*)>& release);

// ---------------------------------------------------------------------------
// Segment 29: the mission load helpers around the Lua pickler.
// ---------------------------------------------------------------------------

// _stricmp ordering: ASCII lower-casing, unsigned byte comparison. The native
// tree's comparator treats an empty NativeString (null data) as smaller than
// any nonempty one, which is what _stricmp gives for "".
struct CaseInsensitiveLess {
    bool operator()(const std::string& a, const std::string& b) const;
};

// The object built by 006AFE80 from BSP_Game_OnInit (vtable 00CF82BC):
// +4 a case-insensitive std::map<NativeString, int> (head node from
// 005826B0, size +0Ch). 006AF870 is the destructor and 006AFEC0 the scalar
// deleting destructor; both erase the tree and free the head.
class MissionEntityNameMap {
public:
    using Map = std::map<std::string, std::int32_t, CaseInsensitiveLess>;

    // 006AF420, __fastcall(ECX this), RET: clears the map, then for every
    // thisTable member whose "Dead" (00CF829C) is false and whose "Name"
    // (00CE8ED0) is a Lua string stores atol(member key) under the name, or
    // -1 when the name is already present.
    void rebuild_006af420(lua_State* L);

    // The inline clear at the end of 006AFB00 (and at the head of 006AF420).
    void clear() { names_.clear(); }

    const Map& names() const { return names_; }

private:
    Map names_;
};

// 006AFB00, __thiscall(ECX = name map), RET. Collects the integer keys
// (00B66290: float32 narrowing then _ftol) of _savedata._entities, sorts
// them ascending (00623170), builds a second vector holding one NULL entity
// pointer per key (006AFD23/006AFD4F store the constant 0), calls 006AF780
// with each of those pointers, then clears the name map. Returns the sorted
// keys for inspection. `restore_entity` receives the native argument, which
// is always nullptr; 006AF780 dereferences it (009238A0, then vtable +98h),
// so the native routine faults on any nonempty _entities table.
std::vector<std::int32_t> restore_entities_006afb00(lua_State* L, MissionEntityNameMap& names,
                                                    const std::function<void(void* entity)>& restore_entity);

// The archive-reader interface the load code drives: vtable +4h begins a
// section (int 0, name), +8h ends it.
struct ArchiveSectionReader {
    virtual ~ArchiveSectionReader() = default;
    virtual void begin_section(std::int32_t flags, const char* name) = 0;
    virtual void end_section() = 0;
};

// 006AD470, __thiscall(ECX = 00F89A08, reader), RET 4: section "cont0"
// (00CF827C) read through 00951820 into ECX, then section "cont1" (00CF8274)
// read through 00951820 into ECX+54h.
void read_entity_id_registries_006ad470(ArchiveSectionReader& reader,
                                        const std::function<void(int table_index)>& load_table);
// 006AD4D0: ECX=registry pair, stack archive/RET4. Same section protocol as
// reader, but 00951940 saves used entries of this and this+54h, in that order.
void write_entity_id_registries_006ad4d0(ArchiveSectionReader& writer,
    const std::function<void(int table_index)>& save_table);

// Native steps of the 006B5960 load tail that are not Lua-state work.
struct MissionLoadFinishHooks {
    // 006B59C9..006B59F5: section "entIDcont" (00CF830C), 006AD470, end.
    std::function<void()> read_entity_ids;
    // 006B5A01..006B5A2A: section "_game" (00CF8304), 004B4A20, end.
    std::function<void()> read_game;
    // 006B5A39..006B5A66: section "scoring", 00916A00 on the commit record, end.
    std::function<void()> read_scoring;
    // 006B5A74..006B5A82: commit record +8h -> [game+650h+5Ch].
    std::function<void()> copy_commit_field;
    // 006B5A7C..006B5A85: 00874640(0.0f), the mission clock reset.
    std::function<void(float)> reset_mission_clock;
    // 006B5A8A..006B5AB4: "actUnitID" (00CF82F8) read as variant type 4.
    std::function<void*()> read_spectated_unit;
    // 006B5ABE..006B5AC8: 00647300 on [[00E198C4]+40h] for a non-null unit.
    std::function<void(void*)> set_spectated_unit;
    // 006B5B94..006B5BB0: [[game+1A08h]+4] != 0 gates the collect chunk.
    std::function<bool()> collect_garbage_enabled;
    // 006B8AD0(text, 0, 0, 2) -> 006B89F0 with the text as its own chunk name.
    std::function<void(const char* source)> run_string;
};

// 006B5960, no arguments, RET: the load tail. Order: the three archive
// sections, the commit copy, the clock reset, actUnitID, then 006B3DD0
// (register global functions), 006B23E0 (restore), the 006B17B0 registry
// clear (005FE9E0), _savedata = nil, and collectgarbage("collect")
// (00CE4DE0, run through 006B8AD0) when enabled. The LuaReader construction
// over _savedata (004425C0) and its teardown (00441210) are folded into the
// hooks. `names` is the 006B17B0 registry; the Lua steps are the pickler
// routines in mission_savedata_lua.hpp.
void finish_mission_load_006b5960(lua_State* L, MissionLuaFunctionNames& names,
                                  const MissionLuaEntityHooks& entities,
                                  const MissionLoadFinishHooks& hooks);

} // namespace bsp::cc13_save
