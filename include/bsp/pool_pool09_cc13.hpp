#pragma once

#include <cstddef>
#include <cstdint>

// cc13 pool09 (segment 34): the create (slot +20h) of three bomb-family
// projectile classes, the Lua field readers of two of them and of the vehicle
// class that carries OpenTime / SlowingFactor, and the class-35h minimap
// predicate. Evidence, ABIs and uncertainty are in src/pool_pool09_cc13.cpp.
// Names are hypotheses, not recovered symbols.
//
// These are new C++ interfaces over host data, not drop-in binary
// replacements. Every image routine these bodies call is reached through
// Pool09Host, each virtual named by the address it stands for; nothing is
// stubbed. Build-tested only; nothing here is wired into a host. No routine
// here draws from the shared generator 00BD2F10.

namespace bsp::cc13_pool09 {

inline constexpr float kOne_00d7a24c = 1.0f;  // 00D7A24C, float 3F800000
inline constexpr float kTwo_00ce3958 = 2.0f;  // 00CE3958, float 40000000

// Literal keys, read from the image bytes.
inline constexpr const char kKeyOpenAfterTime_00cfc878[] = "OpenAfterTime";
inline constexpr const char kKeySubmarineClass_00cfcd08[] = "SubmarineClass";
inline constexpr const char kKeyOpenAtFallingSpeed_00cfd1f4[] = "OpenAtFallingSpeed";
inline constexpr const char kKeyDummyTargetVehicleClass_00cfd1dc[] = "DummyTargetVehicleClass";
inline constexpr const char kKeyExplosionEfx_00ce44ac[] = "ExplosionEfx";
inline constexpr const char kKeyOpenTime_00cfd1d0[] = "OpenTime";
inline constexpr const char kKeySlowingFactor_00cfd1c0[] = "SlowingFactor";

// The engine LuaObject is 14h bytes; the readers pass copies by value.
struct LuaObjectStorage {
    alignas(4) std::uint8_t bytes[0x14];
};

// The three projectile kinds that share the create template (006E2C00's body
// with another size and constructor).
enum class DummyProjectileKind : std::uint8_t {
    kKamikazePlane,  // 006FEF10: 4A4h bytes, 006FE950, SEH handler 00C83B48
    kSubmarine,      // 006FFDF0: 4BCh bytes, 006FF800, SEH handler 00C83C48
    kTarget,         // 00700BE0: 4A0h bytes, 00700A20, SEH handler 00C83D08
};

struct Pool09Host {
    virtual ~Pool09Host() = default;

    // ---- CRT (bound, no credit) ----
    virtual void* operator_new_00bf55be(std::size_t size) = 0;
    virtual void memset_00bf79f0(void* dst, int value, std::size_t count) = 0;
    virtual void memcpy_00bf7680(void* dst, const void* src, std::size_t count) = 0;
    // Imports [00CE221C] / [00CE2220]: InterlockedIncrement / InterlockedDecrement.
    virtual std::int32_t interlocked_increment_00ce221c(std::int32_t* value) = 0;
    virtual std::int32_t interlocked_decrement_00ce2220(std::int32_t* value) = 0;

    // ---- projectile constructors, __thiscall(memory, a1), RET 4 ----
    virtual void* dummy_kamikaze_plane_construct_006fe950(void* memory, std::uint32_t a1) = 0;
    virtual void* dummy_submarine_construct_006ff800(void* memory, std::uint32_t a1) = 0;
    virtual void* dummy_target_construct_00700a20(void* memory, std::uint32_t a1) = 0;

    // ---- scene / pose ----
    // record->vtable[98h](a, scene, matrix), __thiscall, three stack arguments.
    virtual void entity_attach_scene_vt98(void* record, std::int32_t a, void* scene, const float* matrix) = 0;
    // [[00E188A8]+19CCh].
    virtual void* scene_root_00e188a8_19cc() = 0;
    // 0085DC80, __fastcall(float* m /*ECX*/): orthonormalize the basis rows in place.
    virtual void matrix_orthonormalize_basis_rows_0085dc80(float* m) = 0;
    // 0042ED50, __thiscall(node): invalidate the subtree pose.
    virtual void scene_node_invalidate_subtree_pose_0042ed50(void* node) = 0;

    // ---- Lua objects ----
    // 00B66FA0, __thiscall(dst, const LuaObject* src), RET 4.
    virtual void lua_object_copy_construct_00b66fa0(void* dst, const void* src) = 0;
    // 00B67800, __thiscall(row, out, const char* name), RET 8; returns the object to read.
    virtual void* lua_object_get_by_name_00b67800(const void* row, void* out, const char* name) = 0;
    // 00B66290, __thiscall(object): lua_tonumber truncated toward zero.
    virtual std::int32_t lua_object_get_integer_00b66290(void* object) = 0;
    // 00B67700, __thiscall(object).
    virtual void lua_object_destruct_00b67700(void* object) = 0;
    // The by-value number readers: __fastcall(key /*ECX*/, float* out /*EDX*/,
    // LuaObject copy [, float default]), RET 18h / 14h. Each destroys the copy.
    virtual void lua_read_number_or_default_006fe780(const char* key, float* out, void* copy, float fallback) = 0;
    virtual void lua_read_number_or_default_00700290(const char* key, float* out, void* copy, float fallback) = 0;
    virtual void lua_read_number_00700330(const char* key, float* out, void* copy) = 0;
    virtual void lua_read_number_007003b0(const char* key, float* out, void* copy) = 0;

    // ---- class readers and registries ----
    // 006E1BE0, __thiscall(cls, const LuaObject* row), RET 4: the bomb class reader.
    virtual void bomb_class_read_lua_fields_006e1be0(void* cls, const void* row) = 0;
    // 00960230, __thiscall(descriptor, const LuaObject* row), RET 4: the vehicle class reader.
    virtual void vehicle_class_read_lua_fields_00960230(void* descriptor, const void* row) = 0;
    // 00964790, __fastcall(int type_id /*ECX*/, bool read_race /*DL*/): descriptor.
    virtual void* vehicle_class_get_or_create_00964790(std::int32_t type_id, bool read_race) = 0;
    // 0041DD40, __thiscall(NativeString* s, uint length, char preserve), RET 8.
    virtual void native_string_resize_0041dd40(void* string, std::uint32_t length, char preserve) = 0;
    // 00870CD0, ECX = out handle slot, EDX = effect id, stack (1), RET 4; returns the slot.
    virtual void** effect_handle_acquire_00870cd0(void** out, std::int32_t effect_id, std::int32_t a) = 0;
    // handle->vtable[0](), __thiscall, no arguments: called when the count reaches zero.
    virtual void refcounted_destroy_vt0(void* handle) = 0;
};

// 006FEF10 / 006FFDF0 / 00700BE0, __thiscall(cls, a1, a2, const float* position,
// const float* direction, a5, a6, a7), RET 1Ch. Returns record + 310h.
void* dummy_projectile_class_create(Pool09Host& host, DummyProjectileKind kind, void* cls, std::uint32_t a1,
                                    void* a2, const float* position, const float* direction);
void* dummy_kamikaze_plane_class_create_projectile_006fef10(Pool09Host& host, void* cls, std::uint32_t a1, void* a2,
                                                            const float* position, const float* direction,
                                                            std::int32_t a5, std::int32_t a6, std::int32_t a7);
void* dummy_submarine_class_create_projectile_006ffdf0(Pool09Host& host, void* cls, std::uint32_t a1, void* a2,
                                                       const float* position, const float* direction,
                                                       std::int32_t a5, std::int32_t a6, std::int32_t a7);
void* dummy_target_bullet_class_create_projectile_00700be0(Pool09Host& host, void* cls, std::uint32_t a1, void* a2,
                                                           const float* position, const float* direction,
                                                           std::int32_t a5, std::int32_t a6, std::int32_t a7);

// 00700050, __thiscall(cls, const LuaObject* row), RET 4.
void dummy_submarine_class_read_lua_fields_00700050(Pool09Host& host, void* cls, const void* row);
// 00701060, __thiscall(cls, const LuaObject* row), RET 4.
void dummy_target_bullet_class_read_lua_fields_00701060(Pool09Host& host, void* cls, const void* row);
// 00700E40, __thiscall(descriptor, const LuaObject* row), RET 4.
void dummy_target_vehicle_class_read_lua_fields_00700e40(Pool09Host& host, void* descriptor, const void* row);

// 007001E0, __thiscall(this) -> bool, XOR AL,AL / RET.
bool class35_shows_on_minimap_007001e0() noexcept;

}  // namespace bsp::cc13_pool09
