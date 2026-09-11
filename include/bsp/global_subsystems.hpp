#pragma once

#include "bsp/dialog_config.hpp"
#include "bsp/panel_owner.hpp"
#include "bsp/weather_config.hpp"

namespace bsp {

// Borrow the actual publication slots of one game. Opaque owners and the
// effect vector stay in their native layouts; panel/weather use the canonical
// C++ projections. This is not a replacement for the native game object ABI.
struct GlobalSubsystemState {
    void*& mission_lua_1a08;
    void*& traffic_21d0;
    VoicePanelState*& panel_21e4;
    void*& powerup_00f88c30;
    void*& warnings_21e0;
    WeatherConfig*& weather_21c4;
    void* effects_vector_718c;
};

// Unreconstructed callees remain required services. These are individual
// native calls, not default results or substitutes for entire startup phases.
struct GlobalSubsystemHost {
    virtual ~GlobalSubsystemHost() = default;
    // Opens a distinct stack FileBlock and returns its retained host identity.
    virtual void* construct_file_block_00be0a30(const NativeString&, int flag) = 0;
    virtual void destroy_file_block_00bdcb30(void*) = 0;
    virtual void load_global_scripts_00886900(void* mission_lua) = 0;
    virtual void load_races_00800160() = 0;
    virtual void load_robots_00901610() = 0;
    virtual void install_recon_values_00803a40() = 0;
    virtual void load_marker_classes_006dbeb0() = 0;
    virtual void run_string_006b8ad0(void* lua_instance, const char*,
        int capture_results, int capture_error, int result_mode) = 0;

    // Actual singleton layout: NativeString* begin+10/end+14, float+2D8.
    // Every invocation must resolve 00432650 again, including lazy creation.
    virtual void* current_config_00432650() = 0;
    // out aliases the actual local pointer. The returned native EAX is &out.
    virtual void acquire_effect_00871ba0(void*& out, const NativeString*, int flag) = 0;
    // Preserve native refcounted pointer-vector copy/checked-growth semantics.
    virtual void append_effect_004d9c00(void* vector, void* const* source) = 0;
    // Called only after a real InterlockedDecrement at object+4 reaches zero.
    // Dispatch its CURRENT virtual slot+0 with ECX=object and no stack args.
    virtual void effect_zero_references(void* object) = 0;

    // Supplied storage is the actual native-size allocation. Return the native
    // constructor's EAX; loaders receive it even if it is null.
    virtual void* construct_traffic_004a43c0(void*) = 0;
    virtual void load_traffic_0049d690(void*) = 0;
    virtual void* construct_powerup_008edc60(void*) = 0;
    virtual void initialize_powerup_008ecec0(void*) = 0;
    virtual void* construct_warnings_0098a020(void*) = 0;
    virtual void initialize_warnings_009870a0(void*) = 0;
};

struct GlobalSubsystemContext {
    NativeStringStorage& strings;
    LuaStateOwnerEnvironment lua_environment;
    LuaScriptRuntime& scripts;
    DialogConfigContext& dialog;
    const PanelOwnerAllocationWords& panel_allocation;
    const WeatherConfigAllocationWords& weather_allocation;
    const SingletonLifetimeCallbacks& validation;
};

// Complete normal-flow004DC6A0, ECX=game, RET at004DC93F. Allocation and
// refcount operations are concrete; the two recovered owners/loaders execute
// directly. Other callees above still require real game implementations.
// C++ exception cleanup is an adaptation, not recovered native SEH/ABI proof.
void construct_global_subsystems_004dc6a0(GlobalSubsystemState,
    GlobalSubsystemHost&, GlobalSubsystemContext&);

struct GlobalSubsystemInvocation {
    GlobalSubsystemState state;
    GlobalSubsystemHost& host;
    GlobalSubsystemContext& context;
};
} // namespace bsp
