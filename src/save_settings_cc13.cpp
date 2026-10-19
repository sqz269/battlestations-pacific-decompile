#include "bsp/save_settings_cc13.hpp"

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <stdexcept>
extern "C" {
#include <lua.h>
}

namespace bsp::cc13_save {
namespace {

constexpr const char* kEntityTypeName = "Entity"; // 00CE3C7C
constexpr const char* kEmptyName = "";            // 00E1767D
constexpr float kOne = 1.0f;                      // 00D7A24C, bytes 00 00 80 3F

constexpr const char* kThisTable = "thisTable"; // 00CE7494
constexpr const char* kDeadField = "Dead";      // 00CF829C
constexpr const char* kNameField = "Name";      // 00CE8ED0
constexpr const char* kSaveData = "_savedata";  // 00CF8250
constexpr const char* kEntities = "_entities";  // 00CF825C
constexpr const char* kRegistry0 = "cont0";     // 00CF827C
constexpr const char* kRegistry1 = "cont1";     // 00CF8274
constexpr const char* kCollect = "collectgarbage(\"collect\")"; // 00CE4DE0

void write_identity(float out[16]) {
    for (int i = 0; i < 16; ++i) out[i] = 0.0f;
    out[0] = out[5] = out[10] = out[15] = kOne;
}

unsigned char lower_ascii(unsigned char c) {
    return c >= 'A' && c <= 'Z' ? static_cast<unsigned char>(c - 'A' + 'a') : c;
}

int abs_index(lua_State* L, int index) {
    return index > 0 || index <= LUA_REGISTRYINDEX ? index : lua_gettop(L) + index + 1;
}

// 00B662B0 -> lua_tolstring on a pushed copy (LuaObject pushes its slot), so a
// number key is converted on the copy and lua_next is not disturbed.
std::string text_of(lua_State* L, int index) {
    lua_pushvalue(L, index);
    std::size_t length = 0;
    const char* data = lua_tolstring(L, -1, &length);
    std::string text = data ? std::string(data, length) : std::string();
    lua_pop(L, 1);
    return text;
}

// 00B66290: lua_tonumber, FSTP float (the float32 narrowing), FLD, _ftol2
// (00BF7420); the low dword of the truncated 64-bit result is returned.
std::int32_t integer_of(lua_State* L, int index) {
    const float narrowed = static_cast<float>(lua_tonumber(L, index));
    constexpr float kLimit = 9.2233720368547758e18f;
    const std::int64_t wide = narrowed > -kLimit && narrowed < kLimit ? static_cast<std::int64_t>(narrowed)
                                                                      : std::numeric_limits<std::int64_t>::min();
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(static_cast<std::uint64_t>(wide)));
}

} // namespace

// --- segment 2: entity-interface slots ------------------------------------

const char* entity_type_name_0042b860() {
    return kEntityTypeName;
}

const char* entity_class_name_0042b9e0(const char* const* class_name_table, std::int32_t kind) {
    // 0042B9E0: MOV EAX,[ECX+C4h]; MOV EAX,[EAX*4+00E0CD80]; RET.
    return class_name_table[kind];
}

Vec3f entity_world_position_0042ba00(bool world_pose_valid, const std::function<void()>& refresh_world,
                                     const Vec3f& world_position) {
    if (!world_pose_valid) refresh_world();
    return world_position;
}

Vec3f entity_predict_position_0042d680(const std::function<Vec3f()>& velocity, bool world_pose_valid,
                                       const std::function<void()>& refresh_world,
                                       const Vec3f& world_position, float t) {
    // 0042D688..0042D692: vtable +34h before the pose test.
    const Vec3f v = velocity();
    // 0042D6A3..0042D6B7: each product is stored as float before the add.
    const float dx = t * v.x;
    const float dy = v.y * t;
    const float dz = t * v.z;
    // 0042D6BB..0042D6BF: refresh when +C8h is clear, as in 0042BA00.
    if (!world_pose_valid) refresh_world();
    return {world_position.x + dx, world_position.y + dy, world_position.z + dz};
}

void entity_identity_matrix_0042d770(float out[16]) {
    write_identity(out);
}

const char* entity_name_0042e960(const char* name_at_158, std::uint32_t selector_at_15c,
                                 const char* name_at_160) {
    const char* name = selector_at_15c == 0 ? name_at_158 : name_at_160;
    return name ? name : kEmptyName;
}

void camera_mover_matrix_0042def0(std::uint32_t field_1bc, std::uint32_t field_3ac,
                                  const float matrix_at_2f8[16], float out[16]) {
    if (field_1bc != 0 && field_3ac != 0) {
        std::copy(matrix_at_2f8, matrix_at_2f8 + 16, out);
        return;
    }
    write_identity(out);
}

Vec3f camera_mover_velocity_0042c040(const Vec3f& velocity_at_1b0) {
    return velocity_at_1b0;
}

void copy_matrix_0042c560(const float source[16], float out[16]) {
    std::copy(source, source + 16, out);
}

void cockpit_chase_matrix_0042cc00(const float pose_at_74[16], float chase_at_2b8[16], float out[16]) {
    std::copy(pose_at_74, pose_at_74 + 16, chase_at_2b8); // 0042CC0E CALL 004134F0
    std::copy(chase_at_2b8, chase_at_2b8 + 16, out);      // 0042CC1E REP MOVSD
}

void camera_node_world_matrix_0042da30(bool has_node, bool node_world_valid,
                                       const std::function<void()>& refresh_node,
                                       const float node_world_at_f0[16], float out[16]) {
    if (has_node) {
        if (!node_world_valid) refresh_node(); // 0042DA3F..0042DA47
        std::copy(node_world_at_f0, node_world_at_f0 + 16, out);
        return;
    }
    write_identity(out);
}

void destroy_cstring_key_00439000(OwnedCStringKey& key, const std::function<void(void*)>& free_block) {
    // 00439000: CMP byte [ECX+4],0; JZ ret; PUSH [ECX]; CALL _free. The key
    // fields are left as they were.
    if (key.owns) free_block(key.data);
}

void resize_descriptor_cache_00437bd0(std::vector<void*>& cache, std::int32_t size,
                                      const std::function<void(void*)>& release) {
    // Native compares signed ints; a negative size would walk below slot 0.
    if (size < 0) throw std::invalid_argument("descriptor cache size is negative");
    const std::size_t target = static_cast<std::size_t>(size);
    // Growth: reserve through 00436530, then null-fill [old size, size).
    if (cache.size() < target) cache.resize(target, nullptr);
    // Shrink: decrement the size first, then release and null that slot.
    while (cache.size() > target) {
        void* descriptor = cache.back();
        if (descriptor) {
            release(descriptor);
            cache.back() = nullptr;
        }
        cache.pop_back();
    }
}

// --- segment 29: mission load helpers --------------------------------------

bool CaseInsensitiveLess::operator()(const std::string& a, const std::string& b) const {
    const std::size_t n = std::min(a.size(), b.size());
    for (std::size_t i = 0; i < n; ++i) {
        const unsigned char ca = lower_ascii(static_cast<unsigned char>(a[i]));
        const unsigned char cb = lower_ascii(static_cast<unsigned char>(b[i]));
        if (ca != cb) return ca < cb;
    }
    return a.size() < b.size();
}

void MissionEntityNameMap::rebuild_006af420(lua_State* L) {
    // 006AF450..006AF46A: erase the tree (0058B520) and reset the head.
    names_.clear();
    // 006AF470..006AF4A1: globals.thisTable through 00B67800 (lua_gettable
    // semantics, so a missing table is nil and is not iterated).
    lua_getfield(L, LUA_GLOBALSINDEX, kThisTable);
    const int table = lua_gettop(L);
    if (lua_istable(L, table)) {
        lua_pushnil(L);
        while (lua_next(L, table) != 0) {
            const int key = lua_gettop(L) - 1;
            const int value = key + 1;
            // 006AF4F9..006AF51B: GetBoolean = lua_toboolean, so nil is false.
            lua_getfield(L, value, kDeadField);
            const bool dead = lua_toboolean(L, -1) != 0;
            lua_pop(L, 1);
            bool use = false;
            std::string name;
            if (!dead) {
                // 006AF531..006AF54C: IsString (00B660A0) is lua_type == LUA_TSTRING.
                lua_getfield(L, value, kNameField);
                if (lua_type(L, -1) == LUA_TSTRING) {
                    use = true;
                    name = text_of(L, -1);
                }
                lua_pop(L, 1);
            }
            if (use) {
                auto found = names_.find(name);
                if (found == names_.end()) {
                    // 006AF697..006AF6AD: map[name] = atol(GetString(key)).
                    names_[name] = static_cast<std::int32_t>(std::atol(text_of(L, key).c_str()));
                } else {
                    // 006AF6B9..006AF6BE: a repeated name is ambiguous.
                    found->second = -1;
                }
            }
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 1);
}

std::vector<std::int32_t> restore_entities_006afb00(lua_State* L, MissionEntityNameMap& names,
                                                    const std::function<void(void* entity)>& restore_entity) {
    std::vector<std::int32_t> keys;
    // 006AFB44..006AFB73: globals._savedata._entities, both through 00B67800.
    lua_getfield(L, LUA_GLOBALSINDEX, kSaveData);
    lua_getfield(L, -1, kEntities);
    const int table = lua_gettop(L);
    if (lua_istable(L, table)) {
        // 006AFBCE..006AFC9F: push_back(GetInteger(key)) for every slot.
        lua_pushnil(L);
        while (lua_next(L, table) != 0) {
            keys.push_back(integer_of(L, abs_index(L, -2)));
            lua_pop(L, 1);
        }
    }
    lua_pop(L, 2);
    // 006AFCAF: 00623170, ascending.
    std::sort(keys.begin(), keys.end());
    // 006AFCC6..006AFD6A: one zero pointer per key; no lookup is made.
    const std::vector<void*> entities(keys.size(), nullptr);
    // 006AFD81..006AFD96: 006AF780(entity) for each pointer in order.
    for (void* entity : entities) restore_entity(entity);
    // 006AFD9B..006AFDC1: clear the name map at this+4 (0058B520 + head reset).
    names.clear();
    return keys;
}

void read_entity_id_registries_006ad470(ArchiveSectionReader& reader,
                                        const std::function<void(int table_index)>& load_table) {
    reader.begin_section(0, kRegistry0);
    load_table(0); // 00951820, ECX = 00F89A08
    reader.end_section();
    reader.begin_section(0, kRegistry1);
    load_table(1); // 00951820, ECX = 00F89A08 + 54h
    reader.end_section();
}

void write_entity_id_registries_006ad4d0(ArchiveSectionReader& writer,
    const std::function<void(int table_index)>& save_table) {
    writer.begin_section(0,kRegistry0);
    save_table(0);
    writer.end_section();
    writer.begin_section(0,kRegistry1);
    save_table(1);
    writer.end_section();
}

void finish_mission_load_006b5960(lua_State* L, MissionLuaFunctionNames& names,
                                  const MissionLuaEntityHooks& entities,
                                  const MissionLoadFinishHooks& hooks) {
    hooks.read_entity_ids();
    hooks.read_game();
    hooks.read_scoring();
    hooks.copy_commit_field();
    hooks.reset_mission_clock(0.0f); // 006B5A7C FLDZ
    if (void* unit = hooks.read_spectated_unit()) hooks.set_spectated_unit(unit);
    register_global_functions_006b3dd0(L, names); // 006B5ACD
    decode_mission_lua_state_006b23e0(L, entities); // 006B5AD2
    names.clear();                                  // 006B5AD7..006B5AEA
    // 006B5B00..006B5B6B: globals["_savedata"] = nil through 00B67350.
    lua_pushnil(L);
    lua_setfield(L, LUA_GLOBALSINDEX, kSaveData);
    if (hooks.collect_garbage_enabled()) hooks.run_string(kCollect);
}

} // namespace bsp::cc13_save
