#include "bsp/native_unit_killed_base.hpp"

#include "bsp/native_string_pool_storage.hpp"

namespace bsp {
namespace {

struct CapturedSection final {
    TrackedCriticalSection* const section;
    explicit CapturedSection(TrackedCriticalSection* value) : section(value) {
        if (section) {
            EnterCriticalSection(&section->native);
            auto& depth = reinterpret_cast<volatile std::uint32_t&>(section->depth);
            depth = depth + 1u;
        }
    }
    ~CapturedSection() {
        if (section) {
            auto& depth = reinterpret_cast<volatile std::uint32_t&>(section->depth);
            depth = depth - 1u;
            LeaveCriticalSection(&section->native);
        }
    }
};

struct LuaCleanup final {
    NativeLuaObjectStorage& object;
    bool live = false;
    ~LuaCleanup() { if (live) destroy_native_lua_object_00b67700(object); }
    void finish() {
        live = false; // original moves to preceding state BEFORE destruction
        destroy_native_lua_object_00b67700(object);
    }
};

struct KeyCleanup final {
    NativeString& key;
    NativeStringRawPoolContext& pool;
    bool live = false;
    ~KeyCleanup() { if (live) destroy_native_string_header_0041dd20(&key, pool); }
    void finish() {
        live = false; // preceding Lua-object cleanup remains armed if getter throws
        destroy_native_string_header_0041dd20(&key, pool);
    }
};

// Native x87 load/store round at8DA4/8DB0 and the argument stores at8DD9,
//8E25,8E71. Preserve float32 value semantics; native FPU status ABI is external.
float x87_float(const volatile float& input) noexcept {
    float result;
    __asm {
        mov eax, input
        fld dword ptr [eax]
        fstp result
    }
    return result;
}

// Each is a distinct proven one-byte RET in this executable. Retain native
// argument evaluation without inventing logging, callback or teardown work.
void inert_trace_004254b0(const char*, std::uint32_t, const char*, const char*) noexcept {}
void inert_base_00923050(const NativeUnitObserverAlias&) noexcept {}

} // namespace

void killed_native_unit_base_00928c80(
    NativeUnitKilledBaseView unit, NativeUnitKilledBaseContext& context) {
    if (unit.self_key_length_178 != 0) {
        const auto holder = context.access.lock_owner_004c1570();
        auto* const section = holder.section_04;
        CapturedSection lock(section);
        NativeLuaObjectStorage self; // raw fresh14h; required getter constructs
        LuaCleanup self_cleanup{self};
        context.access.construct_self_00927b40(unit.alias, self);
        self_cleanup.live = true;
        NativeLuaObjectStorage last;
        // Declare the still-unarmed LastPosition guard before the key guard:
        // native states5/6/7 release the key BEFORE LastPosition on unwind.
        LuaCleanup last_cleanup{last};

        ActualNativeStringPoolStorage strings(
            context.strings.actual_published_01090aa8,
            context.strings.actual_small_returns_disabled_01090aa4,
            context.strings.actual_manager_publication_01090aa0);
        NativeString key; // SAME8-byte scratch header reused by all five calls
        KeyCleanup key_cleanup{key, context.strings};
        key.assign_0041e870(strings, "Ptr");
        key_cleanup.live = true;
        context.access.set_lightuserdata_00b67530(self, key, nullptr);
        key_cleanup.finish();

        key.assign_0041e870(strings, "LastPosition");
        key_cleanup.live = true;
        context.access.set_new_table_00b67580(self, key);
        key_cleanup.finish();

        native_lua_get_by_name_00b67800(self, &last, "LastPosition");
        last_cleanup.live = true;
        if (static_cast<volatile std::uint8_t&>(unit.pose.world_valid_c8) == 0) {
            refresh_pose_00414db0(unit.pose);
        }
        // Native reads y then x then z before any coordinate-name allocation
        // or Lua callback. These three captured values intentionally survive
        // later mutations of the actual unit's world matrix.
        const float y = static_cast<volatile float&>(unit.pose.world_cc[13]);
        const float x = x87_float(unit.pose.world_cc[12]);
        const float z = static_cast<volatile float&>(unit.pose.world_cc[14]);

        key.assign_0041e870(strings, "x");
        key_cleanup.live = true;
        context.access.set_number_00b67400(last, key, x87_float(x));
        key_cleanup.finish();
        key.assign_0041e870(strings, "y");
        key_cleanup.live = true;
        context.access.set_number_00b67400(last, key, x87_float(y));
        key_cleanup.finish();
        key.assign_0041e870(strings, "z");
        key_cleanup.live = true;
        context.access.set_number_00b67400(last, key, x87_float(z));
        key_cleanup.finish();

        last_cleanup.finish();
        self_cleanup.finish();
    }

    const bool has_name = unit.name_length_154 != 0;
    const std::uint32_t type = unit.type_c4;
    const char* const type_name = context.type_names_00e0cd80[type];
    const char* name;
    if (has_name) {
        name = unit.name_data_158;
        if (!name) name = context.empty_name_00f89ac8;
    } else {
        name = "<null name>";
    }
    const std::uint32_t id = unit.network_id_174; // native MOVZX, not signed short
    inert_trace_004254b0("Entity %u killed: %s (%s)", id, name, type_name);
    if (unit.owner_30 != nullptr) {
        const std::uint32_t profile = unit.alias.prefixes.observed_00.native_vtable_00;
        context.access.call_virtual_134(unit.alias, profile);
    }
    inert_base_00923050(unit.alias);
}

} // namespace bsp
